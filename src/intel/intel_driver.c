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

/*
 * Copyright 2009 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Xiang Haihao <haihao.xiang@intel.com>
 *    Zou Nan hai <nanhai.zou@intel.com>
 *
 */

#if defined(HAS_GL_EGL)
#define EGL_EGLEXT_PROTOTYPES
#include "GL/gl.h"
#include "EGL/egl.h"
#include <EGL/eglext.h>
#endif

#ifdef HAS_X11
#include <X11/Xlibint.h>
#include "x11/dricommon.h"
#endif

#include "intel_driver.h"
#include "intel_gpgpu.h"
#include "intel_batchbuffer.h"
#include "intel_bufmgr.h"
#include "cl_mem.h"

#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <xf86drm.h>
#include <stdio.h>

#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_context.h"
#include "cl_driver.h"
#include "cl_device_id.h"
#include "cl_platform_id.h"

static void
intel_driver_delete(intel_driver_t *driver)
{
  if (driver == NULL)
    return;

  cl_free(driver);
}

static intel_driver_t*
intel_driver_new(void)
{
  intel_driver_t *driver = NULL;

  TRY_ALLOC_NO_ERR (driver, CALLOC(intel_driver_t));
  driver->fd = -1;

exit:
  return driver;
error:
intel_driver_delete(driver);
driver = NULL;
goto exit;
}

/* just used for maximum relocation number in drm_intel */
#define BATCH_SIZE 0x4000

/* set OCL_DUMP_AUB=1 to get aub file */
static void
intel_driver_aub_dump(intel_driver_t *driver)
{
char *val;
val = getenv("OCL_DUMP_AUB");
if (!val)
  return;
if (atoi(val) != 0) {
  drm_intel_bufmgr_gem_set_aub_filename(driver->bufmgr,
          "beignet.aub");
  drm_intel_bufmgr_gem_set_aub_dump(driver->bufmgr, 1);
}
}

static int
intel_driver_memman_init(intel_driver_t *driver)
{
driver->bufmgr = drm_intel_bufmgr_gem_init(driver->fd, BATCH_SIZE);
if (!driver->bufmgr) return 0;
drm_intel_bufmgr_gem_enable_reuse(driver->bufmgr);
driver->device_id = drm_intel_bufmgr_gem_get_devid(driver->bufmgr);
intel_driver_aub_dump(driver);
return 1;
}

static int
intel_driver_context_init(intel_driver_t *driver)
{
  driver->ctx = drm_intel_gem_context_create(driver->bufmgr);
  if (!driver->ctx)
    return 0;
  driver->null_bo = NULL;
#ifdef HAS_BO_SET_SOFTPIN
  drm_intel_bo *bo = dri_bo_alloc(driver->bufmgr, "null_bo", 64*1024, 4096);
  drm_intel_bo_set_softpin_offset(bo, 0);
  // don't reuse it, that would make two bo trying to bind to same address,
  // which is un-reasonable.
  drm_intel_bo_disable_reuse(bo);

  drm_intel_bo_map(bo, 1);
  *(uint32_t *)bo->virtual = MI_BATCH_BUFFER_END;
  drm_intel_bo_unmap(bo);

  if (drm_intel_gem_bo_context_exec(bo, driver->ctx, 0, 0) == 0) {
    driver->null_bo = bo;
  } else {
    drm_intel_bo_unreference(bo);
  }
#endif
    return 1;
}

static void
intel_driver_context_destroy(intel_driver_t *driver)
{
if (driver->null_bo)
  drm_intel_bo_unreference(driver->null_bo);
if(driver->ctx)
  drm_intel_gem_context_destroy(driver->ctx);
driver->ctx = NULL;
}

static int
intel_driver_init(intel_driver_t *driver, int dev_fd)
{
driver->fd = dev_fd;
driver->locked = 0;
pthread_mutex_init(&driver->ctxmutex, NULL);

if (!intel_driver_memman_init(driver)) return 0;
if (!intel_driver_context_init(driver)) return 0;

#if EMULATE_GEN
driver->gen_ver = EMULATE_GEN;
if (EMULATE_GEN == 75)
  driver->device_id = PCI_CHIP_HASWELL_L;       /* we pick L for HSW */
else if (EMULATE_GEN == 7)
  driver->device_id = PCI_CHIP_IVYBRIDGE_GT2; /* we pick GT2 for IVB */
else if (EMULATE_GEN == 6)
  driver->device_id = PCI_CHIP_SANDYBRIDGE_GT2; /* we pick GT2 for SNB */
else
  FATAL ("Unsupported Gen for emulation");
#else
if (IS_GEN9(driver->device_id))
  driver->gen_ver = 9;
else if (IS_GEN8(driver->device_id))
  driver->gen_ver = 8;
else if (IS_GEN75(driver->device_id))
  driver->gen_ver = 75;
else if (IS_GEN7(driver->device_id))
  driver->gen_ver = 7;
else if (IS_GEN6(driver->device_id))
  driver->gen_ver = 6;
else if(IS_IGDNG(driver->device_id))
  driver->gen_ver = 5;
else
  driver->gen_ver = 4;
#endif /* EMULATE_GEN */
return 1;
}

