#include "dfcl_config.hpp"

int
dfcl_get_bool_option(const char *key, int default_value) {
    const char *value = std::getenv(key);
    if (value != nullptr) {
        return strncmp(value, "1", 1) == 0;
    }
    return default_value;
}