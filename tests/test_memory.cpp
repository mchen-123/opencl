// buffer_write_read_case.c
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(err, msg) \
    do { \
        if (err != CL_SUCCESS) { \
            fprintf(stderr, "[ERROR] %s: %d (line %d)\n", msg, err, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

int main() {
    cl_int err;
    cl_platform_id platform = NULL;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_mem buffer = NULL;

    const size_t BUFFER_SIZE = 64;
    const size_t DATA_BYTES = BUFFER_SIZE * sizeof(float);

    // 1. 初始化平台和设备
    err = clGetPlatformIDs(1, &platform, NULL);
    CHECK(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err == CL_DEVICE_NOT_FOUND) {
        printf("GPU not found, falling back to CPU...\n");
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
    }
    CHECK(err, "clGetDeviceIDs");

    // 2. 创建上下文和命令队列
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    CHECK(err, "clCreateContext");

    cl_queue_properties properties[] = { 0 };
    queue = clCreateCommandQueue(context, device, 0, &err);
    CHECK(err, "clCreateCommandQueueWithProperties");

    // 3. 准备主机数据
    float *host_write = (float*)malloc(DATA_BYTES);
    float *host_read  = (float*)malloc(DATA_BYTES);

    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        host_write[i] = (float)(i * 1000);  // 明显值，便于验证
    }

    printf("Host data prepared (first 5): ");
    for (int i = 0; i < 5; i++) printf("%.0f ", host_write[i]);
    printf("...\n");

    // =========================================================
    // 核心：clCreateBuffer + clEnqueueWriteBuffer + clEnqueueReadBuffer
    // =========================================================

    // Step 1: 使用 clCreateBuffer 创建一个 READ_WRITE Buffer
    buffer = clCreateBuffer(
        context,
        CL_MEM_READ_WRITE,      // 主机可读可写
        DATA_BYTES,
        NULL,                   // 不使用 host_ptr 初始化
        &err
    );
    CHECK(err, "clCreateBuffer");

    printf("clCreateBuffer: Buffer created (%zu bytes)\n", DATA_BYTES);

    // Step 2: 使用 clEnqueueWriteBuffer 将主机数据写入设备 Buffer
    err = clEnqueueWriteBuffer(
        queue,
        buffer,
        CL_TRUE,                // 阻塞写（等待完成）
        0,                      // offset
        DATA_BYTES,
        host_write,             // 源数据
        0, NULL, NULL
    );
    CHECK(err, "clEnqueueWriteBuffer");

    printf("clEnqueueWriteBuffer: Data written to device\n");

    // Step 3: 使用 clEnqueueReadBuffer 将设备数据读回主机
    err = clEnqueueReadBuffer(
        queue,
        buffer,
        CL_TRUE,                // 阻塞读
        0,
        DATA_BYTES,
        host_read,              // 目标缓冲区
        0, NULL, NULL
    );
    CHECK(err, "clEnqueueReadBuffer");

    printf("clEnqueueReadBuffer: Data read back to host\n");

    // =========================================================

    // 4. 验证数据一致性
    // int errors = 0;
    // for (size_t i = 0; i < BUFFER_SIZE; i++) {
    //     if (host_write[i] != host_read[i]) {
    //         if (errors < 5) {
    //             printf("  [FAIL] index %zu: wrote %.1f, read %.1f\n",
    //                    i, host_write[i], host_read[i]);
    //         }
    //         errors++;
    //     }
    // }

    // if (errors == 0) {
    //     printf("Verification: PASSED (all %zu elements match)\n", BUFFER_SIZE);
    // } else {
    //     printf("Verification: FAILED (%d errors)\n", errors);
    // }

    // 5. 清理资源
    if (buffer) clReleaseMemObject(buffer);
    if (queue)  clReleaseCommandQueue(queue);
    if (context) clReleaseContext(context);
    free(host_write);
    free(host_read);

    printf("Cleanup complete.\n");
    return 0;
}