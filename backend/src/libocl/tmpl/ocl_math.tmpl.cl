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
#include "ocl_math.h"
#include "ocl_float.h"
#include "ocl_relational.h"
#include "ocl_common.h"
#include "ocl_integer.h"
#include "ocl_convert.h"

extern constant int __ocl_math_fastpath_flag;

CONST float __gen_ocl_fabs(float x) __asm("llvm.fabs" ".f32");
CONST float __gen_ocl_sin(float x) __asm("llvm.sin" ".f32");
CONST float __gen_ocl_cos(float x) __asm("llvm.cos" ".f32");
CONST float __gen_ocl_sqrt(float x) __asm("llvm.sqrt" ".f32");
PURE CONST float __gen_ocl_rsqrt(float x);
CONST float __gen_ocl_log(float x) __asm("llvm.log2" ".f32");
CONST float __gen_ocl_exp(float x) __asm("llvm.exp2" ".f32");
PURE CONST float __gen_ocl_pow(float x, float y) __asm("llvm.pow" ".f32");
PURE CONST float __gen_ocl_rcp(float x);
CONST float __gen_ocl_rndz(float x) __asm("llvm.trunc" ".f32");
CONST float __gen_ocl_rnde(float x) __asm("llvm.rint" ".f32");
CONST float __gen_ocl_rndu(float x) __asm("llvm.ceil" ".f32");
CONST float __gen_ocl_rndd(float x) __asm("llvm.floor" ".f32");

