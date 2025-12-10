#pragma once

#ifndef _CL_DEVICE_HPP_
#define _CL_DEVICE_HPP_

#include <vector>
#include <shared_mutex>

#include "cl_icd_structs.hpp"
#include "cl_object.hpp"

#define CL_PLATFORM_ICD_SUFFIX_KHR                  0x0920

struct _cl_platform_id
{
    CL_OBJECT_BODY;
    const char *profile;
    const char *version;
    const char *name;
    const char *vendor;
    const char *extensions;
    const char *suffix;
};

enum OclExtensions {
  ClKhrFp64 = 0,
  ClAmdFp64,
  ClKhrSelectFpRoundingMode,
  ClKhrGlobalInt32BaseAtomics,
  ClKhrGlobalInt32ExtendedAtomics,
  ClKhrLocalInt32BaseAtomics,
  ClKhrLocalInt32ExtendedAtomics,
  ClKhrInt64BaseAtomics,
  ClKhrInt64ExtendedAtomics,
  ClKhr3DImageWrites,
  ClKhrByteAddressableStore,
  ClKhrFp16,
  ClKhrGlSharing,
  ClKhrGLDepthImages,
  ClExtDeviceFission,
  ClAmdDeviceAttributeQuery,
  ClAmdVec3,
  ClAmdPrintf,
  ClAmdMediaOps,
  ClAmdMediaOps2,
  ClAmdPopcnt,
  ClKhrImage2dFromBuffer,
  ClAMDBusAddressableMemory,
  ClAMDC11Atomics,
  ClKhrSpir,
  ClKhrSubGroups,
  ClKhrGlEvent,
  ClKhrDepthImages,
  ClKhrMipMapImage,
  ClKhrMipMapImageWrites,
  ClAmdCopyBufferP2P,
  ClAmdAssemblyProgram,
  ClExtTotal
};

static constexpr const char* OclExtensionsString[] = {"cl_khr_fp64 ",
                                            "cl_amd_fp64 ",
                                            "cl_khr_select_fprounding_mode ",
                                            "cl_khr_global_int32_base_atomics ",
                                            "cl_khr_global_int32_extended_atomics ",
                                            "cl_khr_local_int32_base_atomics ",
                                            "cl_khr_local_int32_extended_atomics ",
                                            "cl_khr_int64_base_atomics ",
                                            "cl_khr_int64_extended_atomics ",
                                            "cl_khr_3d_image_writes ",
                                            "cl_khr_byte_addressable_store ",
                                            "cl_khr_fp16 ",
                                            "cl_khr_gl_sharing ",
                                            "cl_khr_gl_depth_images ",
                                            "cl_ext_device_fission ",
                                            "cl_amd_device_attribute_query ",
                                            "cl_amd_vec3 ",
                                            "cl_amd_printf ",
                                            "cl_amd_media_ops ",
                                            "cl_amd_media_ops2 ",
                                            "cl_amd_popcnt ",
                                            "cl_khr_image2d_from_buffer ",
                                            "cl_amd_bus_addressable_memory ",
                                            "cl_amd_c11_atomics ",
                                            "cl_khr_spir ",
                                            "cl_khr_subgroups ",
                                            "cl_khr_gl_event ",
                                            "cl_khr_depth_images ",
                                            "cl_khr_mipmap_image ",
                                            "cl_khr_mipmap_image_writes ",
                                            "cl_amd_copy_buffer_p2p ",
                                            "cl_amd_assembly_program ",
                                            NULL};

class OpenclDevice : public OpenclObject {
public:
    using cl_type = cl_device_id;
    explicit OpenclDevice(DFDevice d, uint32_t id) : device_(d), devId_(id) {};

    DFDevice get() const { return device_; }
    
    uint32_t id() const { return devId_; }

    static char* getExtensionString();

    virtual ObjectType type() const override { return OBJECT_TYPE_DEVICE; }

private:
    DFDevice device_;
    uint32_t devId_;
};

inline OpenclDevice* as_internal(cl_device_id cl_handle) {
    if (!cl_handle) return nullptr;
    return ICDDispatchedObject::fromHandle<OpenclDevice>(cl_handle);
}

#endif /*_CL_DEVICE_HPP_*/