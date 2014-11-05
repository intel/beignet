/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "utest_file_map.hpp"
#include "utest_helper.hpp"
#include "utest_error.h"
#include "CL/cl.h"
#include "CL/cl_intel.h"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <cmath>

#define FATAL(...) \
do { \
  fprintf(stderr, "error: "); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n");\
  assert(0); \
  exit(-1); \
} while (0)

#define FATAL_IF(COND, ...) \
do { \
  if (COND) FATAL(__VA_ARGS__); \
} while (0)

cl_platform_id platform = NULL;
cl_device_id device = NULL;
cl_context ctx = NULL;
cl_program program = NULL;
cl_kernel kernel = NULL;
cl_command_queue queue = NULL;
cl_mem buf[MAX_BUFFER_N] = {};
void *buf_data[MAX_BUFFER_N] = {};
size_t globals[3] = {};
size_t locals[3] = {};

#ifdef HAS_EGL
Display    *xDisplay;
EGLDisplay  eglDisplay;
EGLContext  eglContext = NULL;
EGLSurface  eglSurface;
Window xWindow;

void cl_ocl_destroy_egl_window() {
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
    XDestroyWindow(xDisplay, xWindow);
    XCloseDisplay(xDisplay);
}

bool init_egl_window(int width, int height) {
    XSetWindowAttributes swa;
    Window      win, root;
    EGLint attr[] = {       // some attributes to set up our egl-interface
    EGL_BUFFER_SIZE, 16,
    EGL_RENDERABLE_TYPE,
    EGL_OPENGL_BIT,
    EGL_NONE
    };
    //// egl-contexts collect all state descriptions needed required for operation
    EGLint ctxattr[] = {
            #if 0
            EGL_CONTEXT_CLIENT_VERSION, 2,
            #endif
            EGL_NONE
    };

    EGLConfig  ecfg;
    EGLint     numConfig;

    eglContext = EGL_NO_CONTEXT;
    xDisplay = XOpenDisplay(NULL);
    if (xDisplay == NULL) {
      fprintf(stderr, "Failed to open DISPLAY.\n");
      return false;
    }
    root = DefaultRootWindow(xDisplay);
    swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;

    win = XCreateWindow(
                    xDisplay, root, 0, 0, width, height, 0,
                    CopyFromParent, InputOutput,
                    CopyFromParent, CWEventMask,
                    &swa);
    xWindow = win;

    ///////  the egl part  //////////////////////////////////////////////////////////////////
    //  egl provides an interface to connect the graphics related functionality of openGL ES
    //  with the windowing interface and functionality of the native operation system (X11
    //  in our case.

    eglDisplay  =  eglGetDisplay( (EGLNativeDisplayType) xDisplay );
    if ( eglDisplay == EGL_NO_DISPLAY ) {
            fprintf(stderr, "Got no EGL display.\n");
            return false;
    }
    eglBindAPI(EGL_OPENGL_API);
    int m,n;
    if ( !eglInitialize( eglDisplay, &m, &n ) ) {
      fprintf(stderr, "Unable to initialize EGL\n");
      return false;
    }
    if ( !eglChooseConfig( eglDisplay, attr, &ecfg, 1, &numConfig ) ) {
      fprintf(stderr, "Failed to choose config (eglError: %d)\n", eglGetError());
      return false;
    }
    if ( numConfig != 1 ) {
      fprintf(stderr, "Didn't get exactly one config, but %d", numConfig);
      return false;
    }
    eglSurface = eglCreateWindowSurface ( eglDisplay, ecfg, win, NULL );
    if ( eglSurface == EGL_NO_SURFACE ) {
      fprintf(stderr, "Unable to create EGL surface (eglError: %d)\n", eglGetError());
      return false;
    }
    eglContext = eglCreateContext ( eglDisplay, ecfg, EGL_NO_CONTEXT, ctxattr );
    if ( eglContext == EGL_NO_CONTEXT ) {
      fprintf(stderr, "Unable to create EGL context (eglError: %d)\n", eglGetError());
      return false;
    }
    //// associate the egl-context with the egl-surface
    eglMakeCurrent( eglDisplay, eglSurface, eglSurface, eglContext);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFinish();
    eglSwapBuffers(eglDisplay, eglSurface);
    return true;
}
#endif

static const char*
cl_test_channel_order_string(cl_channel_order order)
{
  switch(order) {
#define DECL_ORDER(WHICH) case CL_##WHICH: return "CL_"#WHICH
    DECL_ORDER(R);
    DECL_ORDER(A);
    DECL_ORDER(RG);
    DECL_ORDER(RA);
    DECL_ORDER(RGB);
    DECL_ORDER(RGBA);
    DECL_ORDER(BGRA);
    DECL_ORDER(ARGB);
    DECL_ORDER(INTENSITY);
    DECL_ORDER(LUMINANCE);
    DECL_ORDER(Rx);
    DECL_ORDER(RGx);
    DECL_ORDER(RGBx);
#undef DECL_ORDER
    default: return "Unsupported image channel order";
  };
}

static const char*
cl_test_channel_type_string(cl_channel_type type)
{
  switch(type) {
#define DECL_TYPE(WHICH) case CL_##WHICH: return "CL_"#WHICH
    DECL_TYPE(SNORM_INT8);
    DECL_TYPE(SNORM_INT16);
    DECL_TYPE(UNORM_INT8);
    DECL_TYPE(UNORM_INT16);
    DECL_TYPE(UNORM_SHORT_565);
    DECL_TYPE(UNORM_SHORT_555);
    DECL_TYPE(UNORM_INT_101010);
    DECL_TYPE(SIGNED_INT8);
    DECL_TYPE(SIGNED_INT16);
    DECL_TYPE(SIGNED_INT32);
    DECL_TYPE(UNSIGNED_INT8);
    DECL_TYPE(UNSIGNED_INT16);
    DECL_TYPE(UNSIGNED_INT32);
    DECL_TYPE(HALF_FLOAT);
    DECL_TYPE(FLOAT);
#undef DECL_TYPE
    default: return "Unsupported image channel type";
  };
}

static void
clpanic(const char *msg, int rval)
{
  printf("Failed: %s (%d)\n", msg, rval);
  exit(-1);
}

char*
cl_do_kiss_path(const char *file, cl_device_id device)
{
  cl_int ver;
  const char *sub_path = NULL;
  char *ker_path = NULL;
  const char *kiss_path = getenv("OCL_KERNEL_PATH");
  size_t sz = strlen(file);

  if (device == NULL)
    sub_path = "";
  else {
    if (clGetGenVersionIntel(device, &ver) != CL_SUCCESS)
      clpanic("Unable to get Gen version", -1);
    sub_path = "";
  }

  if (kiss_path == NULL)
    clpanic("set OCL_KERNEL_PATH. This is where the kiss kernels are", -1);
  sz += strlen(kiss_path) + strlen(sub_path) + 2; /* +1 for end of string, +1 for '/' */
  if ((ker_path = (char*) malloc(sz)) == NULL)
    clpanic("Allocation failed", -1);
  sprintf(ker_path, "%s/%s%s", kiss_path, sub_path, file);
  return ker_path;
}

int
cl_kernel_init(const char *file_name, const char *kernel_name, int format, const char * build_opt)
{
  cl_file_map_t *fm = NULL;
  char *ker_path = NULL;
  cl_int status = CL_SUCCESS;
  static const char *prevFileName = NULL;

  /* Load the program and build it */
  if (!program || (program && (!prevFileName || strcmp(prevFileName, file_name)))) {
    if (program) clReleaseProgram(program);
    ker_path = cl_do_kiss_path(file_name, device);
    if (format == LLVM)
      program = clCreateProgramWithLLVMIntel(ctx, 1, &device, ker_path, &status);
    else if (format == SOURCE) {
      cl_file_map_t *fm = cl_file_map_new();
      FATAL_IF (cl_file_map_open(fm, ker_path) != CL_FILE_MAP_SUCCESS,
                "Failed to open file \"%s\" with kernel \"%s\". Did you properly set OCL_KERNEL_PATH variable?",
                file_name, kernel_name);
      const char *src = cl_file_map_begin(fm);
      const size_t sz = cl_file_map_size(fm);
      program = clCreateProgramWithSource(ctx, 1, &src, &sz, &status);
      cl_file_map_delete(fm);
    } else
      FATAL("Not able to create program from binary");

    if (status != CL_SUCCESS) {
      fprintf(stderr, "error calling clCreateProgramWithBinary\n");
      goto error;
    }
    prevFileName = file_name;

    /* OCL requires to build the program even if it is created from a binary */
    OCL_CALL (clBuildProgram, program, 1, &device, build_opt, NULL, NULL);
  }

  /* Create a kernel from the program */
  if (kernel)
    clReleaseKernel(kernel);
  kernel = clCreateKernel(program, kernel_name, &status);
  if (status != CL_SUCCESS) {
    fprintf(stderr, "error calling clCreateKernel\n");
    goto error;
  }

exit:
  free(ker_path);
  cl_file_map_delete(fm);
  return status;
error:
  prevFileName = NULL;
  goto exit;
}

#define GET_PLATFORM_STR_INFO(LOWER_NAME, NAME) \
  { \
    size_t param_value_size; \
    OCL_CALL (clGetPlatformInfo, platform, CL_PLATFORM_##NAME, 0, 0, &param_value_size); \
    std::vector<char> param_value(param_value_size); \
    OCL_CALL (clGetPlatformInfo, platform, CL_PLATFORM_##NAME, \
              param_value_size, param_value.empty() ? NULL : &param_value.front(), \
              &param_value_size); \
    std::string str; \
    if (!param_value.empty()) \
      str = std::string(&param_value.front(), param_value_size-1); \
    printf("platform_" #LOWER_NAME " \"%s\"\n", str.c_str()); \
  }

#include <cstring>
#define GET_DEVICE_STR_INFO(LOWER_NAME, NAME) \
    std::string LOWER_NAME ##Str; \
    OCL_CALL (clGetDeviceInfo, device, CL_DEVICE_##NAME, 0, 0, &param_value_size); \
    { \
      std::vector<char> param_value(param_value_size); \
      OCL_CALL (clGetDeviceInfo, device, CL_DEVICE_##NAME, \
                param_value_size, param_value.empty() ? NULL : &param_value.front(), \
                &param_value_size); \
      if (!param_value.empty()) \
        LOWER_NAME ##Str = std::string(&param_value.front(), param_value_size-1); \
    } \
    printf("device_" #LOWER_NAME " \"%s\"\n", LOWER_NAME ##Str.c_str());

int
cl_ocl_init(void)
{
  cl_int status = CL_SUCCESS;
  cl_uint platform_n;
  size_t i;
#ifdef HAS_EGL
  bool hasGLExt = false;
#endif
  cl_context_properties *props = NULL;

  /* Get the platform number */
  OCL_CALL (clGetPlatformIDs, 0, NULL, &platform_n);
  printf("platform number %u\n", platform_n);
  assert(platform_n >= 1);

  /* Get a valid platform */
  OCL_CALL (clGetPlatformIDs, 1, &platform, &platform_n);
  GET_PLATFORM_STR_INFO(profile, PROFILE);
  GET_PLATFORM_STR_INFO(name, NAME);
  GET_PLATFORM_STR_INFO(vendor, VENDOR);
  GET_PLATFORM_STR_INFO(version, VERSION);
  GET_PLATFORM_STR_INFO(extensions, EXTENSIONS);

  /* Get the device (only GPU device is supported right now) */
  try {
    OCL_CALL (clGetDeviceIDs, platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    {
      size_t param_value_size;
      GET_DEVICE_STR_INFO(profile, PROFILE);
      GET_DEVICE_STR_INFO(name, NAME);
      GET_DEVICE_STR_INFO(vendor, VENDOR);
      GET_DEVICE_STR_INFO(version, VERSION);
      GET_DEVICE_STR_INFO(extensions, EXTENSIONS);
      GET_DEVICE_STR_INFO(opencl_c_version, OPENCL_C_VERSION);
#ifdef HAS_EGL
      if (std::strstr(extensionsStr.c_str(), "cl_khr_gl_sharing")) {
        hasGLExt = true;
      }
#endif
    }
  } catch (...) {
     fprintf(stderr, "error calling clGetDeviceIDs\n");
     status = CL_DEVICE_NOT_FOUND;
     goto error;
  }

#ifdef HAS_EGL
  if (hasGLExt) {
    int i = 0;
    props = new cl_context_properties[7];
    props[i++] = CL_CONTEXT_PLATFORM;
    props[i++] = (cl_context_properties)platform;
    if (init_egl_window(EGL_WINDOW_WIDTH, EGL_WINDOW_HEIGHT)) {
      props[i++] = CL_EGL_DISPLAY_KHR;
      props[i++] = (cl_context_properties)eglGetCurrentDisplay();
      props[i++] = CL_GL_CONTEXT_KHR;
      props[i++] = (cl_context_properties)eglGetCurrentContext();
    }
    props[i++] = 0;
  }
#endif
  /* Now create a context */
  ctx = clCreateContext(props, 1, &device, NULL, NULL, &status);
  if (status != CL_SUCCESS) {
    fprintf(stderr, "error calling clCreateContext\n");
    goto error;
  }

  /* All image types currently supported by the context */
  cl_image_format fmt[256];
  cl_uint fmt_n;
  clGetSupportedImageFormats(ctx, 0, CL_MEM_OBJECT_IMAGE2D, 256, fmt, &fmt_n);
  printf("%u image formats are supported\n", fmt_n);
  for (i = 0; i < fmt_n; ++i)
    printf("[%s %s]\n",
        cl_test_channel_order_string(fmt[i].image_channel_order),
        cl_test_channel_type_string(fmt[i].image_channel_data_type));

  /* We are going to push NDRange kernels here */
  queue = clCreateCommandQueue(ctx, device, 0, &status);
  if (status != CL_SUCCESS) {
    fprintf(stderr, "error calling clCreateCommandQueue\n");
    goto error;
  }

error:
  if (props)
    delete props;
  return status;
}

int
cl_test_init(const char *file_name, const char *kernel_name, int format)
{
  cl_int status = CL_SUCCESS;

  /* Initialize OCL */
  if ((status = cl_ocl_init()) != CL_SUCCESS)
    goto error;

  /* Load the kernel */
  if ((status = cl_kernel_init(file_name, kernel_name, format, NULL)) != CL_SUCCESS)
    goto error;

error:
  return status;
}

void
cl_kernel_destroy(bool needDestroyProgram)
{
  if (kernel) {
    clReleaseKernel(kernel);
    kernel = NULL;
  }
  if (needDestroyProgram && program) {
    clReleaseProgram(program);
    program = NULL;
  }
}

void
cl_ocl_destroy(void)
{
  clReleaseCommandQueue(queue);
  clReleaseContext(ctx);
#ifdef HAS_EGL
  if (eglContext != NULL) {
    cl_ocl_destroy_egl_window();
    eglContext = NULL;
  }
#endif
}

void
cl_test_destroy(void)
{
  cl_kernel_destroy();
  cl_ocl_destroy();
  printf("%i memory leaks\n", clReportUnfreedIntel());
  assert(clReportUnfreedIntel() == 0);
}

void
cl_buffer_destroy(void)
{
  int i;
  for (i = 0; i < MAX_BUFFER_N; ++i) {
    if (buf_data[i] != NULL) {
      clUnmapBufferIntel(buf[i]);
      buf_data[i] = NULL;
    }
    if (buf[i] != NULL) {
      clReleaseMemObject(buf[i]);
      buf[i] = NULL;
    }
  }
}

void
cl_report_perf_counters(cl_mem perf)
{
  cl_int status = CL_SUCCESS;
  uint32_t *start = NULL, *end = NULL;
  uint32_t i;
  if (perf == NULL)
    return;
  start = (uint32_t*) clMapBufferIntel(perf, &status);
  assert(status == CL_SUCCESS && start != NULL);
  end = start + 128;

  printf("BEFORE\n");
  for (i = 0; i < 6*8; ++i) {
    if (i % 8 == 0) printf("\n");
    printf("[%3u 0x%8x] ", i, start[i]);
  }
  printf("\n\n");

  printf("AFTER\n");
  for (i = 0; i < 6*8; ++i) {
    if (i % 8 == 0) printf("\n");
    printf("[%3u 0x%8x] ", i, end[i]);
  }
  printf("\n\n");

  printf("DIFF\n");
  for (i = 0; i < 6*8; ++i) {
    if (i % 8 == 0) printf("\n");
    printf("[%3u %8i] ", i, end[i] - start[i]);
  }
  printf("\n\n");

  clUnmapBufferIntel(perf);
}

struct bmphdr {
  //   2 bytes of magic here, "BM", total header size is 54 bytes!
  int filesize;		//   4 total file size incl header
  short as0, as1;		//   8 app specific
  int bmpoffset;		//  12 ofset of bmp data 
  int headerbytes;	//  16 bytes in header from this point (40 actually)
  int width;		//  20 
  int height;		//  24 
  short nplanes;		//  26 no of color planes
  short bpp;		//  28 bits/pixel
  int compression;	//  32 BI_RGB = 0 = no compression
  int sizeraw;		//  36 size of raw bmp file, excluding header, incl padding
  int hres;		//  40 horz resolutions pixels/meter
  int vres;		//  44
  int npalcolors;		//  48 No of colors in palette
  int nimportant;		//  52 No of important colors
  // raw b, g, r data here, dword aligned per scan line
};

int *cl_read_bmp(const char *filename, int *width, int *height)
{
  struct bmphdr hdr;
  char *bmppath = cl_do_kiss_path(filename, device);
  FILE *fp = fopen(bmppath, "rb");
  assert(fp);

  char magic[2];
  int ret;
  ret = fread(&magic[0], 1, 2, fp);
  if(2 != ret){
    fclose(fp);
    free(bmppath);
    return NULL;
  }
  assert(magic[0] == 'B' && magic[1] == 'M');

  ret = fread(&hdr, sizeof(hdr), 1, fp);
  if(1 != ret){
    fclose(fp);
    free(bmppath);
    return NULL;
  }

  assert(hdr.width > 0 && hdr.height > 0 && hdr.nplanes == 1 && hdr.compression == 0);

  int *rgb32 = (int *) malloc(hdr.width * hdr.height * sizeof(int));
  assert(rgb32);
  int x, y;

  int *dst = rgb32;
  for (y = 0; y < hdr.height; y++) {
    for (x = 0; x < hdr.width; x++) {
      assert(!feof(fp));
      int b = (getc(fp) & 0x0ff);
      int g = (getc(fp) & 0x0ff);
      int r = (getc(fp) & 0x0ff);
      *dst++ = (r | (g << 8) | (b << 16) | 0xff000000);	/* abgr */
    }
    while (x & 3) {
      getc(fp);
      x++;
    }		// each scanline padded to dword
    // printf("read row %d\n", y);
    // fflush(stdout);
  }
  fclose(fp);
  *width = hdr.width;
  *height = hdr.height;
  free(bmppath);
  return rgb32;
}

void cl_write_bmp(const int *data, int width, int height, const char *filename)
{
  int x, y;

  FILE *fp = fopen(filename, "wb");
  assert(fp);

  char *raw = (char *) malloc(width * height * sizeof(int));	// at most
  assert(raw);
  char *p = raw;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      int c = *data++;
      *p++ = ((c >> 16) & 0xff);
      *p++ = ((c >> 8) & 0xff);
      *p++ = ((c >> 0) & 0xff);
    }
    while (x & 3) {
      *p++ = 0;
      x++;
    } // pad to dword
  }
  int sizeraw = p - raw;
  int scanline = (width * 3 + 3) & ~3;
  assert(sizeraw == scanline * height);

  struct bmphdr hdr;

  hdr.filesize = scanline * height + sizeof(hdr) + 2;
  hdr.as0 = 0;
  hdr.as1 = 0;
  hdr.bmpoffset = sizeof(hdr) + 2;
  hdr.headerbytes = 40;
  hdr.width = width;
  hdr.height = height;
  hdr.nplanes = 1;
  hdr.bpp = 24;
  hdr.compression = 0;
  hdr.sizeraw = sizeraw;
  hdr.hres = 0;		// 2834;
  hdr.vres = 0;		// 2834;
  hdr.npalcolors = 0;
  hdr.nimportant = 0;

  /* Now write bmp file */
  char magic[2] = { 'B', 'M' };
  fwrite(&magic[0], 1, 2, fp);
  fwrite(&hdr, 1, sizeof(hdr), fp);
  fwrite(raw, 1, hdr.sizeraw, fp);

  fclose(fp);
  free(raw);
}

static const float pixel_threshold = 0.05f;
static const float max_error_ratio = 0.001f;

int cl_check_image(const int *img, int w, int h, const char *bmp)
{
  int refw, refh;
  int *ref = cl_read_bmp(bmp, &refw, &refh);
  if (ref == NULL || refw != w || refh != h) return 0;
  const int n = w*h;
  int discrepancy = 0;
  for (int i = 0; i < n; ++i) {
    const float r = (float) (img[i] & 0xff);
    const float g = (float) ((img[i] >> 8) & 0xff);
    const float b = (float) ((img[i] >> 16) & 0xff);
    const float rr = (float) (ref[i] & 0xff);
    const float rg = (float) ((ref[i] >> 8) & 0xff);
    const float rb = (float) ((ref[i] >> 16) & 0xff);
    const float dr = fabs(r-rr) / (1.f/255.f + std::max(r,rr));
    const float dg = fabs(g-rg) / (1.f/255.f + std::max(g,rg));
    const float db = fabs(b-rb) / (1.f/255.f + std::max(b,rb));
    const float err = sqrtf(dr*dr+dg*dg+db*db);
    if (err > pixel_threshold) discrepancy++;
  }
  free(ref);
  return (float(discrepancy) / float(n) > max_error_ratio) ? 0 : 1;
}

float cl_FLT_ULP(float float_number)
{
  SF floatBin, ulpBin, ulpBinBase;
  floatBin.f = float_number;

  ulpBin.spliter.sign     = ulpBinBase.spliter.sign     = 0;
  ulpBin.spliter.exponent = ulpBinBase.spliter.exponent = floatBin.spliter.exponent;
  ulpBin.spliter.mantissa = 0x1;
  ulpBinBase.spliter.mantissa = 0x0;
  
  return ulpBin.f - ulpBinBase.f;
}

int cl_INT_ULP(int int_number)
{
  return 0;
}
