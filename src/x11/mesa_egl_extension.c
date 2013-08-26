#include <stdio.h>
#include "mesa_egl_extension.h"
#include "mesa_egl_res_share.h"
#include "src/cl_driver.h"

struct _egl_display;
struct _egl_resource;
struct _egl_thread_info;
struct _egl_config;
struct _egl_surface;
struct _egl_driver;

typedef struct _egl_display _EGLDisplay;
typedef struct _egl_resource _EGLResource;
typedef struct _egl_thread_info _EGLThreadInfo;
typedef struct _egl_config _EGLConfig;
typedef struct _egl_surface _EGLSurface;
typedef struct _egl_driver _EGLDriver;

/**
 * A resource of a display.
 */
struct _egl_resource
{
   /* which display the resource belongs to */
   _EGLDisplay *Display;
   EGLBoolean IsLinked;
   EGLint RefCount;

   /* used to link resources of the same type */
   _EGLResource *Next;
};

/**
 * "Base" class for device driver contexts.
 */
struct _egl_context
{
   /* A context is a display resource */
   _EGLResource Resource;

   /* The bound status of the context */
   _EGLThreadInfo *Binding;
   _EGLSurface *DrawSurface;
   _EGLSurface *ReadSurface;

   _EGLConfig *Config;

   EGLint ClientAPI; /**< EGL_OPENGL_ES_API, EGL_OPENGL_API, EGL_OPENVG_API */
   EGLint ClientMajorVersion;
   EGLint ClientMinorVersion;
   EGLint Flags;
   EGLint Profile;
   EGLint ResetNotificationStrategy;

   /* The real render buffer when a window surface is bound */
   EGLint WindowRenderBuffer;
};

typedef struct _egl_context _EGLContext;

struct dri2_egl_display
{
   int                       dri2_major;
   int                       dri2_minor;
   __DRIscreen              *dri_screen;
   int                       own_dri_screen;
   const __DRIconfig       **driver_configs;
   void                     *driver;
};

enum _egl_platform_type {
   _EGL_PLATFORM_WINDOWS,
   _EGL_PLATFORM_X11,
   _EGL_PLATFORM_WAYLAND,
   _EGL_PLATFORM_DRM,
   _EGL_PLATFORM_FBDEV,
   _EGL_PLATFORM_NULL,
   _EGL_PLATFORM_ANDROID,

   _EGL_NUM_PLATFORMS,
   _EGL_INVALID_PLATFORM = -1
};
typedef enum _egl_platform_type _EGLPlatformType;

typedef pthread_mutex_t _EGLMutex;

struct _egl_display
{
   /* used to link displays */
   _EGLDisplay *Next;

   _EGLMutex Mutex;

   _EGLPlatformType Platform; /**< The type of the platform display */
   void *PlatformDisplay;     /**< A pointer to the platform display */

   _EGLDriver *Driver;        /**< Matched driver of the display */
   EGLBoolean Initialized;    /**< True if the display is initialized */

   /* options that affect how the driver initializes the display */
   struct {
      EGLBoolean TestOnly;    /**< Driver should not set fields when true */
      EGLBoolean UseFallback; /**< Use fallback driver (sw or less features) */
   } Options;

   /* these fields are set by the driver during init */
   void *DriverData;          /**< Driver private data */
};

static struct dri2_egl_display *
dri2_egl_display(_EGLDisplay *dpy)
{
  return (struct dri2_egl_display *)dpy->DriverData;
}

static _EGLDisplay *
_eglLockDisplay(EGLDisplay dpy)
{
  return (_EGLDisplay *)dpy;
}

static _EGLContext *
_eglLookupContext(EGLContext ctx, EGLDisplay disp)
{
  disp = disp;
  return (_EGLContext *) ctx;
}

struct dri2_egl_context
{
   _EGLContext   base;
   __DRIcontext *dri_context;
};

static struct dri2_egl_context *
dri2_egl_context(_EGLContext *ctx)
{
  return (struct dri2_egl_context *)ctx;
}

static EGLBoolean
dri2_acquire_texture(_EGLDisplay *disp,
                     _EGLContext *ctx,
                     const EGLint *attr_list,
                     void *user_data)
{
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   GLuint texture = 0;
   GLenum gl_target = 0;
   GLint level = 0;
   GLboolean ret;

   if (_eglParseTextureAttribList(&texture, &gl_target, &level, attr_list) != EGL_SUCCESS)
      return EGL_FALSE;

   ret = cl_gl_acquire_texture(dri2_dpy->driver,
                               dri2_ctx->dri_context,
                               gl_target, level, texture,
                               user_data);
   return ret;
}

