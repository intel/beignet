/*
 * Copyright © 2012 - 2014 Intel Corporation
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
#ifndef __OCL_FLOAT_H__
#define __OCL_FLOAT_H__

/////////////////////////////////////////////////////////////////////////////
// OpenCL floating-point macros and pragmas
/////////////////////////////////////////////////////////////////////////////
#define FLT_DIG 6
#define FLT_MANT_DIG 24
#define FLT_MAX_10_EXP +38
#define FLT_MAX_EXP +128
#define FLT_MIN_10_EXP -37
#define FLT_MIN_EXP -125
#define FLT_RADIX 2
#define FLT_ONE 1.0000000000e+00         /* 0x3F800000 */
#define FLT_MAX 0x1.fffffep127f
#define FLT_MIN 0x1.0p-126f
#define FLT_EPSILON 0x1.0p-23f

#define MAXFLOAT     3.40282347e38F
INLINE_OVERLOADABLE float __ocl_inff(void) {
  union { uint u; float f; } u;
  u.u = 0x7F800000;
  return u.f;
}
INLINE_OVERLOADABLE float __ocl_nanf(void) {
  union { uint u; float f; } u;
  u.u = 0x7F800001;
  return u.f;
}
typedef union
{
  float value;
  uint  word;
} float_shape_type;

/* Get a 32 bit int from a float.  */
#ifndef GEN_OCL_GET_FLOAT_WORD
# define GEN_OCL_GET_FLOAT_WORD(i,d)  \
do {                                  \
  float_shape_type gf_u;              \
  gf_u.value = (d);                   \
  (i) = gf_u.word;                    \
} while (0)
#endif
/* Set a float from a 32 bit int.  */
#ifndef GEN_OCL_SET_FLOAT_WORD
# define GEN_OCL_SET_FLOAT_WORD(d,i)  \
do {                                  \
  float_shape_type sf_u;              \
  sf_u.word = (i);                    \
  (d) = sf_u.value;                   \
} while (0)
#endif

INLINE_OVERLOADABLE int __ocl_finitef (float x){
  unsigned ix;
  GEN_OCL_GET_FLOAT_WORD (ix, x);
  return (ix & 0x7fffffff) < 0x7f800000;
}

#define HUGE_VALF    (__ocl_inff())
#define INFINITY     (__ocl_inff())
#define NAN          (__ocl_nanf())
#define M_E_F        2.718281828459045F
#define M_LOG2E_F    1.4426950408889634F
#define M_LOG10E_F   0.43429448190325176F
#define M_LN2_F      0.6931471805599453F
#define M_LN10_F     2.302585092994046F
#define M_PI_F       3.141592653589793F
#define M_PI_2_F     1.5707963267948966F
#define M_PI_4_F     0.7853981633974483F
#define M_1_PI_F     0.3183098861837907F
#define M_2_PI_F     0.6366197723675814F
#define M_180_PI_F   57.295779513082321F
#define M_2_SQRTPI_F 1.1283791670955126F
#define M_SQRT2_F    1.4142135623730951F
#define M_SQRT1_2_F  0.7071067811865476F
#define FP_ILOGB0    (-0x7FFFFFFF-1)
#define FP_ILOGBNAN  FP_ILOGB0

#endif /* __OCL_FLOAT_H__ */
