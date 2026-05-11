#pragma once
#ifndef DFCL_DFRUNTIME_DEVICE_OPS_H_
#define DFCL_DFRUNTIME_DEVICE_OPS_H_

#include "../../cl_helper.h"     /* 改为 .h */

#ifdef __cplusplus
extern "C" {
#endif

int dfcl_dfruntime_probe(void);

cl_int dfcl_dfruntime_init(unsigned int dev_id, _cl_device_id *dev);


#ifdef __cplusplus
}
#endif

#endif /* DFCL_DFRUNTIME_DEVICE_OPS_H_ */