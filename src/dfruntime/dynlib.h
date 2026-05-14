#ifndef _DYNLIB_H_
#define _DYNLIB_H_

#include <dlfcn.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Open a dynamic library with specified loading behavior.
 *
 * @param[in] path   Path to the shared library (.so / .dll)
 * @param[in] lazy   If true, use RTLD_LAZY (resolve symbols on first use);
 *                   otherwise use RTLD_NOW (resolve all symbols immediately)
 * @param[in] local  If true, use RTLD_LOCAL (symbols not available to other libraries);
 *                   otherwise use RTLD_GLOBAL (symbols available globally)
 *
 * @return
 * - Valid handle to the loaded library on success.
 * - NULL on failure (error message printed to stderr).
 *
 * @note This is a wrapper around `dlopen()`.
 */
void *dfcl_dynlib_open(const char *path, int lazy, int local);

/**
 * @brief Get the address of a symbol from a dynamically loaded library.
 *
 * @param[in] dynlib_handle  Handle returned by dfcl_dynlib_open()
 * @param[in] symbol_name    Name of the symbol to resolve
 *
 * @return
 * - Pointer to the symbol on success.
 * - NULL on failure (error message printed to stderr).
 *
 * @note This is a wrapper around `dlsym()`.
 */
void *dfcl_dynlib_symbol_address(void *dynlib_handle, const char *symbol_name);

#ifdef __cplusplus
}
#endif

#endif /* _DYNLIB_H_ */
