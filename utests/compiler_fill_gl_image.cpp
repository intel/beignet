#include "utest_helper.hpp"

static void read_back(int tex, int width, int height, uint32_t * resultColor)
{
  float vertices[8] = {-1, 1, 1, 1, 1, -1, -1, -1};
  float tex_coords[8] = {0, 0, 1, 0, 1, 1, 0, 1};

  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, vertices);
  glEnableClientState(GL_VERTEX_ARRAY);
  glClientActiveTexture(GL_TEXTURE0);
  glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 2, tex_coords);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glFlush();
  OCL_SWAP_EGL_BUFFERS();

  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, resultColor);
}


static void compiler_fill_gl_image(void)
{
  const size_t w = EGL_WINDOW_WIDTH;
  const size_t h = EGL_WINDOW_HEIGHT;
  uint32_t color = 0x123456FF;
  uint32_t *resultColor;
  GLuint tex;

  if (eglContext == EGL_NO_CONTEXT) {
    fprintf(stderr, "There is no valid egl context. Ignore this case.\n");
    return;
  }
  // Setup kernel and images
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  // Must set the all filters to GL_NEAREST!
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);

  OCL_CREATE_KERNEL("test_fill_gl_image");
  OCL_CREATE_GL_IMAGE(buf[0], 0, GL_TEXTURE_2D, 0, tex);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(color), &color);
  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 16;
  glFinish();
  OCL_ENQUEUE_ACQUIRE_GL_OBJECTS(0);
  OCL_NDRANGE(2);
  OCL_FLUSH();

  // Check result
  resultColor = new uint32_t[w * h * 4];
  if (resultColor == NULL)
    assert(0);

  read_back(tex, w, h, resultColor);
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      OCL_ASSERT(resultColor[j * w + i] == color);
  OCL_UNMAP_BUFFER(0);
  delete resultColor;
}

MAKE_UTEST_FROM_FUNCTION(compiler_fill_gl_image);
