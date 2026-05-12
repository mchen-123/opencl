#ifndef _DFCL_CONFIG_H_
#define _DFCL_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cl_helper.h"

typedef pthread_mutex_t dfcl_lock_t;

/* ============ Validation ============ */
static inline int is_valid(void *handle) {
  return handle != NULL;
}

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

/* Generic functionality for handling different types of
   OpenCL (host) objects. */
#define DFCL_LOCK(__LOCK__) pthread_mutex_lock(&(__LOCK__))
#define DFCL_UNLOCK(__LOCK__) pthread_mutex_unlock(&(__LOCK__))

int dfcl_buffer_boundcheck(cl_mem buffer, size_t offset, size_t size);

int dfcl_check_event_wait_list(cl_command_queue command_queue, cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list);

int dfcl_get_bool_option(const char *key, int default_value);

#ifdef __cplusplus
}
#endif

#endif /* _DFCL_CONFIG_H_ */
