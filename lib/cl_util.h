#ifndef _DFCL_CONFIG_H_
#define _DFCL_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ============ Validation ============ */
static inline int is_valid(void *handle) {
    return handle != NULL;
}

/* For scalar/value types (int, uint, bool, etc.) */
#define DFCL_RETURN_VALUE(val, type)                                      \
    do {                                                                  \
        if (param_value_size_ret)                                         \
            *param_value_size_ret = sizeof(type);                         \
        if (param_value) {                                                \
            if (param_value_size < sizeof(type))                          \
                return CL_INVALID_VALUE;                                  \
            *(type*)param_value = (val);                                  \
        }                                                                 \
    } while (0)

#define DFCL_RETURN_STRING(str)                                           \
    do {                                                                  \
        size_t len = strlen(str) + 1;                                     \
        if (param_value_size_ret)                                         \
            *param_value_size_ret = len;                                  \
                                                                          \
        if (param_value) {                                                \
            if (param_value_size < len)                                   \
                return CL_INVALID_VALUE;                                  \
            strncpy((char*)param_value, (str), param_value_size);         \
            ((char*)param_value)[param_value_size - 1] = '\0';            \
        }                                                                 \
    } while (0)

int dfcl_buffer_boundcheck(cl_mem buffer, size_t offset, size_t size);

int dfcl_check_event_wait_list(cl_command_queue command_queue,
                            cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list);

int dfcl_get_bool_option(const char *key, int default_value);

#ifdef __cplusplus
}
#endif

#endif /* _DFCL_CONFIG_H_ */