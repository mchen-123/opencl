#ifndef _CL_HELPER_H_
#define _CL_HELPER_H_

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <CL/cl.h>
#include <CL/cl_icd.h>

/* Need to rename all CL API functions to prevent ICD loader functions calling
 * themselves via the dispatch table. */
#include "cl_rename_api.h"
#include "cl_icd_structs.h"
#include "runtime_mock.h"

/* dfcl specific flag, for "hidden" default queues allocated in each context */
#define CL_QUEUE_HIDDEN (1 << 10)

/* ============ Memory helpers ============ */
#define DFCL_NEW(type_name) ((type_name *)malloc(sizeof(type_name)))
#define DFCL_DELETE(ptr) free(ptr)
#define DFCL_NEW_ARRAY(type_name, count) ((type_name *)malloc(sizeof(type_name) * (count)))
#define DFCL_DELETE_ARRAY(ptr) free(ptr)

#define DFCL_MEM_FREE(PTR)                          \
    do                                              \
    {                                               \
        free((void *)(PTR));                        \
        (PTR) = NULL;                               \
    }                                               \
    while (0)

/* ============ Atomic helpers (C11 atomics) ============ */
#include <stdatomic.h>

static inline int DFCL_ATOMIC_INC(atomic_int *x) {
    return atomic_fetch_add(x, 1) + 1;
}

static inline int DFCL_ATOMIC_DEC(atomic_int *x) {
    return atomic_fetch_sub(x, 1) - 1;
}

/* ============ Error handling ============ */
#define DFCL_SET_FUNCTION_VALUE_RETURN(VALUE, ERROR, RET)                 \
    do                                                                  \
    {                                                                   \
        errcode = VALUE;                                                \
        if (ERROR)                                                      \
            *(ERROR) = errcode;                                         \
        return RET;                                                     \
    }                                                                   \
    while(0)

#define DFCL_SET_RETURN_VAL(param_value_size, param_value_ptr, param_value_size_ret, param_value_type, val, result) \
    do                                                                                                              \
    {                                                                                                               \
        if (param_value_size < sizeof(param_value_type) && (param_value_size > 0))                                  \
            return CL_INVALID_VALUE;                                                                                \
        if (param_value_ptr)                                                                                        \
            *(param_value_type *)(param_value_ptr) = val;                                                           \
        if (param_value_size_ret)                                                                                   \
            *param_value_size_ret = sizeof(param_value_type);                                                       \
    }                                                                                                               \
    while(0)

#define DFCL_SET_RETURN_PTR(param_value_size, param_value_ptr, param_value_size_ret, param_arr, param_arr_size, result) \
    do                                                                                                                  \
    {                                                                                                                   \
        if (param_value_size < sizeof(param_arr_size) && (param_value_size > 0))                                        \
            result = CL_INVALID_VALUE;                                                                                  \
        if (param_value_ptr)                                                                                            \
            memcpy(param_value_ptr, param_arr, param_arr_size);                                                         \
        if (param_value_size_ret)                                                                                       \
            *param_value_size_ret = param_arr_size;                                                                     \
    }                                                                                                                   \
    while(0)

#define DFCL_GOTO_ERROR_ON(cond, err_code, ...)                             \
    do                                                                      \
    {                                                                       \
        if (cond)                                                           \
        {                                                                   \
            DFCL_MSG_ERR(__VA_ARGS__);                                      \
            errcode = err_code;                                             \
            goto ERROR;                                                     \
        }                                                                   \
    }                                                                       \
    while (0)

#define DFCL_GOTO_ERROR_COND(cond, err_code)                           \
    do                                                                 \
    {                                                                  \
        if (cond)                                                      \
        {                                                              \
            errcode = err_code;                                        \
            goto ERROR;                                                \
        }                                                              \
    }                                                                  \
    while (0)

