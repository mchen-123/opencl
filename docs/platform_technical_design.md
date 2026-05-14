# THRIVE OpenCL Platform 技术方案

## 1. 架构概览

```
┌──────────────────────────────────────────┐
│              OpenCL Application           │
└────────────────┬─────────────────────────┘
                 │ clGetPlatformIDs / clGetPlatformInfo
┌────────────────▼─────────────────────────┐
│           OpenCL ICD Loader               │
│   (libOpenCL.so / ocl-icd)               │
└────────────────┬─────────────────────────┘
                 │ clIcdGetPlatformIDsKHR
┌────────────────▼─────────────────────────┐
│         THRIVE ICD Provider               │
│  ┌───────────────────────────────────┐   │
│  │        cl_platform.c               │   │
│  │  ┌─────────────────────────────┐  │   │
│  │  │     initialize_once()       │  │   │
│  │  │  ├─ ICD Dispatch Table 创建  │  │   │
│  │  │  ├─ Platform 对象分配/初始化  │  │   │
│  │  │  └─ dfInit() 底层运行时初始化 │  │   │
│  │  └─────────────────────────────┘  │   │
│  │  thrivePlatform (单例)            │   │
│  └───────────────────────────────────┘   │
│  ┌───────────────────────────────────┐   │
│  │        cl_device.c                │   │
│  │  dfcl_init_devices() → Device 列表 │   │
│  └───────────────────────────────────┘   │
│  ┌───────────────────────────────────┐   │
│  │        dfRuntime (底层运行时)       │   │
│  │  dfInit / dfDeviceGet / ...       │   │
│  └───────────────────────────────────┘   │
└──────────────────────────────────────────┘
```

**核心设计原则：**
- **单 Platform**：全局仅暴露 1 个 Platform 对象，代表整个 THRIVE 加速计算引擎
- **懒初始化**：首次 API 调用时触发一次性初始化，避免静态构造顺序问题
- **ICD 标准兼容**：通过 ICD Dispatch Table + `clIcdGetPlatformIDsKHR` 扩展接口注册到 ICD Loader

---

## 2. 模块划分

| 模块 | 文件 | 职责 |
|------|------|------|
| Platform 管理 | `src/cl_platform.c` | Platform 生命周期、clGetPlatformIDs、clGetPlatformInfo、ICD 注册 |
| ICD Dispatch | `src/cl_icd.c` | Dispatch Table 构建，映射所有 OpenCL API 入口 |
| 对象基类 | `src/cl_helper.h` | ThiveclObject 基类（引用计数、magic、锁）、Platform/Device 结构体定义 |
| 工具宏 | `src/cl_util.h` | 内存分配、日志、错误处理、返回值辅助宏 |
| Device 管理 | `src/cl_device.c` | Device 发现、属性查询、SubDevice 分区 |
| 底层运行时 | `runtime_mock/` | dfRuntime 抽象层（dfInit、dfDeviceGet 等） |

---

## 3. 核心数据结构

### 3.1 Platform 对象

```c
struct _cl_platform_id {
    ThiveclObject base;       // 继承：dispatch_、refcount、magic、lock
    const char *profile;      // "EMBEDDED_PROFILE"
    const char *version;      // "OpenCL 3.0 THRIVE 1.0"
    const char *name;         // "THRIVE Accelerated Compute Engine"
    const char *vendor;       // "THRIVE Corporation"
    const char *extensions;   // "cl_khr_il_program cl_khr_fp64"
    const char *suffix;       // "THRIVE"（ICD 后缀）
};
```

### 3.2 ICD Dispatch Table

```c
struct CLIicdDispatchTable_st {
    void *entries[512];       // 函数指针数组，按 OpenCL ICD 规范排序
    int entryCount;           // 实际填充的条目数
};
```

Dispatch Table 按 OpenCL 版本演进顺序排列（1.0 → 1.1 → 1.2 → 2.0 → 2.1 → 2.2 → 3.0），未实现的 API 填 NULL，ICD Loader 会回退到自己的实现。

---

## 4. API 说明（精简版）

### 4.1 clGetPlatformIDs

获取系统中可用的 Platform 列表。本实现始终返回 1 个 Platform（thrivePlatform）。首次调用触发懒初始化。

### 4.2 clGetPlatformInfo

查询指定 Platform 的属性信息（名称、版本、厂商、扩展等），以字符串形式返回。

### 4.3 clIcdGetPlatformIDsKHR

ICD 扩展接口，行为与 clGetPlatformIDs 完全一致，供 ICD Loader 发现已安装的 Platform。

---

## 5. 初始化流程

```
clGetPlatformIDs / clIcdGetPlatformIDsKHR
        │
        ▼
  initialize_once()
        │
        ├─ 1. cliIcdDispatchTableCreate()
        │     分配并填充 256+ 条目的 Dispatch Table
        │     注册已实现的 API（GetPlatformIDs、GetDeviceIDs、CreateContext 等）
        │
        ├─ 2. 分配 thrivePlatform（DFCL_NEW + dfcl_init_object）
        │     设置 magic、refcount=1、dispatch_ 指针
        │
        ├─ 3. 填充平台属性字符串
        │     profile / version / name / vendor / extensions / suffix
        │
        └─ 4. 标记 g_initialized = CL_TRUE
```

