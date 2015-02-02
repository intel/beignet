/*
 * Copyright (c) 2012, 2015 Intel Corporation. All Rights Reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <va/va.h>
#include "va_display.h"

#include <va/va_drmcommon.h>
#include "utest_helper.hpp"

#include <getopt.h>             /* getopt_long() */

typedef cl_mem (OCLCREATEIMAGEFROMLIBVAINTEL)(cl_context, const cl_libva_image *, cl_int *);
OCLCREATEIMAGEFROMLIBVAINTEL *oclCreateImageFromLibvaIntel = NULL;

const char *input_nv12;
const char *output_nv12;
int frame_size;
int picture_width, picture_height;
unsigned char *newImageBuffer;
VADisplay	va_dpy;
cl_int cl_status;
VAStatus va_status;
bool putsurface=true;

static const char short_options[] = "i:r:o:h";

static const struct option
long_options[] = {
  { "input", required_argument, NULL, 'i' },
  { "help",   no_argument,       NULL, 'h' },
  { "resolution", required_argument,       NULL, 'r' },
  { "output",  required_argument, NULL, 'o' },
  { 0, 0, 0, 0 }
};

#define WIDTH_DEFAULT 256
#define HEIGHT_DEFAULT 128

#define CHECK_VASTATUS(va_status,func)                                  \
  if (va_status != VA_STATUS_SUCCESS) {                                   \
    fprintf(stderr, "status = %d, %s:%s (%d) failed,exit\n",va_status, __func__, func, __LINE__); \
    exit(1);                                                            \
  }

#define CHECK_CLSTATUS(status,func)                                  \
  if (status != CL_SUCCESS) {                                   \
    fprintf(stderr, "status = %d, %s:%s (%d) failed,exit\n", status, __func__, func, __LINE__); \
    exit(1);                                                            \
  }

static void usage(FILE *fp, int argc, char **argv)
{
  fprintf(fp,
      "\n"
      "This example aims to demostrate the usage of gpu buffer sharing between libva and Beignet.\n"
      "The result will be shown on screen if you haven't specified -o option.\n"
      "The input and output file are nv12 format.\n"
      "Please use the following command to see these files:\n"
      "gst-launch-1.0 filesrc location=file_name ! videoparse format=nv12 width=xxx height=xxx ! imagefreeze ! videoconvert ! video/x-raw, format=BGRx ! ximagesink\n"
      "(Please install gstreamer1.0-plugins-base, gstreamer1.0-plugins-bad, \n"
      " gstreamer1.0-x by apt on Ubuntu, in order to use gst-launch-1.0)\n"
      "For more details, please read docs/howto/libva-buffer-sharing-howto.mdwn.\n"
      "\nUsage: %s [options]\n\n"
      "Options:\n"
      "-i | --input=<file_name>           Specify input nv12 file name like /home/xxx/in.nv12\n"
      "-h | --help                        Print this message\n"
      "-r | --resolution=<width,height>   Set input resolution\n"
      "-o | --output=<file_name>          Specify input nv12 file name like /home/xxx/out.nv12\n"
      "",
      argv[0]);
}

static void analyse_args(int argc, char *argv[])
{
  input_nv12 = NULL;
  picture_width = 0;
  picture_height = 0;
  output_nv12 = NULL;
  putsurface = true;

  int c, idx;
  for (;;) {

    c = getopt_long(argc, argv,
        short_options, long_options, &idx);

    if (-1 == c)
      break;

    switch (c) {
      case 0: /* getopt_long() flag */
        break;

      case 'i':
        input_nv12 = optarg;
        break;

      case '?':
      case 'h':
        usage(stdout, argc, argv);
        exit(0);

      case 'r':
        sscanf(optarg, "%d,%d", &picture_width, &picture_height);
        break;

      case 'o':
        output_nv12 = optarg;
        putsurface = false;
        break;

      default:
        usage(stderr, argc, argv);
        exit(1);
    }
  }

  if(!input_nv12){
    input_nv12 = INPUT_NV12_DEFAULT;
  }
  if(picture_width == 0 && picture_height == 0){
    picture_width = WIDTH_DEFAULT;
    picture_height = HEIGHT_DEFAULT;
  }
  return;
}


static void initialize_va_ocl(){
  int major_ver, minor_ver;

  printf("\n***********************libva info: ***********************\n");
  fflush(stdout);
  va_dpy = va_open_display();
  va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);
  CHECK_VASTATUS(va_status, "vaInitialize");

  //ocl initialization: basic & create kernel & get extension
  printf("\n***********************OpenCL info: ***********************\n");
  if ((cl_status = cl_test_init("runtime_mirror_effect.cl", "runtime_mirror_effect", SOURCE)) != 0){
    fprintf(stderr, "cl_test_init error\n");
    exit(1);
  }

