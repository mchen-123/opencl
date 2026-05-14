#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <iostream>
#include <CL/cl.h>
#include <cstdlib>

int main() {
  cl_int err;
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue command_queue;

  // 获取第一个平台
  err = clGetPlatformIDs(1, &platform, nullptr);
  if (err != CL_SUCCESS) {
    std::cerr << "错误: 找不到OpenCL平台" << std::endl;
    return -1;
  }

  // 获取第一个GPU设备
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
  if (err != CL_SUCCESS) {
    std::cerr << "错误: 找不到GPU设备" << std::endl;
    return -1;
  }

  // 创建上下文
  context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
  if (err != CL_SUCCESS) {
    std::cerr << "错误: 创建OpenCL上下文失败" << std::endl;
    return -1;
  }
  std::cout << "成功创建GPU上下文" << std::endl;

  // 创建命令队列（stream）
  command_queue = clCreateCommandQueue(context, device, 0, &err);
  if (err != CL_SUCCESS) {
    std::cerr << "错误：创建命令队列失败" << std::endl;
    clReleaseContext(context);
    return -1;
  }
  std::cout << "成功创建GPU命令队列! " << std::endl;

  // 释放资源
  clReleaseCommandQueue(command_queue);
  clReleaseContext(context);

  std::cout << "资源已释放，程序正常结束" << std::endl;
  return 0;
}