static cl_int
intel_driver_open(intel_driver_t *intel, cl_context_prop props)
{
int cardi;
#ifdef HAS_X11
char *driver_name;
#endif
if (props != NULL
    && props->gl_type != CL_GL_NOSHARE
    && props->gl_type != CL_GL_GLX_DISPLAY
    && props->gl_type != CL_GL_EGL_DISPLAY) {
  fprintf(stderr, "Unsupported gl share type %d.\n", props->gl_type);
  return CL_INVALID_OPERATION;
}

#ifdef HAS_X11
intel->x11_display = XOpenDisplay(NULL);

if(intel->x11_display) {
  if((intel->dri_ctx = getDRI2State(intel->x11_display,
                                   DefaultScreen(intel->x11_display),
                                   &driver_name))) {
    intel_driver_init_shared(intel, intel->dri_ctx);
    Xfree(driver_name);
  }
}
#endif

if(!intel_driver_is_active(intel)) {
  char card_name[20];
  for(cardi = 0; cardi < 16; cardi++) {
    sprintf(card_name, "/dev/dri/renderD%d", 128+cardi);
    if (access(card_name, R_OK) != 0)
      continue;
    if(intel_driver_init_render(intel, card_name))
      break;
  }
}

if(!intel_driver_is_active(intel)) {
  char card_name[20];
  for(cardi = 0; cardi < 16; cardi++) {
    sprintf(card_name, "/dev/dri/card%d", cardi);
    if (access(card_name, R_OK) != 0)
      continue;
    if(intel_driver_init_master(intel, card_name))
      break;
  }
}

if(!intel_driver_is_active(intel)) {
  fprintf(stderr, "Device open failed, aborting...\n");
  return CL_DEVICE_NOT_FOUND;
}

#ifdef HAS_GL_EGL
if (props && props->gl_type == CL_GL_EGL_DISPLAY) {
  assert(props->egl_display);
}
#endif
return CL_SUCCESS;
}

static void
intel_driver_close(intel_driver_t *intel)
{
//Due to the drm change about the test usrptr, we need to destroy the bufmgr
//befor the driver was closed, otherwise the test usrptr will not be freed.
if (intel->bufmgr)
  drm_intel_bufmgr_destroy(intel->bufmgr);
#ifdef HAS_X11
if(intel->dri_ctx) dri_state_release(intel->dri_ctx);
if(intel->x11_display) XCloseDisplay(intel->x11_display);
#endif
if(intel->need_close) {
  close(intel->fd);
  intel->need_close = 0;
}
intel->dri_ctx = NULL;
intel->x11_display = NULL;
intel->fd = -1;
}

LOCAL int
intel_driver_is_active(intel_driver_t *driver) {
return driver->fd >= 0;
}

#ifdef HAS_X11
LOCAL int 
intel_driver_init_shared(intel_driver_t *driver, dri_state_t *state)
{
int ret;
assert(state);
if(state->driConnectedFlag != DRI2)
  return 0;
ret = intel_driver_init(driver, state->fd);
driver->need_close = 0;
return ret;
}
#endif

static int
intel_driver_check_device(int dev_fd)
{
  // Ensure that this is actually an i915 DRM device.
  drmVersion *version;
  int ret;
  version = drmGetVersion(dev_fd);
  if (!version) {
    fprintf(stderr, "drmGetVersion(%d) failed: %s\n", dev_fd, strerror(errno));
    close(dev_fd);
    return 0;
  }
  ret = !strcmp(version->name, "i915");
  drmFreeVersion(version);
  // Don't print an error here if this device is using a different driver,
  // because we might be iterating over multiple devices looking for a
  // compatible one.
  return ret;
}

