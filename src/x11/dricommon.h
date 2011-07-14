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

