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
 * Note: the code is taken from libva code base
 */

/*
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
 */

#include <X11/Xlibint.h>
#include <X11/Xlib.h>
#include "x11/va_dri2.h"
#include "x11/va_dri2tokens.h"
#include "x11/dricommon.h"
#include "cl_utils.h"
#include "cl_alloc.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#define LOCAL __attribute__ ((visibility ("internal")))

LOCAL dri_drawable_t*
dri_state_do_drawable_hash(dri_state_t *state, XID drawable)
{
  int index = drawable % DRAWABLE_HASH_SZ;
  struct dri_drawable *dri_drawable = state->drawable_hash[index];

  while (dri_drawable) {
    if (dri_drawable->x_drawable == drawable)
      return dri_drawable;
    dri_drawable = dri_drawable->next;
  }

  dri_drawable = dri_state_create_drawable(state, drawable);
  dri_drawable->x_drawable = drawable;
  dri_drawable->next = state->drawable_hash[index];
  state->drawable_hash[index] = dri_drawable;

  return dri_drawable;
}

LOCAL void
dri_state_free_drawable_hash(dri_state_t *state)
{
  int i;
  struct dri_drawable *dri_drawable, *prev;

  for (i = 0; i < DRAWABLE_HASH_SZ; i++) {
    dri_drawable = state->drawable_hash[i];

    while (dri_drawable) {
      prev = dri_drawable;
      dri_drawable = prev->next;
      dri_state_destroy_drawable(state, prev);
    }
  }
}

LOCAL dri_drawable_t*
dri_state_get_drawable(dri_state_t *state, XID drawable)
{
  return dri_state_do_drawable_hash(state, drawable);
}

LOCAL void
dri_state_init_drawable_hash_table(dri_state_t *state)
{
  int i;
  for(i=0; i < DRAWABLE_HASH_SZ; i++)
    state->drawable_hash[i] = NULL;
}

LOCAL void
dri_state_delete(dri_state_t *state)
{
  if (state == NULL)
    return;
  dri_state_close(state);
  cl_free(state);
}

LOCAL dri_state_t*
dri_state_new(void)
{
  dri_state_t *state = NULL;
  TRY_ALLOC_NO_ERR (state, CALLOC(dri_state_t));
  state->fd = -1;
  state->driConnectedFlag = NONE;
  dri_state_init_drawable_hash_table(state);

exit:
  return state;
error:
  dri_state_delete(state);
  state = NULL;
  goto exit;
}

#define __DRI_BUFFER_FRONT_LEFT         0
#define __DRI_BUFFER_BACK_LEFT          1
#define __DRI_BUFFER_FRONT_RIGHT        2
#define __DRI_BUFFER_BACK_RIGHT         3
#define __DRI_BUFFER_DEPTH              4
#define __DRI_BUFFER_STENCIL            5
#define __DRI_BUFFER_ACCUM              6
#define __DRI_BUFFER_FAKE_FRONT_LEFT    7
#define __DRI_BUFFER_FAKE_FRONT_RIGHT   8

typedef struct dri2_drawable
{
  struct dri_drawable base;
  union dri_buffer buffers[5];
  int width;
  int height;
  int has_backbuffer;
  int back_index;
  int front_index;
} dri2_drawable_t;

LOCAL dri_drawable_t*
dri_state_create_drawable(dri_state_t *state, XID x_drawable)
{
  dri2_drawable_t *dri2_drwble;
  dri2_drwble = (dri2_drawable_t*)calloc(1, sizeof(*dri2_drwble));

  if (!dri2_drwble)
    return NULL;

  dri2_drwble->base.x_drawable = x_drawable;
  dri2_drwble->base.x = 0;
  dri2_drwble->base.y = 0;
  VA_DRI2CreateDrawable(state->x11_dpy, x_drawable);

  return &dri2_drwble->base;
}

LOCAL void
dri_state_destroy_drawable(dri_state_t *state, dri_drawable_t *dri_drwble)
{
  VA_DRI2DestroyDrawable(state->x11_dpy, dri_drwble->x_drawable);
  free(dri_drwble);
}