LOCAL int
intel_driver_init_master(intel_driver_t *driver, const char* dev_name)
{
int dev_fd, ret;

drm_client_t client;

// usually dev_name = "/dev/dri/card%d"
dev_fd = open(dev_name, O_RDWR);
if (dev_fd == -1) {
  fprintf(stderr, "open(\"%s\", O_RDWR) failed: %s\n", dev_name, strerror(errno));
  return 0;
}

if (!intel_driver_check_device(dev_fd)) {
  close(dev_fd);
  return 0;
}

// Check that we're authenticated
memset(&client, 0, sizeof(drm_client_t));
ret = ioctl(dev_fd, DRM_IOCTL_GET_CLIENT, &client);
if (ret == -1) {
  fprintf(stderr, "ioctl(dev_fd, DRM_IOCTL_GET_CLIENT, &client) failed: %s\n", strerror(errno));
  close(dev_fd);
  return 0;
}

if (!client.auth) {
  fprintf(stderr, "%s not authenticated\n", dev_name);
  close(dev_fd);
  return 0;
}

ret = intel_driver_init(driver, dev_fd);
driver->need_close = 1;

return ret;
}

LOCAL int
intel_driver_init_render(intel_driver_t *driver, const char* dev_name)
{
int dev_fd, ret;

dev_fd = open(dev_name, O_RDWR);
if (dev_fd == -1)
  return 0;

if (!intel_driver_check_device(dev_fd)) {
  close(dev_fd);
  return 0;
}

ret = intel_driver_init(driver, dev_fd);
driver->need_close = 1;

return ret;
}

LOCAL int 
intel_driver_terminate(intel_driver_t *driver)
{
pthread_mutex_destroy(&driver->ctxmutex);

if(driver->need_close) {
  close(driver->fd);
  driver->need_close = 0;
}
driver->fd = -1;
return 1;
}

LOCAL void
intel_driver_lock_hardware(intel_driver_t *driver)
{

PPTHREAD_MUTEX_LOCK(driver);
assert(!driver->locked);
driver->locked = 1;
}

LOCAL void 
intel_driver_unlock_hardware(intel_driver_t *driver)
{
driver->locked = 0;
PPTHREAD_MUTEX_UNLOCK(driver);
}

LOCAL dri_bo*
intel_driver_share_buffer_from_name(intel_driver_t *driver, const char *sname, uint32_t name)
{
dri_bo *bo = intel_bo_gem_create_from_name(driver->bufmgr,
                                           sname,
                                           name);
if (bo == NULL) {
  fprintf(stderr, "intel_bo_gem_create_from_name create \"%s\" bo from name %d failed: %s\n", sname, name, strerror(errno));
  return NULL;
}
return bo;
}

LOCAL dri_bo*
intel_driver_share_buffer_from_fd(intel_driver_t *driver, int fd, int size)
{
dri_bo *bo = drm_intel_bo_gem_create_from_prime(driver->bufmgr,
                                                fd,
                                                size);
if (bo == NULL) {
  fprintf(stderr, "drm_intel_bo_gem_create_from_prime create bo(size %d) from fd %d failed: %s\n", size, fd, strerror(errno));
  return NULL;
}
return bo;
}

LOCAL uint32_t
intel_driver_shared_name(intel_driver_t *driver, dri_bo *bo)
{
uint32_t name;
assert(bo);
dri_bo_flink(bo, &name);
return name;
}
/* XXX a null props is ok? */
static int
intel_get_device_id(void)
{
intel_driver_t *driver = NULL;
int intel_device_id;

driver = intel_driver_new();
assert(driver != NULL);
if(UNLIKELY(intel_driver_open(driver, NULL) != CL_SUCCESS)) return INVALID_CHIP_ID;
intel_device_id = driver->device_id;
intel_driver_context_destroy(driver);
intel_driver_close(driver);
intel_driver_terminate(driver);
intel_driver_delete(driver);

return intel_device_id;
}

extern void intel_gpgpu_delete_all(intel_driver_t *driver);
static void
cl_intel_driver_delete(intel_driver_t *driver)
{
if (driver == NULL)
  return;
intel_gpgpu_delete_all(driver);
intel_driver_context_destroy(driver);
intel_driver_close(driver);
intel_driver_terminate(driver);
intel_driver_delete(driver);
}

#include "cl_gbe_loader.h"
static intel_driver_t*
cl_intel_driver_new(cl_context_prop props)
{
intel_driver_t *driver = NULL;
TRY_ALLOC_NO_ERR (driver, intel_driver_new());
if(UNLIKELY(intel_driver_open(driver, props) != CL_SUCCESS)) goto error;
exit:
return driver;
error:
cl_intel_driver_delete(driver);
driver = NULL;
goto exit;
}

static drm_intel_bufmgr*
intel_driver_get_bufmgr(intel_driver_t *drv)
{
return drv->bufmgr;
}

