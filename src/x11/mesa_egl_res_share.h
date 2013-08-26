/**************************************************************************
 *
 * Copyright 2013-2014 Zhigang Gong <zhigang.gong@linux.intel.com>
 * Copyright 2013-2014 Intel, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


#ifndef EGLRESSHARE_INCLUDED
#define EGLRESSHARE_INCLUDED

#include <EGL/egl.h>

EGLint
_eglParseTextureAttribList(unsigned int *texture, EGLenum *gl_target,
                           EGLint *level, const EGLint *attrib_list);
EGLint
_eglParseBufferObjAttribList(unsigned int *bufobj,
                             const EGLint *attrib_list);

EGLint
_eglParseRenderBufferAttribList(unsigned int *rb, const EGLint *attrib_list);
#endif