**关键点：**
- `initialize_once()` 通过 `g_initialized` 标志保证仅执行一次
- Platform 属性为静态字符串常量，无动态内存管理负担
- Device 的初始化延迟到首次 `clGetDeviceIDs` 调用时执行（`dfcl_init_devices`）

---

## 6. 当前实现存在的问题与建议

### 6.1 线程安全 ⚠️ 高优先级

**问题：** `initialize_once()` 使用普通 `cl_bool` 标志，无任何同步机制。多线程同时首次调用会导致竞态条件（重复初始化、内存泄漏）。

**建议：** 使用 `pthread_once` 或 atomic flag + mutex 双重检查锁：

```c
static pthread_once_t g_init_once = PTHREAD_ONCE_INIT;

// 或
static atomic_int g_init_state = 0;
static pthread_mutex_t g_init_mutex = PTHREAD_MUTEX_INITIALIZER;
```

### 6.2 缺少 dfInit 调用 ⚠️ 高优先级

**问题：** 设计文档描述 Platform 初始化应调用 `dfInit()` 初始化底层运行时，但 `initialize_once()` 中并未调用。当前 `dfInit` 的调用可能散落在 Device 初始化路径中，缺乏统一入口。

**建议：** 在 `initialize_once()` 中显式调用 `dfInit(0)`，确保底层运行时在 Platform 就绪后立即可用：

```c
static cl_int initialize_once(void) {
    // ... existing code ...
    DFResult df_ret = dfInit(0);
    if (df_ret != DF_SUCCESS) return CL_INIT_FAIL;  // 需定义错误码映射
    // ...
}
```

### 6.3 clGetPlatformIDs 参数校验顺序 ⚠️ 中优先级

**问题：** 当前代码先写入 `platforms[0]`，再检查 `num_entries > 0 && platforms == NULL`。如果 `platforms == NULL`，已经发生了空指针解引用。

**建议：** 将 NULL 检查提前到写入之前：

```c
if (num_entries > 0 && platforms == NULL) {
    return CL_INVALID_VALUE;
}
if (platforms != NULL && num_entries > 0) {
    platforms[0] = thrivePlatform;
}
```

### 6.4 缺少 Platform 销毁逻辑 🔵 低优先级

**问题：** 没有 `clIcdDispatchTableDestroy` 的调用点，Platform 对象永不释放。对于常驻的 ICD Provider 这通常不是问题，但不利于内存检测工具（Valgrind/ASan）的使用。

**建议：** 可注册 `atexit` 清理函数，或在 ICD 卸载入口（如果存在）中释放资源。

### 6.5 缺少 OpenCL 3.0 新增查询项 🔵 低优先级

**问题：** OpenCL 3.0 新增了 `CL_PLATFORM_HOST_TIMER_RESOLUTION` 等查询项，当前 `clGetPlatformInfo` 未支持。

**建议：** 按需补充。对于嵌入式 Profile，可返回 0 或具体平台的 timer resolution。

### 6.6 Platform Extensions 与 Device Extensions 的一致性 🔵 建议

**问题：** Platform 声明支持 `cl_khr_fp64`，但 Device 查询 `CL_DEVICE_DOUBLE_FP_CONFIG` 返回 0（不支持双精度）。这会导致 CTS 测试失败。

**建议：** 确保 Platform 声明的扩展与 Device 实际能力一致。要么 Device 支持 fp64，要么从 Platform extensions 中移除 `cl_khr_fp64`。

### 6.7 错误码映射缺失 🔵 建议

**问题：** `dfInit` 返回 `DFResult`（DF_SUCCESS / DF_INIT_FAIL / DF_ERROR），但 OpenCL API 需要返回 `cl_int`（CL_SUCCESS / CL_OUT_OF_HOST_MEMORY 等）。当前缺少统一的错误码映射层。

**建议：** 增加 `df_result_to_cl()` 转换函数，集中管理 dfRuntime → OpenCL 的错误码映射。

---

## 7. 总结

当前 Platform 模块的核心骨架已经完整：ICD 注册、单 Platform 懒初始化、属性查询均已实现。主要待完善的点集中在：

| 优先级 | 项目 | 影响 |
|--------|------|------|
| 高 | 线程安全 | 多线程环境下的正确性 |
| 高 | dfInit 调用 | 底层运行时初始化入口 |
| 中 | 参数校验顺序 | clGetPlatformIDs 空指针安全 |
| 低 | 资源清理 | 内存检测友好性 |
| 低 | Extensions 一致性 | CTS 兼容性 |

整体架构清晰，模块职责分明，符合 OpenCL ICD 标准规范。
