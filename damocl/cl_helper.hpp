#ifndef _CL_HELPER_HPP_
#define _CL_HELPER_HPP_

#include <new>

#define DF_NEW(className) new (std::nothrow) className

#define DF_DELETE(ptr) (delete ptr)

#define DF_NEW_ARRAY(className, classCnt) (new (std::nothrow) className[classCnt])

#define DF_DELETE_ARRAY(ptr) (delete[] ptr)

template <typename CL> bool is_valid(CL *handle) {
    return handle != nullptr;
};

#define CL_SET_FUNCTION_VALUE_RETURN(VALUE, ERROR, RET)                 \
    {                                                                   \
        result = VALUE;                                                 \
        if (ERROR)                                                      \
            *ERROR = result;                                            \
        return RET;                                                     \
    }

#define CL_SET_RETURN_VAL(param_value_size, param_value_ptr, param_value_size_ret, param_value_type, val, result)           \
    {                                                                                                                       \
        if (param_value_size < sizeof(param_value_type) && (param_value_size > 0))                                          \
            return CL_INVALID_VALUE;                                                                                        \
        if (param_value_ptr)                                                                                                \
            *(param_value_type*)(param_value_ptr) = val;                                                                    \
        if (param_value_size_ret)                                                                                           \
            *param_value_size_ret = sizeof(param_value_type);                                                               \
    }

#define CL_SET_RETURN_PTR(param_value_size, param_value_ptr, param_value_size_ret, param_arr, param_arr_size, result)       \
    {                                                                                                                       \
        if (param_value_size < sizeof(param_arr_size) && (param_value_size > 0))                                            \
            result = CL_INVALID_VALUE;                                                                                      \
        if (param_value_ptr)                                                                                                \
            memcpy(param_value_ptr, param_arr, param_arr_size);                                                             \
        if (param_value_size_ret)                                                                                           \
            *param_value_size_ret = param_arr_size;                                                                         \
    }

#endif  // _CL_HELPER_HPP_