static uint32_t
intel_driver_get_ver(struct intel_driver *drv)
{
return drv->gen_ver;
}

static void
intel_driver_enlarge_stack_size(struct intel_driver *drv, int32_t *stack_size)
{
  if (drv->gen_ver == 75)
    *stack_size = *stack_size * 4;
  else if (drv->device_id == PCI_CHIP_BROXTON_1 || drv->device_id == PCI_CHIP_BROXTON_3 ||
           IS_CHERRYVIEW(drv->device_id))
    *stack_size = *stack_size * 2;
}

static void
intel_driver_set_atomic_flag(intel_driver_t *drv, int atomic_flag)
{
drv->atomic_test_result = atomic_flag;
}

static size_t drm_intel_bo_get_size(drm_intel_bo *bo) { return bo->size; }
static void* drm_intel_bo_get_virtual(drm_intel_bo *bo) { return bo->virtual; }

static int get_cl_tiling(uint32_t drm_tiling)
{
switch(drm_tiling) {
case I915_TILING_X: return CL_TILE_X;
case I915_TILING_Y: return CL_TILE_Y;
case I915_TILING_NONE: return CL_NO_TILE;
default:
  assert(0);
}
return CL_NO_TILE;
}

static uint32_t intel_buffer_get_tiling_align(cl_context ctx, uint32_t tiling_mode, uint32_t dim)
{
uint32_t gen_ver = ((intel_driver_t *)ctx->drv)->gen_ver;
uint32_t ret = 0;

switch (tiling_mode) {
case CL_TILE_X:
  if (dim == 0) { //tileX width in bytes
    ret = 512;
  } else if (dim == 1) { //tileX height in number of rows
    ret = 8;
  }  else if (dim == 2) { //height to calculate slice pitch
    if (gen_ver == 9) //SKL same as tileY height
      ret = 8;
    else if (gen_ver == 8)  //IVB, HSW, BDW same as CL_NO_TILE vertical alignment
      ret = 4;
    else
      ret = 2;
  } else
    assert(0);
  break;

case CL_TILE_Y:
  if (dim == 0) { //tileY width in bytes
    ret = 128;
  } else if (dim == 1) { //tileY height in number of rows
    ret = 32;
  } else if (dim == 2) { //height to calculate slice pitch
    if (gen_ver == 9) //SKL same as tileY height
      ret = 32;
    else if (gen_ver == 8) //IVB, HSW, BDW same as CL_NO_TILE vertical alignment
      ret = 4;
    else
      ret = 2;
  } else
    assert(0);
  break;

case CL_NO_TILE:
  if (dim == 1 || dim == 2) { //vertical alignment
    if (gen_ver == 8 || gen_ver == 9) //SKL 1D array need 4 alignment qpitch
      ret = 4;
    else
      ret = 2;
  } else
    assert(0);
  break;
}

return ret;
}

#if defined(HAS_GL_EGL)
#include "intel_cl_gl_share_image_info.h"
#include "cl_image.h"

static PFNEGLEXPORTDMABUFIMAGEMESAPROC eglExportDMABUFImageMESA_func = NULL;

static int
get_required_egl_extensions(){

if(eglExportDMABUFImageMESA_func == NULL){
  eglExportDMABUFImageMESA_func =  (PFNEGLEXPORTDMABUFIMAGEMESAPROC) eglGetProcAddress("eglExportDMABUFImageMESA");
  if(eglExportDMABUFImageMESA_func == NULL){
    fprintf(stderr, "Failed to get EGL extension function eglExportDMABUFImageMESA\n");
    return -1;
  }
}
return 0;
}


