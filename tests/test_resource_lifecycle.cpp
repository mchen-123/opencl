#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static int failures = 0;

#define CHECK(cond, msg, ...)                     \
  do {                                            \
    if (!(cond)) {                                \
      printf("  FAIL: " msg "\n", ##__VA_ARGS__); \
      failures++;                                 \
    } else {                                      \
      printf("  PASS: " msg "\n", ##__VA_ARGS__); \
    }                                             \
  } while (0)

#define CHECK_ERR(err, expected, msg, ...)                                                 \
  do {                                                                                     \
    if ((err) != (expected)) {                                                             \
      printf("  FAIL: " msg " (got %d, expected %d)\n", ##__VA_ARGS__, (err), (expected)); \
      failures++;                                                                          \
    } else {                                                                               \
      printf("  PASS: " msg "\n", ##__VA_ARGS__);                                          \
    }                                                                                      \
  } while (0)

/* ========================================================================
 * Test 1: clRetainContext / clReleaseContext basic lifecycle
 * ======================================================================== */
static int test_context_retain_release(void) {
  printf("\n=== Test 1: Context Retain/Release ===\n");

  cl_int err;
  cl_platform_id platform;
  cl_device_id device;
  cl_context ctx;

  err = clGetPlatformIDs(1, &platform, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetPlatformIDs");

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetDeviceIDs");

  ctx = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  CHECK_ERR(err, CL_SUCCESS, "clCreateContext");

  /* Retain multiple times */
  for (int i = 0; i < 3; i++) {
    err = clRetainContext(ctx);
    CHECK_ERR(err, CL_SUCCESS, "clRetainContext #%d", i + 1);
  }

  /* Release matching number of times */
  for (int i = 0; i < 3; i++) {
    err = clReleaseContext(ctx);
    CHECK_ERR(err, CL_SUCCESS, "clReleaseContext #%d", i + 1);
  }

  /* Final release should destroy */
  err = clReleaseContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseContext (final, should destroy)");

  return failures;
}

/* ========================================================================
 * Test 2: Context with multiple devices
 * ======================================================================== */
static int test_context_multi_device(void) {
  printf("\n=== Test 2: Context with Multiple Devices ===\n");

  cl_int err;
  cl_platform_id platform;
  cl_device_id devices[4];
  cl_uint num_devices;
  cl_context ctx;

  err = clGetPlatformIDs(1, &platform, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetPlatformIDs");

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
  CHECK_ERR(err, CL_SUCCESS, "clGetDeviceIDs (query count)");
  printf("  Found %u GPU device(s)\n", num_devices);

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetDeviceIDs (get devices)");

  /* Create context with all devices */
  ctx = clCreateContext(NULL, num_devices, devices, NULL, NULL, &err);
  CHECK_ERR(err, CL_SUCCESS, "clCreateContext with %u device(s)", num_devices);

  /* Retain and release */
  err = clRetainContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clRetainContext");

  err = clReleaseContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseContext (retained)");

  err = clReleaseContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseContext (final)");

  return failures;
}

/* ========================================================================
 * Test 3: Sub-Device Retain/Release lifecycle
 * ======================================================================== */
static int test_subdevice_retain_release(void) {
  printf("\n=== Test 3: Sub-Device Retain/Release ===\n");

  cl_int err;
  cl_platform_id platform;
  cl_device_id root_dev;
  cl_device_id sub_devs[4] = {NULL};
  cl_uint refcount;

  err = clGetPlatformIDs(1, &platform, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetPlatformIDs");

  printf("=== Using our custom OpenCL implementation ===\n");
  printf("thrivePlatform = %p\n", platform);
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &root_dev, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetDeviceIDs");

  /* Create sub-devices */
  cl_device_partition_property props[] = {CL_DEVICE_PARTITION_BY_NAMES_EXT, 0, 1, 2, 0};
  err = clCreateSubDevices(root_dev, props, 3, sub_devs, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clCreateSubDevices (3 sub-devices)");

  /* Check initial refcount (should be 1 from creation) */
  for (int i = 0; i < 3; i++) {
    clGetDeviceInfo(sub_devs[i], CL_DEVICE_REFERENCE_COUNT, sizeof(refcount), &refcount, NULL);
    CHECK(refcount >= 1, "sub_dev[%d] initial refcount = %u", i, refcount);
  }

  // /* Retain each sub-device */
  // for (int i = 0; i < 3; i++) {
  //   err = clRetainDevice(sub_devs[i]);
  //   CHECK_ERR(err, CL_SUCCESS, "clRetainDevice sub_dev[%d]", i);
  // }

  // /* Check refcount after retain */
  // for (int i = 0; i < 3; i++) {
  //   clGetDeviceInfo(sub_devs[i], CL_DEVICE_REFERENCE_COUNT, sizeof(refcount), &refcount, NULL);
  //   CHECK(refcount >= 2, "sub_dev[%d] refcount after retain = %u", i, refcount);
  // }

  // /* Release each sub-device (once for retain) */
  // for (int i = 0; i < 3; i++) {
  //   err = clReleaseDevice(sub_devs[i]);
  //   CHECK_ERR(err, CL_SUCCESS, "clReleaseDevice sub_dev[%d] (retain release)", i);
  // }

  // /* Release each sub-device (final - should free) */
  // for (int i = 0; i < 3; i++) {
  //   err = clReleaseDevice(sub_devs[i]);
  //   CHECK_ERR(err, CL_SUCCESS, "clReleaseDevice sub_dev[%d] (final)", i);
  // }

  // /* Root device release is a no-op (managed by platform) */
  // err = clReleaseDevice(root_dev);
  // CHECK_ERR(err, CL_SUCCESS, "clReleaseDevice root_dev (no-op)");

  return failures;
}

/* ========================================================================
 * Test 4: Context with Sub-Devices
 * ======================================================================== */
static int test_context_with_subdevices(void) {
  printf("\n=== Test 4: Context with Sub-Devices ===\n");

  cl_int err;
  cl_platform_id platform;
  cl_device_id root_dev;
  cl_device_id sub_devs[2] = {NULL};
  cl_context ctx;

  err = clGetPlatformIDs(1, &platform, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetPlatformIDs");

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &root_dev, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetDeviceIDs");

  /* Create sub-devices */
  cl_device_partition_property props[] = {CL_DEVICE_PARTITION_BY_NAMES_EXT, 4, 5, 0};
  err = clCreateSubDevices(root_dev, props, 2, sub_devs, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clCreateSubDevices (2 sub-devices)");

  /* Create context with sub-devices */
  ctx = clCreateContext(NULL, 2, sub_devs, NULL, NULL, &err);
  CHECK_ERR(err, CL_SUCCESS, "clCreateContext with sub-devices");

  /* Retain context */
  err = clRetainContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clRetainContext");

  err = clReleaseContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseContext (retained)");

  /* Final release - should release sub-devices and default queues */
  err = clReleaseContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseContext (final)");

  /* Release sub-devices (context retained them, so we release our refs too) */
  for (int i = 0; i < 2; i++) {
    err = clReleaseDevice(sub_devs[i]);
    CHECK_ERR(err, CL_SUCCESS, "clReleaseDevice sub_dev[%d]", i);
  }

  err = clReleaseDevice(root_dev);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseDevice root_dev");

  return failures;
}

/* ========================================================================
 * Test 5: Command Queue lifecycle with explicit retain/release
 * ======================================================================== */
static int test_commandqueue_lifecycle(void) {
  printf("\n=== Test 5: Command Queue Lifecycle ===\n");

  cl_int err;
  cl_platform_id platform;
  cl_device_id device;
  cl_context ctx;
  cl_command_queue cq1, cq2, cq3;

  err = clGetPlatformIDs(1, &platform, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetPlatformIDs");

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetDeviceIDs");

  ctx = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  CHECK_ERR(err, CL_SUCCESS, "clCreateContext");

  /* Create multiple command queues */
  cq1 = clCreateCommandQueue(ctx, device, 0, &err);
  CHECK_ERR(err, CL_SUCCESS, "clCreateCommandQueue #1");

  cq2 = clCreateCommandQueue(ctx, device, 0, &err);
  CHECK_ERR(err, CL_SUCCESS, "clCreateCommandQueue #2");

  cq3 = clCreateCommandQueue(ctx, device, 0, &err);
  CHECK_ERR(err, CL_SUCCESS, "clCreateCommandQueue #3");

  /* Release queues in different order than creation */
  err = clReleaseCommandQueue(cq2);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseCommandQueue #2");

  err = clReleaseCommandQueue(cq1);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseCommandQueue #1");

  err = clReleaseCommandQueue(cq3);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseCommandQueue #3");

  /* Context should still be alive (queues retained it) */
  err = clReleaseContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseContext (final)");

  return failures;
}

/* ========================================================================
 * Test 6: Interleaved retain/release stress test
 * ======================================================================== */
static int test_interleaved_retain_release(void) {
  printf("\n=== Test 6: Interleaved Retain/Release ===\n");

  cl_int err;
  cl_platform_id platform;
  cl_device_id device;
  cl_context ctx;
  cl_command_queue cq;

  err = clGetPlatformIDs(1, &platform, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetPlatformIDs");

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetDeviceIDs");

  ctx = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  CHECK_ERR(err, CL_SUCCESS, "clCreateContext");

  /* Create queue (retains context) */
  cq = clCreateCommandQueue(ctx, device, 0, &err);
  CHECK_ERR(err, CL_SUCCESS, "clCreateCommandQueue");

  /* Retain context multiple times */
  clRetainContext(ctx);
  clRetainContext(ctx);

  /* Release context - should not destroy (queue + 2 retains hold it) */
  err = clReleaseContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseContext #1 (still alive)");

  /* Release queue - releases its context ref */
  err = clReleaseCommandQueue(cq);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseCommandQueue");

  /* Release remaining context retains */
  err = clReleaseContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseContext #2");

  err = clReleaseContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseContext #3 (final)");

  return failures;
}

/* ========================================================================
 * Test 7: Sub-device + Context + Command Queue full lifecycle
 * ======================================================================== */
static int test_full_lifecycle(void) {
  printf("\n=== Test 7: Full Lifecycle (Sub-Device + Context + CQ) ===\n");

  cl_int err;
  cl_platform_id platform;
  cl_device_id root_dev;
  cl_device_id sub_devs[3] = {NULL};
  cl_context ctx;
  cl_command_queue cqs[3] = {NULL};

  err = clGetPlatformIDs(1, &platform, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetPlatformIDs");

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &root_dev, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clGetDeviceIDs");

  /* Create sub-devices */
  cl_device_partition_property props[] = {CL_DEVICE_PARTITION_BY_NAMES_EXT, 0, 1, 2, 0};
  err = clCreateSubDevices(root_dev, props, 3, sub_devs, NULL);
  CHECK_ERR(err, CL_SUCCESS, "clCreateSubDevices (3 sub-devices)");

  /* Create context with sub-devices */
  ctx = clCreateContext(NULL, 3, sub_devs, NULL, NULL, &err);
  CHECK_ERR(err, CL_SUCCESS, "clCreateContext with 3 sub-devices");

  /* Create command queues on each sub-device */
  for (int i = 0; i < 3; i++) {
    cqs[i] = clCreateCommandQueue(ctx, sub_devs[i], 0, &err);
    CHECK_ERR(err, CL_SUCCESS, "clCreateCommandQueue on sub_dev[%d]", i);
  }

  /* Retain context */
  clRetainContext(ctx);

  /* Release queues in reverse order */
  for (int i = 2; i >= 0; i--) {
    err = clReleaseCommandQueue(cqs[i]);
    CHECK_ERR(err, CL_SUCCESS, "clReleaseCommandQueue cqs[%d]", i);
  }

  /* Release context retains */
  err = clReleaseContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseContext (retained)");

  err = clReleaseContext(ctx);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseContext (final)");

  /* Release sub-devices */
  for (int i = 0; i < 3; i++) {
    err = clReleaseDevice(sub_devs[i]);
    CHECK_ERR(err, CL_SUCCESS, "clReleaseDevice sub_dev[%d]", i);
  }

  err = clReleaseDevice(root_dev);
  CHECK_ERR(err, CL_SUCCESS, "clReleaseDevice root_dev");

  return failures;
}

/* ========================================================================
 * Test 8: Error handling - invalid parameters
 * ======================================================================== */
static int test_error_handling(void) {
  printf("\n=== Test 8: Error Handling ===\n");

  cl_int err;

  /* NULL context */
  err = clRetainContext(NULL);
  CHECK_ERR(err, CL_INVALID_CONTEXT, "clRetainContext(NULL)");

  err = clReleaseContext(NULL);
  CHECK_ERR(err, CL_INVALID_CONTEXT, "clReleaseContext(NULL)");

  /* NULL device */
  err = clRetainDevice(NULL);
  CHECK_ERR(err, CL_INVALID_DEVICE, "clRetainDevice(NULL)");

  err = clReleaseDevice(NULL);
  CHECK_ERR(err, CL_INVALID_DEVICE, "clReleaseDevice(NULL)");

  return failures;
}

/* ========================================================================
 * Main
 * ======================================================================== */
int main(void) {
  printf("============================================================\n");
  printf("  OpenCL Resource Lifecycle Test Suite\n");
  printf("============================================================\n");

  // test_context_retain_release();
  // test_context_multi_device();
  // test_subdevice_retain_release();
  // test_context_with_subdevices();
  // test_commandqueue_lifecycle();
  // test_interleaved_retain_release();
  test_full_lifecycle();
  // test_error_handling();

  printf("\n============================================================\n");
  if (failures == 0) {
    printf("  ALL TESTS PASSED\n");
  } else {
    printf("  %d TEST(S) FAILED\n", failures);
  }
  printf("============================================================\n");

  return failures > 0 ? 1 : 0;
}
