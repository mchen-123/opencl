#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

#include "cl_helper.h"
#include "devices.h"

extern cl_platform_id thrivePlatform;
extern int dfcl_offline_compile;

unsigned int cl_context_count = 0;
static pthread_mutex_t dfcl_context_handling_lock = PTHREAD_MUTEX_INITIALIZER;

atomic_int context_c = {0};

int
context_set_properties(cl_context context,
                       const cl_context_properties *properties)
{
    if (properties == NULL) {
        context->properties = NULL;
        context->num_properties = 0;
        return 0;
    }
    /* todo: properties are not supported yet */
    return 0;
}

/* todo: fill in the context structure */
int
dfcl_setup_context(cl_context context) {
    unsigned int i;
    for(i = 0; i < context->num_devices; i++) {
        cl_device_id dev = context->devices[i];
        if (dev->die_max_mem_alloc_size > context->die_max_mem_alloc_size)
            context->die_max_mem_alloc_size = dev->die_max_mem_alloc_size;
    }

    return CL_SUCCESS;
}

CL_API_ENTRY cl_context CL_API_CALL
clCreateContext(const cl_context_properties* properties,
                cl_uint num_devices,
                const cl_device_id* devices,
                void(CL_CALLBACK* pfn_notify)(const char *, const void *, size_t, void *),
                void* user_data,
                cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_int errcode = CL_SUCCESS;
    cl_uint i = 0;
    cl_context context = NULL;

    if (devices == NULL || num_devices == 0) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, NULL);
    }
    if (pfn_notify == NULL && user_data != NULL) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, NULL);
    }

    {
        pthread_mutex_lock(&dfcl_context_handling_lock);
        errcode = dfcl_init_devices(thrivePlatform);
        /* clCreateContext cannot return CL_DEVICE_NOT_FOUND, which is what
         * dfcl_init_devices() returns if no devices could be probed. Hence,
         * remap this error to CL_INVALID_DEVICE. Note that this particular
         * situation should never arise, since an application should issue
         * clGetDeviceIDs before clCreateContext, and we would have returned
         * CL_DEVICE_NOT_FOUND already from clGetDeviceIDs. Still, no reason
         * not to handle it.
         */
        if (errcode == CL_DEVICE_NOT_FOUND) {
            errcode = CL_INVALID_DEVICE;
        }
        if (errcode != CL_SUCCESS) {
            pthread_mutex_unlock(&dfcl_context_handling_lock);
            DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, NULL);
        }

        for (i = 0; i < num_devices; i++) {
            if (devices[i] == NULL) {
               pthread_mutex_unlock(&dfcl_context_handling_lock);
               DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, NULL);
            }
        }

        context = (cl_context)malloc(sizeof(struct _cl_context));
        if (context == NULL) {
            pthread_mutex_unlock(&dfcl_context_handling_lock);
            DFCL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, NULL);
        }
        CL_INIT_OBJECT(context, devices[0]);
        atomic_init(&context->refcount, 1);

        /* todo: 1. raw ptrs create */
        errcode = context_set_properties(context, properties);
        if (errcode != CL_SUCCESS) {
            DFCL_MEM_FREE(context);
            pthread_mutex_unlock(&dfcl_context_handling_lock);
            DFCL_SET_FUNCTION_VALUE_RETURN(errcode, errcode_ret, NULL);
        }

        context->create_devices = (cl_device_id *)malloc(sizeof(cl_device_id) * num_devices);
        if (context->create_devices == NULL) {
            DFCL_MEM_FREE(context->properties);
            DFCL_MEM_FREE(context);
            pthread_mutex_unlock(&dfcl_context_handling_lock);
            DFCL_SET_FUNCTION_VALUE_RETURN(errcode, errcode_ret, NULL);
        }
        memcpy(context->create_devices, devices, num_devices * sizeof(cl_device_id));
        context->num_create_devices = num_devices;

        /* todo: 2. transform subDevice to parent device. */
        context->devices = context->create_devices;
        context->num_devices = num_devices;

        context->default_queues = (cl_command_queue *)calloc(num_devices, sizeof(cl_command_queue));
        if (context->default_queues == NULL) {
            DFCL_MEM_FREE(context->create_devices);
            DFCL_MEM_FREE(context->properties);
            DFCL_MEM_FREE(context);
            pthread_mutex_unlock(&dfcl_context_handling_lock);
            DFCL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, NULL);
        }

        for (i = 0; i < num_devices; i++) {
            cl_device_id dev = context->devices[i];
            if (!dfcl_offline_compile && (dev->available == CL_FALSE)) {
                DFCL_MEM_FREE(context->create_devices);
                DFCL_MEM_FREE(context->properties);
                DFCL_MEM_FREE(context->default_queues);
                DFCL_MEM_FREE(context);
                pthread_mutex_unlock(&dfcl_context_handling_lock);
                DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, NULL);
            }
        }

        if (!dfcl_offline_compile) {
            errcode = dfcl_setup_context(context);
            if (errcode != CL_SUCCESS) {
                DFCL_MEM_FREE(context->create_devices);
                DFCL_MEM_FREE(context->properties);
                DFCL_MEM_FREE(context->default_queues);
                DFCL_MEM_FREE(context);
                pthread_mutex_unlock(&dfcl_context_handling_lock);
                DFCL_SET_FUNCTION_VALUE_RETURN(errcode, errcode_ret, NULL);
            }
        }
        for (i = 0; i < context->num_create_devices; i++) {
            if (devices[i] == NULL || devices[i]->available != CL_TRUE)
                continue;
            dfcl_retain_object((void *)devices[i]);
        }
        pthread_mutex_unlock(&dfcl_context_handling_lock);
    }

    if (errcode_ret) {
        *errcode_ret = CL_SUCCESS;
    }

    DFCL_ATOMIC_INC(&context_c);

    cl_context_count += 1;

    DFCL_MSG_INFO("Create Context: %p, RefCount: %d", context, dfcl_object_get_refcount((ThiveclObject *)context));
    return context;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    if (!is_valid(context)) {
        return CL_INVALID_CONTEXT;
    }

    dfcl_retain_object(context);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    if (!is_valid(context)) {
        return CL_INVALID_CONTEXT;
    }

    cl_uint i = 0;
    int new_refcount = dfcl_release_object(context);
    DFCL_MSG_INFO("Release Context: %p, new_refcount =%d", context, new_refcount);

    if (new_refcount == 0) {

        DFCL_ATOMIC_DEC(&context_c);

        for (i = 0; i < context->num_devices; i++) {
            /* cl_device_id dev = context->devices[i]; */
            if (context->default_queues && context->default_queues[i]) {
                /* todo: release default queue */
            }
        }

        for (i = 0; i < context->num_create_devices; i++) {
            /* todo: +releaseDevice(context->create_devices[i]); */
        }

        DFCL_MEM_FREE(context->create_devices);
        DFCL_MEM_FREE(context->default_queues);
        /* DFCL_MEM_FREE(context->devices); */
        DFCL_MEM_FREE(context->properties);

        DFCL_MEM_FREE(context);
        --cl_context_count;
    }

    return CL_SUCCESS;
}