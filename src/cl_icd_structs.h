#ifndef _CL_ICD_STRUCTS_H_
#define _CL_ICD_STRUCTS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CLIicdDispatchTable_st CLIicdDispatchTable;

struct CLIicdDispatchTable_st {
  void *entries[512]; /* 增大到 512，支持 OpenCL 3.0 完整 dispatch table */
  int entryCount;
};

#define CL_OBJECT_BODY CLIicdDispatchTable *dispatch_

/* ============ OpenCL Object Structures ============ */
typedef struct _cl_platform_id _cl_platform_id;
typedef struct _cl_device_id _cl_device_id;
// typedef struct _cl_context *cl_context;
// typedef struct _cl_command_queue *cl_command_queue;
// typedef struct _cl_mem *cl_mem;
// typedef struct _cl_event *cl_event;
// typedef struct _cl_program *cl_program;
// typedef struct _cl_kernel *cl_kernel;
// typedef struct _cl_sampler *cl_sampler;

// #define CL_INIT_OBJECT(obj, parent)
//   do {
//     (obj)->refcount = 1;
//     (obj)->dispatch_ = (parent)->dispatch_;
//   } while (0)

// #define CL_INIT_PLATFORM(obj, table)
//   do {
//     (obj)->dispatch_ = (table);
//   } while (0)

#ifdef __cplusplus
}
#endif

#endif /* _CL_ICD_STRUCTS_H_ */