#ifdef CL_VERSION_1_2
  oclCreateImageFromLibvaIntel = (OCLCREATEIMAGEFROMLIBVAINTEL *)clGetExtensionFunctionAddressForPlatform(platform, "clCreateImageFromLibvaIntel");
#else
  oclCreateImageFromLibvaIntel = (OCLCREATEIMAGEFROMLIBVAINTEL *)clGetExtensionFunctionAddress("clCreateImageFromLibvaIntel");
#endif
  if(!oclCreateImageFromLibvaIntel){
    fprintf(stderr, "Failed to get extension clCreateImageFromLibvaIntel\n");
    exit(1);
  }
}

static void upload_nv12_to_surface(FILE *nv12_fp, VASurfaceID surface_id)
{
  VAImage surface_image;
  void *surface_p = NULL;
  unsigned char *y_src, *u_src;
  unsigned char *y_dst, *u_dst;
  int y_size = picture_width * picture_height;
  int row, col;
  size_t n_items;

  n_items = fread(newImageBuffer, frame_size, 1, nv12_fp);
  if(n_items != 1){
    fprintf(stderr, "Haven't read expected size data from file\n");
    exit(1);
  }

  va_status = vaDeriveImage(va_dpy, surface_id, &surface_image);
  CHECK_VASTATUS(va_status,"vaDeriveImage");

  va_status = vaMapBuffer(va_dpy, surface_image.buf, &surface_p);
  CHECK_VASTATUS(va_status,"vaMapBuffer");

  y_src = newImageBuffer;
  u_src = newImageBuffer + y_size; /* U offset for NV12 */

  y_dst = (unsigned char *)surface_p + surface_image.offsets[0];
  u_dst = (unsigned char *)surface_p + surface_image.offsets[1]; /* U offset for NV12 */

  /* Y plane */
  for (row = 0; row < surface_image.height; row++) {
    memcpy(y_dst, y_src, surface_image.width);
    y_dst += surface_image.pitches[0];
    y_src += picture_width;
  }

  assert(surface_image.format.fourcc == VA_FOURCC_NV12); /* UV plane */
  for (row = 0; row < surface_image.height / 2; row++) {
    for (col = 0; col < surface_image.width / 2; col++) {
      u_dst[col * 2] = u_src[col * 2];
      u_dst[col * 2 + 1] = u_src[col * 2 + 1];
    }
    u_dst += surface_image.pitches[1];
    u_src += picture_width;
  }

  vaUnmapBuffer(va_dpy, surface_image.buf);
  vaDestroyImage(va_dpy, surface_image.image_id);
}

static void create_y_image_object_from_libva(VAImage *surface_image,
                                             VABufferInfo *buf_info,
                                             cl_mem *yio_p)
{
  cl_libva_image info_image;
  info_image.bo_name = buf_info->handle;
  info_image.offset = surface_image->offsets[0];
  info_image.width = surface_image->width;
  info_image.height = surface_image->height;
  info_image.fmt.image_channel_order = CL_R;
  info_image.fmt.image_channel_data_type = CL_UNSIGNED_INT8;
  info_image.row_pitch = surface_image->pitches[0];
  *yio_p = oclCreateImageFromLibvaIntel(ctx, &info_image, &cl_status);
  CHECK_CLSTATUS(cl_status, "oclCreateImageFromLibvaIntel");
  printf("\nSuccessfully create ocl image object from y plane of VASurface...\n");
}

static void create_uv_image_object_from_libva(VAImage *surface_image,
                                              VABufferInfo *buf_info,
                                              cl_mem *yio_p)
{
  cl_libva_image info_image;
  info_image.bo_name = buf_info->handle;
  info_image.offset = surface_image->offsets[1];
  info_image.width = surface_image->width / 2;
  info_image.height = surface_image->height / 2;
  info_image.fmt.image_channel_order = CL_R;
  info_image.fmt.image_channel_data_type = CL_UNSIGNED_INT16;
  info_image.row_pitch = surface_image->pitches[1];
  *yio_p = oclCreateImageFromLibvaIntel(ctx, &info_image, &cl_status);
  CHECK_CLSTATUS(cl_status, "oclCreateImageFromLibvaIntel");
  printf("\nSuccessfully create ocl image object from uv plane of VASurface...\n");
}

