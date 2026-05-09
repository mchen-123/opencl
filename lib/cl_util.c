#include <CL/cl.h>
#include <string.h>
#include <stdint.h>
#include "cl_helper.h"

int dfcl_buffer_boundcheck(cl_mem buffer, size_t offset, size_t size) {
    if ((offset > buffer->size) || (size > buffer->size) || (offset + size > buffer->size)) {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

int dfcl_check_event_wait_list(cl_command_queue command_queue,
                            cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list) {
    uint32_t i;
    if ((event_wait_list == NULL && num_events_in_wait_list > 0) || (event_wait_list != NULL && num_events_in_wait_list == 0)) {
        return CL_INVALID_EVENT_WAIT_LIST;
    }

    if (event_wait_list) {
        for (i = 0; i < num_events_in_wait_list; i++) {
            if (!is_valid((void *)event_wait_list[i])) {
                return CL_INVALID_EVENT_WAIT_LIST;
            }
            if (event_wait_list[i]->context != command_queue->context) {
                return CL_INVALID_CONTEXT;
            }
        }
    }

    return CL_SUCCESS;
}

int dfcl_get_bool_option(const char *key, int default_value) {
    const char *value = getenv(key);
    if (value != NULL) {
        return strncmp(value, "1", 1) == 0;
    }
    return default_value;
}