OVERLOADABLE float __gen_ocl_internal_fastpath_sincos (float x, __global float *cosval) {
    *cosval = native_cos(x);
    return native_sin(x);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_sincos (float x, __local float *cosval) {
    *cosval = native_cos(x);
    return native_sin(x);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_sincos (float x, __private float *cosval) {
    *cosval = native_cos(x);
    return native_sin(x);
}

OVERLOADABLE float __kernel_sinf_12(float x)
{
  /* copied from fdlibm */
  const float
  S1  = -1.6666667163e-01, /* 0xbe2aaaab */
  S2  =  8.3333337680e-03, /* 0x3c088889 */
  S3  = -1.9841270114e-04, /* 0xb9500d01 */
  S4  =  2.7557314297e-06; /* 0x3638ef1b */
  float z,r,v;
  z =  x*x;
  v =  z*x;
  r = mad(z, mad(z, mad(z, S4, S3), S2), S1);

  return mad(v, r, x);
}

float __kernel_cosf_12(float x, float y)
{
  /* copied from fdlibm */
  const float
  one =  1.0000000000e+00, /* 0x3f800000 */
  C1  =  4.1666667908e-02, /* 0x3d2aaaab */
  C2  = -1.3888889225e-03, /* 0xbab60b61 */
  C3  =  2.4801587642e-05; /* 0x37d00d01 */
  float a,hz,z,r,qx;
  int ix;
  GEN_OCL_GET_FLOAT_WORD(ix,x);
  ix &= 0x7fffffff;     /* ix = |x|'s high word*/
  z  = x*x;
  r = z * mad(z, mad(z, C3, C2), C1);

  if(ix < 0x3e99999a)       /* if |x| < 0.3 */
      return one - ((float)0.5*z - (z*r - x*y));
  else {
      GEN_OCL_SET_FLOAT_WORD(qx,ix-0x01000000); /* x/4 */
      hz = (float)0.5*z-qx;
      a  = one-qx;
      return a - (hz - (z*r-x*y));
  }
}

OVERLOADABLE float __gen_ocl_internal_floor_12(float x) { return __gen_ocl_rndd(x); }
OVERLOADABLE float __gen_ocl_internal_sinpi_12(float x) {
  float sign = 1.0f;
  int ix;
  if(isinf(x)) return NAN;
  if(x < 0.0f) { x = -x; sign = -1.0f; }
  GEN_OCL_GET_FLOAT_WORD(ix, x);
  if(x> 0x1.0p24) return 0.0f;
  float m = __gen_ocl_internal_floor_12(x);
  ix = (int)m;
  m = x-m;
  if((ix&0x1) != 0) m+=1.0f;
    ix = __gen_ocl_internal_floor_12(m*4.0f);

  switch(ix) {
   case 0:
    return sign*__kernel_sinf_12(m*M_PI_F);
   case 1:
   case 2:
    return sign*__kernel_cosf_12((m-0.5f)*M_PI_F, 0.0f);
   case 3:
   case 4:
    return -sign*__kernel_sinf_12((m-1.0f)*M_PI_F);
   case 5:
   case 6:
    return -sign*__kernel_cosf_12((m-1.5f)*M_PI_F, 0.0f);
   default:
    return -sign*__kernel_sinf_12((2.0f-m)*M_PI_F);
   }

}

#define BODY \
  if (isnan(x)) { \
    *p = x; \
    return x; \
  } \
  *p = __gen_ocl_rndd(x); \
  if (isinf(x)) { \
    return x > 0 ? +0. : -0.; \
  } \
  return min(x - *p, 0x1.FFFFFep-1F);
OVERLOADABLE float fract(float x, global float *p) { BODY; }
OVERLOADABLE float fract(float x, local float *p) { BODY; }
OVERLOADABLE float fract(float x, private float *p) { BODY; }
#undef BODY

OVERLOADABLE half fract(half x, global half *p) {
  float _x = (float)x;
  float _p;
  half ret = (half)fract(_x, &_p);
  *p = (half)_p;
  return ret;
}
OVERLOADABLE half fract(half x, local half *p) {
  float _x = (float)x;
  float _p;
  half ret = (half)fract(_x, &_p);
  *p = (half)_p;
  return ret;
}
OVERLOADABLE half fract(half x, private half *p) {
  float _x = (float)x;
  float _p;
  half ret = (half)fract(_x, &_p);
  *p = (half)_p;
  return ret;
}

#define BODY \
  if (isnan(x) || isinf(x)) { \
    *exp = 0; \
    return x; \
  } \
  uint u = as_uint(x); \
  uint a = u & 0x7FFFFFFFu; \
  if (a == 0) { \
    *exp = 0; \
    return x; \
  } \
  if (a >= 0x800000) { \
    *exp = (a >> 23) - 126; \
    return as_float((u & (0x807FFFFFu)) | 0x3F000000); \
  } \
  int e = -126; \
  while (a < 0x400000) { \
    e --; \
    a <<= 1; \
  } \
  a <<= 1; \
  *exp = e; \
  return as_float((a & (0x807FFFFFu)) | (u & 0x80000000u) | 0x3F000000);
OVERLOADABLE float frexp(float x, global int *exp) { BODY; }
OVERLOADABLE float frexp(float x, local int *exp) { BODY; }
OVERLOADABLE float frexp(float x, private int *exp) { BODY; }
#undef BODY

OVERLOADABLE half frexp(half x, global int *exp) {
  float _x = (float)x;
  return (half)frexp(_x, exp);
}
OVERLOADABLE half frexp(half x, local int *exp) {
  float _x = (float)x;
  return (half)frexp(_x, exp);
}
OVERLOADABLE half frexp(half x, private int *exp) {
  float _x = (float)x;
  return (half)frexp(_x, exp);
}

#define BODY \
  uint hx = as_uint(x), ix = hx & 0x7FFFFFFF; \
  if (ix > 0x7F800000) { \
    *i = nan(0u); \
    return nan(0u); \
  } \
  if (ix == 0x7F800000) { \
    *i = x; \
    return as_float(hx & 0x80000000u); \
  } \
  *i = __gen_ocl_rndz(x); \
  return x - *i;
OVERLOADABLE float modf(float x, global float *i) { BODY; }
OVERLOADABLE float modf(float x, local float *i) { BODY; }
OVERLOADABLE float modf(float x, private float *i) { BODY; }
#undef BODY

OVERLOADABLE half modf(half x, global half *i) {
  float _x = (float)x;
  float _i;
  half ret = (half)modf(_x, &_i);
  *i = (half)_i;
  return ret;
}
OVERLOADABLE half modf(half x, local half *i) {
  float _x = (float)x;
  float _i;
  half ret = (half)modf(_x, &_i);
  *i = (half)_i;
  return ret;
}
OVERLOADABLE half modf(half x, private half *i) {
  float _x = (float)x;
  float _i;
  half ret = (half)modf(_x, &_i);
  *i = (half)_i;
  return ret;
}

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
#define BODY \
    const float  \
        zero=  0.,  \
        one =  1.0000000000e+00,  \
        pi  =  3.1415927410e+00,  \
        a0  =  7.7215664089e-02,  \
        a1  =  3.2246702909e-01,  \
        a2  =  6.7352302372e-02,  \
        a3  =  2.0580807701e-02,  \
        a4  =  7.3855509982e-03,  \
        a5  =  2.8905137442e-03,  \
        a6  =  1.1927076848e-03,  \
        a7  =  5.1006977446e-04,  \
        a8  =  2.2086278477e-04,  \
        a9  =  1.0801156895e-04,  \
        a10 =  2.5214456400e-05,  \
        a11 =  4.4864096708e-05,  \
        tc  =  1.4616321325e+00,  \
        tf  = -1.2148628384e-01,  \
        tt  =  6.6971006518e-09,  \
        t0  =  4.8383611441e-01,  \
        t1  = -1.4758771658e-01,  \
        t2  =  6.4624942839e-02,  \
        t3  = -3.2788541168e-02,  \
        t4  =  1.7970675603e-02,  \
        t5  = -1.0314224288e-02,  \
        t6  =  6.1005386524e-03,  \
        t7  = -3.6845202558e-03,  \
        t8  =  2.2596477065e-03,  \
        t9  = -1.4034647029e-03,  \
        t10 =  8.8108185446e-04,  \
        t11 = -5.3859531181e-04,  \
        t12 =  3.1563205994e-04,  \
        t13 = -3.1275415677e-04,  \
        t14 =  3.3552918467e-04,  \
        u0  = -7.7215664089e-02,  \
        u1  =  6.3282704353e-01,  \
        u2  =  1.4549225569e+00,  \
        u3  =  9.7771751881e-01,  \
        u4  =  2.2896373272e-01,  \
        u5  =  1.3381091878e-02,  \
        v1  =  2.4559779167e+00,  \
        v2  =  2.1284897327e+00,  \
        v3  =  7.6928514242e-01,  \
        v4  =  1.0422264785e-01,  \
        v5  =  3.2170924824e-03,  \
        s0  = -7.7215664089e-02,  \
        s1  =  2.1498242021e-01,  \
        s2  =  3.2577878237e-01,  \
        s3  =  1.4635047317e-01,  \
        s4  =  2.6642270386e-02,  \
        s5  =  1.8402845599e-03,  \
        s6  =  3.1947532989e-05,  \
        r1  =  1.3920053244e+00,  \
        r2  =  7.2193557024e-01,  \
        r3  =  1.7193385959e-01,  \
        r4  =  1.8645919859e-02,  \
        r5  =  7.7794247773e-04,  \
        r6  =  7.3266842264e-06,  \
        w0  =  4.1893854737e-01,  \
        w1  =  8.3333335817e-02,  \
        w2  = -2.7777778450e-03,  \
        w3  =  7.9365057172e-04,  \
        w4  = -5.9518753551e-04,  \
        w5  =  8.3633989561e-04,  \
        w6  = -1.6309292987e-03;  \
	float t, y, z, nadj, p, p1, p2, p3, q, r, w;  \
	int i, hx, ix;  \
	nadj = 0;  \
	hx = *(int *)&x;  \
	*signgamp = 1;  \
	ix = hx & 0x7fffffff;  \
	if (ix >= 0x7f800000)  \
		return x * x;  \
	if (ix == 0)  \
		return ((x + one) / zero);  \
	if (ix < 0x1c800000) {  \
		if (hx < 0) {  \
			*signgamp = -1;  \
			return -native_log(-x);  \
		} else  \
			return -native_log(x);  \
	}  \
	if (hx < 0) {  \
		if (ix >= 0x4b000000)  \
			return ((-x) / zero);  \
		t = __gen_ocl_internal_sinpi_12(x);  \
		if (t == zero)  \
			return ((-x) / zero);  \
		nadj = native_log(pi / __gen_ocl_fabs(t * x));  \
		if (t < zero)  \
			*signgamp = -1;  \
		x = -x;  \
	}  \
	if (ix == 0x3f800000 || ix == 0x40000000)  \
		r = 0;  \
	else if (ix < 0x40000000) {  \
		if (ix <= 0x3f666666) {  \
			r = -native_log(x);  \
			if (ix >= 0x3f3b4a20) {  \
				y = one - x;  \
				i = 0;  \
			} else if (ix >= 0x3e6d3308) {  \
				y = x - (tc - one);  \
				i = 1;  \
			} else {  \
				y = x;  \
				i = 2;  \
			}  \
		} else {  \
			r = zero;  \
			if (ix >= 0x3fdda618) {  \
				y = (float) 2.0 - x;  \
				i = 0;  \
			}  \
			else if (ix >= 0x3F9da620) {  \
				y = x - tc;  \
				i = 1;  \
			}  \
			else {  \
				y = x - one;  \
				i = 2;  \
			}  \
		}  \
		switch (i) {  \
		case 0:  \
			z = y * y;  \
			p1 = mad(z, mad(z, mad(z, mad(z, mad(z, a10, a8), a6), a4), a2), a0);	\
			p2 = z * mad(z, mad(z, mad(z, mad(z, mad(z, a11, a9), a7), a5), a3), a1);	\
			p = mad(y, p1, p2);	\
			r = r - mad(y, 0.5f, -p);	\
			break;  \
		case 1:  \
			z = y * y;  \
			w = z * y;  \
			p1 = mad(w, mad(w, mad(w, mad(w, t12, t9), t6), t3), t0);	\
			p2 = mad(w, mad(w, mad(w, mad(w, t13, t10), t7), t4), t1);	\
			p3 = mad(w, mad(w, mad(w, mad(w, t14, t11), t8), t5), t2);	\
			p = z * p1 + mad(w, mad(y, p3, p2), -tt);	\
			r += (tf + p);  \
			break;  \
		case 2:  \
			p1 = y * mad(y, mad(y, mad(y, mad(y, mad(y, u5, u4), u3), u2), u1), u0);	\
			p2 = mad(y, mad(y, mad(y, mad(y, mad(y, v5, v4), v3), v2), v1), one);	\
			r = r + mad(y, -0.5f, p1 / p2);	\
		}  \
	} else if (ix < 0x41000000) {  \
		i = (int) x;  \
		t = zero;  \
		y = x - (float) i;  \
		p = y * mad(y, mad(y, mad(y, mad(y, mad(y, mad(y, s6, s5), s4), s3), s2), s1), s0);		\
		q = mad(y, mad(y, mad(y, mad(y, mad(y, mad(y, r6, r5), r4), r3), r2), r1), one);	\
		r = mad(y, 0.5f, p / q);	\
		z = one;  \
		switch (i) {  \
		case 7:  \
			z *= (y + (float) 6.0);  \
		case 6:  \
			z *= (y + (float) 5.0);  \
		case 5:  \
			z *= (y + (float) 4.0);  \
		case 4:  \
			z *= (y + (float) 3.0);  \
		case 3:  \
			z *= (y + (float) 2.0);  \
			r += native_log(z);  \
			break;  \
		}  \
		  \
	} else if (ix < 0x5c800000) {  \
		t = native_log(x);  \
		z = one / x;  \
		y = z * z;  \
		w = mad(z, mad(y, mad(y, mad(y, mad(y, mad(y, w6, w5), w4), w3), w2), w1), w0);  \
		r = (x - .5f) * (t - one) + w;  \
	} else  \
		r = x * (native_log(x) - one);	\
	if (hx < 0)  \
		r = nadj - r;  \
	return r;
OVERLOADABLE float lgamma_r(float x, global int *signgamp) { BODY; }
OVERLOADABLE float lgamma_r(float x, local int *signgamp) { BODY; }
OVERLOADABLE float lgamma_r(float x, private int *signgamp) { BODY; }
#undef BODY

OVERLOADABLE half lgamma_r(half x, global int *signgamp) {
  float _x = (float)x;
  return (half)lgamma_r(_x, signgamp);
}
OVERLOADABLE half lgamma_r(half x, local int *signgamp) {
  float _x = (float)x;
  return (half)lgamma_r(_x, signgamp);
}
OVERLOADABLE half lgamma_r(half x, private int *signgamp) {
  float _x = (float)x;
  return (half)lgamma_r(_x, signgamp);
}

#define BODY \
  float Zero[2]; \
  int n,hx,hy,hz,ix,iy,sx,i,sy; \
  uint q,sxy; \
  Zero[0] = 0.0;Zero[1] = -0.0; \
  if (x == 0.0f) { x = 0.0f; }; \
  if (y == 0.0f) { y = 0.0f; }\
  GEN_OCL_GET_FLOAT_WORD(hx,x);GEN_OCL_GET_FLOAT_WORD(hy,y); \
  sxy = (hx ^ hy) & 0x80000000;sx = hx&0x80000000;sy = hy&0x80000000; \
  hx ^=sx; hy &= 0x7fffffff; \
  if (hx < 0x00800000)hx = 0;if (hy < 0x00800000)hy = 0; \
  if(hy==0||hx>=0x7f800000||hy>0x7f800000){ \
    *quo = 0;return NAN; \
  } \
  if( hy == 0x7F800000 || hx == 0 ) { \
    *quo = 0;return x; \
  } \
  if( hx == hy ) { \
    *quo = (x == y) ? 1 : -1; \
    return sx ? -0.0 : 0.0; \
  } \
  if(hx<hy) { \
    q = 0; \
    goto fixup; \
  } else if(hx==hy) { \
    *quo = (sxy ? -1 : 1); \
    return Zero[(uint)sx>>31]; \
  } \
  ix = (hx>>23)-127; \
  iy = (hy>>23)-127; \
  hx = 0x00800000|(0x007fffff&hx); \
  hy = 0x00800000|(0x007fffff&hy); \
  n = ix - iy; \
  q = 0; \
  while(n--) { \
    hz=hx-hy; \
    if(hz<0) hx = hx << 1; \
    else {hx = hz << 1; q++;} \
    q <<= 1; \
  } \
  hz=hx-hy; \
  if(hz>=0) {hx=hz;q++;} \
  if(hx==0) { \
    q &= 0x0000007f; \
    *quo = (sxy ? -q : q); \
    return Zero[(uint)sx>>31]; \
  } \
  while(hx<0x00800000) { \
    hx <<= 1;iy -= 1; \
  } \
  if(iy>= -126) { \
    hx = ((hx-0x00800000)|((iy+127)<<23)); \
  } else {\
    n = -126 - iy; \
    hx >>= n; \
  } \
fixup: \
  GEN_OCL_SET_FLOAT_WORD(x,hx); \
  if(hx<0x00800000){ \
    GEN_OCL_GET_FLOAT_WORD(hy,y); \
    hy &= 0x7fffffff; \
    if(hx+hx > hy ||(hx+hx==hy && (q & 1)))q++; \
    x = 0; \
  }else{ \
    y = __gen_ocl_fabs(y); \
    if (y < 0x1p-125f) { \
      if (x+x>y || (x+x==y && (q & 1))) { \
        q++;x-=y; \
      } \
    }else if (x>0.5f*y || (x==0.5f*y && (q & 1))) { \
      q++;x-=y; \
    } \
    GEN_OCL_GET_FLOAT_WORD(hx,x);GEN_OCL_SET_FLOAT_WORD(x,hx^sx); \
  } \
  int sign = sx==sy?0:1; \
  q &= 0x0000007f; \
  *quo = (sign ? -q : q); \
  return x;

OVERLOADABLE float remquo(float x, float y, global int *quo) {
	BODY;
}
OVERLOADABLE float remquo(float x, float y, local int *quo) { BODY; }
OVERLOADABLE float remquo(float x, float y, private int *quo) { BODY; }
#undef BODY

const __constant unsigned int two_over_pi12[] = {
0, 0, 0xA2F, 0x983, 0x6E4, 0xe44, 0x152, 0x9FC,
0x275, 0x7D1, 0xF53, 0x4DD, 0xC0D, 0xB62,
0x959, 0x93C, 0x439, 0x041, 0xFE5, 0x163,
};

int payne_hanek12(float x, float *y) {
  union { float f; unsigned u;} ieee;
  ieee.f = x;
  unsigned u = ieee.u;
  int k = ((u & 0x7f800000) >> 23)-127;
  int ma = (u & 0x7fffff) | 0x800000;
  unsigned  high, low;
  high = (ma & 0xfff000) >> 12;
  low = ma & 0xfff;

  // Two tune below macro, you need to fully understand the algorithm
#define CALC_BLOCKS 7
#define ZERO_BITS 2

  unsigned result[CALC_BLOCKS];

  // round down, note we need 2 bits integer precision
  int index = (k-23-2) < 0 ? (k-23-2-11)/12 : (k-23-2)/12;

  for (int i = 0; i < CALC_BLOCKS; i++) {
    result[i] =  low * two_over_pi12[index+i+ZERO_BITS] ;
    result[i] +=  high * two_over_pi12[index+i+1+ZERO_BITS];
  }

  for (int i = CALC_BLOCKS-1; i > 0; i--) {
    int temp = result[i] >> 12;
    result[i]  -= temp << 12;
    result[i-1] += temp;
  }
#undef CALC_BLOCKS
#undef ZERO_BITS

  // get number of integer digits in result[0], note we only consider 12 valid bits
  // and also it means the fraction digits in result[0] is (12-intDigit)

  int intDigit = index*(-12) + (k-23);

  // As the integer bits may be all included in result[0], and also maybe
  // some bits in result[0], and some in result[1]. So we merge succesive bits,
  // which makes easy coding.

  unsigned b0 = (result[0] << 12) | result[1];
  unsigned b1 = (result[2] << 12) | result[3];
  unsigned b2 = (result[4] << 12) | result[5];
  unsigned b3 = (result[6] << 12);

  unsigned intPart = b0 >> (24-intDigit);

  unsigned fract1 = ((b0 << intDigit) | (b1 >> (24-intDigit))) & 0xffffff;
  unsigned fract2 = ((b1 << intDigit) | (b2 >> (24-intDigit))) & 0xffffff;
  unsigned fract3 = ((b2 << intDigit) | (b3 >> (24-intDigit))) & 0xffffff;

  // larger than 0.5? which mean larger than pi/4, we need
  // transform from [0,pi/2] to [-pi/4, pi/4] through -(1.0-fract)
  int largerPiBy4 = ((fract1 & 0x800000) != 0);
  int sign = largerPiBy4 ? 1 : 0;
  intPart = largerPiBy4 ? (intPart+1) : intPart;

  fract1 = largerPiBy4 ? (fract1 ^ 0x00ffffff) : fract1;
  fract2 = largerPiBy4 ? (fract2 ^ 0x00ffffff) : fract2;
  fract3 = largerPiBy4 ? (fract3 ^ 0x00ffffff) : fract3;

  int leadingZero = (fract1 == 0);

  // +1 is for the hidden bit 1 in floating-point format
  int exponent = leadingZero ? -(24+1) : -(0+1);

  fract1 = leadingZero ? fract2 : fract1;
  fract2 = leadingZero ? fract3 : fract2;

  // fract1 may have leading zeros, add it
  int shift = clz(fract1)-8;
  exponent += -shift;

  float pio2 = 0x1.921fb6p+0;
  unsigned fdigit = ((fract1 << shift) | (fract2 >> (24-shift))) & 0xffffff;

  // we know that denormal number will not appear here
  ieee.u = (sign << 31) | ((exponent+127) << 23) | (fdigit & 0x7fffff);
  *y = ieee.f * pio2;
  return intPart;
}

int argumentReduceSmall12(float x, float * remainder) {
    float halfPi = 2.0f/3.14159265f;
    // pi/2 = 0.C90FDAA22168C234C4p+1;
    float halfPi_p1 = (float) 0xC908/0x1.0p15,
            halfPi_p2 = (float) 0x7DAA/0x1.0p27,
            halfPi_p3 = (float) 0x22168C/0x1.0p51,
            halfPi_p4 = (float) 0x234C4/0x1.0p71;

    uint iy = (uint)mad(halfPi, x, 0.5f);
    float y = (float)iy;
    float rem = mad(y, -halfPi_p1, x);
    rem = mad(y, -halfPi_p2, rem);
    rem = mad(y, -halfPi_p3, rem);
    *remainder = rem;

    return iy;
}

int __ieee754_rem_pio2f12(float x, float *y) {
  if (x < 2.5e2) {
    return argumentReduceSmall12(x, y);
  } else {
    return payne_hanek12(x, y);
  }
}

#define BODY \
    float y; \
    float na ; \
    uint n, ix; \
    float negative = x < 0.0f? -1.0f : 1.0f; \
    x = __gen_ocl_fabs(x); \
    na = x -x; \
    uint n0, n1; \
    float v; \
    n = __ieee754_rem_pio2f12(x,&y); \
    float s = __kernel_sinf_12(y); \
    float c = sqrt(fabs(mad(s, s, -1.0f))); \
    n0 = (n&0x1); \
    n1 = (n&0x2); \
    float ss = n1 - 1.0f; \
    v = (n0)?s:-c; \
    *cosval = mad(v, ss, na); \
    v = (n0)?c:s; \
    v = (n1)?-v:v; \
    return mad(v, negative, na);

OVERLOADABLE float sincos(float x, global float *cosval) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_sincos(x, cosval);
  BODY;
}
OVERLOADABLE float sincos(float x, local float *cosval) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_sincos(x, cosval);
  BODY;
}
OVERLOADABLE float sincos(float x, private float *cosval) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_sincos(x, cosval);
  BODY;
}
#undef BODY