static int cl_get_clformat_from_texture(GLint tex_format, cl_image_format * cl_format)
{
cl_int ret = CL_SUCCESS;

switch (tex_format) {
case GL_RGBA8:
case GL_RGBA:
case GL_RGBA16:
case GL_RGBA8I:
case GL_RGBA16I:
case GL_RGBA32I:
case GL_RGBA8UI:
case GL_RGBA16UI:
case GL_RGBA32UI:
case GL_RGBA16F:
case GL_RGBA32F:
  cl_format->image_channel_order = CL_RGBA;
  break;
case GL_BGRA:
  cl_format->image_channel_order = CL_BGRA;
  break;
default:
  ret = -1;
  goto error;
}

switch (tex_format) {
case GL_RGBA8:
case GL_RGBA:
case GL_BGRA:
  cl_format->image_channel_data_type = CL_UNORM_INT8;
  break;
case GL_RGBA16:
  cl_format->image_channel_data_type = CL_UNORM_INT16;
  break;
case GL_RGBA8I:
  cl_format->image_channel_data_type = CL_SIGNED_INT8;
  break;
case GL_RGBA16I:
  cl_format->image_channel_data_type = CL_SIGNED_INT16;
  break;
case GL_RGBA32I:
  cl_format->image_channel_data_type = CL_SIGNED_INT32;
  break;
case GL_RGBA8UI:
  cl_format->image_channel_data_type = CL_UNSIGNED_INT8;
  break;
case GL_RGBA16UI:
  cl_format->image_channel_data_type = CL_UNSIGNED_INT16;
  break;
case GL_RGBA32UI:
  cl_format->image_channel_data_type = CL_UNSIGNED_INT32;
  break;
case GL_RGBA16F:
  cl_format->image_channel_data_type = CL_HALF_FLOAT;
  break;
case GL_RGBA32F:
  cl_format->image_channel_order = CL_FLOAT;
  break;
default:
  ret = -1;
  goto error;
}

error:
return ret;
}

static int
get_mem_type_from_target(GLenum texture_target, cl_mem_object_type *type)
{
switch(texture_target) {
case GL_TEXTURE_1D: *type = CL_MEM_OBJECT_IMAGE1D; break;
case GL_TEXTURE_2D: *type = CL_MEM_OBJECT_IMAGE2D; break;
case GL_TEXTURE_3D: *type = CL_MEM_OBJECT_IMAGE3D; break;
case GL_TEXTURE_1D_ARRAY: *type = CL_MEM_OBJECT_IMAGE1D_ARRAY; break;
case GL_TEXTURE_2D_ARRAY: *type = CL_MEM_OBJECT_IMAGE2D_ARRAY; break;
default:
  return -1;
}
return CL_SUCCESS;
}

static cl_buffer
intel_alloc_buffer_from_texture_egl(cl_context ctx, unsigned int target,
                                  int miplevel, unsigned int texture,
                                  struct _cl_mem_image *image)
{
drm_intel_bo *intel_bo = NULL;
struct _intel_cl_gl_share_image_info info;
unsigned int bpp, intel_fmt;
cl_image_format cl_format;
EGLBoolean ret;

EGLenum e_target;
//We just support GL_TEXTURE_2D because we can't query info like slice_pitch now.
if(target == GL_TEXTURE_2D)
  e_target = EGL_GL_TEXTURE_2D;
else
  return NULL;

if(get_required_egl_extensions() != 0)
  return NULL;

EGLAttrib attrib_list[] = {EGL_GL_TEXTURE_LEVEL, miplevel,
                          EGL_NONE};
EGLImage e_image = eglCreateImage(EGL_DISP(ctx), EGL_CTX(ctx), e_target,
                                  (EGLClientBuffer)texture, &attrib_list[0]);
if(e_image == EGL_NO_IMAGE)
  return NULL;

int fd, stride, offset;
ret = eglExportDMABUFImageMESA_func(EGL_DISP(ctx), e_image, &fd, &stride, &offset);
if(ret != EGL_TRUE){
  eglDestroyImage(EGL_DISP(ctx), e_image);
  return NULL;
}
info.fd = fd;

/* The size argument just takes effect in intel_driver_share_buffer_from_fd when
 * Linux kernel is older than 3.12, so it doesn't matter we set to 0 here.
 */
int size = 0;
intel_bo = intel_driver_share_buffer_from_fd((intel_driver_t *)ctx->drv, fd, size);

if (intel_bo == NULL) {
  eglDestroyImage(EGL_DISP(ctx), e_image);
  return NULL;
}

GLint param_value;
glGetTexLevelParameteriv(target, miplevel, GL_TEXTURE_WIDTH, &param_value);
info.w = param_value;
glGetTexLevelParameteriv(target, miplevel, GL_TEXTURE_HEIGHT, &param_value);
info.h = param_value;
glGetTexLevelParameteriv(target, miplevel, GL_TEXTURE_DEPTH, &param_value);
info.depth = 1;
info.pitch = stride;
uint32_t tiling_mode, swizzle_mode;
drm_intel_bo_get_tiling(intel_bo, &tiling_mode, &swizzle_mode);
info.offset = offset;
info.tile_x = 0;
info.tile_y = 0;
glGetTexLevelParameteriv(target, miplevel, GL_TEXTURE_INTERNAL_FORMAT, &param_value);
info.gl_format = param_value;
info.row_pitch = stride;
info.slice_pitch = 0;

info.tiling = get_cl_tiling(tiling_mode);
if (cl_get_clformat_from_texture(info.gl_format, &cl_format) != 0)
  goto error;

if (cl_image_byte_per_pixel(&cl_format, &bpp) != CL_SUCCESS)
  goto error;
intel_fmt = cl_image_get_intel_format(&cl_format);
if (intel_fmt == INTEL_UNSUPPORTED_FORMAT)
  goto error;
cl_mem_object_type image_type;
if (get_mem_type_from_target(target, &image_type) != 0)
  goto error;

cl_mem_image_init(image, info.w, info.h,
                  image_type, info.depth, cl_format,
                  intel_fmt, bpp, info.row_pitch,
                  info.slice_pitch, info.tiling,
                  info.tile_x, info.tile_y, info.offset);

struct _cl_mem_gl_image *gl_image = (struct _cl_mem_gl_image*)image;
gl_image->fd = fd;
gl_image->egl_image = e_image;

return (cl_buffer) intel_bo;

error:
drm_intel_bo_unreference(intel_bo);
close(fd);
eglDestroyImage(EGL_DISP(ctx), e_image);
return NULL;
}

