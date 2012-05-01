/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "common.h"

#include <stdlib.h>
#include <malloc.h>

void usage()
{
  printf("Bad usage\n");
  exit(0);
}

void clpanic(const char *msg, int rval)
{
  printf("Failed: %s (%d)\n", msg, rval);
  exit(-1);
}

void parseArgs(int argc, char **argv, struct args *pargs)
{
  char *p;
  argv++;

  while ((p = *argv++) != NULL) {
    if (*p == '-') {
      switch (*++p) {
        case 'x':
          pargs->x = atoi(*argv++);
          break;
        case 'y':
          pargs->y = atoi(*argv++);
          break;
        case 'z':
          pargs->z = atoi(*argv++);
          break;
        case 'W':
          pargs->W = atoi(*argv++);
          break;
        case 'i':
          pargs->i = atoi(*argv++);
          break;
        case 'v':
          pargs->v = 1;
          break;
        case 'd':
          pargs->d = atoi(*argv++);
          break;
        default:
          usage();
          break;
      }
    } else {
      usage();
    }
  }
  printf("x %d, y %d, z %d, W %d, d %d, v %d, i %d\n",
      pargs->x, pargs->y, pargs->z, pargs->W, pargs->d, pargs->v,
      pargs->i);

}

float randf()
{
  return ((float) rand())/((float)RAND_MAX);
}

float randf2(float lo, float hi)
{
  assert(lo <= hi);
  return lo + (hi - lo) * randf();
}

cl_device_id getDeviceID(int devtype)
{
  int rval;
  cl_platform_id platform;
  cl_device_id device;

  rval = clGetPlatformIDs(1, &platform, NULL);
  if (rval != CL_SUCCESS)
    clpanic("clGetPlatformIDs", rval);

  rval =
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
  if (rval != CL_SUCCESS)
    clpanic("GetDeviceIDs", rval);

  return device;
}

int filesize(FILE *fp)
{
  fseek(fp, 0L, SEEK_END);
  int fsize = ftell(fp);
  fseek(fp, 0L, SEEK_SET);	// Note this resets fp to beginning.
  return fsize;
}

cl_kernel
getKernel(cl_device_id dev, cl_context ctx,
          const char *filename,
          const char *kernelname)
{
  FILE *fp;
  char *my_program;
  size_t fsize;
  int e;

  fp = fopen(filename, "r");
  assert(fp);
  fsize = filesize(fp);

  printf("File: %s, size=%d\n", filename, (int) fsize);
  my_program = malloc(fsize);
  assert(my_program);
  e = fread(my_program, 1, fsize, fp);
  assert(e == fsize);

  cl_program program =
    clCreateProgramWithSource(ctx, 1, (const char **) &my_program, &fsize, &e);
  CHK_ERR(e);
  e = clBuildProgram(program, 1, &dev, NULL, NULL, NULL);
  CHK_ERR(e);

  if (e != CL_SUCCESS) {
    /* Print out build log */
    char build_log[1000];
    size_t logsize = sizeof(build_log) - 1;
    e = clGetProgramBuildInfo(program, dev,
        CL_PROGRAM_BUILD_LOG, logsize,
        build_log, &logsize);
    build_log[logsize] = 0;
    printf("Build Failed:\n%s\n", build_log);
    exit(-1);
  }

  cl_kernel kernel = clCreateKernel(program, kernelname, &e);
  CHK_ERR(e);
  printf("Extracted kernel %s from file %s\n", kernelname, filename);
  return kernel;
}

cl_kernel
getKernelFromBinary(cl_device_id dev, cl_context ctx,
                    const char *filename,
                    const char *kernelname)
{
  FILE *fp;
  char *my_program;
  size_t fsize;
  int e;

  fp = fopen(filename, "r");
  assert(fp);
  fsize = filesize(fp);

  printf("File: %s, size=%d\n", filename, (int) fsize);
  my_program = malloc(fsize);
  assert(my_program);
  e = fread(my_program, 1, fsize, fp);
  assert(e == fsize);

  cl_program program = clCreateProgramWithBinary(ctx,
                                                 1, &dev, &fsize,
                                                 (const unsigned char **) &my_program,
                                                 NULL, &e);
  CHK_ERR(e);
  e = clBuildProgram(program, 1, &dev, NULL, NULL, NULL);
  CHK_ERR(e);
  cl_kernel kernel = clCreateKernel(program, kernelname, &e);
  CHK_ERR(e);
  printf("Extracted kernel %s from file %s\n", kernelname, filename);
  return kernel;
}