OVERLOADABLE half remquo(half x, half y, global int *quo) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)remquo(_x, _y, quo);
}
OVERLOADABLE half remquo(half x, half y, local int *quo) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)remquo(_x, _y, quo);
}
OVERLOADABLE half remquo(half x, half y, private int *quo) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)remquo(_x, _y, quo);
}

OVERLOADABLE half sincos(half x, global half *cosval) {
  float _x = (float)x;
  float _cosval;
  half ret = (half)sincos(_x, &_cosval);
  *cosval = (half)_cosval;
  return ret;
}
OVERLOADABLE half sincos(half x, local half *cosval) {
  float _x = (float)x;
  float _cosval;
  half ret = (half)sincos(_x, &_cosval);
  *cosval = (half)_cosval;
  return ret;
}
OVERLOADABLE half sincos(half x, private half *cosval) {
  float _x = (float)x;
  float _cosval;
  half ret = (half)sincos(_x, &_cosval);
  *cosval = (half)_cosval;
  return ret;
}

//-----------------double -----------------------
INLINE int  __HI(double x){
    long x64 = as_long(x);
    int high = convert_int((x64 >> 32) & 0xFFFFFFFF);
    return high;
};

INLINE int  __LO(double x){
    long x64 = as_long(x);
    int low = convert_int(x64  & 0xFFFFFFFFUL);
    return low;
};

INLINE void  __setHigh(double *x, int val){
    long x64 = as_long(*x);
    long high = x64  & 0x00000000FFFFFFFF;
    high |= ((long)val << 32);
    *x = as_double(high);
};

INLINE void  __setLow(double *x, unsigned int val){
    long x64 = as_long(*x);
    long low = x64  & 0xFFFFFFFF00000000;
    low |= (long)val;
    *x = as_double(low);
};

