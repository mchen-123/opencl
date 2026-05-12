// cl_platform.cpp
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <CL/cl.h>

using namespace std;

// 辅助函数：安全查询字符串信息
string getPlatformInfo(cl_platform_id platform, cl_platform_info param) {
  size_t size = 0;
  clGetPlatformInfo(platform, param, 0, nullptr, &size);
  string result(size, '\0');
  clGetPlatformInfo(platform, param, size, &result[0], nullptr);
  // 去掉末尾的空字符
  if (!result.empty() && result.back() == '\0') result.pop_back();
  return result;
}

string getDeviceInfo(cl_device_id device, cl_device_info param) {
  size_t size = 0;
  clGetDeviceInfo(device, param, 0, nullptr, &size);
  string result(size, '\0');
  clGetDeviceInfo(device, param, size, &result[0], nullptr);
  if (!result.empty() && result.back() == '\0') result.pop_back();
  return result;
}

// 设备类型转字符串
string deviceTypeToString(cl_device_type type) {
  vector<string> types;
  if (type & CL_DEVICE_TYPE_GPU) types.push_back("GPU");
  if (type & CL_DEVICE_TYPE_CPU) types.push_back("CPU");
  if (type & CL_DEVICE_TYPE_ACCELERATOR) types.push_back("加速器");
  if (type & CL_DEVICE_TYPE_CUSTOM) types.push_back("自定义");
  if (type == CL_DEVICE_TYPE_DEFAULT) types.push_back("默认");

  if (types.empty()) return "未知";
  string result = types[0];
  for (size_t i = 1; i < types.size(); ++i) {
    result += " | " + types[i];
  }
  return result;
}

int main() {
  cl_uint num_platforms = 0;
  cl_int err = clGetPlatformIDs(0, nullptr, &num_platforms);
  if (err != CL_SUCCESS || num_platforms == 0) {
    cerr << "错误：未找到 OpenCL 平台！错误码: " << err << endl;
    return -1;
  }

  cout << "检测到 " << num_platforms << " 个 OpenCL 平台：" << endl;
  cout << string(80, '=') << endl;

  vector<cl_platform_id> platforms(num_platforms);
  err = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);
  if (err != CL_SUCCESS) {
    cerr << "获取平台列表失败！" << endl;
    return -1;
  }

  for (cl_uint i = 0; i < num_platforms; ++i) {
    cl_platform_id platform = platforms[i];

    // === 平台信息 ===
    string name = getPlatformInfo(platform, CL_PLATFORM_NAME);
    string vendor = getPlatformInfo(platform, CL_PLATFORM_VENDOR);
    string version = getPlatformInfo(platform, CL_PLATFORM_VERSION);
    string profile = getPlatformInfo(platform, CL_PLATFORM_PROFILE);

    cout << "\n[" << i << "] 平台: " << name << endl;
    cout << "    厂商: " << vendor << endl;
    cout << "    版本: " << version << endl;
    cout << "    模式: " << profile << endl;

    // === 查询该平台下的设备 ===
    cl_uint num_devices = 0;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &num_devices);
    if (err != CL_SUCCESS || num_devices == 0) {
      cout << "    无设备" << endl;
      continue;
    }
    vector<cl_device_id> devices(num_devices);
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, devices.data(), nullptr);
    if (err != CL_SUCCESS) {
      cout << "    获取设备失败" << endl;
      continue;
    }
    cout << "    共有 " << num_devices << " 个设备：" << endl;
  }

  cout << string(80, '=') << endl;
  cout << "测试完成。" << endl;
  return 0;
}
