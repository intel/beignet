#ifndef __INTEL_CL_GL_SHARE_IMAGE_INFO_
#define __INTEL_CL_GL_SHARE_IMAGE_INFO_

struct _intel_cl_gl_share_image_info {
  int fd;
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

#endif
