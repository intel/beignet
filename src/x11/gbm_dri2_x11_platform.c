#include "GL/gl.h" /* dri_interface need gl types definitions. */
#include "GL/internal/dri_interface.h"
#include "gbm_deps/gbm_driint.h"
#include "gbm_deps/gbmint.h"
#include "dricommon.h"

/* image_lookup_extension is from egl_dri2.c. */
extern const __DRIimageLookupExtension image_lookup_extension;

/* We are use DRI2 x11 platform, and by default, gbm doesn't register
 * a valid image extension, and actually, it doesn't know how to register
 * it based on current interface. We have to hack it here. */
void cl_gbm_set_image_extension(struct gbm_device *gbm, void *display)
{
  struct gbm_dri_device *gbm_dri = gbm_dri_device(gbm);
  if (gbm_dri->lookup_image == NULL) {
    gbm_dri->lookup_image = image_lookup_extension.lookupEGLImage;
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
