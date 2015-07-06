#include "llvm/Config/llvm-config.h"
#ifdef HAS_EGL
#include "EGL/egl.h"
#include "EGL/eglext.h"
#endif

#include "cl_platform_id.h"
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
#if  LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 5
    if (id == EXT_ID(khr_spir))
      extensions->extensions[id].base.ext_enabled = 1;
#endif
  }
}

void
check_gl_extension(cl_extensions_t *extensions) {
#if defined(HAS_EGL)
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
  /* Should put those map/unmap extensions here. */
}

void
process_extension_str(cl_extensions_t *extensions)
{
  int str_max = sizeof(extensions->ext_str);
  int str_offset = 0;
  int id;

  extensions->ext_str[str_max-1] = '\0';

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
cl_intel_platform_enable_fp16_extension(cl_platform_id intel_platform)
{
  cl_extensions_t *extensions = &intel_platform_extensions;
  int id;

  for(id = OPT1_EXT_START_ID; id <= OPT1_EXT_END_ID; id++)
  {
    if (id == EXT_ID(khr_fp16))
      extensions->extensions[id].base.ext_enabled = 1;
  }

  process_extension_str(extensions);
  intel_platform->internal_extensions = &intel_platform_extensions;
  intel_platform->extensions = intel_platform_extensions.ext_str;
  intel_platform->extensions_sz = strlen(intel_platform->extensions) + 1;
}

LOCAL void
cl_intel_platform_extension_init(cl_platform_id intel_platform)
{
  static int ext_initialized = 0;

  /* The EXT should be only inited once. */
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