OVERLOADABLE double fract(double x, global double *p)
{
    ulong ux = as_ulong(x);
    if((ux&DF_ABS_MASK) > DF_POSITIVE_INF)
    {
	*p = as_double(DF_POSITIVE_INF + 1);
	return as_double(DF_POSITIVE_INF + 1);
    }

    double ret = floor(x);
    *p =  ret;

    if(ret == as_double(DF_POSITIVE_INF))
	return 0.0;

    if(ret == as_double(DF_NEGTIVE_INF))
	return as_double(DF_SIGN_MASK);;

    return fmin(x -ret, 0x1.fffffffffffffp-1);
}

OVERLOADABLE double fract(double x, local double *p)
{
    ulong ux = as_ulong(x);
    if((ux&DF_ABS_MASK) > DF_POSITIVE_INF)
    {
	*p = as_double(DF_POSITIVE_INF + 1);
	return as_double(DF_POSITIVE_INF + 1);
    }

    double ret = floor(x);
    *p =  ret;

    if(ret == as_double(DF_POSITIVE_INF))
	return 0.0;

    if(ret == as_double(DF_NEGTIVE_INF))
	return as_double(DF_SIGN_MASK);;

    return fmin(x -ret, 0x1.fffffffffffffp-1);
}

OVERLOADABLE double fract(double x, private double *p)
{
    ulong ux = as_ulong(x);
    if((ux&DF_ABS_MASK) > DF_POSITIVE_INF)
    {
	*p = as_double(DF_POSITIVE_INF + 1);
	return as_double(DF_POSITIVE_INF + 1);
    }

    double ret = floor(x);
    *p =  ret;

    if(ret == as_double(DF_POSITIVE_INF))
	return 0.0;

    if(ret == as_double(DF_NEGTIVE_INF))
	return as_double(DF_SIGN_MASK);;

    return fmin(x -ret, 0x1.fffffffffffffp-1);
}

#define BODY \
double two54 =  1.80143985094819840000e+16; /* 0x43500000, 0x00000000 */ \
int  hx, ix, lx; \
hx = __HI(x); \
ix = 0x7fffffff&hx; \
lx = __LO(x); \
*exp = 0; \
if(ix>=0x7ff00000||((ix|lx)==0)) return x;  /* 0,inf,nan */ \
if (ix<0x00100000) {        /* subnormal */ \
    x *= two54; \
    hx = __HI(x); \
    ix = hx&0x7fffffff; \
    *exp = -54; \
} \
*exp += (ix>>20)-1022; \
hx = (hx&0x800fffff)|0x3fe00000; \
__setHigh(&x, hx); \
return x;

OVERLOADABLE double frexp(double x, global int *exp){BODY;}
OVERLOADABLE double frexp(double x, local int *exp){BODY;}
OVERLOADABLE double frexp(double x, private int *exp){BODY;}
#undef BODY

OVERLOADABLE double lgamma_r(double x, global int *signgamp)
{
	double two52=  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
	halfD=  5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
	one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
	pi  =  3.14159265358979311600e+00, /* 0x400921FB, 0x54442D18 */
	a0  =  7.72156649015328655494e-02, /* 0x3FB3C467, 0xE37DB0C8 */
	a1  =  3.22467033424113591611e-01, /* 0x3FD4A34C, 0xC4A60FAD */
	a2  =  6.73523010531292681824e-02, /* 0x3FB13E00, 0x1A5562A7 */
	a3  =  2.05808084325167332806e-02, /* 0x3F951322, 0xAC92547B */
	a4  =  7.38555086081402883957e-03, /* 0x3F7E404F, 0xB68FEFE8 */
	a5  =  2.89051383673415629091e-03, /* 0x3F67ADD8, 0xCCB7926B */
	a6  =  1.19270763183362067845e-03, /* 0x3F538A94, 0x116F3F5D */
	a7  =  5.10069792153511336608e-04, /* 0x3F40B6C6, 0x89B99C00 */
	a8  =  2.20862790713908385557e-04, /* 0x3F2CF2EC, 0xED10E54D */
	a9  =  1.08011567247583939954e-04, /* 0x3F1C5088, 0x987DFB07 */
	a10 =  2.52144565451257326939e-05, /* 0x3EFA7074, 0x428CFA52 */
	a11 =  4.48640949618915160150e-05, /* 0x3F07858E, 0x90A45837 */
	tc  =  1.46163214496836224576e+00, /* 0x3FF762D8, 0x6356BE3F */
	tf  = -1.21486290535849611461e-01, /* 0xBFBF19B9, 0xBCC38A42 */
	/* tt = -(tail of tf) */
	tt  = -3.63867699703950536541e-18, /* 0xBC50C7CA, 0xA48A971F */
	t0  =  4.83836122723810047042e-01, /* 0x3FDEF72B, 0xC8EE38A2 */
	t1  = -1.47587722994593911752e-01, /* 0xBFC2E427, 0x8DC6C509 */
	t2  =  6.46249402391333854778e-02, /* 0x3FB08B42, 0x94D5419B */
	t3  = -3.27885410759859649565e-02, /* 0xBFA0C9A8, 0xDF35B713 */
	t4  =  1.79706750811820387126e-02, /* 0x3F9266E7, 0x970AF9EC */
	t5  = -1.03142241298341437450e-02, /* 0xBF851F9F, 0xBA91EC6A */
	t6  =  6.10053870246291332635e-03, /* 0x3F78FCE0, 0xE370E344 */
	t7  = -3.68452016781138256760e-03, /* 0xBF6E2EFF, 0xB3E914D7 */
	t8  =  2.25964780900612472250e-03, /* 0x3F6282D3, 0x2E15C915 */
	t9  = -1.40346469989232843813e-03, /* 0xBF56FE8E, 0xBF2D1AF1 */
	t10 =  8.81081882437654011382e-04, /* 0x3F4CDF0C, 0xEF61A8E9 */
	t11 = -5.38595305356740546715e-04, /* 0xBF41A610, 0x9C73E0EC */
	t12 =  3.15632070903625950361e-04, /* 0x3F34AF6D, 0x6C0EBBF7 */
	t13 = -3.12754168375120860518e-04, /* 0xBF347F24, 0xECC38C38 */
	t14 =  3.35529192635519073543e-04, /* 0x3F35FD3E, 0xE8C2D3F4 */
	u0  = -7.72156649015328655494e-02, /* 0xBFB3C467, 0xE37DB0C8 */
	u1  =  6.32827064025093366517e-01, /* 0x3FE4401E, 0x8B005DFF */
	u2  =  1.45492250137234768737e+00, /* 0x3FF7475C, 0xD119BD6F */
	u3  =  9.77717527963372745603e-01, /* 0x3FEF4976, 0x44EA8450 */
	u4  =  2.28963728064692451092e-01, /* 0x3FCD4EAE, 0xF6010924 */
	u5  =  1.33810918536787660377e-02, /* 0x3F8B678B, 0xBF2BAB09 */
	v1  =  2.45597793713041134822e+00, /* 0x4003A5D7, 0xC2BD619C */
	v2  =  2.12848976379893395361e+00, /* 0x40010725, 0xA42B18F5 */
	v3  =  7.69285150456672783825e-01, /* 0x3FE89DFB, 0xE45050AF */
	v4  =  1.04222645593369134254e-01, /* 0x3FBAAE55, 0xD6537C88 */
	v5  =  3.21709242282423911810e-03, /* 0x3F6A5ABB, 0x57D0CF61 */
	s0  = -7.72156649015328655494e-02, /* 0xBFB3C467, 0xE37DB0C8 */
	s1  =  2.14982415960608852501e-01, /* 0x3FCB848B, 0x36E20878 */
	s2  =  3.25778796408930981787e-01, /* 0x3FD4D98F, 0x4F139F59 */
	s3  =  1.46350472652464452805e-01, /* 0x3FC2BB9C, 0xBEE5F2F7 */
	s4  =  2.66422703033638609560e-02, /* 0x3F9B481C, 0x7E939961 */
	s5  =  1.84028451407337715652e-03, /* 0x3F5E26B6, 0x7368F239 */
	s6  =  3.19475326584100867617e-05, /* 0x3F00BFEC, 0xDD17E945 */
	r1  =  1.39200533467621045958e+00, /* 0x3FF645A7, 0x62C4AB74 */
	r2  =  7.21935547567138069525e-01, /* 0x3FE71A18, 0x93D3DCDC */
	r3  =  1.71933865632803078993e-01, /* 0x3FC601ED, 0xCCFBDF27 */
	r4  =  1.86459191715652901344e-02, /* 0x3F9317EA, 0x742ED475 */
	r5  =  7.77942496381893596434e-04, /* 0x3F497DDA, 0xCA41A95B */
	r6  =  7.32668430744625636189e-06, /* 0x3EDEBAF7, 0xA5B38140 */
	w0  =  4.18938533204672725052e-01, /* 0x3FDACFE3, 0x90C97D69 */
	w1  =  8.33333333333329678849e-02, /* 0x3FB55555, 0x5555553B */
	w2  = -2.77777777728775536470e-03, /* 0xBF66C16C, 0x16B02E5C */
	w3  =  7.93650558643019558500e-04, /* 0x3F4A019F, 0x98CF38B6 */
	w4  = -5.95187557450339963135e-04, /* 0xBF4380CB, 0x8C0FE741 */
	w5  =  8.36339918996282139126e-04, /* 0x3F4B67BA, 0x4CDAD5D1 */
	w6  = -1.63092934096575273989e-03; /* 0xBF5AB89D, 0x0B9E43E4 */

	double zero=  0.00000000000000000000e+00;

	double t,y,z,nadj,p,p1,p2,p3,q,r,w;
	int i,hx,lx,ix;

	hx = __HI(x);
	lx = __LO(x);

	/* purge off +-inf, NaN, +-0, and negative arguments */
	*signgamp = 1;
	ix = hx&0x7fffffff;
	if(ix>=0x7ff00000) return x*x;
	if((ix|lx)==0) return one/zero;
	if(ix<0x3b900000) {	/* |x|<2**-70, return -log(|x|) */
		if(hx<0) {
			*signgamp = -1;
			return -log(-x);
		} else return -log(x);
	}
	if(hx<0) {
		if(ix>=0x43300000) 	/* |x|>=2**52, must be -integer */
		return one/zero;
		t = sinpi(x);
		if(t==zero) return one/zero; /* -integer */
		nadj = log(pi/fabs(t*x));
		if(t<zero) *signgamp = -1;
		x = -x;
	}

	/* purge off 1 and 2 */
	if((((ix-0x3ff00000)|lx)==0)||(((ix-0x40000000)|lx)==0)) r = 0;
	/* for x < 2.0 */
	else if(ix<0x40000000) {
		if(ix<=0x3feccccc) { 	/* lgamma(x) = lgamma(x+1)-log(x) */
		r = -log(x);
		if(ix>=0x3FE76944) {y = one-x; i= 0;}
		else if(ix>=0x3FCDA661) {y= x-(tc-one); i=1;}
		else {y = x; i=2;}
		} else {
		r = zero;
			if(ix>=0x3FFBB4C3) {y=2.0-x;i=0;} /* [1.7316,2] */
			else if(ix>=0x3FF3B4C4) {y=x-tc;i=1;} /* [1.23,1.73] */
		else {y=x-one;i=2;}
		}
		switch(i) {
		  case 0:
		z = y*y;
		p1 = a0+z*(a2+z*(a4+z*(a6+z*(a8+z*a10))));
		p2 = z*(a1+z*(a3+z*(a5+z*(a7+z*(a9+z*a11)))));
		p  = y*p1+p2;
		r  += (p-0.5*y); break;
		  case 1:
		z = y*y;
		w = z*y;
		p1 = t0+w*(t3+w*(t6+w*(t9 +w*t12)));	/* parallel comp */
		p2 = t1+w*(t4+w*(t7+w*(t10+w*t13)));
		p3 = t2+w*(t5+w*(t8+w*(t11+w*t14)));
		p  = z*p1-(tt-w*(p2+y*p3));
		r += (tf + p); break;
		  case 2:
		p1 = y*(u0+y*(u1+y*(u2+y*(u3+y*(u4+y*u5)))));
		p2 = one+y*(v1+y*(v2+y*(v3+y*(v4+y*v5))));
		r += (-0.5*y + p1/p2);
		}
	}
	else if(ix<0x40200000) { 			/* x < 8.0 */
		i = (int)x;
		t = zero;
		y = x-(double)i;
		p = y*(s0+y*(s1+y*(s2+y*(s3+y*(s4+y*(s5+y*s6))))));
		q = one+y*(r1+y*(r2+y*(r3+y*(r4+y*(r5+y*r6)))));
		r = halfD*y+p/q;
		z = one;	/* lgamma(1+s) = log(s) + lgamma(s) */
		switch(i) {
		case 7: z *= (y+6.0);	/* FALLTHRU */
		case 6: z *= (y+5.0);	/* FALLTHRU */
		case 5: z *= (y+4.0);	/* FALLTHRU */
		case 4: z *= (y+3.0);	/* FALLTHRU */
		case 3: z *= (y+2.0);	/* FALLTHRU */
			r += log(z); break;
		}
	/* 8.0 <= x < 2**58 */
	} else if (ix < 0x43900000) {
		t = log(x);
		z = one/x;
		y = z*z;
		w = w0+z*(w1+y*(w2+y*(w3+y*(w4+y*(w5+y*w6)))));
		r = (x-halfD)*(t-one)+w;
	} else
	/* 2**58 <= x <= inf */
		r =  x*(log(x)-one);
	if(hx<0) r = nadj - r;
	return r;
}

