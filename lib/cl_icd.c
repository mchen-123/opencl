#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "cl_icd_structs.h"

#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define CL_USE_DEPRECATED_OPENCL_2_1_APIS
#define CL_USE_DEPRECATED_OPENCL_2_2_APIS

/* Need to rename all CL API functions to prevent ICD loader functions calling
 * themselves via the dispatch table. Include this before cl headers. */
#include "cl_rename_api.h"

#include <CL/cl.h>
#include <CL/cl_gl.h>

CLIicdDispatchTable *g_icd_dispatchTable = NULL;

#define ICD_DISPATCH_TABLE_ENTRY(fn) \
    assert(dispatchTable->entryCount < 256); \
    dispatchTable->entries[dispatchTable->entryCount++] = (void *)(intptr_t)(fn)

cl_int cliIcdDispatchTableCreate(void)
{
    CLIicdDispatchTable *dispatchTable = (CLIicdDispatchTable *)malloc(sizeof(*dispatchTable));
    if (!dispatchTable) {
        return CL_OUT_OF_HOST_MEMORY;
    }
    memset(dispatchTable, 0, sizeof(*dispatchTable));

    /* OpenCL 1.0 */
#ifdef CL_ENABLE_ICD2
    ICD_DISPATCH_TABLE_ENTRY ( CL_ICD2_TAG_KHR               );
#else
    ICD_DISPATCH_TABLE_ENTRY ( clGetPlatformIDs              );
#endif
    ICD_DISPATCH_TABLE_ENTRY ( clGetPlatformInfo             );
    ICD_DISPATCH_TABLE_ENTRY ( clGetDeviceIDs                );
    ICD_DISPATCH_TABLE_ENTRY ( clGetDeviceInfo               );
    ICD_DISPATCH_TABLE_ENTRY ( NULL              );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL               );
    ICD_DISPATCH_TABLE_ENTRY ( NULL              );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       ); /* ICD_DISPATCH_TABLE_ENTRY ( clGetContextInfo              ); */
    ICD_DISPATCH_TABLE_ENTRY ( NULL          );
    ICD_DISPATCH_TABLE_ENTRY ( NULL         );
    ICD_DISPATCH_TABLE_ENTRY ( NULL         );
    ICD_DISPATCH_TABLE_ENTRY ( NULL               );
    ICD_DISPATCH_TABLE_ENTRY ( NULL               );
    ICD_DISPATCH_TABLE_ENTRY ( NULL                );
    ICD_DISPATCH_TABLE_ENTRY ( NULL               );
    ICD_DISPATCH_TABLE_ENTRY ( NULL               );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL            );
    ICD_DISPATCH_TABLE_ENTRY ( NULL               );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL               );
    ICD_DISPATCH_TABLE_ENTRY ( NULL               );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL               );
    ICD_DISPATCH_TABLE_ENTRY ( NULL               );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       ); /* ICD_DISPATCH_TABLE_ENTRY ( clCreateProgramWithBinary     ); */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       ); /* ICD_DISPATCH_TABLE_ENTRY ( clReleaseProgram              ); */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       ); /* ICD_DISPATCH_TABLE_ENTRY ( clBuildProgram                ); */
#ifdef CL_ENABLE_ICD2
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
#else
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
#endif
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       ); /* ICD_DISPATCH_TABLE_ENTRY ( clCreateKernel                ); */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       ); /* ICD_DISPATCH_TABLE_ENTRY ( clReleaseKernel               ); */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       ); /* ICD_DISPATCH_TABLE_ENTRY ( clSetKernelArg                ); */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL    );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       ); /* ICD_DISPATCH_TABLE_ENTRY ( clFinish                      ); */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       ); /* ICD_DISPATCH_TABLE_ENTRY ( clEnqueueReadBuffer           ); */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       ); /* ICD_DISPATCH_TABLE_ENTRY ( clEnqueueNDRangeKernel        ); */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );

    /* cl_khr_gl_sharing */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );

    /* cl_khr_d3d10_sharing (windows-only) */
#if 0 && defined(_WIN32)
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
#else
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
#endif

    /* OpenCL 1.1 */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );

    /* cl_ext_device_fission */
    ICD_DISPATCH_TABLE_ENTRY ( /*clCreateSubDevicesEXT*/NULL);
    ICD_DISPATCH_TABLE_ENTRY ( /*clRetainDeviceEXT*/ NULL);
    ICD_DISPATCH_TABLE_ENTRY ( /*clReleaseDevice*/NULL);

    /* cl_khr_gl_event */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );

    /* OpenCL 1.2 */
    ICD_DISPATCH_TABLE_ENTRY ( clCreateSubDevices       );
    ICD_DISPATCH_TABLE_ENTRY ( clRetainDevice       );
    ICD_DISPATCH_TABLE_ENTRY ( clReleaseDevice      );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );

    /* cl_khr_d3d11_sharing */
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );

    /* cl_khr_dx9_media_sharing */
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );

    /* cl_khr_egl_image */
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );

    /* cl_khr_egl_event */
    ICD_DISPATCH_TABLE_ENTRY( NULL );

    /* OpenCL 2.0 */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       ); /* ICD_DISPATCH_TABLE_ENTRY( clCreateCommandQueueWithProperties ); */
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );

    /* cl_khr_sub_groups */
    ICD_DISPATCH_TABLE_ENTRY( NULL );

    /* OpenCL 2.1 */
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );

    /* OpenCL 2.2 */
    ICD_DISPATCH_TABLE_ENTRY( NULL );
    ICD_DISPATCH_TABLE_ENTRY( NULL );

    /* OpenCL 3.0 */
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );
    ICD_DISPATCH_TABLE_ENTRY ( NULL       );

    /* return success */
    g_icd_dispatchTable = dispatchTable;
    return CL_SUCCESS;
}

void
cliIcdDispatchTableDestroy(CLIicdDispatchTable *dispatchTable)
{
    free(dispatchTable);
}