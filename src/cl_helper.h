#ifndef _CL_HELPER_H_
#define _CL_HELPER_H_

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdatomic.h>
#include <pthread.h>
#include <CL/cl.h>
#include <CL/cl_icd.h>

/* Need to rename all CL API functions to prevent ICD loader functions calling
 * themselves via the dispatch table. */
#include "cl_rename_api.h"
#include "cl_icd_structs.h"
#include "runtime_mock.h"

extern CLIicdDispatchTable *g_icd_dispatchTable;

/* ============ Forward declarations for cross-file internal calls ============ */
cl_int ___clGetPlatformIDs(cl_uint, cl_platform_id *, cl_uint *);
cl_int ___clGetPlatformInfo(cl_platform_id, cl_platform_info, size_t, void *, size_t *);
cl_int ___clGetDeviceIDs(cl_platform_id, cl_device_type, cl_uint, cl_device_id *, cl_uint *);
cl_int ___clGetDeviceInfo(cl_device_id, cl_device_info, size_t, void *, size_t *);
cl_int ___clCreateSubDevices(cl_device_id, const cl_device_partition_property *, cl_uint,
                             cl_device_id *, cl_uint *);
cl_int ___clRetainDevice(cl_device_id);
cl_int ___clReleaseDevice(cl_device_id);
cl_context ___clCreateContext(const cl_context_properties *, cl_uint, const cl_device_id *,
                              void (*)(const char *, const void *, size_t, void *), void *,
                              cl_int *);
cl_int ___clRetainContext(cl_context);
cl_int ___clReleaseContext(cl_context);
cl_command_queue ___clCreateCommandQueue(cl_context, cl_device_id, cl_command_queue_properties,
                                         cl_int *);
cl_int ___clReleaseCommandQueue(cl_command_queue);


/* for "hidden" default queues allocated in each context */
#define CL_QUEUE_HIDDEN (1 << 10)

/* ============ ThiveclObject - Base structure for all OpenCL objects with ref counting ============
 */
typedef struct ThiveclObject {
  CL_OBJECT_BODY;
  atomic_int refcount;
  pthread_mutex_t lock;
  uint64_t id;
  uint64_t magic;
} ThiveclObject;

#define CL_OBJECT_MAGIC 0x5448495645434C42ULL

#define IS_VALID_OBJECT(obj) ((obj) != NULL && ((ThiveclObject *)(obj))->magic == CL_OBJECT_MAGIC)

/* ====================== 锁操作宏 ====================== */
#define DFCL_LOCK(__LOCK__) pthread_mutex_lock(&(__LOCK__))
#define DFCL_UNLOCK(__LOCK__) pthread_mutex_unlock(&(__LOCK__))

#define DFCL_LOCK_OBJ(obj)                                                         \
  do {                                                                             \
    if (IS_VALID_OBJECT(obj)) pthread_mutex_lock(&((ThiveclObject *)(obj))->lock); \
  } while (0)

#define DFCL_UNLOCK_OBJ(obj)                                                         \
  do {                                                                               \
    if (IS_VALID_OBJECT(obj)) pthread_mutex_unlock(&((ThiveclObject *)(obj))->lock); \
  } while (0)

/* ============ Object Management Functions ============ */
static inline cl_int dfcl_retain_object(void *obj) {
  if (!IS_VALID_OBJECT(obj)) return CL_INVALID_VALUE;
  atomic_fetch_add_explicit(&((ThiveclObject *)obj)->refcount, 1, memory_order_relaxed);
  return CL_SUCCESS;
}

static inline int dfcl_release_object(void *obj) {
  if (!IS_VALID_OBJECT(obj)) return -1;

  ThiveclObject *o = (ThiveclObject *)obj;
  return atomic_fetch_sub_explicit(&o->refcount, 1, memory_order_acq_rel) - 1;
}

static inline int DFCL_ATOMIC_INC(atomic_int *x) {
  return atomic_fetch_add(x, 1) + 1;
}

static inline int DFCL_ATOMIC_DEC(atomic_int *x) {
  return atomic_fetch_sub(x, 1) - 1;
}