OVERLOADABLE double lgamma_r(double x, local int *signgamp)
{
	double two52=  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
	halfD=  5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
	one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
	pi  =  3.14159265358979311600e+00, /* 0x400921FB, 0x54442D18 */
	a0  =  7.72156649015328655494e-02, /* 0x3FB3C467, 0xE37DB0C8 */
	a1  =  3.22467033424113591611e-01, /* 0x3FD4A34C, 0xC4A60FAD */
	a2  =  6.73523010531292681824e-02, /* 0x3FB13E00, 0x1A5562A7 */
	a3  =  2.05808084325167332806e-02, /* 0x3F951322, 0xAC92547B */
	a4  =  7.38555086081402883957e-03, /* 0x3F7E404F, 0xB68FEFE8 */
	a5  =  2.89051383673415629091e-03, /* 0x3F67ADD8, 0xCCB7926B */
	a6  =  1.19270763183362067845e-03, /* 0x3F538A94, 0x116F3F5D */
	a7  =  5.10069792153511336608e-04, /* 0x3F40B6C6, 0x89B99C00 */
	a8  =  2.20862790713908385557e-04, /* 0x3F2CF2EC, 0xED10E54D */
	a9  =  1.08011567247583939954e-04, /* 0x3F1C5088, 0x987DFB07 */
	a10 =  2.52144565451257326939e-05, /* 0x3EFA7074, 0x428CFA52 */
	a11 =  4.48640949618915160150e-05, /* 0x3F07858E, 0x90A45837 */
	tc  =  1.46163214496836224576e+00, /* 0x3FF762D8, 0x6356BE3F */
	tf  = -1.21486290535849611461e-01, /* 0xBFBF19B9, 0xBCC38A42 */
	/* tt = -(tail of tf) */
	tt  = -3.63867699703950536541e-18, /* 0xBC50C7CA, 0xA48A971F */
	t0  =  4.83836122723810047042e-01, /* 0x3FDEF72B, 0xC8EE38A2 */
	t1  = -1.47587722994593911752e-01, /* 0xBFC2E427, 0x8DC6C509 */
	t2  =  6.46249402391333854778e-02, /* 0x3FB08B42, 0x94D5419B */
	t3  = -3.27885410759859649565e-02, /* 0xBFA0C9A8, 0xDF35B713 */
	t4  =  1.79706750811820387126e-02, /* 0x3F9266E7, 0x970AF9EC */
	t5  = -1.03142241298341437450e-02, /* 0xBF851F9F, 0xBA91EC6A */
	t6  =  6.10053870246291332635e-03, /* 0x3F78FCE0, 0xE370E344 */
	t7  = -3.68452016781138256760e-03, /* 0xBF6E2EFF, 0xB3E914D7 */
	t8  =  2.25964780900612472250e-03, /* 0x3F6282D3, 0x2E15C915 */
	t9  = -1.40346469989232843813e-03, /* 0xBF56FE8E, 0xBF2D1AF1 */
	t10 =  8.81081882437654011382e-04, /* 0x3F4CDF0C, 0xEF61A8E9 */
	t11 = -5.38595305356740546715e-04, /* 0xBF41A610, 0x9C73E0EC */
	t12 =  3.15632070903625950361e-04, /* 0x3F34AF6D, 0x6C0EBBF7 */
	t13 = -3.12754168375120860518e-04, /* 0xBF347F24, 0xECC38C38 */
	t14 =  3.35529192635519073543e-04, /* 0x3F35FD3E, 0xE8C2D3F4 */
	u0  = -7.72156649015328655494e-02, /* 0xBFB3C467, 0xE37DB0C8 */
	u1  =  6.32827064025093366517e-01, /* 0x3FE4401E, 0x8B005DFF */
	u2  =  1.45492250137234768737e+00, /* 0x3FF7475C, 0xD119BD6F */
	u3  =  9.77717527963372745603e-01, /* 0x3FEF4976, 0x44EA8450 */
	u4  =  2.28963728064692451092e-01, /* 0x3FCD4EAE, 0xF6010924 */
	u5  =  1.33810918536787660377e-02, /* 0x3F8B678B, 0xBF2BAB09 */
	v1  =  2.45597793713041134822e+00, /* 0x4003A5D7, 0xC2BD619C */
	v2  =  2.12848976379893395361e+00, /* 0x40010725, 0xA42B18F5 */
	v3  =  7.69285150456672783825e-01, /* 0x3FE89DFB, 0xE45050AF */
	v4  =  1.04222645593369134254e-01, /* 0x3FBAAE55, 0xD6537C88 */
	v5  =  3.21709242282423911810e-03, /* 0x3F6A5ABB, 0x57D0CF61 */
	s0  = -7.72156649015328655494e-02, /* 0xBFB3C467, 0xE37DB0C8 */
	s1  =  2.14982415960608852501e-01, /* 0x3FCB848B, 0x36E20878 */
	s2  =  3.25778796408930981787e-01, /* 0x3FD4D98F, 0x4F139F59 */
	s3  =  1.46350472652464452805e-01, /* 0x3FC2BB9C, 0xBEE5F2F7 */
	s4  =  2.66422703033638609560e-02, /* 0x3F9B481C, 0x7E939961 */
	s5  =  1.84028451407337715652e-03, /* 0x3F5E26B6, 0x7368F239 */
	s6  =  3.19475326584100867617e-05, /* 0x3F00BFEC, 0xDD17E945 */
	r1  =  1.39200533467621045958e+00, /* 0x3FF645A7, 0x62C4AB74 */
	r2  =  7.21935547567138069525e-01, /* 0x3FE71A18, 0x93D3DCDC */
	r3  =  1.71933865632803078993e-01, /* 0x3FC601ED, 0xCCFBDF27 */
	r4  =  1.86459191715652901344e-02, /* 0x3F9317EA, 0x742ED475 */
	r5  =  7.77942496381893596434e-04, /* 0x3F497DDA, 0xCA41A95B */
	r6  =  7.32668430744625636189e-06, /* 0x3EDEBAF7, 0xA5B38140 */
	w0  =  4.18938533204672725052e-01, /* 0x3FDACFE3, 0x90C97D69 */
	w1  =  8.33333333333329678849e-02, /* 0x3FB55555, 0x5555553B */
	w2  = -2.77777777728775536470e-03, /* 0xBF66C16C, 0x16B02E5C */
	w3  =  7.93650558643019558500e-04, /* 0x3F4A019F, 0x98CF38B6 */
	w4  = -5.95187557450339963135e-04, /* 0xBF4380CB, 0x8C0FE741 */
	w5  =  8.36339918996282139126e-04, /* 0x3F4B67BA, 0x4CDAD5D1 */
	w6  = -1.63092934096575273989e-03; /* 0xBF5AB89D, 0x0B9E43E4 */

	double zero=  0.00000000000000000000e+00;

	double t,y,z,nadj,p,p1,p2,p3,q,r,w;
	int i,hx,lx,ix;

	hx = __HI(x);
	lx = __LO(x);

	/* purge off +-inf, NaN, +-0, and negative arguments */
	*signgamp = 1;
	ix = hx&0x7fffffff;
	if(ix>=0x7ff00000) return x*x;
	if((ix|lx)==0) return one/zero;
	if(ix<0x3b900000) { /* |x|<2**-70, return -log(|x|) */
	    if(hx<0) {
	        *signgamp = -1;
	        return -log(-x);
	    } else return -log(x);
	}
	if(hx<0) {
	    if(ix>=0x43300000)  /* |x|>=2**52, must be -integer */
	    return one/zero;
	    t = sinpi(x);
	    if(t==zero) return one/zero; /* -integer */
	    nadj = log(pi/fabs(t*x));
	    if(t<zero) *signgamp = -1;
	    x = -x;
	}

	/* purge off 1 and 2 */
	if((((ix-0x3ff00000)|lx)==0)||(((ix-0x40000000)|lx)==0)) r = 0;
	/* for x < 2.0 */
	else if(ix<0x40000000) {
	    if(ix<=0x3feccccc) {    /* lgamma(x) = lgamma(x+1)-log(x) */
	    r = -log(x);
	    if(ix>=0x3FE76944) {y = one-x; i= 0;}
	    else if(ix>=0x3FCDA661) {y= x-(tc-one); i=1;}
	    else {y = x; i=2;}
	    } else {
	    r = zero;
	        if(ix>=0x3FFBB4C3) {y=2.0-x;i=0;} /* [1.7316,2] */
	        else if(ix>=0x3FF3B4C4) {y=x-tc;i=1;} /* [1.23,1.73] */
	    else {y=x-one;i=2;}
	    }
	    switch(i) {
	      case 0:
	    z = y*y;
	    p1 = a0+z*(a2+z*(a4+z*(a6+z*(a8+z*a10))));
	    p2 = z*(a1+z*(a3+z*(a5+z*(a7+z*(a9+z*a11)))));
	    p  = y*p1+p2;
	    r  += (p-0.5*y); break;
	      case 1:
	    z = y*y;
	    w = z*y;
	    p1 = t0+w*(t3+w*(t6+w*(t9 +w*t12)));    /* parallel comp */
	    p2 = t1+w*(t4+w*(t7+w*(t10+w*t13)));
	    p3 = t2+w*(t5+w*(t8+w*(t11+w*t14)));
	    p  = z*p1-(tt-w*(p2+y*p3));
	    r += (tf + p); break;
	      case 2:
	    p1 = y*(u0+y*(u1+y*(u2+y*(u3+y*(u4+y*u5)))));
	    p2 = one+y*(v1+y*(v2+y*(v3+y*(v4+y*v5))));
	    r += (-0.5*y + p1/p2);
	    }
	}
	else if(ix<0x40200000) {            /* x < 8.0 */
	    i = (int)x;
	    t = zero;
	    y = x-(double)i;
	    p = y*(s0+y*(s1+y*(s2+y*(s3+y*(s4+y*(s5+y*s6))))));
	    q = one+y*(r1+y*(r2+y*(r3+y*(r4+y*(r5+y*r6)))));
	    r = halfD*y+p/q;
	    z = one;    /* lgamma(1+s) = log(s) + lgamma(s) */
	    switch(i) {
	    case 7: z *= (y+6.0);   /* FALLTHRU */
	    case 6: z *= (y+5.0);   /* FALLTHRU */
	    case 5: z *= (y+4.0);   /* FALLTHRU */
	    case 4: z *= (y+3.0);   /* FALLTHRU */
	    case 3: z *= (y+2.0);   /* FALLTHRU */
	        r += log(z); break;
	    }
	/* 8.0 <= x < 2**58 */
	} else if (ix < 0x43900000) {
	    t = log(x);
	    z = one/x;
	    y = z*z;
	    w = w0+z*(w1+y*(w2+y*(w3+y*(w4+y*(w5+y*w6)))));
	    r = (x-halfD)*(t-one)+w;
	} else
	/* 2**58 <= x <= inf */
	    r =  x*(log(x)-one);
	if(hx<0) r = nadj - r;
	return r;
}


