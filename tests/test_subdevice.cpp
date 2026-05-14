#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>

int main(void) {
  cl_platform_id platform = NULL;
  cl_device_id root_dev = NULL;
  cl_device_id sub_devs[4] = {NULL};
  cl_uint num_platforms = 0;
  cl_uint num_devices = 0;
  cl_int err;

  printf("=== THRIVE Sub-Device + Retain/Release Test ===\n\n");

  /* 1. 获取 Platform */
  err = clGetPlatformIDs(1, &platform, &num_platforms);
  if (err != CL_SUCCESS || platform == NULL) {
    printf("clGetPlatformIDs failed: %d\n", err);
    return -1;
  }

  /* 2. 获取 Root Device */
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &root_dev, &num_devices);
  if (err != CL_SUCCESS || root_dev == NULL) {
    printf("clGetDeviceIDs failed: %d\n", err);
    return -1;
  }
  printf("Root Device acquired: %p\n", (void *)root_dev);

  /* 3. 创建 Sub Devices（使用 die2 和 die3） */
  cl_device_partition_property props[] = {CL_DEVICE_PARTITION_BY_NAMES_EXT, 2, 3,  // die2 和 die3
                                          0};

  err = clCreateSubDevices(root_dev, props, 2, sub_devs, NULL);
  if (err != CL_SUCCESS) {
    printf("clCreateSubDevices failed: %d\n", err);
    return -1;
  }

  printf("Successfully created 2 Sub-Devices:\n");
  printf("  sub_devs[0] (die2) = %p\n", (void *)sub_devs[0]);
  printf("  sub_devs[1] (die3) = %p\n", (void *)sub_devs[1]);

  /* 4. 测试 clRetainDevice */
  printf("\n--- Testing clRetainDevice ---\n");
  for (int i = 0; i < 2; i++) {
    err = clRetainDevice(sub_devs[i]);
  }

  /* 5. 测试 clGetDeviceInfo on Sub Devices */
  printf("\n--- Testing clGetDeviceInfo on Sub-Devices ---\n");
  for (int i = 0; i < 2; i++) {
    cl_uint compute_units = 0;
    cl_uint vendor_id = 0;
    cl_uint refCt = 0;
    clGetDeviceInfo(sub_devs[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units,
                    NULL);

    clGetDeviceInfo(sub_devs[i], CL_DEVICE_REFERENCE_COUNT, sizeof(refCt), &refCt, NULL);

    clGetDeviceInfo(sub_devs[i], CL_DEVICE_VENDOR_ID, sizeof(vendor_id), &vendor_id, NULL);

    printf("  Sub Device %d: CU=%u, reference_count =%u, VendorID=0x%04x\n", i, compute_units,
           refCt, vendor_id);
  }

  /* 6. 测试 clReleaseDevice */
  printf("\n--- Testing clReleaseDevice ---\n");
  for (int i = 0; i < 2; i++) {
    err = clReleaseDevice(sub_devs[i]);
  }

  /* 7. Final cleanup */
  if (root_dev) clReleaseDevice(root_dev);

  printf("\n=== All Tests Completed ===\n");
  return 0;
}