static void store_surface_to_nv12(VASurfaceID surface_id, FILE *nv12_fp)
{
  VAImage surface_image;
  void *surface_p = NULL;
  unsigned char *y_src, *u_src;
  unsigned char *y_dst, *u_dst;
  int y_size = picture_width * picture_height;
  int row, col;

  va_status = vaDeriveImage(va_dpy, surface_id, &surface_image);
  CHECK_VASTATUS(va_status,"vaDeriveImage");

  va_status = vaMapBuffer(va_dpy, surface_image.buf, &surface_p);
  CHECK_VASTATUS(va_status,"vaMapBuffer");

  y_src = (unsigned char *)surface_p + surface_image.offsets[0];
  u_src = (unsigned char *)surface_p + surface_image.offsets[1]; /* U offset for NV12 */

  y_dst = newImageBuffer;
  u_dst = newImageBuffer + y_size; /* U offset for NV12 */

  /* Y plane */
  for (row = 0; row < surface_image.height; row++) {
    memcpy(y_dst, y_src, surface_image.width);
    y_src += surface_image.pitches[0];
    y_dst += picture_width;
  }

  assert(surface_image.format.fourcc == VA_FOURCC_NV12); /* UV plane */
  for (row = 0; row < surface_image.height / 2; row++) {
    for (col = 0; col < surface_image.width / 2; col++) {
      u_dst[col * 2] = u_src[col * 2];
      u_dst[col * 2 + 1] = u_src[col * 2 + 1];
    }
    u_src += surface_image.pitches[1];
    u_dst += picture_width;
  }

  fwrite(newImageBuffer, frame_size, 1, nv12_fp);

  vaUnmapBuffer(va_dpy, surface_image.buf);
  vaDestroyImage(va_dpy, surface_image.image_id);
}