OVERLOADABLE double lgamma_r(double x, private int *signgamp)
{
	double two52=  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
	halfD=  5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
	one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
	pi  =  3.14159265358979311600e+00, /* 0x400921FB, 0x54442D18 */
	a0  =  7.72156649015328655494e-02, /* 0x3FB3C467, 0xE37DB0C8 */
	a1  =  3.22467033424113591611e-01, /* 0x3FD4A34C, 0xC4A60FAD */
	a2  =  6.73523010531292681824e-02, /* 0x3FB13E00, 0x1A5562A7 */
	a3  =  2.05808084325167332806e-02, /* 0x3F951322, 0xAC92547B */
	a4  =  7.38555086081402883957e-03, /* 0x3F7E404F, 0xB68FEFE8 */
	a5  =  2.89051383673415629091e-03, /* 0x3F67ADD8, 0xCCB7926B */
	a6  =  1.19270763183362067845e-03, /* 0x3F538A94, 0x116F3F5D */
	a7  =  5.10069792153511336608e-04, /* 0x3F40B6C6, 0x89B99C00 */
	a8  =  2.20862790713908385557e-04, /* 0x3F2CF2EC, 0xED10E54D */
	a9  =  1.08011567247583939954e-04, /* 0x3F1C5088, 0x987DFB07 */
	a10 =  2.52144565451257326939e-05, /* 0x3EFA7074, 0x428CFA52 */
	a11 =  4.48640949618915160150e-05, /* 0x3F07858E, 0x90A45837 */
	tc  =  1.46163214496836224576e+00, /* 0x3FF762D8, 0x6356BE3F */
	tf  = -1.21486290535849611461e-01, /* 0xBFBF19B9, 0xBCC38A42 */
	/* tt = -(tail of tf) */
	tt  = -3.63867699703950536541e-18, /* 0xBC50C7CA, 0xA48A971F */
	t0  =  4.83836122723810047042e-01, /* 0x3FDEF72B, 0xC8EE38A2 */
	t1  = -1.47587722994593911752e-01, /* 0xBFC2E427, 0x8DC6C509 */
	t2  =  6.46249402391333854778e-02, /* 0x3FB08B42, 0x94D5419B */
	t3  = -3.27885410759859649565e-02, /* 0xBFA0C9A8, 0xDF35B713 */
	t4  =  1.79706750811820387126e-02, /* 0x3F9266E7, 0x970AF9EC */
	t5  = -1.03142241298341437450e-02, /* 0xBF851F9F, 0xBA91EC6A */
	t6  =  6.10053870246291332635e-03, /* 0x3F78FCE0, 0xE370E344 */
	t7  = -3.68452016781138256760e-03, /* 0xBF6E2EFF, 0xB3E914D7 */
	t8  =  2.25964780900612472250e-03, /* 0x3F6282D3, 0x2E15C915 */
	t9  = -1.40346469989232843813e-03, /* 0xBF56FE8E, 0xBF2D1AF1 */
	t10 =  8.81081882437654011382e-04, /* 0x3F4CDF0C, 0xEF61A8E9 */
	t11 = -5.38595305356740546715e-04, /* 0xBF41A610, 0x9C73E0EC */
	t12 =  3.15632070903625950361e-04, /* 0x3F34AF6D, 0x6C0EBBF7 */
	t13 = -3.12754168375120860518e-04, /* 0xBF347F24, 0xECC38C38 */
	t14 =  3.35529192635519073543e-04, /* 0x3F35FD3E, 0xE8C2D3F4 */
	u0  = -7.72156649015328655494e-02, /* 0xBFB3C467, 0xE37DB0C8 */
	u1  =  6.32827064025093366517e-01, /* 0x3FE4401E, 0x8B005DFF */
	u2  =  1.45492250137234768737e+00, /* 0x3FF7475C, 0xD119BD6F */
	u3  =  9.77717527963372745603e-01, /* 0x3FEF4976, 0x44EA8450 */
	u4  =  2.28963728064692451092e-01, /* 0x3FCD4EAE, 0xF6010924 */
	u5  =  1.33810918536787660377e-02, /* 0x3F8B678B, 0xBF2BAB09 */
	v1  =  2.45597793713041134822e+00, /* 0x4003A5D7, 0xC2BD619C */
	v2  =  2.12848976379893395361e+00, /* 0x40010725, 0xA42B18F5 */
	v3  =  7.69285150456672783825e-01, /* 0x3FE89DFB, 0xE45050AF */
	v4  =  1.04222645593369134254e-01, /* 0x3FBAAE55, 0xD6537C88 */
	v5  =  3.21709242282423911810e-03, /* 0x3F6A5ABB, 0x57D0CF61 */
	s0  = -7.72156649015328655494e-02, /* 0xBFB3C467, 0xE37DB0C8 */
	s1  =  2.14982415960608852501e-01, /* 0x3FCB848B, 0x36E20878 */
	s2  =  3.25778796408930981787e-01, /* 0x3FD4D98F, 0x4F139F59 */
	s3  =  1.46350472652464452805e-01, /* 0x3FC2BB9C, 0xBEE5F2F7 */
	s4  =  2.66422703033638609560e-02, /* 0x3F9B481C, 0x7E939961 */
	s5  =  1.84028451407337715652e-03, /* 0x3F5E26B6, 0x7368F239 */
	s6  =  3.19475326584100867617e-05, /* 0x3F00BFEC, 0xDD17E945 */
	r1  =  1.39200533467621045958e+00, /* 0x3FF645A7, 0x62C4AB74 */
	r2  =  7.21935547567138069525e-01, /* 0x3FE71A18, 0x93D3DCDC */
	r3  =  1.71933865632803078993e-01, /* 0x3FC601ED, 0xCCFBDF27 */
	r4  =  1.86459191715652901344e-02, /* 0x3F9317EA, 0x742ED475 */
	r5  =  7.77942496381893596434e-04, /* 0x3F497DDA, 0xCA41A95B */
	r6  =  7.32668430744625636189e-06, /* 0x3EDEBAF7, 0xA5B38140 */
	w0  =  4.18938533204672725052e-01, /* 0x3FDACFE3, 0x90C97D69 */
	w1  =  8.33333333333329678849e-02, /* 0x3FB55555, 0x5555553B */
	w2  = -2.77777777728775536470e-03, /* 0xBF66C16C, 0x16B02E5C */
	w3  =  7.93650558643019558500e-04, /* 0x3F4A019F, 0x98CF38B6 */
	w4  = -5.95187557450339963135e-04, /* 0xBF4380CB, 0x8C0FE741 */
	w5  =  8.36339918996282139126e-04, /* 0x3F4B67BA, 0x4CDAD5D1 */
	w6  = -1.63092934096575273989e-03; /* 0xBF5AB89D, 0x0B9E43E4 */

	double zero=  0.00000000000000000000e+00;

	double t,y,z,nadj,p,p1,p2,p3,q,r,w;
	int i,hx,lx,ix;

	hx = __HI(x);
	lx = __LO(x);

	/* purge off +-inf, NaN, +-0, and negative arguments */
	*signgamp = 1;
	ix = hx&0x7fffffff;
	if(ix>=0x7ff00000) return x*x;
	if((ix|lx)==0) return one/zero;
	if(ix<0x3b900000) { /* |x|<2**-70, return -log(|x|) */
	    if(hx<0) {
	        *signgamp = -1;
	        return -log(-x);
	    } else return -log(x);
	}
	if(hx<0) {
	    if(ix>=0x43300000)  /* |x|>=2**52, must be -integer */
	    return one/zero;
	    t = sinpi(x);
	    if(t==zero) return one/zero; /* -integer */
	    nadj = log(pi/fabs(t*x));
	    if(t<zero) *signgamp = -1;
	    x = -x;
	}

	/* purge off 1 and 2 */
	if((((ix-0x3ff00000)|lx)==0)||(((ix-0x40000000)|lx)==0)) r = 0;
	/* for x < 2.0 */
	else if(ix<0x40000000) {
	    if(ix<=0x3feccccc) {    /* lgamma(x) = lgamma(x+1)-log(x) */
	    r = -log(x);
	    if(ix>=0x3FE76944) {y = one-x; i= 0;}
	    else if(ix>=0x3FCDA661) {y= x-(tc-one); i=1;}
	    else {y = x; i=2;}
	    } else {
	    r = zero;
	        if(ix>=0x3FFBB4C3) {y=2.0-x;i=0;} /* [1.7316,2] */
	        else if(ix>=0x3FF3B4C4) {y=x-tc;i=1;} /* [1.23,1.73] */
	    else {y=x-one;i=2;}
	    }
	    switch(i) {
	      case 0:
	    z = y*y;
	    p1 = a0+z*(a2+z*(a4+z*(a6+z*(a8+z*a10))));
	    p2 = z*(a1+z*(a3+z*(a5+z*(a7+z*(a9+z*a11)))));
	    p  = y*p1+p2;
	    r  += (p-0.5*y); break;
	      case 1:
	    z = y*y;
	    w = z*y;
	    p1 = t0+w*(t3+w*(t6+w*(t9 +w*t12)));    /* parallel comp */
	    p2 = t1+w*(t4+w*(t7+w*(t10+w*t13)));
	    p3 = t2+w*(t5+w*(t8+w*(t11+w*t14)));
	    p  = z*p1-(tt-w*(p2+y*p3));
	    r += (tf + p); break;
	      case 2:
	    p1 = y*(u0+y*(u1+y*(u2+y*(u3+y*(u4+y*u5)))));
	    p2 = one+y*(v1+y*(v2+y*(v3+y*(v4+y*v5))));
	    r += (-0.5*y + p1/p2);
	    }
	}
	else if(ix<0x40200000) {            /* x < 8.0 */
	    i = (int)x;
	    t = zero;
	    y = x-(double)i;
	    p = y*(s0+y*(s1+y*(s2+y*(s3+y*(s4+y*(s5+y*s6))))));
	    q = one+y*(r1+y*(r2+y*(r3+y*(r4+y*(r5+y*r6)))));
	    r = halfD*y+p/q;
	    z = one;    /* lgamma(1+s) = log(s) + lgamma(s) */
	    switch(i) {
	    case 7: z *= (y+6.0);   /* FALLTHRU */
	    case 6: z *= (y+5.0);   /* FALLTHRU */
	    case 5: z *= (y+4.0);   /* FALLTHRU */
	    case 4: z *= (y+3.0);   /* FALLTHRU */
	    case 3: z *= (y+2.0);   /* FALLTHRU */
	        r += log(z); break;
	    }
	/* 8.0 <= x < 2**58 */
	} else if (ix < 0x43900000) {
	    t = log(x);
	    z = one/x;
	    y = z*z;
	    w = w0+z*(w1+y*(w2+y*(w3+y*(w4+y*(w5+y*w6)))));
	    r = (x-halfD)*(t-one)+w;
	} else
	/* 2**58 <= x <= inf */
	    r =  x*(log(x)-one);
	if(hx<0) r = nadj - r;
	return r;
}

