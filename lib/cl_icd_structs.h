#ifndef _CL_ICD_STRUCTS_H_
#define _CL_ICD_STRUCTS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CLIicdDispatchTable_st  CLIicdDispatchTable;
typedef struct CLIplatform_st          CLIplatform;

struct CLIicdDispatchTable_st
{
    void *entries[512];   /* 增大到 512，支持 OpenCL 3.0 完整 dispatch table */
    int   entryCount;
};

#define CL_OBJECT_BODY \
    CLIicdDispatchTable* dispatch_

#define CL_INIT_OBJECT(obj, parent) \
do { \
    (obj)->refcount = 1; \
    (obj)->dispatch_ = (parent)->dispatch_; \
} while (0)

#define CL_INIT_PLATFORM(obj, table) \
do { \
    (obj)->dispatch_ = (table); \
} while (0)

/* ====================== 平台结构 ====================== */
struct CLIplatform_st
{
    CL_OBJECT_BODY;
    /* 你以后可以在这里添加更多平台相关字段 */
};

#ifdef __cplusplus
}
#endif

#endif /* _CL_ICD_STRUCTS_H_ */