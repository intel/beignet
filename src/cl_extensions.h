#ifndef __CL_EXTENSIONS_H__
#define __CL_EXTENSIONS_H__
/* The following approved Khronos extension
 * names must be returned by all device that
 * support OpenCL C 1.2. */
#define DECL_BASE_EXTENSIONS \
  DECL_EXT(khr_global_int32_base_atomics) \
  DECL_EXT(khr_global_int32_extended_atomics) \
  DECL_EXT(khr_local_int32_base_atomics) \
  DECL_EXT(khr_local_int32_extended_atomics) \
  DECL_EXT(khr_byte_addressable_store) \
  DECL_EXT(khr_3d_image_writes)\
  DECL_EXT(khr_image2d_from_buffer)\
  DECL_EXT(khr_depth_images)\
  DECL_EXT(khr_fp64)

/* The OPT1 extensions are those optional extensions
 * which don't have external dependecies*/
#define DECL_OPT1_EXTENSIONS \
  DECL_EXT(khr_int64_base_atomics)\
  DECL_EXT(khr_int64_extended_atomics)\
  DECL_EXT(khr_fp16)\
  DECL_EXT(khr_initialize_memory)\
  DECL_EXT(khr_context_abort)\
  DECL_EXT(khr_spir) \
  DECL_EXT(khr_icd)

#define DECL_INTEL_EXTENSIONS \
  DECL_EXT(intel_accelerator) \
  DECL_EXT(intel_motion_estimation) \
  DECL_EXT(intel_subgroups) \
  DECL_EXT(intel_subgroups_short) \
  DECL_EXT(intel_required_subgroup_size) \
  DECL_EXT(intel_media_block_io) \
  DECL_EXT(intel_planar_yuv) \
  DECL_EXT(intel_device_side_avc_motion_estimation)

#define DECL_GL_EXTENSIONS \
  DECL_EXT(khr_gl_sharing)\
  DECL_EXT(khr_gl_event)\
  DECL_EXT(khr_gl_depth_images)\
  DECL_EXT(khr_gl_msaa_sharing)

#define DECL_D3D_EXTENSIONS \
  DECL_EXT(khr_d3d10_sharing)\
  DECL_EXT(khr_dx9_media_sharing)\
  DECL_EXT(khr_d3d11_sharing)\

#define DECL_ALL_EXTENSIONS \
  DECL_BASE_EXTENSIONS \
  DECL_OPT1_EXTENSIONS \
  DECL_INTEL_EXTENSIONS \
  DECL_GL_EXTENSIONS \
  DECL_D3D_EXTENSIONS

#define EXT_ID(name) cl_ ## name ## _ext_id
#define EXT_STRUCT_NAME(name) cl_ ## name ## ext
/*Declare enum ids */
typedef enum {
#define DECL_EXT(name) EXT_ID(name),
DECL_ALL_EXTENSIONS
#undef DECL_EXT
cl_khr_extension_id_max
}cl_extension_enum;

#define BASE_EXT_START_ID EXT_ID(khr_global_int32_base_atomics)
#define BASE_EXT_END_ID EXT_ID(khr_fp64)
#define OPT1_EXT_START_ID EXT_ID(khr_int64_base_atomics)
#define OPT1_EXT_END_ID EXT_ID(khr_icd)
#define INTEL_EXT_START_ID EXT_ID(intel_accelerator)
#define INTEL_EXT_END_ID EXT_ID(intel_device_side_avc_motion_estimation)
#define GL_EXT_START_ID EXT_ID(khr_gl_sharing)
#define GL_EXT_END_ID EXT_ID(khr_gl_msaa_sharing)

#define IS_BASE_EXTENSION(id)  (id >= BASE_EXT_START_ID && id <= BASE_EXT_END_ID)
#define IS_OPT1_EXTENSION(id)  (id >= OPT1_EXT_START_ID && id <= OPT1_EXT_END_ID)
#define IS_GL_EXTENSION(id)    (id >= GL_EXT_START_ID && id <= GL_EXT_END_ID)

struct cl_extension_base {
  cl_extension_enum ext_id;
  int  ext_enabled;
  char *ext_name;
};

/* Declare each extension structure. */
#define DECL_EXT(name) \
struct EXT_STRUCT_NAME(name) { \
  struct cl_extension_base base;\
};

DECL_BASE_EXTENSIONS
DECL_OPT1_EXTENSIONS
DECL_INTEL_EXTENSIONS
DECL_D3D_EXTENSIONS
DECL_GL_EXTENSIONS
#undef DECL_EXT

/* Union all extensions together. */
typedef union {
  struct cl_extension_base base;
  #define DECL_EXT(name) struct EXT_STRUCT_NAME(name) EXT_STRUCT_NAME(name);
  DECL_ALL_EXTENSIONS
  #undef DECL_EXT
} extension_union;

#include "cl_device_id.h"
typedef struct cl_extensions {
  extension_union extensions[cl_khr_extension_id_max];
  char ext_str[EXTENSTION_LENGTH];
} cl_extensions_t;

extern void
cl_intel_platform_extension_init(cl_platform_id intel_platform);
extern void
cl_intel_platform_enable_extension(cl_device_id device, uint32_t name);
extern void
cl_intel_platform_get_default_extension(cl_device_id device);
#endif /* __CL_EXTENSIONS_H__ */