OVERLOADABLE double modf(double x, global double *i)
{
	if(x == as_double(DF_POSITIVE_INF))
	{
		*i = x;
		return 0.0;
	}

	if(-x == as_double(DF_POSITIVE_INF))
	{
		*i = x;
		return -0.0;
	}

	double retVal = floor(fabs(x));
	if(x >= 0.0)
		*i = retVal;
	else
	{
		retVal = -retVal;
		*i = retVal;
	}

	return x - retVal;
}

OVERLOADABLE double modf(double x, local double *i)
{
	if(x == as_double(DF_POSITIVE_INF))
	{
		*i = x;
		return 0.0;
	}

	if(-x == as_double(DF_POSITIVE_INF))
	{
		*i = x;
		return -0.0;
	}

	double retVal = floor(fabs(x));
	if(x >= 0.0)
		*i = retVal;
	else
	{
		retVal = -retVal;
		*i = retVal;
	}

	return x - retVal;
}

OVERLOADABLE double modf(double x, private double *i)
{
	if(x == as_double(DF_POSITIVE_INF))
	{
		*i = x;
		return 0.0;
	}

	if(-x == as_double(DF_POSITIVE_INF))
	{
		*i = x;
		return -0.0;
	}

	double retVal = floor(fabs(x));
	if(x >= 0.0)
		*i = retVal;
	else
	{
		retVal = -retVal;
		*i = retVal;
	}

	return x - retVal;
}