static cl_buffer
intel_alloc_buffer_from_texture(cl_context ctx, unsigned int target,
                              int miplevel, unsigned int texture,
                              struct _cl_mem_image *image)
{

if (IS_EGL_CONTEXT(ctx))
  return intel_alloc_buffer_from_texture_egl(ctx, target, miplevel, texture, image);

return NULL;
}

static int
intel_release_buffer_from_texture(cl_context ctx, struct _cl_mem_gl_image *gl_image)
{
if (IS_EGL_CONTEXT(ctx)) {
  close(gl_image->fd);
  eglDestroyImage(EGL_DISP(ctx), gl_image->egl_image);
  return CL_SUCCESS;
}
return -1;
}
#endif

cl_buffer intel_share_buffer_from_libva(cl_context ctx,
                                      unsigned int bo_name,
                                      size_t *sz)
{
drm_intel_bo *intel_bo;

intel_bo = intel_driver_share_buffer_from_name((intel_driver_t *)ctx->drv, "shared from libva", bo_name);

if (intel_bo == NULL)
  return NULL;

if (sz)
  *sz = intel_bo->size;

return (cl_buffer)intel_bo;
}

cl_buffer intel_share_image_from_libva(cl_context ctx,
                                     unsigned int bo_name,
                                     struct _cl_mem_image *image)
{
drm_intel_bo *intel_bo;
uint32_t intel_tiling, intel_swizzle_mode;

intel_bo = intel_driver_share_buffer_from_name((intel_driver_t *)ctx->drv, "shared from libva", bo_name);

if (intel_bo == NULL)
  return NULL;

drm_intel_bo_get_tiling(intel_bo, &intel_tiling, &intel_swizzle_mode);
image->tiling = get_cl_tiling(intel_tiling);

return (cl_buffer)intel_bo;
}

cl_buffer intel_share_buffer_from_fd(cl_context ctx,
                                   int fd,
                                   int buffer_size)
{
drm_intel_bo *intel_bo;

intel_bo = intel_driver_share_buffer_from_fd((intel_driver_t *)ctx->drv, fd, buffer_size);

if (intel_bo == NULL)
  return NULL;

return (cl_buffer)intel_bo;
}

cl_buffer intel_share_image_from_fd(cl_context ctx,
                                  int fd,
                                  int image_size,
                                  struct _cl_mem_image *image)
{
drm_intel_bo *intel_bo;
uint32_t intel_tiling, intel_swizzle_mode;

intel_bo = intel_driver_share_buffer_from_fd((intel_driver_t *)ctx->drv, fd, image_size);

if (intel_bo == NULL)
  return NULL;

drm_intel_bo_get_tiling(intel_bo, &intel_tiling, &intel_swizzle_mode);
image->tiling = get_cl_tiling(intel_tiling);

return (cl_buffer)intel_bo;
}

static cl_buffer intel_buffer_alloc_userptr(cl_buffer_mgr bufmgr, const char* name, void *data,size_t size, unsigned long flags)
{
#ifdef HAS_USERPTR
drm_intel_bo *bo;
bo = drm_intel_bo_alloc_userptr((drm_intel_bufmgr *)bufmgr, name, data, I915_TILING_NONE, 0, size, flags);
/* Fallback to unsynchronized userptr allocation if kernel has no MMU notifier enabled. */
if (bo == NULL)
  bo = drm_intel_bo_alloc_userptr((drm_intel_bufmgr *)bufmgr, name, data, I915_TILING_NONE, 0, size, flags | I915_USERPTR_UNSYNCHRONIZED);
return (cl_buffer)bo;
#else
return NULL;
#endif
}

