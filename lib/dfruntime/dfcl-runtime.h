#pragma once
#ifndef DFCL_DFRUNTIME_DEVICE_OPS_H_
#define DFCL_DFRUNTIME_DEVICE_OPS_H_

#include "../../cl_helper.h"     /* 改为 .h */

#ifdef __cplusplus
extern "C" {
#endif

void dfcl_dfruntime_init_device_ops(struct dfcl_device_ops *ops);

#ifdef __cplusplus
}
#endif

#endif /* DFCL_DFRUNTIME_DEVICE_OPS_H_ */