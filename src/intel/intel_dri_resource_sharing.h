#ifndef __INTEL_DRI_RESOURCE_SHARING_H__
#define __INTEL_DRI_RESOURCE_SHARING_H__

struct _intel_dri_share_image_region {
  unsigned int name;
  size_t w;
  size_t h;
  size_t depth;
  size_t pitch;
  int tiling;
  size_t offset;
  size_t tile_x;
  size_t tile_y;
  unsigned int gl_format;
  size_t row_pitch, slice_pitch;
};

struct _intel_dri_share_buffer_object {
  unsigned int name;
  size_t sz;
  size_t offset;
};

inline static struct _intel_dri_share_image_region *
intel_dri_share_image_region(void *user_data)
{
   return (struct _intel_dri_share_image_region *)user_data;
}

inline static struct _intel_dri_share_buffer_object *
intel_dri_share_buffer_object(void *user_data)
{
   return (struct _intel_dri_share_buffer_object *)user_data;
}

extern void intel_set_cl_gl_callbacks(void);


#endif
