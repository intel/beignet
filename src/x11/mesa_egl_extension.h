#ifndef __MESA_EGL_EXTENSION_H__
#define __MESA_EGL_EXTENSION_H__

#include <EGL/egl.h>
#include <GL/gl.h>
#include <GL/internal/dri_interface.h>

#define EGL_GL_TEXTURE_MESA             0x3300  /* eglAcuireResource target */
#define EGL_GL_BUFFER_OBJECT_MESA       0x3301  /* eglAcuireResource target */
#define EGL_GL_RENDER_BUFFER_MESA       0x3302  /* eglAcuireResource target */
#define EGL_GL_TEXTURE_ID_MESA          0x3303  /* eglAcuireResource attribute */
#define EGL_GL_TEXTURE_LEVEL_MESA       0x3304  /* eglAcuireResource attribute */
#define EGL_GL_TEXTURE_TARGET_MESA      0x3305  /* eglAcuireResource attribute */
#define EGL_GL_BUFFER_OBJECT_ID_MESA    0x3306  /* eglAcuireResource attribute */
#define EGL_GL_RENDER_BUFFER_ID_MESA    0x3307  /* eglAcuireResource attribute */

EGLBoolean eglAcquireResourceMESA(EGLDisplay dpy, EGLContext ctx, EGLenum target, const EGLint *attrib_list, void * user_data);
EGLBoolean eglReleaseResourceMESA(EGLDisplay dpy, EGLContext ctx, EGLenum target, const EGLint *attrib_list);

#endif