void *newBuffer(int bufsiz, int etype)
{
  char *p;
  float *f, scale;
  int i;

  void *buf = (void *) memalign(32, bufsiz);
  assert(buf);
  rand();

  switch (etype) {
    case 0:
    case '0':
      memset(buf, 0, bufsiz);
      break;

    case 'i':
      for (i = 0, p = buf; i < bufsiz; i++) {
        p[i] = (rand() & 0xff);
      }
      break;

    case 'f':
    case 'p':
      scale = (etype == 'p') ? 255.0f : 1.0f;
      for (i = 0, f = buf; i < (bufsiz / sizeof(float)); i++) {
        f[i] = scale * ((float) rand()) / RAND_MAX;
      }
      break;

    default:
      assert(0);
  }
  return buf;

}

int comparef(const float *refData, const float *data, int n, float eps)
{
  float err = 0.0f;
  float ref = 0.0f;
  int i;

  for (i = 1; i < n; ++i) {
    float diff = refData[i] - data[i];
    err += diff * diff;
    ref += refData[i] * refData[i];
  }

  float normRef = sqrtf(ref);
  if (fabsf(ref) < 1e-7f) {
    printf("*FAIL* comparef: ref < 1e-7 (%12.8f)\n", ref);
    return 0;
  }
  float normError = sqrtf(err);
  err = normError / normRef;
  printf("comparef: err=%12.8f, eps=%12.8f\n", err, eps);
  if (err < eps) {
    printf("PASSED\n");
    return 1;
  } else {
    printf("FAILED\n");
    return 0;
  }
}

#if 0
// Unit test
int main(int argc, char **argv)
{
  struct args args = { 0 };

  parseArgs(argc, argv, &args);
  printf("x %d, y %d, z %d, W %d, d %d, v %d, i %d\n",
      args.x, args.y, args.z, args.W, args.d, args.v, args.i);

}
#endif

struct bmphdr {
  //   2 bytes of magic here, "BM", total header size is 54 bytes!
  int filesize;		//   4 total file size incl header
  short as0, as1;		//   8 app specific
  int bmpoffset;		//  12 ofset of bmp data 
  int headerbytes;	//  16 bytes in header from this point (40 actually)
  int width;		//  20 
  int height;		//  24 
  short nplanes;		//  26 no of color planes
  short bpp;		//  28 bits/pixel
  int compression;	//  32 BI_RGB = 0 = no compression
  int sizeraw;		//  36 size of raw bmp file, excluding header, incl padding
  int hres;		//  40 horz resolutions pixels/meter
  int vres;		//  44
  int npalcolors;		//  48 No of colors in palette
  int nimportant;		//  52 No of important colors
  // raw b, g, r data here, dword aligned per scan line
};

int *readBmp(const char *filename, int *width, int *height)
{
#ifndef NDEBUG
  int n = 0;
#endif /* NDEBUG */
  struct bmphdr hdr;

  FILE *fp = fopen(filename, "rb");
  assert(fp);

  char magic[2];
  IF_DEBUG(n =) fread(&magic[0], 1, 2, fp);
  assert(n == 2 && magic[0] == 'B' && magic[1] == 'M');

  IF_DEBUG(n =) fread(&hdr, 1, sizeof(hdr), fp);
  assert(n == sizeof(hdr));

#define DEBUG 1
#ifdef DEBUG
  // Dump stuff out
  printf("   filesize = %d\n", hdr.filesize);	// total file size incl header
  printf("        as0 = %d\n", hdr.as0);
  printf("        as1 = %d\n", hdr.as1);
  printf("  bmpoffset = %d\n", hdr.bmpoffset);	// ofset of bmp data 
  printf("headerbytes = %d\n", hdr.headerbytes);	// bytes in header from this point (40 actually)
  printf("      width = %d\n", hdr.width);
  printf("     height = %d\n", hdr.height);
  printf("    nplanes = %d\n", hdr.nplanes);	// no of color planes
  printf("        bpp = %d\n", hdr.bpp);	// bits/pixel
  printf("compression = %d\n", hdr.compression);	// BI_RGB = 0 = no compression
  printf("    sizeraw = %d\n", hdr.sizeraw);	// size of raw bmp file, excluding header, incl padding
  printf("       hres = %d\n", hdr.hres);	// horz resolutions pixels/meter
  printf("       vres = %d\n", hdr.vres);
  printf(" npalcolors = %d\n", hdr.npalcolors);	// No of colors in palette
  printf(" nimportant = %d\n", hdr.nimportant);	// No of important colors
#endif
  assert(hdr.width > 0 && hdr.height > 0 && hdr.nplanes == 1 && hdr.compression == 0);

  int *rgb32 = (int *) malloc(hdr.width * hdr.height * sizeof(int));
  assert(rgb32);
  int x, y;

  int *dst = rgb32;
  for (y = 0; y < hdr.height; y++) {
    for (x = 0; x < hdr.width; x++) {
      assert(!feof(fp));
      int b = (getc(fp) & 0x0ff);
      int g = (getc(fp) & 0x0ff);
      int r = (getc(fp) & 0x0ff);
      *dst++ = (r | (g << 8) | (b << 16) | 0xff000000);	/* abgr */
    }
    while (x & 3) {
      getc(fp);
      x++;
    }		// each scanline padded to dword
    // printf("read row %d\n", y);
    // fflush(stdout);
  }
  fclose(fp);
  *width = hdr.width;
  *height = hdr.height;
  return rgb32;
}