static EGLBoolean
dri2_release_texture(_EGLDisplay *disp, _EGLContext *ctx, const EGLint *attr_list)
{
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   GLuint texture = 0;
   GLenum gl_target = 0;
   GLint level = 0;
   GLboolean ret;

   if (_eglParseTextureAttribList(&texture, &gl_target, &level, attr_list) != EGL_SUCCESS)
      return EGL_FALSE;

   ret = cl_gl_release_texture(dri2_dpy->driver, dri2_ctx->dri_context,
                               gl_target, level, texture);
   return ret;
}

static EGLBoolean
dri2_acquire_buffer_object(_EGLDisplay *disp, _EGLContext *ctx, const EGLint *attr_list,
                           void *user_data)
{
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   GLuint bufobj = 0;
   GLboolean ret;

   if (_eglParseBufferObjAttribList(&bufobj, attr_list) != EGL_SUCCESS)
      return EGL_FALSE;

   ret = cl_gl_acquire_buffer_object(dri2_dpy->driver,
                                     dri2_ctx->dri_context,
                                     bufobj, user_data);
   return ret;
}

static EGLBoolean
dri2_release_buffer_object(_EGLDisplay *disp, _EGLContext *ctx, const EGLint *attr_list)
{
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   GLuint bufobj = 0;
   GLboolean ret;

   if (_eglParseBufferObjAttribList(&bufobj, attr_list) != EGL_SUCCESS)
      return EGL_FALSE;

   ret = cl_gl_release_buffer_object(dri2_dpy->driver,
                                     dri2_ctx->dri_context,
                                     bufobj);
   return ret;
}

static EGLBoolean
dri2_acquire_render_buffer(_EGLDisplay *disp,
                           _EGLContext *ctx,
                           const EGLint *attr_list,
                           void *user_data)
{
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   GLuint rb = 0;
   GLboolean ret;

   if (_eglParseBufferObjAttribList(&rb, attr_list) != EGL_SUCCESS)
      return EGL_FALSE;

   ret = cl_gl_acquire_render_buffer(dri2_dpy->driver,
                                     dri2_ctx->dri_context,
                                     rb, user_data);
   return ret;
}

static EGLBoolean
dri2_release_render_buffer(_EGLDisplay *disp, _EGLContext *ctx, const EGLint *attr_list)
{
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   GLuint rb = 0;
   GLboolean ret;

   if (_eglParseBufferObjAttribList(&rb, attr_list) != EGL_SUCCESS)
      return EGL_FALSE;

   ret = cl_gl_release_render_buffer(dri2_dpy->driver,
                                     dri2_ctx->dri_context,
                                     rb);
   return ret;
}

static EGLBoolean
dri2_acquire_resource_mesa(_EGLDisplay *disp, _EGLContext *ctx, const EGLenum target,
                           const EGLint *attrib_list, void *user_data)
{
   switch (target) {
   case EGL_GL_TEXTURE_MESA:
     return dri2_acquire_texture(disp, ctx, attrib_list, user_data);
   case EGL_GL_BUFFER_OBJECT_MESA:
     return dri2_acquire_buffer_object(disp, ctx, attrib_list, user_data);
   case EGL_GL_RENDER_BUFFER_MESA:
     return dri2_acquire_render_buffer(disp, ctx, attrib_list, user_data);
   default:
      fprintf(stderr, "bad resource target value 0x%04x",
              target);
   }
   return EGL_FALSE;
}

static EGLBoolean
dri2_release_resource_mesa(_EGLDisplay *disp, _EGLContext *ctx, const EGLenum target,
                           const EGLint *attrib_list)
{
   switch (target) {
   case EGL_GL_TEXTURE_MESA:
     return dri2_release_texture(disp, ctx, attrib_list);
   case EGL_GL_BUFFER_OBJECT_MESA:
     return dri2_release_buffer_object(disp, ctx, attrib_list);
   case EGL_GL_RENDER_BUFFER_MESA:
     return dri2_release_render_buffer(disp, ctx, attrib_list);
   default:
      fprintf(stderr, "bad resource target value 0x%04x",
              target);
   }
   return EGL_FALSE;
}

EGLBoolean
eglAcquireResourceMESA(EGLDisplay dpy, EGLContext ctx, EGLenum target, const EGLint *attrib_list, void *user)
{
   _EGLDisplay *disp = _eglLockDisplay(dpy);
   _EGLContext *context = _eglLookupContext(ctx, disp);

   return dri2_acquire_resource_mesa(disp, context, target, attrib_list, user);
}

EGLBoolean
eglReleaseResourceMESA(EGLDisplay dpy, EGLContext ctx, EGLenum target, const EGLint *attrib_list)
{
   _EGLDisplay *disp = _eglLockDisplay(dpy);
   _EGLContext *context = _eglLookupContext(ctx, disp);

   return dri2_release_resource_mesa(disp, context, target, attrib_list);
}
