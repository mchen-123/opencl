#include <cassert>
#include <mutex>

#include "devices.hpp"
#include "dfcl_debug.hpp"

std::mutex dfcl_init_lock;

static bool first_init_done = false;
static bool init_in_progress = false;
uint64_t dfcl_num_devices = 0;

/* Head for the dfcl_devices linked list */
inline std::atomic<_cl_device_id *> dfcl_devices_list = {nullptr};

typedef void (*init_device_ops)(struct dfcl_device_ops *);
static init_device_ops dfcl_devices_init_ops;

static struct dfcl_device_ops dfcl_device_ops_t;
constexpr std::string_view DFCL_DEVICE_LIB = "libdfcl-device.so";
static void *dfcl_device_handle = nullptr;

static uint64_t device_count;

/* Indexes each device added to the platform by setting the device id. First
 * used and modified during init, to index devices present since launch. May
 * also used and modified when devices are dynamically added. */
static uint64_t dev_index;

extern pthread_mutex_t dfcl_context_handling_lock; 
extern std::once_flag  dfcl_context_lock_init_flag;

inline void init_context_handling_lock() {
    std::call_once(dfcl_context_lock_init_flag, [] {
        pthread_mutexattr_t attrs;
        pthread_mutexattr_init(&attrs);
        pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_ADAPTIVE_NP);
        pthread_mutex_init(&dfcl_context_handling_lock, &attrs);
        pthread_mutexattr_destroy(&attrs);
    });
}

inline void ll_append_atomic(std::atomic<_cl_device_id *>& head, _cl_device_id *new_node) {
    assert(new_node != nullptr);

    new_node->next.store(nullptr, std::memory_order_relaxed); // new node tile pointer is nullptr

    _cl_device_id *last = head.load(std::memory_order_relaxed);
    _cl_device_id *next;
    if (last == nullptr) {
        // if head is nullptr, try to set it to new_node
        if (head.compare_exchange_weak(last, new_node,
                                    std::memory_order_release,
                                    std::memory_order_relaxed)) {
            return;
        }
    }

    do {
        while ((next = last->next.load(std::memory_order_acquire)) != nullptr) {
            last = next;
        }
    } while (!last->next.compare_exchange_weak(next, new_node,
                                            std::memory_order_release,
                                            std::memory_order_relaxed));
}


inline std::string get_dfcl_device_lib_path(bool absolute_path = true) {
    if (!absolute_path) return std::string(DFCL_DEVICE_LIB);

    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(get_dfcl_device_lib_path), &info) && info.dli_fname) {
        return (std::filesystem::path(info.dli_fname).parent_path() / DFCL_DEVICE_LIB).string();
    }

    return std::string(DFCL_DEVICE_LIB);
}

cl_int
dfcl_init_devices(cl_platform_id platform) {
    int errcode = CL_SUCCESS;

    if (init_in_progress) {
        return errcode;
    }

    std::lock_guard<std::mutex> lock(dfcl_init_lock);
    init_in_progress = true;

    if (first_init_done) {
        // 第二次及以后：直接返回当前状态; todo: 需要重新探测设备
        init_in_progress = false;
        return dfcl_num_devices > 0  ? CL_SUCCESS : CL_DEVICE_NOT_FOUND;
    } else {
        init_context_handling_lock();
    }

    std::string deviceLibrary = get_dfcl_device_lib_path(true);
    dfcl_device_handle = dfcl_dynlib_open(deviceLibrary, 1, 0);
    if (dfcl_device_handle == nullptr) {
        DFCL_MSG_ERR("Loading" << deviceLibrary << "failed\n");
        return CL_DEVICE_NOT_FOUND;
    }
    DFCL_MSG_INFO("Fallback Loaded " << deviceLibrary << " succeeded\n");
    
    std::string init_device_ops_name = "dfcl_dfruntime_init_device_ops";
    dfcl_devices_init_ops = (init_device_ops)dfcl_dynlib_symbol_address(
        dfcl_device_handle, init_device_ops_name);
    if (!dfcl_devices_init_ops) {
        DFCL_MSG_ERR("Loading symbol " << init_device_ops_name << " from " << deviceLibrary << " failed\n");
        return CL_DEVICE_NOT_FOUND;
    }

    dfcl_devices_init_ops(&dfcl_device_ops_t);
    assert(dfcl_device_ops_t.device_name != nullptr);

    /* Probe and add the result to the number of probed devices */
    assert(dfcl_device_ops_t.probe != nullptr);
    device_count = dfcl_device_ops_t.probe(&dfcl_device_ops_t);
    dfcl_num_devices = device_count;

    DFCL_GOTO_ERROR_ON(device_count == 0, CL_DEVICE_NOT_FOUND, "No device found\n");

    dev_index = 0;

    for(int i = 0; i < device_count; ++i) {
        cl_device_id dev;
        dev = (cl_device_id)calloc(1, sizeof(*dev));

        dev->ops = &dfcl_device_ops_t;
        dev->dev_id = dev_index;
        CL_INIT_OBJECT(dev, platform);
        dev->driver_version = "0.3.0";
        if (dev->version == nullptr) {
            dev->version = "OpenCL 3.0 DAMO 1.0";
        }
        errcode = dev->ops->init(i, dev);
        DFCL_GOTO_ERROR_ON(errcode != CL_SUCCESS, errcode, "Device init failed\n");

        ll_append_atomic(dfcl_devices_list, dev);
        ++dev_index;
    }
    first_init_done = true;
ERROR:
    init_in_progress = false;
    return errcode;
}

uint32_t
dfcl_get_device_type_count(cl_device_type device_type) {
    uint32_t count = 0;
    cl_device_id device;

    if (device_type == CL_DEVICE_TYPE_DEFAULT) {
        dfcl_devices_list.load(std::memory_order_relaxed) ? 1 : 0;
    }

    for (_cl_device_id *dev = dfcl_devices_list.load(std::memory_order_seq_cst);
        dev != nullptr; 
        dev = dev->next.load(std::memory_order_seq_cst)) 
    {
        if (dev->available == CL_FALSE) {
            continue;
        }    

        if (dev->type & device_type) {
            ++count;
        }
    }
    return count;
}

uint32_t
dfcl_get_devices(cl_device_type device_type, cl_device_id *devices, uint32_t num_entries) {
    uint32_t dev_added = 0;
    
    for (_cl_device_id *dev = dfcl_devices_list.load(std::memory_order_acquire);
        dev != nullptr; 
        dev = dev->next.load(std::memory_order_acquire)) 
    {
        if (dev->available == CL_FALSE) {
            continue;
        }

        if (device_type == CL_DEVICE_TYPE_DEFAULT) {
            devices[dev_added] = dev;
            ++dev_added;
            break;
        }

        if (dev->type & device_type) {
            if (dev_added < num_entries) {
                devices[dev_added] = dev;
                ++dev_added;
                
            } else {
                break;
            }
        }
    }

    return dev_added;
}