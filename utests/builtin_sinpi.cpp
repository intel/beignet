#include <cmath>
#include "utest_helper.hpp"

static int as_int(float x) {
  union {float f; int i;} u;
  u.f = x;
  return u.i;
}

static float sinpi(float x) {
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
  float y, z;
  int n = 0, ix;
  const float pi = 3.1415927410e+00f;

  ix = as_int(x) & 0x7fffffff;

  if (ix < 0x3e800000)
    return sinf(pi * x);
  y = -x;
  z = floorf(y);
  if (z != y) {
    y *= 0.5f;
    y = 2.f * (y - floorf(y));
    n = y * 4.f;
  } else {
    if (ix >= 0x4b800000) {
      y = 0;
      n = 0;
    } else {
      if (ix < 0x4b000000)
        z = y + 8.3886080000e+06f;
      int n = as_int(z);
      n &= 1;
      y = n;
      n <<= 2;
    }
  }
  switch (n) {
  case 0:
    y = sinf(pi * y);
    break;
  case 1:
  case 2:
    y = cosf(pi * ((float) 0.5 - y));
    break;
  case 3:
  case 4:
    y = sinf(pi * (1.f - y));
    break;
  case 5:
  case 6:
    y = -cosf(pi * (y - (float) 1.5));
    break;
  default:
    y = sinf(pi * (y - (float) 2.0));
    break;
  }
  return -y;
}

void builtin_sinpi(void)
{
  const int n = 1024;
  float src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("builtin_sinpi");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int j = 0; j < 1000; j ++) {
    OCL_MAP_BUFFER(0);
    for (int i = 0; i < n; ++i) {
      src[i] = ((float*)buf_data[0])[i] = (j*n + i) * 0.01f;
    }
    OCL_UNMAP_BUFFER(0);

    OCL_NDRANGE(1);

    OCL_MAP_BUFFER(1);
    float *dst = (float*)buf_data[1];
    for (int i = 0; i < n; ++i) {
      float cpu = sinpi(src[i]);
      OCL_ASSERT (fabsf(cpu - dst[i]) < 1e-4);
    }
    OCL_UNMAP_BUFFER(1);
  }
}

MAKE_UTEST_FROM_FUNCTION(builtin_sinpi);
