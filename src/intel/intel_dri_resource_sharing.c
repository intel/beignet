/**************************************************************************
 *
 * Copyright 2003 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
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
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#define HAVE_PTHREAD 1
#include <errno.h>
#include <time.h>
#include "main/context.h"
#include "main/renderbuffer.h"
#include "main/texobj.h"
#include <stdbool.h>
#include <string.h>
#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>
#include <GL/internal/dri_interface.h>
#include "intel_mipmap_tree.h"
#include "intel_regions.h"
#include "intel_context.h"

#include "intel_dri_resource_sharing.h"
#include "intel_dri_resource_sharing_int.h"

#include <dlfcn.h>
/**
 * Sets up a DRIImage structure to point to our shared image in a region
 */
static bool
intel_setup_cl_region_from_mipmap_tree(void *driver,
                                       struct intel_context *intel,
                                       struct intel_mipmap_tree *mt,
                                       GLuint level, GLuint zoffset,
                                       struct _intel_dri_share_image_region *region)
{
   unsigned int draw_x, draw_y;
   uint32_t mask_x, mask_y;
   struct intel_region *null_region = (struct intel_region *)NULL;

   intel_miptree_check_level_layer(mt, level, zoffset);

   _intel_region_get_tile_masks(mt->region, &mask_x, &mask_y, false);
   _intel_miptree_get_image_offset(mt, level, zoffset, &draw_x, &draw_y);

   region->w = mt->level[level].width;
   region->h = mt->level[level].height;
   region->tile_x = draw_x & mask_x;
   region->tile_y = draw_y & mask_y;
   region->tiling = mt->region->tiling;
   /* XXX hard code to 1 right now. */
   region->depth = 1;
   region->row_pitch = mt->region->pitch;

   region->offset = _intel_region_get_aligned_offset(mt->region,
                                                     draw_x & ~mask_x,
                                                     draw_y & ~mask_y,
                                                     false);
   if (!_intel_region_flink(mt->region, &region->name))
      return false;
   _intel_region_reference(&null_region, mt->region);
   return true;
}

typedef void
_mesa_test_texobj_completeness_t( const struct gl_context *ctx,
                                struct gl_texture_object *t );
_mesa_test_texobj_completeness_t *__mesa_test_texobj_completeness;

typedef struct gl_texture_object *
_mesa_lookup_texture_t( const struct gl_context *ctx, GLuint id);
_mesa_lookup_texture_t *__mesa_lookup_texture;

static struct gl_texture_object *
intel_get_gl_obj_from_texture(void *driver,
                              struct intel_context *intel,
                              GLenum target, GLint level,
                              GLuint texture, GLuint face)
{
   struct gl_texture_object *obj;
   __mesa_lookup_texture = dlsym(driver, "_mesa_lookup_texture");
   obj = __mesa_lookup_texture(&intel->ctx, texture);
   if (!obj || obj->Target != target) {
      return NULL;
   }

   __mesa_test_texobj_completeness = dlsym(driver, "_mesa_test_texobj_completeness");
   __mesa_test_texobj_completeness(&intel->ctx, obj);
   if (!obj->_BaseComplete || (level > 0 && !obj->_MipmapComplete)) {
      return NULL;
   }

   if (level < obj->BaseLevel || level > obj->_MaxLevel) {
      return NULL;
   }

   return obj;
}

static GLenum
get_cl_gl_format(mesa_format format)
{
   switch (format) {
   case MESA_FORMAT_R8G8B8A8_UNORM:
      return GL_RGBA;
   case MESA_FORMAT_A8R8G8B8_UNORM:
      return GL_BGRA;
   default:
      return GL_BGRA;
  }
}

static bool
intelAcquireTexture(void *driver, __DRIcontext *context, GLenum target,
                    GLint level, GLuint texture, void *user_data)
{
   struct _intel_dri_share_image_region *region = intel_dri_share_image_region(user_data);
   struct intel_context *intel = context->driverPrivate;
   struct gl_texture_object *obj;
   struct intel_texture_object *iobj;
   /* XXX Always be face 0? */
   GLuint face = 0;

   obj = intel_get_gl_obj_from_texture(driver, intel, target, level, texture, face);
   if (obj == NULL)
     return false;
   iobj = intel_texture_object(obj);
   region->gl_format = get_cl_gl_format(obj->Image[face][level]->TexFormat);
   return intel_setup_cl_region_from_mipmap_tree(driver, intel, iobj->mt, level, 0, region);
}

static bool
intelReleaseTexture(void *driver, __DRIcontext *context, GLenum target,
                    GLint level, GLuint texture)
{
   struct intel_context *intel = context->driverPrivate;
   struct gl_texture_object *obj;
   struct intel_texture_object *iobj;
   /* XXX Always be face 0? */
   GLuint face = 0;

   obj = intel_get_gl_obj_from_texture(driver, intel, target, level, texture, face);
   if (obj == NULL)
     return false;

   iobj = intel_texture_object(obj);
   _intel_region_release(&iobj->mt->region);
   return true;
}

static bool
intelAcquireBufferObj(void *driver, __DRIcontext *driContextPriv,
                      GLuint bufobj, void *user_data)
{
  return false;
}

static bool
intelReleaseBufferObj(void *driver, __DRIcontext *driContextPriv, GLuint bufobj)
{
  return false;
}

static bool
intelAcquireRenderBuffer(void *driver, __DRIcontext *driContextPriv,
                         GLuint bufobj, void *user_data)
{
  return false;
}

static bool
intelReleaseRenderBuffer(void *driver, __DRIcontext *driContextPriv, GLuint bufobj)
{
  return false;
}

#include "cl_driver.h"
void
intel_set_cl_gl_callbacks(void)
{
  cl_gl_acquire_texture = (cl_gl_acquire_texture_cb*)intelAcquireTexture;
  cl_gl_release_texture = (cl_gl_release_texture_cb*)intelReleaseTexture;
  cl_gl_acquire_buffer_object = (cl_gl_acquire_buffer_object_cb*)intelAcquireBufferObj;
  cl_gl_release_buffer_object = (cl_gl_release_buffer_object_cb*)intelReleaseBufferObj;
  cl_gl_acquire_render_buffer = (cl_gl_acquire_render_buffer_cb*)intelAcquireRenderBuffer;
  cl_gl_release_render_buffer = (cl_gl_release_render_buffer_cb*)intelReleaseRenderBuffer;
}
