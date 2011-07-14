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

#ifndef _VA_DRI2_H_
#define _VA_DRI2_H_

#include <X11/extensions/Xfixes.h>
#include <X11/Xfuncproto.h>
#include <xf86drm.h>

typedef struct {
    unsigned int attachment;
    unsigned int name;
    unsigned int pitch;
    unsigned int cpp;
    unsigned int flags;
} VA_DRI2Buffer;

extern Bool
VA_DRI2QueryExtension(Display *display, int *eventBase, int *errorBase);
extern Bool
VA_DRI2QueryVersion(Display *display, int *major, int *minor);
extern Bool
VA_DRI2Connect(Display *display, XID window,
	    char **driverName, char **deviceName);
extern Bool
VA_DRI2Authenticate(Display *display, XID window, drm_magic_t magic);
extern void
VA_DRI2CreateDrawable(Display *display, XID drawable);
extern void
VA_DRI2DestroyDrawable(Display *display, XID handle);
extern VA_DRI2Buffer *
VA_DRI2GetBuffers(Display *dpy, XID drawable,
	       int *width, int *height,
	       unsigned int *attachments, int count,
	       int *outcount);
#if 1
extern void
VA_DRI2CopyRegion(Display *dpy, XID drawable, XserverRegion region,
	       CARD32 dest, CARD32 src);
#endif
#endif
