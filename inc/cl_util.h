#ifndef _DFCL_CONFIG_H_
#define _DFCL_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cl_helper.h"

typedef pthread_mutex_t dfcl_lock_t;

/* ============ Logging ============ */
#ifndef DFCL_MSG_ERR
#define DFCL_MSG_ERR(...)            \
  do {                               \
    fprintf(stderr, "DFCL ERROR: "); \
    fprintf(stderr, __VA_ARGS__);    \
    fprintf(stderr, "\n");           \
  } while (0)
#endif

#ifndef DFCL_MSG_INFO
#define DFCL_MSG_INFO(...) \
  do {                     \
    printf("DFCL INFO: "); \
    printf(__VA_ARGS__);   \
    printf("\n");          \
  } while (0)
#endif

#define DFCL_GOTO_ERROR_ON(cond, err_code, ...) \
  do {                                          \
    if (cond) {                                 \
      DFCL_MSG_ERR(__VA_ARGS__);                \
      errcode = err_code;                       \
      goto ERROR;                               \
    }                                           \
  } while (0)

#define DFCL_GOTO_ERROR_COND(cond, err_code) \
  do {                                       \
    if (cond) {                              \
      errcode = err_code;                    \
      goto ERROR;                            \
    }                                        \
  } while (0)

/* ============ Error handling ============ */
#define DFCL_RETURN(VALUE, ERROR, RET)     \
  do {                                     \
    cl_int __err = (VALUE);                \
    if ((ERROR) != NULL) *(ERROR) = __err; \
    return (RET);                          \
  } while (0)

/* For scalar/value types (int, uint, bool, etc.) */
#define DFCL_RETURN_VALUE(val, type)                                \
  do {                                                              \
    if (param_value_size_ret) *param_value_size_ret = sizeof(type); \
    if (param_value) {                                              \
      if (param_value_size < sizeof(type)) return CL_INVALID_VALUE; \
      *(type *)param_value = (val);                                 \
    }                                                               \
  } while (0)

#define DFCL_RETURN_STRING(str)                                      \
  do {                                                               \
    const char *__str = (str);                                       \
    size_t __len = (__str ? strlen(__str) : 0) + 1;                  \
                                                                     \
    if (param_value_size_ret) *param_value_size_ret = __len;         \
                                                                     \
    if (param_value) {                                               \
      if (param_value_size < __len) {                                \
        if (param_value_size > 0) {                                  \
          strncpy((char *)param_value, __str, param_value_size - 1); \
          ((char *)param_value)[param_value_size - 1] = '\0';        \
        }                                                            \
        return CL_INVALID_VALUE;                                     \
      }                                                              \
      strncpy((char *)param_value, __str, __len);                    \
    }                                                                \
  } while (0)

/* ============ Memory helpers ============ */
#define DFCL_NEW(type_name) ((type_name *)calloc(1, sizeof(type_name)))
#define DFCL_NEW_ARRAY(type_name, count) ((type_name *)calloc((count), sizeof(type_name)))
#define DFCL_MEM_FREE(PTR) \
  do {                     \
    free((void *)(PTR));   \
    (PTR) = NULL;          \
  } while (0)

/* ============ Object destruction ============ */
#define DFCL_DESTROY_LOCK(__LOCK__) pthread_mutex_destroy(&(__LOCK__))

#define DFCL_DESTROY_OBJECT(__OBJ__)                 \
  do {                                               \
    ThiveclObject *__o = (ThiveclObject *)(__OBJ__); \
    __o->magic = 0;                                  \
    DFCL_DESTROY_LOCK(__o->lock);                    \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif /* _DFCL_CONFIG_H_ */
