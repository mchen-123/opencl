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