#include <iostream>
#include <CL/cl.h>

int main() {
    cl_int err;
    cl_platform_id platform;
    cl_uint num_devices;
    cl_device_id* all_devices = NULL;
    cl_context context;

    // 获取平台（默认取第一个平台）
    err = clGetPlatformIDs(1, &platform, nullptr);
    if (err != CL_SUCCESS) {
        std::cerr << "No OpenCL platforms found." << std::endl;
        return -1;
    }

    // 获取该平台下所有 GPU 设备
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to get device IDs." << std::endl;
        return -1;
    }

    all_devices = (cl_device_id*)malloc(num_devices * sizeof(cl_device_id));
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, all_devices, NULL);
    if (err != CL_SUCCESS) {
        std::cerr << "No OpenCL devices GPU found." << std::endl;
        return -1;
    }

    context = clCreateContext(
        NULL,               // properties
        1,                  // num_devices
        all_devices,        // 两个 GPU
        NULL, NULL,         // 回调
        &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create OpenCL context." << std::endl;
        return -1;
    }

    printf("成功创建包含 1 个 GPU 的 context!\n");

    clReleaseContext(context);
    return 0;
}