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


#include <assert.h>
#include <string.h>

#include "mesa_egl_extension.h"
#include "mesa_egl_res_share.h"

/**
 * Parse the list of share texture attributes and return the proper error code.
 */
EGLint
_eglParseTextureAttribList(unsigned int *texture, EGLenum *gl_target, EGLint *level,
                           const EGLint *attrib_list)
{
   EGLint i, err = EGL_SUCCESS;

   *texture = 0;
   *gl_target = 0;
   *level = 0;

   if (!attrib_list)
      return EGL_BAD_ATTRIBUTE;

   for (i = 0; attrib_list[i] != EGL_NONE; i++) {
      EGLint attr = attrib_list[i++];
      EGLint val = attrib_list[i];

      switch (attr) {
      case EGL_GL_TEXTURE_LEVEL_MESA:
         *level = val;
         break;
      case EGL_GL_TEXTURE_ID_MESA:
         *texture = val;
         break;
      case EGL_GL_TEXTURE_TARGET_MESA:
         *gl_target = val;
         break;
      default:
         /* unknown attrs are ignored */
         break;
      }
   }

   return err;
}

/**
 * Parse the list of share texture attributes and return the proper error code.
 */
EGLint
_eglParseBufferObjAttribList(unsigned int *bufobj, const EGLint *attrib_list)
{
   EGLint i, err = EGL_SUCCESS;
   *bufobj = 0;

   if (!attrib_list)
      return EGL_BAD_ATTRIBUTE;

   for (i = 0; attrib_list[i] != EGL_NONE; i++) {
      EGLint attr = attrib_list[i++];
      EGLint val = attrib_list[i];

      switch (attr) {
      case EGL_GL_BUFFER_OBJECT_ID_MESA:
         *bufobj = val;
         break;
      default:
         /* unknown attrs are ignored */
         break;
      }
   }
   if (*bufobj == 0)
      err = EGL_BAD_ATTRIBUTE;

   return err;
}

/**
 * Parse the list of share texture attributes and return the proper error code.
 */
EGLint
_eglParseRenderBufferAttribList(unsigned int *rb, const EGLint *attrib_list)
{
   EGLint i, err = EGL_SUCCESS;
   *rb = 0;

   if (!attrib_list)
      return EGL_BAD_ATTRIBUTE;

   for (i = 0; attrib_list[i] != EGL_NONE; i++) {
      EGLint attr = attrib_list[i++];
      EGLint val = attrib_list[i];

      switch (attr) {
      case EGL_GL_RENDER_BUFFER_ID_MESA:
         *rb = val;
         break;
      default:
         /* unknown attrs are ignored */
         break;
      }
   }
   if (*rb == 0)
      err = EGL_BAD_ATTRIBUTE;

   return err;
}