LOCAL void
dri_state_swap_buffer(dri_state_t *state, dri_drawable_t *dri_drwble)
{
  dri2_drawable_t *dri2_drwble = (dri2_drawable_t*)dri_drwble;
  XRectangle xrect;
  XserverRegion region;

  if (dri2_drwble->has_backbuffer) {
    xrect.x = 0;
    xrect.y = 0;
    xrect.width = dri2_drwble->width;
    xrect.height = dri2_drwble->height;

    region = XFixesCreateRegion(state->x11_dpy, &xrect, 1);
    VA_DRI2CopyRegion(state->x11_dpy, dri_drwble->x_drawable, region,
        DRI2BufferFrontLeft, DRI2BufferBackLeft);
    XFixesDestroyRegion(state->x11_dpy, region);
  }
}

LOCAL union dri_buffer*
dri_state_get_rendering_buffer(dri_state_t *state, dri_drawable_t *dri_drwble)
{
  dri2_drawable_t *dri2_drwble = (dri2_drawable_t *)dri_drwble;
  int i;
  int count;
  unsigned int attachments[5];
  VA_DRI2Buffer *buffers;

  i = 0;
  attachments[i++] = __DRI_BUFFER_BACK_LEFT;
  attachments[i++] = __DRI_BUFFER_FRONT_LEFT;
  buffers = VA_DRI2GetBuffers(state->x11_dpy,
                              dri_drwble->x_drawable,
                              &dri2_drwble->width,
                              &dri2_drwble->height,
                              attachments,
                              i,
                              &count);
  assert(buffers);
  if (buffers == NULL)
    return NULL;

  dri2_drwble->has_backbuffer = 0;

  for (i = 0; i < count; i++) {
    dri2_drwble->buffers[i].dri2.attachment = buffers[i].attachment;
    dri2_drwble->buffers[i].dri2.name = buffers[i].name;
    dri2_drwble->buffers[i].dri2.pitch = buffers[i].pitch;
    dri2_drwble->buffers[i].dri2.cpp = buffers[i].cpp;
    dri2_drwble->buffers[i].dri2.flags = buffers[i].flags;

    if (buffers[i].attachment == __DRI_BUFFER_BACK_LEFT) {
      dri2_drwble->has_backbuffer = 1;
      dri2_drwble->back_index = i;
    }

    if (buffers[i].attachment == __DRI_BUFFER_FRONT_LEFT)
      dri2_drwble->front_index = i;
  }

  dri_drwble->width = dri2_drwble->width;
  dri_drwble->height = dri2_drwble->height;
  Xfree(buffers);

  if (dri2_drwble->has_backbuffer)
    return &dri2_drwble->buffers[dri2_drwble->back_index];

  return &dri2_drwble->buffers[dri2_drwble->front_index];
}

LOCAL void
dri_state_close(dri_state_t *state) {
  dri_state_free_drawable_hash(state);
  assert(state->fd >= 0);
  close(state->fd);
}

LOCAL void
dri_state_release(dri_state_t *state) {
  dri_state_delete(state);
}

LOCAL dri_state_t*
getDRI2State(Display* dpy, int screen, char **driver_name)
{
  int major, minor;
  int error_base;
  int event_base;
  char *device_name = NULL;
  drm_magic_t magic;
  char * internal_driver_name = NULL;
  int fd = -1;
  dri_state_t* state = NULL;

  if (!VA_DRI2QueryExtension(dpy, &event_base, &error_base))
    goto err_out;

  if (!VA_DRI2QueryVersion(dpy, &major, &minor))
    goto err_out;


  if (!VA_DRI2Connect(dpy, RootWindow(dpy, screen),
        &internal_driver_name, &device_name))
    goto err_out;

  fd = open(device_name, O_RDWR);
  assert(fd >= 0);

  if (fd < 0)
    goto err_out;

  if (drmGetMagic(fd, &magic))
    goto err_out;

  if (!VA_DRI2Authenticate(dpy, RootWindow(dpy, screen),
        magic))
    goto err_out;

  if(driver_name)
    *driver_name = internal_driver_name;
  else
    Xfree(internal_driver_name);

  state = dri_state_new();
  state->fd = fd;
  state->x11_dpy = dpy;
  state->x11_screen = screen;
  state->driConnectedFlag = DRI2;
  if (device_name)
    Xfree(device_name);
  return state;

err_out:
  if (device_name)
    Xfree(device_name);

  if (internal_driver_name)
    Xfree(internal_driver_name);

  if(driver_name) *driver_name = NULL;

  if (fd >= 0)
    close(fd);

  if (driver_name)
    *driver_name = NULL;

  return state;
}