/* ============ Logging ============ */
#ifndef DFCL_MSG_ERR
#define DFCL_MSG_ERR(...) do { \
    fprintf(stderr, "DFCL ERROR: "); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
} while(0)
#endif

#ifndef DFCL_MSG_INFO
#define DFCL_MSG_INFO(...) do { \
    printf("DFCL INFO: "); \
    printf(__VA_ARGS__); \
    printf("\n"); \
} while(0)
#endif

/* ============ Validation ============ */
static inline int is_valid(void *handle) {
    return handle != NULL;
}

/* ============ ThiveclObject - Base structure for all OpenCL objects with ref counting ============ */
typedef struct ThiveclObject {
    CL_OBJECT_BODY;
    atomic_int refcount;
    uint64_t id;
} ThiveclObject;

static inline void dfcl_object_init(ThiveclObject *obj, CLIicdDispatchTable *dispatch) {
    obj->dispatch_ = dispatch;
    atomic_init(&obj->refcount, 1);
}

/* Thread-safe reference counting */
static inline int dfcl_object_retain(ThiveclObject *obj) {
    if (!obj) return CL_INVALID_VALUE;
    return atomic_fetch_add(&obj->refcount, 1) + 1;
}

static inline int dfcl_object_release(ThiveclObject *obj) {
    if (!obj) return CL_INVALID_VALUE;
    return atomic_fetch_sub(&obj->refcount, 1) - 1;
}

/* Get current refcount (for debugging) */
static inline int dfcl_object_get_refcount(ThiveclObject *obj) {
    if (!obj) return 0;
    return atomic_load(&obj->refcount);
}

/* ============ OpenCL Object Structures ============ */
typedef struct _cl_device_id _cl_device_id;
typedef struct _cl_context _cl_context;
typedef struct _cl_command_queue _cl_command_queue;
typedef struct _cl_mem _cl_mem;
typedef struct _cl_event _cl_event;

struct _cl_platform_id
{
    CL_OBJECT_BODY;
    const char *profile;
    const char *version;
    const char *name;
    const char *vendor;
    const char *extensions;
    const char *suffix;
};

struct _cl_device_id
{
    CL_OBJECT_BODY;
    atomic_int refcount;
    uint64_t id;

    /* queries */
    cl_device_type type;
    cl_uint vendor_id;
    cl_uint max_compute_units;

    /* for subdevice support */
    cl_device_id parent_device;

    cl_uint max_work_item_dimensions;
    cl_uint address_bits;
    cl_ulong die_max_mem_alloc_size;
    cl_device_fp_config single_fp_config;

    /* Device specific operations, shared among devices of the same type */
    struct dfcl_device_ops *ops;

    /* Device ID within the device type */
    int dev_id;

    const char *short_name;
    const char *long_name;

    const char *vendor;
    const char *driver_version;
    const char *profile;

    /* these are Device versions, not OpenCL C versions */
    const char *version;

    cl_bool image_support;
    cl_bool available;

    /* Device specific data needed for internal device functions */
    void *data;

    /* OpenCL 2.0 properties */
    cl_command_queue_properties on_dev_queue_props;
    cl_command_queue_properties on_host_queue_props;

    /* linked list next pointer */
    struct _cl_device_id *next;
};

struct _cl_context
{
    CL_OBJECT_BODY;
    atomic_int refcount;
    uint64_t id;

    /* queries */
    cl_device_id *devices;
    cl_context_properties *properties;

    /* implementation */
    uint32_t num_devices;
    uint32_t num_properties;

    /* The largest of die_max_mem_alloc_size of single die in context */
    size_t die_max_mem_alloc_size;

    /* the original device list given to clCreateContext,
     * required for */
    cl_device_id *create_devices;
    uint32_t num_create_devices;

    /* for enqueueing migration commands. Two reasons:
     * 1) since migration commands can execute in parallel
     * to other commands, we can increase parallelism
     * 2) in some cases (migration between 2 devices through
     * host memory), we need to put two commands in two queues,
     * and the clEnqueueX only gives us one (on the destination
     * device). */
    cl_command_queue *default_queues;

