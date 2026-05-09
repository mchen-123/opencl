#ifndef _DFCL_CONFIG_H_
#define _DFCL_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

int dfcl_buffer_boundcheck(cl_mem buffer, size_t offset, size_t size);

int dfcl_check_event_wait_list(cl_command_queue command_queue,
                            cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list);

int dfcl_get_bool_option(const char *key, int default_value);

#ifdef __cplusplus
}
#endif

#endif /* _DFCL_CONFIG_H_ */