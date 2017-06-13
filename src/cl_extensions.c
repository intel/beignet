#include "llvm/Config/llvm-config.h"
#ifdef HAS_GL_EGL
#include "EGL/egl.h"
#include "EGL/eglext.h"
#endif

#include "cl_platform_id.h"
#include "cl_device_id.h"
#include "cl_internals.h"
#include "CL/cl.h"
#include "cl_utils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* This extension should be common for all the intel GPU platform.
   Every device may have its own additional externsions. */
static struct cl_extensions intel_platform_extensions =
{
  {
#define DECL_EXT(name) \
  {(struct cl_extension_base){.ext_id = cl_##name##_ext_id, .ext_name = "cl_" #name, .ext_enabled = 0}},
  DECL_ALL_EXTENSIONS
  },
#undef DECL_EXT
  {""}
};

void check_basic_extension(cl_extensions_t *extensions)
{
  int id;
  for(id = BASE_EXT_START_ID; id <= BASE_EXT_END_ID; id++)
    if (id != EXT_ID(khr_fp64))
      extensions->extensions[id].base.ext_enabled = 1;
}

void check_opt1_extension(cl_extensions_t *extensions)
{
  int id;
  for(id = OPT1_EXT_START_ID; id <= OPT1_EXT_END_ID; id++)
  {
    if (id == EXT_ID(khr_icd))
      extensions->extensions[id].base.ext_enabled = 1;
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 35
    if (id == EXT_ID(khr_spir))
      extensions->extensions[id].base.ext_enabled = 1;
#endif
    if (id == EXT_ID(khr_image2d_from_buffer))
      extensions->extensions[id].base.ext_enabled = 1;
    if (id == EXT_ID(khr_3d_image_writes))
      extensions->extensions[id].base.ext_enabled = 1;
  }
}

void
check_gl_extension(cl_extensions_t *extensions) {
#if defined(HAS_GL_EGL)
  int id;
      /* For now, we only support cl_khr_gl_sharing. */
  for(id = GL_EXT_START_ID; id <= GL_EXT_END_ID; id++)
    if (id == EXT_ID(khr_gl_sharing))
      extensions->extensions[id].base.ext_enabled = 1;
#endif
}

void
check_intel_extension(cl_extensions_t *extensions)
{
  int id;
  for(id = INTEL_EXT_START_ID; id <= INTEL_EXT_END_ID; id++)
  {
    if(id != EXT_ID(intel_motion_estimation) && id != EXT_ID(intel_device_side_avc_motion_estimation))
      extensions->extensions[id].base.ext_enabled = 1;
    if(id == EXT_ID(intel_required_subgroup_size))
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR > 40
      extensions->extensions[id].base.ext_enabled = 1;
#else
      extensions->extensions[id].base.ext_enabled = 0;
#endif
  }
}

void
process_extension_str(cl_extensions_t *extensions)
{
  int str_max = sizeof(extensions->ext_str);
  int str_offset = 0;
  int id;

  memset(extensions->ext_str, 0, sizeof(extensions->ext_str));

  for(id = 0; id < cl_khr_extension_id_max; id++)
  {
    if (extensions->extensions[id].base.ext_enabled) {
      int copy_len;
      char *ext_name = extensions->extensions[id].base.ext_name;
      if (str_offset + 1 >= str_max)
        return;

      if (str_offset != 0)
        extensions->ext_str[str_offset - 1] = ' ';
      copy_len = (strlen(ext_name) + 1 + str_offset) < str_max
                 ? (strlen(ext_name) + 1) : (str_max - str_offset - 1);
      strncpy(&extensions->ext_str[str_offset],
              extensions->extensions[id].base.ext_name, copy_len);
      str_offset += copy_len;
    }
  }
}

LOCAL void
cl_intel_platform_get_default_extension(cl_device_id device)
{
  cl_platform_id pf = device->platform;
  memcpy((char*)device->extensions,
       pf->internal_extensions->ext_str, sizeof(device->extensions));
  device->extensions_sz = strlen(pf->internal_extensions->ext_str) + 1;
}

LOCAL void
cl_intel_platform_enable_extension(cl_device_id device, uint32_t ext)
{
  int id;
  char* ext_str = NULL;
  cl_platform_id pf = device->platform;
  assert(pf);

  for(id = BASE_EXT_START_ID; id < cl_khr_extension_id_max; id++) {
    if (id == ext) {
      if (!pf->internal_extensions->extensions[id].base.ext_enabled)
        ext_str = pf->internal_extensions->extensions[id].base.ext_name;

      break;
    }
  }

  /* already enabled, skip. */
  if (ext_str && strstr(device->extensions, ext_str))
    ext_str = NULL;

  if (ext_str) {
    if (device->extensions_sz <= 1) {
      memcpy((char*)device->extensions, ext_str, strlen(ext_str));
      device->extensions_sz = strlen(ext_str) + 1;
    } else {
      assert(device->extensions_sz + 1 + strlen(ext_str) < EXTENSTION_LENGTH);
      *(char*)(device->extensions + device->extensions_sz - 1) = ' ';
      memcpy((char*)device->extensions + device->extensions_sz, ext_str, strlen(ext_str));
      device->extensions_sz = device->extensions_sz + strlen(ext_str) + 1;
    }

    *(char*)(device->extensions + device->extensions_sz - 1) = 0;
  }
}

LOCAL void
cl_intel_platform_extension_init(cl_platform_id intel_platform)
{
  static int ext_initialized = 0;

  /* The EXT should be only inited once. */
  (void) ext_initialized;
  assert(!ext_initialized);
  check_basic_extension(&intel_platform_extensions);
  check_opt1_extension(&intel_platform_extensions);
  check_gl_extension(&intel_platform_extensions);
  check_intel_extension(&intel_platform_extensions);
  process_extension_str(&intel_platform_extensions);
  ext_initialized = 1;

  intel_platform->internal_extensions = &intel_platform_extensions;
  intel_platform->extensions = intel_platform_extensions.ext_str;
  intel_platform->extensions_sz = strlen(intel_platform->extensions) + 1;
  return;
}