double __fmod (double x, double y, int* ik)
{
	const double one = 1.0, Zero[] = {0.0, -0.0,};
	int n,hx,hy,hz,ix,iy,sx,i;
	uint lx,ly,lz;
	int ifactor = 0;

	hx = __HI(x);
	lx = __LO(x);
	hy = __HI(y);
	ly = __LO(y);
	sx = hx&0x80000000; 	/* sign of x */
	hx ^=sx;		/* |x| */
	hy &= 0x7fffffff;	/* |y| */

	/* purge off exception values */
	if((hy|ly)==0||(hx>=0x7ff00000)||	/* y=0,or x not finite */
		((hy|((ly|-ly)>>31))>0x7ff00000)) /* or y is NaN */
		return (x*y)/(x*y);
	if(hx<=hy) {
		if((hx<hy)||(lx<ly)) return x;	/* |x|<|y| return x */
		if(lx==ly)
		{
			*ik = 2;
			return Zero[(uint)sx>>31];	/* |x|=|y| return x*0*/
		}
	}

	/* determine ix = ilogb(x) */
	if(hx<0x00100000) { /* subnormal x */
		if(hx==0) {
		for (ix = -1043, i=lx; i>0; i<<=1) ix -=1;
		} else {
		for (ix = -1022,i=(hx<<11); i>0; i<<=1) ix -=1;
		}
	} else ix = (hx>>20)-1023;

	/* determine iy = ilogb(y) */
	if(hy<0x00100000) { /* subnormal y */
		if(hy==0) {
		for (iy = -1043, i=ly; i>0; i<<=1) iy -=1;
		} else {
		for (iy = -1022,i=(hy<<11); i>0; i<<=1) iy -=1;
		}
	} else iy = (hy>>20)-1023;

	/* set up {hx,lx}, {hy,ly} and align y to x */
	if(ix >= -1022) 
		hx = 0x00100000|(0x000fffff&hx);
	else {		/* subnormal x, shift x to normal */
		n = -1022-ix;
		if(n<=31) {
			hx = (hx<<n)|(lx>>(32-n));
			lx <<= n;
		} else {
		hx = lx<<(n-32);
		lx = 0;
		}
	}
	if(iy >= -1022) 
		hy = 0x00100000|(0x000fffff&hy);
	else {		/* subnormal y, shift y to normal */
		n = -1022-iy;
		if(n<=31) {
			hy = (hy<<n)|(ly>>(32-n));
			ly <<= n;
		} else {
		hy = ly<<(n-32);
		ly = 0;
		}
	}

	/* fix point fmod */
	n = ix - iy;
	while(n--)
	{
		hz=hx-hy;lz=lx-ly;
		if(lx<ly) hz -= 1;

		ifactor <<= 1;
		if(hz<0)
		{
			hx = hx+hx+(lx>>31);
			lx = lx+lx;
		}
		else
		{
			if(n<16)ifactor |= 1;

			if((hz|lz)==0)		/* return sign(x)*0 */
			{
				*ik = (ifactor << (n + 2));
				return Zero[(uint)sx>>31];
			}

			hx = hz+hz+(lz>>31); lx = lz+lz;
		}
	}

	hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	if(hz>=0){hx=hz;lx=lz;ifactor <<= 1; ifactor |= 1;}
	else ifactor = (ifactor << 1);
	
	ifactor = (ifactor << 1);
	*ik = ifactor;
	/* convert back to floating value and restore the sign */
	if((hx|lx)==0)			/* return sign(x)*0 */
		return Zero[(uint)sx>>31];	
	while(hx<0x00100000) {		/* normalize x */
		hx = hx+hx+(lx>>31); lx = lx+lx;
		iy -= 1;
	}
	if(iy>= -1022) {	/* normalize output */
		hx = ((hx-0x00100000)|((iy+1023)<<20));
		__setHigh(&x,hx|sx);
		__setLow(&x, lx);
	} else {		/* subnormal output */
		n = -1022 - iy;
		if(n<=20) {
		lx = (lx>>n)|((uint)hx<<(32-n));
		hx >>= n;
		} else if (n<=31) {
		lx = (hx<<(32-n))|(lx>>n); hx = sx;
		} else {
		lx = hx>>(n-32); hx = sx;
		}
	__setHigh(&x,hx|sx);
	__setLow(&x, lx);

		x *= one;		/* create necessary signal */
	}
	return x;		/* exact output */

}


OVERLOADABLE double remquo(double x, double p, global int *quo)
{
	int hx,hp, ik = 0;
	unsigned sx,sy, lx,lp;
	double p_half, zero = 0.0;
	double xx = x;

	hx = __HI(x);		/* high word of x */
	lx = __LO(x);		/* low  word of x */
	hp = __HI(p);		/* high word of p */
	lp = __LO(p);		/* low  word of p */
	sx = hx&0x80000000;
	sy = hp&0x80000000;
	hp &= 0x7fffffff;
	hx &= 0x7fffffff;

	/* purge off exception values */
	if((hp|lp)==0){*quo = 0; return (x*p)/(x*p);} 	/* p = 0 */
	if((hx>=0x7ff00000)||			/* x not finite */
	  ((hp>=0x7ff00000)&&			/* p is NaN */
	  (((hp-0x7ff00000)|lp)!=0)))
		{*quo = 0; return (x*p)/(x*p);} 	


	if (hp<=0x7fdfffff) x = __fmod(x,p+p, &ik);	/* now x < 2p */

	x  = fabs(x);
	p  = fabs(p);
	if (hp<0x00200000)
	{
		if(x+x>p)
		{
			x-=p;
			ik += 1;
			if(x+x>=p)
			{
				x -= p;
				ik += 1;
			}
		}
	}
	else
	{
		p_half = 0.5*p;
		if(x>p_half)
		{
			x-=p;
			ik += 1;
			if(x>=p_half)
			{
				ik += 1;
				x -= p;
			}
		}
	}
	__setHigh(&x,  __HI(x) ^sx);

	ik &= 0x7f;
	if(sx != sy) ik = -ik;
	*quo = ik;
	return x;
}

OVERLOADABLE double remquo(double x, double p, local int *quo)
{
	int hx,hp, ik = 0;
	unsigned sx,sy, lx,lp;
	double p_half, zero = 0.0;
	double xx = x;

	hx = __HI(x);		/* high word of x */
	lx = __LO(x);		/* low	word of x */
	hp = __HI(p);		/* high word of p */
	lp = __LO(p);		/* low	word of p */
	sx = hx&0x80000000;
	sy = hp&0x80000000;
	hp &= 0x7fffffff;
	hx &= 0x7fffffff;

	/* purge off exception values */
	if((hp|lp)==0){*quo = 0; return (x*p)/(x*p);}	/* p = 0 */
	if((hx>=0x7ff00000)||			/* x not finite */
	  ((hp>=0x7ff00000)&&			/* p is NaN */
	  (((hp-0x7ff00000)|lp)!=0)))
		{*quo = 0; return (x*p)/(x*p);} 	


	if (hp<=0x7fdfffff) x = __fmod(x,p+p, &ik); /* now x < 2p */

	x  = fabs(x);
	p  = fabs(p);
	if (hp<0x00200000)
	{
		if(x+x>p)
		{
			x-=p;
			ik += 1;
			if(x+x>=p)
			{
				x -= p;
				ik += 1;
			}
		}
	}
	else
	{
		p_half = 0.5*p;
		if(x>p_half)
		{
			x-=p;
			ik += 1;
			if(x>=p_half)
			{
				ik += 1;
				x -= p;
			}
		}
	}
	__setHigh(&x,  __HI(x) ^sx);

	ik &= 0x7f;
	if(sx != sy) ik = -ik;
	*quo = ik;
	return x;
}


OVERLOADABLE double remquo(double x, double p, private int *quo)
{
	int hx,hp, ik = 0;
	unsigned sx,sy, lx,lp;
	double p_half, zero = 0.0;
	double xx = x;

	hx = __HI(x);		/* high word of x */
	lx = __LO(x);		/* low	word of x */
	hp = __HI(p);		/* high word of p */
	lp = __LO(p);		/* low	word of p */
	sx = hx&0x80000000;
	sy = hp&0x80000000;
	hp &= 0x7fffffff;
	hx &= 0x7fffffff;

	/* purge off exception values */
	if((hp|lp)==0){*quo = 0; return (x*p)/(x*p);}	/* p = 0 */
	if((hx>=0x7ff00000)||			/* x not finite */
	  ((hp>=0x7ff00000)&&			/* p is NaN */
	  (((hp-0x7ff00000)|lp)!=0)))
		{*quo = 0; return (x*p)/(x*p);} 	


	if (hp<=0x7fdfffff) x = __fmod(x,p+p, &ik); /* now x < 2p */

	x  = fabs(x);
	p  = fabs(p);
	if (hp<0x00200000)
	{
		if(x+x>p)
		{
			x-=p;
			ik += 1;
			if(x+x>=p)
			{
				x -= p;
				ik += 1;
			}
		}
	}
	else
	{
		p_half = 0.5*p;
		if(x>p_half)
		{
			x-=p;
			ik += 1;
			if(x>=p_half)
			{
				ik += 1;
				x -= p;
			}
		}
	}
	__setHigh(&x,  __HI(x) ^sx);

	ik &= 0x7f;
	if(sx != sy) ik = -ik;
	*quo = ik;
	return x;
}


OVERLOADABLE double sincos(double x, global double *cosval)
{
	*cosval = cos(x);
	return sin(x);
}

OVERLOADABLE double sincos(double x, local double *cosval)
{
	*cosval = cos(x);
	return sin(x);
}

OVERLOADABLE double sincos(double x, private double *cosval)
{
	*cosval = cos(x);
	return sin(x);
}


