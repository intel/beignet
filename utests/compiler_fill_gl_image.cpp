#include "utest_helper.hpp"


static void compiler_fill_gl_image(void)
{
  const size_t w = EGL_WINDOW_WIDTH;
  const size_t h = EGL_WINDOW_HEIGHT;
  uint32_t color0 = 0x123456FF;
  uint32_t color1 = 0x789ABCDE;
  uint32_t *resultColor0;
  uint32_t *resultColor1;
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
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, w/2, h/2, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);

  OCL_CREATE_KERNEL("test_fill_gl_image");
  //Create cl image from miplevel 0
  OCL_CREATE_GL_IMAGE(buf[0], 0, GL_TEXTURE_2D, 0, tex);
  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(color0), &color0);
  globals[0] = w;
  globals[1] = h;
  locals[0] = 16;
  locals[1] = 16;
  glFinish();
  OCL_ENQUEUE_ACQUIRE_GL_OBJECTS(0);
  OCL_NDRANGE(2);
  OCL_FLUSH();
  OCL_ENQUEUE_RELEASE_GL_OBJECTS(0);

  // Check result
  resultColor0 = new uint32_t[w * h];
  if (resultColor0 == NULL)
    assert(0);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, resultColor0);
  for (uint32_t j = 0; j < h; ++j)
    for (uint32_t i = 0; i < w; i++)
      OCL_ASSERT(resultColor0[j * w + i] == color0);


  //Create cl image from miplevel 1
  OCL_CREATE_GL_IMAGE(buf[1], 0, GL_TEXTURE_2D, 1, tex);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(1, sizeof(color1), &color1);
  globals[0] = w/2;
  globals[1] = h/2;
  OCL_ENQUEUE_ACQUIRE_GL_OBJECTS(1);
  OCL_NDRANGE(2);
  OCL_FLUSH();
  OCL_ENQUEUE_RELEASE_GL_OBJECTS(1);

  // Check result
  resultColor1 = new uint32_t[(w/2)*(h/2)];
  glGetTexImage(GL_TEXTURE_2D, 1, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, resultColor1);
  for (uint32_t j = 0; j < h/2; ++j)
    for (uint32_t i = 0; i < w/2; i++)
      OCL_ASSERT(resultColor1[j * (w/2) + i] == color1);
  delete[] resultColor0;
  delete[] resultColor1;
}

MAKE_UTEST_FROM_FUNCTION(compiler_fill_gl_image);