static void load_process_store_nv12()
{
  frame_size = picture_width * picture_height +  ((picture_width * picture_height) >> 1) ;
  newImageBuffer = (unsigned char *)malloc(frame_size);

  VASurfaceID src_surface_id;
  VASurfaceAttrib forcc;
  forcc.type =VASurfaceAttribPixelFormat;
  forcc.flags=VA_SURFACE_ATTRIB_SETTABLE;
  forcc.value.type=VAGenericValueTypeInteger;
  forcc.value.value.i = VA_FOURCC_NV12;
  va_status = vaCreateSurfaces(va_dpy, VA_RT_FORMAT_YUV420,
                               picture_width, picture_height,
                               &src_surface_id, 1, &forcc, 1);
  CHECK_VASTATUS(va_status, "vaCreateSurfaces");

  //load
  FILE *in_nv12_fp;
  in_nv12_fp = fopen(input_nv12, "rb");
  if (in_nv12_fp == NULL){
    fprintf(stderr, "Can't open input nv12 file\n");
    exit(1);
  }
  fseek(in_nv12_fp, 0l, SEEK_END);
  off_t file_size = ftell(in_nv12_fp);

  if ((file_size < frame_size) || (file_size % frame_size) ) {
    fclose(in_nv12_fp);
    fprintf(stderr, "The nv12 file's size is not correct\n");
    exit(1);
  }
  fseek(in_nv12_fp, 0l, SEEK_SET);
  upload_nv12_to_surface(in_nv12_fp, src_surface_id);
  fclose(in_nv12_fp);
  printf("\nSuccessfully load source nv12 file(\"%s\") to VASurface...\n", input_nv12);


  //create two corresponding ocl image objects from source VASurface
  VAImage src_surface_image;
  va_status = vaDeriveImage(va_dpy, src_surface_id, &src_surface_image);
  CHECK_VASTATUS(va_status,"vaDeriveImage");
  VABufferInfo buf_info;
  buf_info.mem_type = VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM;
  va_status = vaAcquireBufferHandle(va_dpy, src_surface_image.buf, &buf_info);
  CHECK_VASTATUS(va_status,"vaAcquireBufferHandle");
  cl_mem src_y, src_uv;
  create_y_image_object_from_libva(&src_surface_image, &buf_info, &src_y);
  OCL_CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &src_y);

  create_uv_image_object_from_libva(&src_surface_image, &buf_info, &src_uv);
  OCL_CALL (clSetKernelArg, kernel, 1, sizeof(cl_mem), &src_uv);


  //create one target VASurface & create corresponding target ocl image object from it
  VASurfaceID dst_surface_id;
  va_status = vaCreateSurfaces(va_dpy,VA_RT_FORMAT_YUV420,
                               picture_width,picture_height,
                               &dst_surface_id, 1, &forcc, 1);
  CHECK_VASTATUS(va_status, "vaCreateSurfaces");

  VAImage dst_surface_image;
  va_status = vaDeriveImage(va_dpy, dst_surface_id, &dst_surface_image);
  CHECK_VASTATUS(va_status,"vaDeriveImage");
  va_status = vaAcquireBufferHandle(va_dpy, dst_surface_image.buf, &buf_info);
  CHECK_VASTATUS(va_status,"vaAcquireBufferHandle");
  cl_mem dst_y, dst_uv;
  create_y_image_object_from_libva(&dst_surface_image, &buf_info, &dst_y);
  OCL_CALL (clSetKernelArg, kernel, 2, sizeof(cl_mem), &dst_y);
  create_uv_image_object_from_libva(&dst_surface_image, &buf_info, &dst_uv);
  OCL_CALL (clSetKernelArg, kernel, 3, sizeof(cl_mem), &dst_uv);
  OCL_CALL (clSetKernelArg, kernel, 4, sizeof(int), &picture_height);


  size_t global_size[2];
  global_size[0] = picture_width;
  global_size[1] = picture_height;
  OCL_CALL (clEnqueueNDRangeKernel, queue, kernel, 2, NULL,
            global_size, NULL, 0, NULL, NULL);
  OCL_CALL (clFinish, queue);
  printf("\nSuccessfully use ocl to do processing...\n");

  va_status = vaReleaseBufferHandle(va_dpy, src_surface_image.buf);
  CHECK_VASTATUS(va_status,"vaReleaseBufferHandle");
  va_status = vaReleaseBufferHandle(va_dpy, dst_surface_image.buf);
  CHECK_VASTATUS(va_status,"vaReleaseBufferHandle");

  OCL_CALL (clReleaseMemObject, src_y);
  OCL_CALL (clReleaseMemObject, src_uv);
  OCL_CALL (clReleaseMemObject, dst_y);
  OCL_CALL (clReleaseMemObject, dst_uv);
  vaDestroyImage(va_dpy, src_surface_image.image_id);
  vaDestroyImage(va_dpy, dst_surface_image.image_id);
  cl_kernel_destroy();
  cl_ocl_destroy();


  if (putsurface) {
    VARectangle src_rect, dst_rect;

    src_rect.x      = 0;
    src_rect.y      = 0;
    src_rect.width  = picture_width;
    src_rect.height = picture_height;
    dst_rect        = src_rect;

    //XXX There is a bug of X server which will cause va_put_surface showing
    //incorrect result. So call va_put_surface twice times to workaround this
    //bug.
    va_status = va_put_surface(va_dpy, dst_surface_id, &src_rect, &dst_rect);
    va_status = va_put_surface(va_dpy, dst_surface_id, &src_rect, &dst_rect);
    CHECK_VASTATUS(va_status, "vaPutSurface");
    printf("press any key to exit\n");
    getchar();
  }
  else{
    //store
    FILE *out_nv12_fp;
    out_nv12_fp = fopen(output_nv12,"wb");
    if ( out_nv12_fp == NULL){
      fprintf(stderr, "Can't open output nv12 file\n");
      exit(1);
    }
    store_surface_to_nv12(dst_surface_id, out_nv12_fp);
    fclose(out_nv12_fp);
    printf("\nSuccessfully store VASurface to dst nv12 file(\"%s\")...\n", output_nv12);
    printf("\nNote: The input and output file are nv12 format.\n");
    printf("Please use the following command to see the result:\n");
    printf("gst-launch-1.0 filesrc location=%s ! videoparse format=nv12 width=%d height=%d ! imagefreeze ! videoconvert ! video/x-raw, format=BGRx ! ximagesink\n", output_nv12, picture_width, picture_height);
    printf("(Please install gstreamer1.0-plugins-base, gstreamer1.0-plugins-bad,\ngstreamer1.0-x by apt on Ubuntu, in order to use gst-launch-1.0)\n");
  }

  //release resources
  vaDestroySurfaces(va_dpy,&src_surface_id,1);
  vaDestroySurfaces(va_dpy,&dst_surface_id,1);

  vaTerminate(va_dpy);
  va_close_display(va_dpy);
}


int main(int argc, char *argv[])
{
  analyse_args(argc, argv);

  initialize_va_ocl();

  load_process_store_nv12();

  printf("\nExample run successfully!\n");

  return 0;
}
