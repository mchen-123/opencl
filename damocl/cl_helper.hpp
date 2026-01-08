#ifndef _CL_HELPER_HPP_
#define _CL_HELPER_HPP_

#include <new>
#include <atomic>

#define DFCL_NEW(className) new (std::nothrow) className

#define DFCL_DELETE(ptr) (delete ptr)

#define DFCL_NEW_ARRAY(className, classCnt) (new (std::nothrow) className[classCnt])

#define DFCL_DELETE_ARRAY(ptr) (delete[] ptr)

inline int DFCL_ATOMIC_INC(std::atomic<int>& x) {
    return x.fetch_add(1, std::memory_order_seq_cst) + 1;
}

inline int DFCL_ATOMIC_DEC(std::atomic<int>& x) {
    return x.fetch_sub(1, std::memory_order_seq_cst) - 1;
}

#define DFCL_SET_FUNCTION_VALUE_RETURN(VALUE, ERROR, RET)                 \
    do                                                                  \
     {                                                                  \
        errcode = VALUE;                                                \
        if (ERROR)                                                      \
            *ERROR = errcode;                                           \
        return RET;                                                     \
     }                                                                  \
    while(0)

#define DFCL_SET_RETURN_VAL(param_value_size, param_value_ptr, param_value_size_ret, param_value_type, val, result)           \
    do                                                                                                                      \
     {                                                                                                                      \
        if (param_value_size < sizeof(param_value_type) && (param_value_size > 0))                                          \
            return CL_INVALID_VALUE;                                                                                        \
        if (param_value_ptr)                                                                                                \
            *(param_value_type*)(param_value_ptr) = val;                                                                    \
        if (param_value_size_ret)                                                                                           \
            *param_value_size_ret = sizeof(param_value_type);                                                               \
     }                                                                                                                      \
    while(0)

#define DFCL_SET_RETURN_PTR(param_value_size, param_value_ptr, param_value_size_ret, param_arr, param_arr_size, result)       \
    do                                                                                                                      \
     {                                                                                                                      \
        if (param_value_size < sizeof(param_arr_size) && (param_value_size > 0))                                            \
            result = CL_INVALID_VALUE;                                                                                      \
        if (param_value_ptr)                                                                                                \
            memcpy(param_value_ptr, param_arr, param_arr_size);                                                             \
        if (param_value_size_ret)                                                                                           \
            *param_value_size_ret = param_arr_size;                                                                         \
     }                                                                                                                      \
    while(0)

#define DFCL_MEM_FREE(PTR)                          \
    do                                              \
     {                                              \
        delete ((PTR));                             \
        (PTR) = nullptr;                            \
     }                                              \
    while (0)

// 宏定义（支持可变参数，方便以后打印 %s %d 等）
#ifndef DFCL_MSG_ERR
#define DFCL_MSG_ERR(...) do { \
    std::cerr << "DFCL ERROR: "; \
    std::cerr << __VA_ARGS__; \
    std::cerr << std::endl; \
} while(0)
#endif

#ifndef DFCL_MSG_INFO
#define DFCL_MSG_INFO(...) do { \
    std::cout << "DFCL INFO: "; \
    std::cout << __VA_ARGS__; \
    std::cout << std::endl; \
} while(0)
#endif

#define DFCL_GOTO_ERROR_ON(cond, err_code, ...)                             \
  do                                                                        \
    {                                                                       \
      if (cond)                                                             \
        {                                                                   \
            DFCL_MSG_ERR(__VA_ARGS__);                                      \
            errcode = err_code;                                             \
            goto ERROR;                                                     \
        }                                                                   \
    }                                                                       \
  while (0)

#define DFCL_GOTO_ERROR_COND(cond, err_code)                           \
  do                                                                   \
    {                                                                  \
      if (cond)                                                        \
        {                                                              \
          errcode = err_code;                                          \
          goto ERROR;                                                  \
        }                                                              \
    }                                                                  \
  while (0)

template <typename CL> bool is_valid(CL *handle) {
    return handle != nullptr;
};

#endif  // _CL_HELPER_HPP_