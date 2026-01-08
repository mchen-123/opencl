#pragma once

#ifndef _CL_ICD_STRUCTS_HPP_
#define _CL_ICD_STRUCTS_HPP_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CLIicdDispatchTable_st  CLIicdDispatchTable;
typedef struct CLIplatform_st CLIplatform;

struct CLIicdDispatchTable_st
{
    void *entries[256];
    int entryCount;
};

#ifdef CL_ENABLE_ICD2

#define CL_OBJECT_BODY                                                         \
    CLIicdDispatchTable* dispatch;                                             \
    void* dispData

#define CL_INIT_OBJECT(obj, parent)                                            \
do                                                                             \
{                                                                              \
    obj->dispatch = parent->dispatch;                                          \
    obj->dispData = parent->dispData;                                           \
} while (0);

#define CL_INIT_PLATFORM(obj, table)                                           \
do                                                                             \
{                                                                              \
    obj->dispatch = table;                                                     \
    obj->dispData = NULL;                                                      \
} while (0);

#else

#define CL_OBJECT_BODY                                                         \
    CLIicdDispatchTable* dispatch_

#define CL_INIT_OBJECT(obj, parent)                                            \
do                                                                             \
{                                                                              \
    obj->refcount = 1;                                                         \
    obj->dispatch_ = parent->dispatch_;                                        \
} while (0)

#define CL_INIT_PLATFORM(obj, table)                                           \
do                                                                             \
{                                                                              \
    obj->dispatch_ = table;                                                     \
} while (0)

#endif

struct CLIplatform_st
{
    CL_OBJECT_BODY;
};

#ifdef __cplusplus
}
#endif
#endif /* _CL_ICD_STRUCTS_HPP_ */