    /* list of command queues created for the context.
     * required for clMemBlockingFreeINTEL */
    cl_command_queue *command_queues;
    uint32_t num_command_queues;
    uint32_t command_queues_capacity;

    /* True if none of devices support cl_ext_buffer_device_address */
    cl_bool no_devices_support_bda;
};

struct _cl_command_queue
{
    CL_OBJECT_BODY;
    atomic_int refcount;
    uint64_t id;

    /* queries */
    cl_context context;
    cl_device_id device;
    cl_command_queue_properties properties;

    /* Number of unfinished command enqueued. */
    unsigned long command_count;
    /* device specific data */
    void *data;
};

struct _cl_mem
{
    CL_OBJECT_BODY;
    atomic_int refcount;
    uint64_t id;

    cl_context context;
    cl_mem_object_type type;
    cl_mem_flags flags;

    size_t size;

    /* Image flags */
    cl_bool is_image;

    /* pipe flags */
    cl_bool is_pipe;
};

struct _cl_event
{
    CL_OBJECT_BODY;
    atomic_int refcount;
    uint64_t id;

    cl_context context;
};

/* ============ Device Operations ============ */
struct dfcl_device_ops {
    const char *device_name;
    /**
     * Called when clFlush is called.
     *
     * This function ensures that
     * commands will be eventually executed. It is up to the device what happens
     * here, if anything. See basic and pthread for reference.*/
    void (*flush) (cl_device_id device, cl_command_queue cq);

    /**
     * Detects & returns the number of available devices the driver finds on the
     * system.
     */
    int (*probe)(struct dfcl_device_ops *ops);

    /**
     * Initializes a device.
     * @param i The device index
     * @param device The cl_device_id to initialize
     */
    cl_int (*init)(unsigned int i, struct _cl_device_id *device);


    /** Optional: If the driver needs to use hardware resources
     * for command queues, it should use these callbacks */
    int (*init_queue) (cl_device_id device, cl_command_queue command_queue);
    int (*free_queue) (cl_device_id device, cl_command_queue queue);
};

/* ============ Object Management Functions ============ */
static inline int dfcl_retain_object(void *obj) {
    if (!obj) return CL_INVALID_VALUE;
    return dfcl_object_retain((ThiveclObject *)obj);
}

static inline int dfcl_release_object(void *obj) {
    if (!obj) return CL_INVALID_VALUE;
    return dfcl_object_release((ThiveclObject *)obj);
}

/* ============ Command queue list helpers ============ */
static inline int context_add_command_queue(struct _cl_context *ctx, cl_command_queue queue) {
    if (ctx->num_command_queues >= ctx->command_queues_capacity) {
        uint32_t new_capacity = ctx->command_queues_capacity == 0 ? 8 : ctx->command_queues_capacity * 2;
        cl_command_queue *new_queues = (cl_command_queue *)realloc(ctx->command_queues, sizeof(cl_command_queue) * new_capacity);
        if (!new_queues) return CL_OUT_OF_HOST_MEMORY;
        ctx->command_queues = new_queues;
        ctx->command_queues_capacity = new_capacity;
    }
    ctx->command_queues[ctx->num_command_queues++] = queue;
    return CL_SUCCESS;
}

static inline int context_remove_command_queue(struct _cl_context *ctx, cl_command_queue queue) {
    uint32_t i;
    for (i = 0; i < ctx->num_command_queues; i++) {
        if (ctx->command_queues[i] == queue) {
            /* Shift remaining elements */
            for (; i < ctx->num_command_queues - 1; i++) {
                ctx->command_queues[i] = ctx->command_queues[i + 1];
            }
            ctx->num_command_queues--;
            return CL_SUCCESS;
        }
    }
    return CL_INVALID_VALUE; /* Not found */
}

#endif /* _CL_HELPER_H_ */