static int32_t get_intel_tiling(cl_int tiling, uint32_t *intel_tiling)
{
switch (tiling) {
  case CL_NO_TILE:
    *intel_tiling = I915_TILING_NONE;
    break;
  case CL_TILE_X:
    *intel_tiling = I915_TILING_X;
    break;
  case CL_TILE_Y:
    *intel_tiling = I915_TILING_Y;
    break;
  default:
    assert(0);
    return -1;
}
return 0;
}

static int intel_buffer_set_tiling(cl_buffer bo,
                                 cl_image_tiling_t tiling, size_t stride)
{
uint32_t intel_tiling;
int ret;
if (UNLIKELY((get_intel_tiling(tiling, &intel_tiling)) < 0))
  return -1;
#ifndef NDEBUG
uint32_t required_tiling;
required_tiling = intel_tiling;
#endif
ret = drm_intel_bo_set_tiling((drm_intel_bo*)bo, &intel_tiling, stride);
assert(intel_tiling == required_tiling);
return ret;
}

#define CHV_CONFIG_WARNING \
      "Warning: can't get GPU's configurations, will use the minimal one. Please update your drm to 2.4.59+ and linux kernel to 4.0.0+.\n"
static void
intel_update_device_info(cl_device_id device)
{
intel_driver_t *driver;

driver = intel_driver_new();
assert(driver != NULL);
if (intel_driver_open(driver, NULL) != CL_SUCCESS) {
  intel_driver_delete(driver);
  return;
}

#ifdef HAS_USERPTR
const size_t sz = 4096;
void *host_ptr;

host_ptr = cl_aligned_malloc(sz, 4096);
if (host_ptr != NULL) {
  cl_buffer bo = intel_buffer_alloc_userptr((cl_buffer_mgr)driver->bufmgr,
    "CL memory object", host_ptr, sz, 0);
  if (bo == NULL)
    device->host_unified_memory = CL_FALSE;
  else
    drm_intel_bo_unreference((drm_intel_bo*)bo);
  cl_free(host_ptr);
}
else
  device->host_unified_memory = CL_FALSE;
#endif

#ifdef HAS_EU_TOTAL
unsigned int eu_total;

/* Prefer driver-queried max compute units if supported */
if (!drm_intel_get_eu_total(driver->fd, &eu_total))
  device->max_compute_unit = eu_total;
else if (IS_CHERRYVIEW(device->device_id))
  printf(CHV_CONFIG_WARNING);
#else
if (IS_CHERRYVIEW(device->device_id)) {
#if defined(__ANDROID__)
  device->max_compute_unit = 12;
#else
  printf(CHV_CONFIG_WARNING);
#endif
}
#endif

#ifdef HAS_SUBSLICE_TOTAL
unsigned int subslice_total;

/* Prefer driver-queried subslice count if supported */
if (!drm_intel_get_subslice_total(driver->fd, &subslice_total))
  device->sub_slice_count = subslice_total;
else if (IS_CHERRYVIEW(device->device_id))
  printf(CHV_CONFIG_WARNING);
#else
if (IS_CHERRYVIEW(device->device_id)) {
#if defined(__ANDROID__)
  device->sub_slice_count = 2;
#else
  printf(CHV_CONFIG_WARNING);
#endif
}
#endif

#ifdef HAS_POOLED_EU
/* BXT pooled eu, 3*6 to 2*9, like sub slice count is 2 */
int has_pooled_eu;
if((has_pooled_eu = drm_intel_get_pooled_eu(driver->fd)) > 0)
  device->sub_slice_count = 2;

#ifdef HAS_MIN_EU_IN_POOL
int min_eu;
/* for fused down 2x6 devices, beignet don't support. */
if (has_pooled_eu > 0 && (min_eu = drm_intel_get_min_eu_in_pool(driver->fd)) > 0) {
  assert(min_eu == 9); //don't support fuse down device.
}
#endif //HAS_MIN_EU_IN_POOL
#endif //HAS_POOLED_EU
//We should get the device memory dynamically, but the
//mapablce mem size usage is unknown. Just ignore it.
size_t total_mem,map_mem;
if(drm_intel_get_aperture_sizes(driver->fd,&map_mem,&total_mem) == 0)
  device->global_mem_size = (cl_ulong)total_mem;

intel_driver_context_destroy(driver);
intel_driver_close(driver);
intel_driver_terminate(driver);
intel_driver_delete(driver);
}