void writeBmp(const int *data, int width, int height, const char *filename)
{
#ifndef NDEBUG
  int n = 0;
#endif /* NDEBUG */
  int x, y;

  FILE *fp = fopen(filename, "wb");
  assert(fp);

  char *raw = (char *) malloc(width * height * sizeof(int));	// at most
  assert(raw);
  char *p = raw;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      int c = *data++;
      *p++ = ((c >> 16) & 0x0ff);
      *p++ = ((c >> 8) & 0x0ff);
      *p++ = ((c >> 0) & 0x0ff);
    }
    while (x & 3) {
      *p++ = 0;
      x++;
    }		// pad to dword
  }
  int sizeraw = p - raw;
  int scanline = (width * 3 + 3) & ~3;
  assert(sizeraw == scanline * height);

  struct bmphdr hdr;

  hdr.filesize = scanline * height + sizeof(hdr) + 2;
  hdr.as0 = 0;
  hdr.as1 = 0;
  hdr.bmpoffset = sizeof(hdr) + 2;
  hdr.headerbytes = 40;
  hdr.width = width;
  hdr.height = height;
  hdr.nplanes = 1;
  hdr.bpp = 24;
  hdr.compression = 0;
  hdr.sizeraw = sizeraw;
  hdr.hres = 0;		// 2834;
  hdr.vres = 0;		// 2834;
  hdr.npalcolors = 0;
  hdr.nimportant = 0;

  /* Now write bmp file */
  char magic[2] = { 'B', 'M' };
  IF_DEBUG(n =) fwrite(&magic[0], 1, 2, fp);
  assert(n == 2);

  IF_DEBUG(n =) fwrite(&hdr, 1, sizeof(hdr), fp);
  assert(n == sizeof(hdr));

  IF_DEBUG(n =) fwrite(raw, 1, hdr.sizeraw, fp);
  assert(n == hdr.sizeraw);

  fclose(fp);
  free(raw);

#ifdef DEBUG
  printf("Write bmp file %s\n", filename);
#endif
}

char*
readFulsimDump(const char *name, size_t *size)
{
  char *raw = NULL, *dump = NULL;
  size_t i, sz;
  int w, h;
  if ((raw = (char*) readBmp(name, &w, &h)) == NULL)
    return NULL;
  sz = w * h;
  dump = (char*) malloc(sz);
  assert(dump);
  for (i = 0; i < sz; ++i)
    dump[i] = raw[4*i];
  free(raw);
  if (size)
    *size = sz;
  return dump;
}

char *do_kiss_path(const char *file, cl_device_id device)
{
  cl_int ver;
  const char *sub_path = NULL;
  char *ker_path = NULL;
  const char *kiss_path = getenv("OCL_KERNEL_PATH");
  size_t sz = strlen(file);

  if (device == NULL)
    sub_path = "";
  else {
    if (clIntelGetGenVersion(device, &ver) != CL_SUCCESS)
      clpanic("Unable to get Gen version", -1);
#if 0
    if (ver == 6)
      sub_path = "gen6/";
    else if (ver == 7)
      sub_path = "gen7/";
    else if (ver == 75)
      sub_path = "gen75/";
    else
      clpanic("unknow gen device", -1);
#else

    sub_path = "";
#endif
  }

  if (kiss_path == NULL)
    clpanic("set OCL_KERNEL_PATH. This is where the kiss kernels are", -1);
  sz += strlen(kiss_path) + strlen(sub_path) + 2; /* +1 for end of string, +1 for '/' */
  if ((ker_path = malloc(sz)) == NULL)
    clpanic("Allocation failed", -1);
  sprintf(ker_path, "%s/%s%s", kiss_path, sub_path, file);
  return ker_path;
}

