#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define CHECK_ERROR(err) do { \
    if (err != CL_SUCCESS) { \
        fprintf(stderr, "OpenCL Error %d at line %d\n", err, __LINE__); \
        exit(1); \
    } \
} while(0)

int main(void)
{
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue[2] = {0};
    cl_mem buf[2] = {0};

    // 1. 获取平台和一个GPU设备
    CHECK_ERROR(clGetPlatformIDs(1, &platform, NULL));
    CHECK_ERROR(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL));

    // 2. 创建主上下文
    cl_context_properties props[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0
    };
    context = clCreateContext(props, 1, &device, NULL, NULL, &err);
    CHECK_ERROR(err);

    // 3. 定义分区方式：这里简单分成两个等份的计算单元（CU）
    cl_device_partition_property partition_prop[] = {
        CL_DEVICE_PARTITION_EQUALLY, 2,  // 尽量均分计算单元
        CL_DEVICE_PARTITION_BY_COUNTS_LIST_END, 0
    };

    cl_uint num_sub_devices;
    CHECK_ERROR(clCreateSubDevices(device, partition_prop, 0, NULL, &num_sub_devices));

    if (num_sub_devices < 2) {
        printf("无法创建2个子设备，只得到了 %u 个\n", num_sub_devices);
        return -1;
    }

    cl_device_id sub_devices[2];
    CHECK_ERROR(clCreateSubDevices(device, partition_prop, 2, sub_devices, NULL));

    printf("成功创建了 %u 个子设备\n", num_sub_devices);

    // 4. 为每个子设备创建独立的命令队列
    for (int i = 0; i < 2; i++) {
        queue[i] = clCreateCommandQueue(context, sub_devices[i], 0, &err);
        CHECK_ERROR(err);
    }

    // 5. 创建两个 buffer（每个子设备用一个）
    const size_t size_per_device = 8 * 1024 * 1024;  // 每个设备拷贝 8MB
    float* host_data = (float*)malloc(size_per_device * 2);
    if (!host_data) return -1;

    // 随便填充一些数据
    for (size_t i = 0; i < size_per_device * 2 / sizeof(float); i++) {
        host_data[i] = (float)i * 1.1f;
    }

    // 创建 buffer（使用 CL_MEM_COPY_HOST_PTR 也可以，但这里用显式 memcpy 演示）
    buf[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, size_per_device, NULL, &err);
    CHECK_ERROR(err);
    buf[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, size_per_device, NULL, &err);
    CHECK_ERROR(err);

    // 6. 分别拷贝数据到两个子设备
    printf("开始 H2D 拷贝...\n");

    // 子设备0
    CHECK_ERROR(clEnqueueWriteBuffer(
        queue[0],           // 子设备0的队列
        buf[0],
        CL_TRUE,            // blocking write
        0,
        size_per_device,
        host_data + 0,
        0, NULL, NULL));

    // 子设备1
    CHECK_ERROR(clEnqueueWriteBuffer(
        queue[1],           // 子设备1的队列
        buf[1],
        CL_TRUE,
        0,
        size_per_device,
        host_data + (size_per_device / sizeof(float)),
        0, NULL, NULL));

    printf("两个子设备的 H2D 拷贝完成\n");

    // （这里可以继续创建 kernel、在两个子设备上分别执行...）

    // 清理
    free(host_data);
    clReleaseMemObject(buf[0]);
    clReleaseMemObject(buf[1]);
    clReleaseCommandQueue(queue[0]);
    clReleaseCommandQueue(queue[1]);
    clReleaseContext(context);

    // 子设备不需要手动释放，释放主设备或上下文时会自动释放

    printf("程序正常结束\n");
    return 0;
}