LOCAL void
intel_setup_callbacks(void)
{
cl_driver_new = (cl_driver_new_cb *) cl_intel_driver_new;
cl_driver_delete = (cl_driver_delete_cb *) cl_intel_driver_delete;
cl_driver_get_ver = (cl_driver_get_ver_cb *) intel_driver_get_ver;
cl_driver_enlarge_stack_size = (cl_driver_enlarge_stack_size_cb *) intel_driver_enlarge_stack_size;
cl_driver_set_atomic_flag = (cl_driver_set_atomic_flag_cb *) intel_driver_set_atomic_flag;
cl_driver_get_bufmgr = (cl_driver_get_bufmgr_cb *) intel_driver_get_bufmgr;
cl_driver_get_device_id = (cl_driver_get_device_id_cb *) intel_get_device_id;
cl_driver_update_device_info = (cl_driver_update_device_info_cb *) intel_update_device_info;
cl_buffer_alloc = (cl_buffer_alloc_cb *) drm_intel_bo_alloc;
cl_buffer_alloc_userptr = (cl_buffer_alloc_userptr_cb*) intel_buffer_alloc_userptr;
#ifdef HAS_BO_SET_SOFTPIN
cl_buffer_set_softpin_offset = (cl_buffer_set_softpin_offset_cb *) drm_intel_bo_set_softpin_offset;
cl_buffer_set_bo_use_full_range = (cl_buffer_set_bo_use_full_range_cb *) drm_intel_bo_use_48b_address_range;
#endif
  cl_buffer_disable_reuse = (cl_buffer_disable_reuse_cb *) drm_intel_bo_disable_reuse;
  cl_buffer_set_tiling = (cl_buffer_set_tiling_cb *) intel_buffer_set_tiling;
#if defined(HAS_GL_EGL)
  cl_buffer_alloc_from_texture = (cl_buffer_alloc_from_texture_cb *) intel_alloc_buffer_from_texture;
  cl_buffer_release_from_texture = (cl_buffer_release_from_texture_cb *) intel_release_buffer_from_texture;
#endif
  cl_buffer_get_buffer_from_libva = (cl_buffer_get_buffer_from_libva_cb *) intel_share_buffer_from_libva;
  cl_buffer_get_image_from_libva = (cl_buffer_get_image_from_libva_cb *) intel_share_image_from_libva;
  cl_buffer_reference = (cl_buffer_reference_cb *) drm_intel_bo_reference;
  cl_buffer_unreference = (cl_buffer_unreference_cb *) drm_intel_bo_unreference;
  cl_buffer_map = (cl_buffer_map_cb *) drm_intel_bo_map;
  cl_buffer_unmap = (cl_buffer_unmap_cb *) drm_intel_bo_unmap;
  cl_buffer_map_gtt = (cl_buffer_map_gtt_cb *) drm_intel_gem_bo_map_gtt;
  cl_buffer_unmap_gtt = (cl_buffer_unmap_gtt_cb *) drm_intel_gem_bo_unmap_gtt;
  cl_buffer_map_gtt_unsync = (cl_buffer_map_gtt_unsync_cb *) drm_intel_gem_bo_map_unsynchronized;
  cl_buffer_get_virtual = (cl_buffer_get_virtual_cb *) drm_intel_bo_get_virtual;
  cl_buffer_get_size = (cl_buffer_get_size_cb *) drm_intel_bo_get_size;
  cl_buffer_pin = (cl_buffer_pin_cb *) drm_intel_bo_pin;
  cl_buffer_unpin = (cl_buffer_unpin_cb *) drm_intel_bo_unpin;
  cl_buffer_subdata = (cl_buffer_subdata_cb *) drm_intel_bo_subdata;
  cl_buffer_get_subdata = (cl_buffer_get_subdata_cb *) drm_intel_bo_get_subdata;
  cl_buffer_wait_rendering = (cl_buffer_wait_rendering_cb *) drm_intel_bo_wait_rendering;
  cl_buffer_get_fd = (cl_buffer_get_fd_cb *) drm_intel_bo_gem_export_to_prime;
  cl_buffer_get_tiling_align = (cl_buffer_get_tiling_align_cb *)intel_buffer_get_tiling_align;
  cl_buffer_get_buffer_from_fd = (cl_buffer_get_buffer_from_fd_cb *) intel_share_buffer_from_fd;
  cl_buffer_get_image_from_fd = (cl_buffer_get_image_from_fd_cb *) intel_share_image_from_fd;
  intel_set_gpgpu_callbacks(intel_get_device_id());
}
