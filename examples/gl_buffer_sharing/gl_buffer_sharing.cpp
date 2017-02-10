/*
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "utest_helper.hpp"
#include <stdio.h>

static cl_int cl_status;

const size_t w = EGL_WINDOW_WIDTH;
const size_t h = EGL_WINDOW_HEIGHT;
static GLuint tex;

static void draw(){
  XEvent event;

  float vertices[8] = {-1, 1, 1, 1, 1, -1, -1, -1};
  float tex_coords[8] = {0, 0, 1, 0, 1, 1, 0, 1};
  uint32_t color0 = 0x0000ff00;

  for (;;)
  {
    XNextEvent(xDisplay, &event);

    if (event.type == Expose)
    {
      glClearColor(0.0, 1.0, 0.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

      OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
      OCL_SET_ARG(1, sizeof(color0), &color0);
      globals[0] = w;
      globals[1] = h;
      locals[0] = 16;
      locals[1] = 16;
      glFinish();
      OCL_ENQUEUE_ACQUIRE_GL_OBJECTS(0);
      OCL_NDRANGE(2);
      OCL_ENQUEUE_RELEASE_GL_OBJECTS(0);
      OCL_FINISH();

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
      glFinish();
      eglSwapBuffers(eglDisplay, eglSurface);
    }
    if (event.type == KeyPress)
      break;
  }
}


static void initialize_ocl_gl(){

  //ocl initialization: basic & create kernel & check extension
  printf("\n***********************OpenCL info: ***********************\n");
  if ((cl_status = cl_test_init("runtime_fill_gl_image.cl", "runtime_fill_gl_image", SOURCE)) != 0){
    fprintf(stderr, "cl_test_init error\n");
    exit(1);
  }

  if (eglContext == EGL_NO_CONTEXT) {
    fprintf(stderr, "There is no valid egl context! Exit!\n");
    exit(1);
  }

  XMapWindow(xDisplay, xWindow);

  // Setup kernel and images
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);

  //Create cl image from miplevel 0
  OCL_CREATE_GL_IMAGE(buf[0], 0, GL_TEXTURE_2D, 0, tex);
}

int main(int argc, char *argv[])
{
  initialize_ocl_gl();

  draw();

  //destroy resource of cl & gl
  cl_test_destroy();

  printf("\nExample run successfully!\n");
}