/* ====================== 对象初始化 ====================== */
static inline void dfcl_init_object(void *obj, uint64_t id) {
  ThiveclObject *o = (ThiveclObject *)obj;
  o->dispatch_ = g_icd_dispatchTable;
  o->id = id;
  o->magic = CL_OBJECT_MAGIC;
  atomic_init(&o->refcount, 1);
  pthread_mutex_init(&o->lock, NULL);
}

struct _cl_platform_id {
  ThiveclObject base;
  const char *profile;
  const char *version;
  const char *name;
  const char *vendor;
  const char *extensions;
  const char *suffix;
};

struct _cl_device_id {
  ThiveclObject base;

  /* Device ID within the device type */
  int dev_id;

  /* Basic OpenCL Properties */
  cl_device_type type;
  cl_uint vendor_id;
  cl_uint max_compute_units;
  cl_uint max_work_item_dimensions;
  cl_uint address_bits;

  cl_device_fp_config single_fp_config;
  cl_bool image_support;
  cl_bool available;

  /* Device Identity */
  const char *short_name;
  const char *long_name;

  const char *vendor;
  const char *driver_version;
  const char *profile;
  /* these are Device versions, not OpenCL C versions */
  const char *version;

  cl_platform_id platform;
  /* ==================== Multi-Die Support ==================== */
  cl_bool is_multi_die;
  uint32_t num_dies;
  DFDieGrid die_grid;

  /* NULL indicates the Root Device. */
  cl_device_id parent_device;

  /* Device specific data needed for internal device functions */
  void *data;

  /* linked list next pointer */
  struct _cl_device_id *next;
};

struct _cl_context {
  ThiveclObject base;

  /* queries */
  cl_device_id *devices;
  cl_context_properties *properties;

  /* implementation */
  uint32_t num_devices;
  uint32_t num_properties;

  /* the original device list given to clCreateContext,
   * required for */
  cl_device_id *create_devices;
  uint32_t num_create_devices;

  cl_command_queue *default_queues;

  cl_command_queue command_queues;

  /* True if none of devices support cl_ext_buffer_device_address */
  cl_bool no_devices_support_bda;
};

struct _cl_command_queue {
  ThiveclObject base;

  /* queries */
  cl_context context;
  cl_device_id device;
  cl_command_queue_properties properties;

  /* Number of unfinished command enqueued. */
  unsigned long command_count;
  /* device specific data */
  void *data;

  /* list of CQs stored in cl_context */
  struct _cl_command_queue *prev, *next;
};

// struct _cl_mem {
//   CL_OBJECT_BODY;
//   atomic_int refcount;
//   uint64_t id;

//   cl_context context;
//   cl_mem_object_type type;
//   cl_mem_flags flags;

//   size_t size;

//   /* Image flags */
//   cl_bool is_image;

//   /* pipe flags */
//   cl_bool is_pipe;
// };

// struct _cl_event {
//   CL_OBJECT_BODY;
//   atomic_int refcount;
//   uint64_t id;

//   cl_context context;
// };

/* ============ List ============ */
#define DL_APPEND(head, add)      \
  do {                            \
    if (head) {                   \
      (add)->prev = (head)->prev; \
      (head)->prev->next = (add); \
      (head)->prev = (add);       \
      (add)->next = NULL;         \
    } else {                      \
      (head) = (add);             \
      (head)->prev = (head);      \
      (head)->next = NULL;        \
    }                             \
  } while (0)

#define DL_DELETE(head, del)           \
  do {                                 \
    if ((del)->prev) {                 \
      (del)->prev->next = (del)->next; \
    } else {                           \
      (head) = (del)->next;            \
    }                                  \
    if ((del)->next) {                 \
      (del)->next->prev = (del)->prev; \
    }                                  \
    (del)->prev = NULL;                \
    (del)->next = NULL;                \
  } while (0)

/* ============ Command queue list helpers ============ */
static inline int context_add_command_queue(struct _cl_context *ctx, cl_command_queue queue) {
  DL_APPEND(ctx->command_queues, queue);
  return CL_SUCCESS;
}

static inline int context_remove_command_queue(struct _cl_context *ctx, cl_command_queue queue) {
  DL_DELETE(ctx->command_queues, queue);
  return CL_SUCCESS;
}

#endif /* _CL_HELPER_H_ */
