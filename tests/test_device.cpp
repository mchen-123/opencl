#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <CL/cl.h>

int main() {
    cl_platform_id platform = NULL;
    cl_device_id device = NULL;
    cl_uint num_platforms;
    cl_uint num_devices;
    cl_device_type device_type;
    cl_uint vendor_id;
    cl_uint compute_units = 0;        // 新增：计算单元数
    size_t size;

    // 1. 获取第一个平台
    clGetPlatformIDs(1, &platform, &num_platforms);

    // 2. 获取第一个 GPU 设备（或所有设备后遍历）
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &num_devices);
    if (num_devices == 0) {
        printf("没有找到 GPU 设备！\n");
        return -1;
    }

    // 获取平台名称
    char platform_name[128];
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
    printf("platform= %s, device= %p\n", platform_name, device);

    // 3. 查询设备类型
    cl_int err = clGetDeviceInfo(
        device,
        CL_DEVICE_TYPE,           // 要查询什么
        sizeof(cl_device_type),   // 缓冲区大小
        &device_type,             // 输出缓冲区
        NULL                      // 不需要返回大小
    );
    if (err != CL_SUCCESS) {
        printf("查询失败: %d\n", err);
        return -1;
    }

    // 4. 判断是否为 GPU
    if (device_type & CL_DEVICE_TYPE_GPU) {
        printf("这是一个 GPU 设备！\n");
    } else {
        printf("不是 GPU 类型 = 0x%llx\n", (unsigned long long)device_type);
    }

    // 5. 打印具体类型（可选）
    printf("设备类型: ");
    if (device_type & CL_DEVICE_TYPE_GPU)        printf("GPU ");
    if (device_type & CL_DEVICE_TYPE_CPU)        printf("CPU ");
    if (device_type & CL_DEVICE_TYPE_ACCELERATOR) printf("加速器 ");
    if (device_type & CL_DEVICE_TYPE_CUSTOM)     printf("自定义 ");
    if (device_type == CL_DEVICE_TYPE_DEFAULT)   printf("(默认) ");
    printf("\n");

    // 6. 查询设备的供应商 ID
    err = clGetDeviceInfo(
        device,
        CL_DEVICE_VENDOR_ID,      // 要查询什么
        sizeof(cl_uint),          // 缓冲区大小
        &vendor_id,               // 输出缓冲区
        NULL                      // 不需要返回大小
    );
    if (err != CL_SUCCESS) {
        printf("查询供应商 ID 失败: %d\n", err);
        return -1;
    }
    // 7. 打印供应商 ID
    printf("设备供应商 ID: 0x%04x\n", vendor_id);

    err = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
    if (err != CL_SUCCESS) {
        printf("查询 CL_DEVICE_MAX_COMPUTE_UNITS 失败: %d\n", err);
        return -1;
    }

    printf("计算单元数量 (SM/CU): %u\n", compute_units);
    
    // 第一步：获取所需缓冲区大小
    err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, 0, NULL, &size);
    if (err != CL_SUCCESS) {
        printf("Error: clGetDeviceInfo (size) failed: %d\n", err);
        return -1;
    }
    // 第二步：分配内存并查询
    char* vendor = (char*)malloc(size);
    err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, size, vendor, NULL);
    if (err == CL_SUCCESS) {
        printf("CL_DEVICE_VENDOR: %s\n", vendor);
    } else {
        printf("Error: clGetDeviceInfo failed: %d\n", err);
    }

    cl_device_fp_config config;
    clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(config), &config, NULL);
    std::cout << "Single FP Config: 0x" << std::hex << config << std::endl;

    char extensions[4096];
	clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(extensions), extensions, NULL);
    printf("Extensions: %s\n", extensions);

    free(vendor);

    return 0;
}