/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef _VA_DRICOMMON_H_
#define _VA_DRICOMMON_H_

#include <X11/Xlib.h>
#include <xf86drm.h>
#include <drm.h>
#include <drm_sarea.h>

union dri_buffer 
{
  struct {
    unsigned int attachment;
    unsigned int name;
    unsigned int pitch;
    unsigned int cpp;
    unsigned int flags;
  } dri2;
};

typedef struct dri_drawable 
{
  XID x_drawable;
  int x;
  int y;
  unsigned int width;
  unsigned int height;
  struct dri_drawable *next;
} dri_drawable_t;

#define DRAWABLE_HASH_SZ 32

enum DRI_VER
{
  NONE = 0,
  // NOT supported VA_DRI1 = 1,
  DRI2 = 2
};

typedef struct dri_state
{
  Display *x11_dpy;
  int x11_screen;
  int fd;
  enum DRI_VER driConnectedFlag; /* 0: disconnected, 2: DRI2 */
  dri_drawable_t *drawable_hash[DRAWABLE_HASH_SZ];
} dri_state_t;

dri_drawable_t *dri_state_create_drawable(dri_state_t*, XID x_drawable);
void dri_state_destroy_drawable(dri_state_t*, dri_drawable_t*);
void dri_state_close(dri_state_t*);
void dri_state_release(dri_state_t*);

// Create a dri2 state from dpy and screen
dri_state_t *getDRI2State(Display* dpy, int screen, char **driver_name);

#endif /* _VA_DRICOMMON_H_ */

