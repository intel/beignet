#include <string.h>
#include "GL/gl.h" /* dri_interface need gl types definitions. */
#include "GL/internal/dri_interface.h"
#include "gbm_deps/gbm_driint.h"
#include "gbm_deps/gbmint.h"
#include "dricommon.h"

typedef struct EGLDisplay _EGLDisplay;
typedef struct EGLDriver  _EGLDriver;
/* XXX should check whether we support pthread.*/
typedef pthread_mutex_t _EGLMutex;

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
typedef unsigned int EGLBoolean;
typedef int32_t EGLint;

struct _hack_egl_display
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
   EGLint VersionMajor;       /**< EGL major version */
   EGLint VersionMinor;       /**< EGL minor version */
   EGLint ClientAPIs;         /**< Bitmask of APIs supported (EGL_xxx_BIT) */
};

struct _hack_dri2_egl_display
{
   int                       dri2_major;
   int                       dri2_minor;
   __DRIscreen              *dri_screen;
   int                       own_dri_screen;
   const __DRIconfig       **driver_configs;
   void                     *driver;
   __DRIcoreExtension       *core;
   __DRIdri2Extension       *dri2;
   __DRIswrastExtension     *swrast;
   __DRI2flushExtension     *flush;
   __DRItexBufferExtension  *tex_buffer;
   __DRIimageExtension      *image;
   __DRIrobustnessExtension *robustness;
   __DRI2configQueryExtension *config;
   int                       fd;

   int                       own_device;
   int                       swap_available;
   int                       invalidate_available;
   int                       min_swap_interval;
   int                       max_swap_interval;
   int                       default_swap_interval;
   struct gbm_dri_device    *gbm_dri;

   char                     *device_name;
   char                     *driver_name;

   __DRIdri2LoaderExtension    dri2_loader_extension;
   __DRIswrastLoaderExtension  swrast_loader_extension;
   const __DRIextension     *extensions[4];
};

static __DRIimageLookupExtension *image_lookup_extension;

/* We are use DRI2 x11 platform, and by default, gbm doesn't register
 * a valid image extension, and actually, it doesn't know how to register
 * it based on current interface. We have to hack it here. */
void cl_gbm_set_image_extension(struct gbm_device *gbm, void *display)
{
  struct gbm_dri_device *gbm_dri = gbm_dri_device(gbm);
  struct _hack_egl_display *egl_dpy = (struct _hack_egl_display*)display;
  struct _hack_dri2_egl_display *dri2_dpy = (struct _hack_dri2_egl_display*)egl_dpy->DriverData;
  int i;

  if (gbm_dri->lookup_image == NULL
      && egl_dpy->Platform == _EGL_PLATFORM_X11) {
    for(i = 0; i < 4; i++)
     if (dri2_dpy->extensions[i]
         && ((strncmp(dri2_dpy->extensions[i]->name,
                      __DRI_IMAGE_LOOKUP,
                      sizeof(__DRI_IMAGE_LOOKUP))) == 0))
       break;
    if (i >= 4) return;
    image_lookup_extension = (__DRIimageLookupExtension*)dri2_dpy->extensions[i];
    gbm_dri->lookup_image = image_lookup_extension->lookupEGLImage;
    gbm_dri->lookup_user_data = display;
  }
}

int cl_gbm_bo_get_name(struct gbm_bo *bo)
{
  int name;
  struct gbm_dri_device *gbm_dri = gbm_dri_device(bo->gbm);
  struct gbm_dri_bo *bo_dri = gbm_dri_bo(bo);

  gbm_dri->image->queryImage(bo_dri->image, __DRI_IMAGE_ATTRIB_NAME,
                             &name);
  return name;
}
