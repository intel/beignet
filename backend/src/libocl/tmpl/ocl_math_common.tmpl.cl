/*
 * Copyright © 2012 - 2017 Intel Corporation
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
#include "ocl_math_common.h"
#include "ocl_float.h"
#include "ocl_relational.h"
#include "ocl_common.h"
#include "ocl_integer.h"
#include "ocl_convert.h"

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

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


/* native functions */
OVERLOADABLE float native_cos(float x) { return __gen_ocl_cos(x); }
OVERLOADABLE float native_sin(float x) { return __gen_ocl_sin(x); }
OVERLOADABLE float native_sqrt(float x) { return __gen_ocl_sqrt(x); }
OVERLOADABLE float native_rsqrt(float x) { return __gen_ocl_rsqrt(x); }
OVERLOADABLE float native_log2(float x) { return __gen_ocl_log(x); }
OVERLOADABLE float native_log(float x) {
  return native_log2(x) * 0.6931472002f;
}
OVERLOADABLE float native_log10(float x) {
  return native_log2(x) * 0.3010299956f;
}
OVERLOADABLE float native_powr(float x, float y) { return __gen_ocl_pow(x,y); }
OVERLOADABLE float native_recip(float x) { return __gen_ocl_rcp(x); }
OVERLOADABLE float native_tan(float x) {
  return native_sin(x) / native_cos(x);
}
OVERLOADABLE float native_exp2(float x) { return __gen_ocl_exp(x); }
OVERLOADABLE float native_exp(float x) { return __gen_ocl_exp(M_LOG2E_F*x); }
OVERLOADABLE float native_exp10(float x) { return __gen_ocl_exp(M_LOG210_F*x); }
OVERLOADABLE float native_divide(float x, float y) { return x/y; }

/* Fast path */
OVERLOADABLE float __gen_ocl_internal_fastpath_acosh (float x) {
    return native_log(x + native_sqrt(x + 1) * native_sqrt(x - 1));
}
OVERLOADABLE float __gen_ocl_internal_fastpath_asinh (float x) {
    return native_log(x + native_sqrt(x * x + 1));
}
OVERLOADABLE float __gen_ocl_internal_fastpath_atanh (float x) {
    return 0.5f * native_log((1 + x) / (1 - x));
}
OVERLOADABLE float __gen_ocl_internal_fastpath_cbrt (float x) {
    return __gen_ocl_pow(x, 0.3333333333f);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_cos (float x) {
    return native_cos(x);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_cosh (float x) {
    return (1 + native_exp(-2 * x)) / (2 * native_exp(-x));
}
OVERLOADABLE float __gen_ocl_internal_fastpath_cospi (float x) {
    return __gen_ocl_cos(x * M_PI_F);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_exp (float x) {
    return native_exp(x);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_exp10 (float x) {
    return native_exp10(x);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_expm1 (float x) {
    return __gen_ocl_pow(M_E_F, x) - 1;
}
OVERLOADABLE float __gen_ocl_internal_fastpath_fmod (float x, float y) {
    return x-y*__gen_ocl_rndz(x/y);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_hypot (float x, float y) {
    return __gen_ocl_sqrt(x*x + y*y);
}
OVERLOADABLE int __gen_ocl_internal_fastpath_ilogb (float x) {
    return __gen_ocl_rndd(native_log2(x));
}
OVERLOADABLE float __gen_ocl_internal_fastpath_ldexp (float x, int n) {
    return __gen_ocl_pow(2, n) * x;
}
OVERLOADABLE float __gen_ocl_internal_fastpath_log (float x) {
    return native_log(x);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_log2 (float x) {
    return native_log2(x);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_log10 (float x) {
    return native_log10(x);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_log1p (float x) {
    return native_log(x + 1);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_logb (float x) {
    return __gen_ocl_rndd(native_log2(x));
}
OVERLOADABLE float __gen_ocl_internal_fastpath_remainder (float x, float y) {
    return x-y*__gen_ocl_rnde(x/y);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_rootn(float x, int n) {
    return __gen_ocl_pow(x, 1.f / n);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_sin (float x) {
    return native_sin(x);
}

OVERLOADABLE float __gen_ocl_internal_fastpath_sinh (float x) {
    return (1 - native_exp(-2 * x)) / (2 * native_exp(-x));
}
OVERLOADABLE float __gen_ocl_internal_fastpath_sinpi (float x) {
    return __gen_ocl_sin(x * M_PI_F);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_tan (float x) {
    return native_tan(x);
}
OVERLOADABLE float __gen_ocl_internal_fastpath_tanh (float x) {
    float y = native_exp(-2 * x);
    return (1 - y) / (1 + y);
}


/* Internal implement, high accuracy. */
OVERLOADABLE float __gen_ocl_internal_floor(float x) { return __gen_ocl_rndd(x); }
OVERLOADABLE float __gen_ocl_internal_copysign(float x, float y) {
  union { unsigned u; float f; } ux, uy;
  ux.f = x;
  uy.f = y;
  ux.u = (ux.u & 0x7fffffff) | (uy.u & 0x80000000u);
  return ux.f;
}

OVERLOADABLE float inline __gen_ocl_internal_log_valid(float x) {
/*
 *  Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
  union { unsigned int i; float f; } u;
  const float
  ln2_hi = 6.9313812256e-01,  /* 0x3f317180 */
  ln2_lo = 9.0580006145e-06,  /* 0x3717f7d1 */
  two25 =  3.355443200e+07, /* 0x4c000000 */
  Lg1 = 6.6666668653e-01, /* 3F2AAAAB */
  Lg2 = 4.0000000596e-01, /* 3ECCCCCD */
  Lg3 = 2.8571429849e-01, /* 3E924925 */
  Lg4 = 2.2222198546e-01; /* 3E638E29 */

  const float zero   =  0.0;
  float fsq, f, s, z, R, w, t1, t2, partial;
  int k, ix, i, j;

  u.f = x;  ix = u.i;
  k = 0;

  k += (ix>>23) - 127;
  ix &= 0x007fffff;
  i = (ix + (0x95f64<<3)) & 0x800000;
  u.i = ix | (i^0x3f800000); x = u.f;
  k += (i>>23);
  f = x - 1.0f;
  fsq = f * f;
  s = mad(-2.0f, 1.0f / (2.0f + f), 1.0f);
  z = s * s;
  w = z * z;
  t1 = w * mad(w, Lg4, Lg2);
  R = mad(z, mad(w, Lg3, Lg1), t1);
  w = 0.5f * fsq;
  partial = -mad(s, w, -w);
  return mad(k, ln2_lo, mad(k, ln2_hi, mad(s, R, f) - partial));
}

OVERLOADABLE float __gen_ocl_internal_log(float x)
{
  return __gen_ocl_internal_log_valid(x) + (x - x);
}

OVERLOADABLE float __gen_ocl_internal_log10(float x)
{
  union { float f; unsigned i; } u;
  const float
  ivln10     =  4.3429449201e-01, /* 0x3ede5bd9 */
  log10_2hi  =  3.0102920532e-01, /* 0x3e9a2080 */
  log10_2lo  =  7.9034151668e-07; /* 0x355427db */

  float y, z;
  int i, k;
  unsigned int hx;

  u.f = x; hx = u.i;
  if (hx >= 0x7f800000)
    return NAN;

  k = (hx >> 23) - 127;
  i  = ((unsigned)k & 0x80000000) >> 31;
  hx = (hx&0x007fffff) | ((0x7f-i) << 23);
  y  = (float)(k + i);
  u.i = hx; x = u.f;

  return  y * log10_2lo + y * log10_2hi + ivln10 * __gen_ocl_internal_log_valid(x);
}


OVERLOADABLE float __gen_ocl_internal_log2(float x)
{
  const float zero   =  0.0,
  invln2 = 0x1.715476p+0f;

  return invln2 * __gen_ocl_internal_log_valid(x) + (x - x);
}


float __gen_ocl_scalbnf (float x, int n){
  /* copy from fdlibm */
  float two25 = 3.355443200e+07,	/* 0x4c000000 */
  twom25 = 2.9802322388e-08,	        /* 0x33000000 */
  huge = 1.0e+30,
  tiny = 1.0e-30;
  int k,ix;
  GEN_OCL_GET_FLOAT_WORD(ix,x);
  k = (ix&0x7f800000)>>23; /* extract exponent */
  if (k==0) {	/* 0 or subnormal x */
    if ((ix&0x7fffffff)==0) return x; /* +-0 */
    x *= two25;
    GEN_OCL_GET_FLOAT_WORD(ix,x);
    k = ((ix&0x7f800000)>>23) - 25;
  }
  if (k==0xff) return x+x;	/* NaN or Inf */
  if (n< -50000)
    return tiny*__gen_ocl_internal_copysign(tiny,x);	/*underflow*/
  if (n> 50000 || k+n > 0xfe)
    return huge*__gen_ocl_internal_copysign(huge,x); /* overflow  */
  /* Now k and n are bounded we know that k = k+n does not overflow. */
  k = k+n;
  if (k > 0) { /* normal result */
    GEN_OCL_SET_FLOAT_WORD(x,(ix&0x807fffff)|(k<<23));
    return x;
  }
  if (k <= -25)
    return tiny*__gen_ocl_internal_copysign(tiny,x);	/*underflow*/
  k += 25;				/* subnormal result */
  GEN_OCL_SET_FLOAT_WORD(x,(ix&0x807fffff)|(k<<23));
  return x*twom25;
}

const __constant unsigned int two_over_pi[] = {
0, 0, 0xA2F, 0x983, 0x6E4, 0xe44, 0x152, 0x9FC,
0x275, 0x7D1, 0xF53, 0x4DD, 0xC0D, 0xB62,
0x959, 0x93C, 0x439, 0x041, 0xFE5, 0x163,
};

// The main idea is from "Radian Reduction for Trigonometric Functions"
// written by Mary H. Payne and Robert N. Hanek. Also another reference
// is "A Continued-Fraction Analysis of Trigonometric Argument Reduction"
// written by Roger Alan Smith, who gave the worst case in this paper.
// for single float, worst x = 0x1.47d0fep34, and there are 29 bit
// leading zeros in the fraction part of x*(2.0/pi). so we need at least
// 29 (leading zero)+ 24 (fraction )+12 (integer) + guard bits. that is,
// 65 + guard bits, as we calculate in 12*7 = 84bits, which means we have
// about 19 guard bits. If we need further precision, we may need more
// guard bits
// Note we place two 0 in two_over_pi, which is used to handle input less
// than 0x1.0p23

int payne_hanek(float x, float *y) {
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
    result[i] =  low * two_over_pi[index+i+ZERO_BITS] ;
    result[i] +=  high * two_over_pi[index+i+1+ZERO_BITS];
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

int argumentReduceSmall(float x, float * remainder) {
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


int __ieee754_rem_pio2f(float x, float *y) {
  if (x < 2.5e2) {
    return argumentReduceSmall(x, y);
  } else {
    return payne_hanek(x, y);
  }
}

OVERLOADABLE float __kernel_sinf(float x)
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

float __kernel_cosf(float x, float y)
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

OVERLOADABLE float sin(float x)
{
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_sin(x);

  float y;
  float na ;
  uint n, ix;
  float negative = x < 0.0f? -1.0f : 1.0f;
  x = fabs(x);

  /* cos(Inf or NaN) is NaN */
  na = x -x;

  uint n0, n1;
  float v;
  n = __ieee754_rem_pio2f(x,&y);
  float s = __kernel_sinf(y);
  float c = sqrt(mad(-s, s, 1.0f));
  n0 = (n&0x1);
  n1 = (n&0x2);
  v = (n0)?c:s;
  v = (n1)?-v:v;
  /* n&3   return
	  0    sin(y)
	  1    cos(y)
	  2   -sin(y)
	  3   -cos(y)
  */
  return mad(v, negative, na);
}

OVERLOADABLE float cos(float x)
{
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_cos(x);

  float y;
  float na ;
  
  uint n, ix;
  x = __gen_ocl_fabs(x);

  /* cos(Inf or NaN) is NaN */
  na = x -x;

  uint n0, n1;
  float v;

  n = __ieee754_rem_pio2f(x,&y);
  float s = __kernel_sinf(y);
  float c = sqrt(fabs(mad(s, s, -1.0f)));

  n0 = (n&0x1);
  n1 = (n&0x2);

  float ss = n1 - 1.0f;
  v = (n0)?s:-c;

  /* n&3   return
      0    cos(y)
      1   -sin(y)
      2   -cos(y)
      3    sin(y)
  */
  return mad(v, ss, na);
}

float __kernel_tanf(float x, float y, int iy)
{
  /* copied from fdlibm */
        float z,r,v,w,s;
        int ix,hx;
        const float
        one   =  1.0000000000e+00, /* 0x3f800000 */
        pio4  =  7.8539812565e-01, /* 0x3f490fda */
        pio4lo=  3.7748947079e-08; /* 0x33222168 */
        float T[13];// =  {
         T[0] = 3.3333334327e-01; /* 0x3eaaaaab */
         T[1] = 1.3333334029e-01; /* 0x3e088889 */
         T[2] = 5.3968254477e-02; /* 0x3d5d0dd1 */
         T[3] = 2.1869488060e-02; /* 0x3cb327a4 */
         T[4] = 8.8632395491e-03; /* 0x3c11371f */
         T[5] = 3.5920790397e-03; /* 0x3b6b6916 */
         T[6] = 1.4562094584e-03; /* 0x3abede48 */
         T[7] = 5.8804126456e-04; /* 0x3a1a26c8 */

        GEN_OCL_GET_FLOAT_WORD(hx,x);
        ix = hx&0x7fffffff;     /* high word of |x| */
        if(ix<0x31800000)                       /* x < 2**-28 */
            {if((int)x==0) {                    /* generate inexact */
                if((ix|(iy+1))==0) return one/__gen_ocl_fabs(x);
                else return (iy==1)? x: -one/x;
            }
            }
        if(ix>=0x3f2ca140) {                    /* |x|>=0.6744 */
            if(hx<0) {x = -x; y = -y;}
            z = pio4-x;
            w = pio4lo-y;
            x = z+w; y = 0.0;
        }
        z       =  x*x;
        w       =  z*z;
		/* Break x^5*(T[1]+x^2*T[2]+...) into
		 *    x^5(T[1]+x^4*T[3]+...+x^20*T[11]) +
		 *    x^5(x^2*(T[2]+x^4*T[4]+...+x^22*[T12]))
		 */

        r = mad(w, mad(w, mad(w, T[7], T[5]), T[3]), T[1]);
        v = z* mad(w, mad(w, T[6], T[4]), T[2]);

        s = z*x;
        r = mad(z, mad(s, r + v, y), y);
        r += T[0]*s;
        w = x+r;
        if(ix>=0x3f2ca140) {
            v = (float)iy;
            return (float)(1-((hx>>30)&2))*(v-(float)2.0*(x-(w*w/(w+v)-r)));
        }
        if(iy==1) return w;
        else
        	return -1.0/(x+r);
}

/*Author : David Defour, Catherine Daramy, Florent de Dinechin, Christoph Lauter Contact :
David.Defour@ens-lyon.fr, catherine_daramy@ens-lyon.fr
    
    This program is free software; you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version. This program is distributed 
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU Lesser General Public License along with this program;
if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA*/

/*                                                        23 */
/*      tan(x) ~ x + T1*x + ... + T13*x      */
float __kernel_tanf_fast(float x, float y, int iy)
{
    float x2 = x*x;
    float sum;

    sum = mad(0.0000392783222196f, x2, 0.0000969153770711f);
    sum = mad(sum, x2, 0.0002391291200183f);
    sum = mad(sum, x2, 0.0005900274263695f);
    sum = mad(sum, x2, 0.0014558343682438f);
    sum = mad(sum, x2, 0.0035921279340982f);
    sum = mad(sum, x2, 0.0088632358238101f);
    sum = mad(sum, x2, 0.0218694880604744f);
    sum = mad(sum, x2, 0.0539682544767857f);
    sum = mad(sum, x2, 0.1333333402872086f);
    sum = mad(sum, x2, 0.3333333432674408f);
    sum = sum*x2;
    sum = mad(sum, x, x);

    if(iy == 1)
    {
        sum = 1.0f/sum;
        sum = -sum;
    }

    return sum;
}

OVERLOADABLE float tan(float x)
{
    if (__ocl_math_fastpath_flag)
      return __gen_ocl_internal_fastpath_tan(x);

    float y,na=0.0;
    int n, ix;
    float negative = x < 0.0f? -1.0f : 1.0f;
    x = fabs(x);
    na = x -x;
    n = __ieee754_rem_pio2f(x,&y);
    return mad(negative , __kernel_tanf_fast(y, 0.0f, (n&1)), na);
}

OVERLOADABLE float __gen_ocl_internal_cospi(float x) {
  int ix;
  if(isinf(x) || isnan(x)) { return NAN; }
  if(x < 0.0f) { x = -x; }
  GEN_OCL_GET_FLOAT_WORD(ix, x);
  if(x> 0x1.0p24) return 1.0f;
  float m = __gen_ocl_internal_floor(x);
  ix = (int)m;
  m = x-m;
  if((ix&0x1) != 0) m+=1.0f;
    ix = __gen_ocl_internal_floor(m*4.0f);

  switch(ix) {
   case 0:
    return __kernel_cosf(m*M_PI_F, 0.0f);
   case 1:
   case 2:
    return __kernel_sinf((0.5f-m)*M_PI_F);
   case 3:
   case 4:
    return -__kernel_cosf((m-1.0f)*M_PI_F, 0.0f);
   case 5:
   case 6:
    return __kernel_sinf((m-1.5f)*M_PI_F);
   default:
    return __kernel_cosf((2.0f-m)*M_PI_F, 0.0f);
   }
}

OVERLOADABLE float __gen_ocl_internal_sinpi(float x) {
  float sign = 1.0f;
  int ix;
  if(isinf(x)) return NAN;
  if(x < 0.0f) { x = -x; sign = -1.0f; }
  GEN_OCL_GET_FLOAT_WORD(ix, x);
  if(x> 0x1.0p24) return 0.0f;
  float m = __gen_ocl_internal_floor(x);
  ix = (int)m;
  m = x-m;
  if((ix&0x1) != 0) m+=1.0f;
    ix = __gen_ocl_internal_floor(m*4.0f);

  switch(ix) {
   case 0:
    return sign*__kernel_sinf(m*M_PI_F);
   case 1:
   case 2:
    return sign*__kernel_cosf((m-0.5f)*M_PI_F, 0.0f);
   case 3:
   case 4:
    return -sign*__kernel_sinf((m-1.0f)*M_PI_F);
   case 5:
   case 6:
    return -sign*__kernel_cosf((m-1.5f)*M_PI_F, 0.0f);
   default:
    return -sign*__kernel_sinf((2.0f-m)*M_PI_F);
   }

}

OVERLOADABLE float lgamma(float x) {
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
    const float
        zero=  0.,
        one =  1.0000000000e+00,
        pi  =  3.1415927410e+00,
        a0  =  7.7215664089e-02,
        a1  =  3.2246702909e-01,
        a2  =  6.7352302372e-02,
        a3  =  2.0580807701e-02,
        a4  =  7.3855509982e-03,
        a5  =  2.8905137442e-03,
        a6  =  1.1927076848e-03,
        a7  =  5.1006977446e-04,
        a8  =  2.2086278477e-04,
        a9  =  1.0801156895e-04,
        a10 =  2.5214456400e-05,
        a11 =  4.4864096708e-05,
        tc  =  1.4616321325e+00,
        tf  = -1.2148628384e-01,
        tt  =  6.6971006518e-09,
        t0  =  4.8383611441e-01,
        t1  = -1.4758771658e-01,
        t2  =  6.4624942839e-02,
        t3  = -3.2788541168e-02,
        t4  =  1.7970675603e-02,
        t5  = -1.0314224288e-02,
        t6  =  6.1005386524e-03,
        t7  = -3.6845202558e-03,
        t8  =  2.2596477065e-03,
        t9  = -1.4034647029e-03,
        t10 =  8.8108185446e-04,
        t11 = -5.3859531181e-04,
        t12 =  3.1563205994e-04,
        t13 = -3.1275415677e-04,
        t14 =  3.3552918467e-04,
        u0  = -7.7215664089e-02,
        u1  =  6.3282704353e-01,
        u2  =  1.4549225569e+00,
        u3  =  9.7771751881e-01,
        u4  =  2.2896373272e-01,
        u5  =  1.3381091878e-02,
        v1  =  2.4559779167e+00,
        v2  =  2.1284897327e+00,
        v3  =  7.6928514242e-01,
        v4  =  1.0422264785e-01,
        v5  =  3.2170924824e-03,
        s0  = -7.7215664089e-02,
        s1  =  2.1498242021e-01,
        s2  =  3.2577878237e-01,
        s3  =  1.4635047317e-01,
        s4  =  2.6642270386e-02,
        s5  =  1.8402845599e-03,
        s6  =  3.1947532989e-05,
        r1  =  1.3920053244e+00,
        r2  =  7.2193557024e-01,
        r3  =  1.7193385959e-01,
        r4  =  1.8645919859e-02,
        r5  =  7.7794247773e-04,
        r6  =  7.3266842264e-06,
        w0  =  4.1893854737e-01,
        w1  =  8.3333335817e-02,
        w2  = -2.7777778450e-03,
        w3  =  7.9365057172e-04,
        w4  = -5.9518753551e-04,
        w5  =  8.3633989561e-04,
        w6  = -1.6309292987e-03;
	float t, y, z, nadj, p, p1, p2, p3, q, r, w;
	int i, hx, ix;
	nadj = 0;
	hx = *(int *)&x;
	ix = hx & 0x7fffffff;
	if (ix >= 0x7f800000)
		return x * x;
	if (ix == 0)
		return ((x + one) / zero);
	if (ix < 0x1c800000) {
		if (hx < 0) {
			return -native_log(-x);
		} else
			return -native_log(x);
	}
	if (hx < 0) {
		if (ix >= 0x4b000000)
			return ((-x) / zero);
		t = __gen_ocl_internal_sinpi(x);
		if (t == zero)
			return ((-x) / zero);
		nadj = native_log(pi / __gen_ocl_fabs(t * x));
		x = -x;
	}
	if (ix == 0x3f800000 || ix == 0x40000000)
		r = 0;
	else if (ix < 0x40000000) {
		if (ix <= 0x3f666666) {
			r = -native_log(x);
			if (ix >= 0x3f3b4a20) {
				y = one - x;
				i = 0;
			} else if (ix >= 0x3e6d3308) {
				y = x - (tc - one);
				i = 1;
			} else {
				y = x;
				i = 2;
			}
		} else {
			r = zero;
			if (ix >= 0x3fdda618) {
				y = (float) 2.0 - x;
				i = 0;
			}
			else if (ix >= 0x3F9da620) {
				y = x - tc;
				i = 1;
			}
			else {
				y = x - one;
				i = 2;
			}
		}
		switch (i) {
		case 0:
			z = y * y;
			p1 = mad(z, mad(z, mad(z, mad(z, mad(z, a10, a8), a6), a4), a2), a0);
			p2 = z * mad(z, mad(z, mad(z, mad(z, mad(z, a11, a9), a7), a5), a3), a1);
			p = mad(y, p1, p2);
			r += (p - (float) 0.5 * y);
			break;
		case 1:
			z = y * y;
			w = z * y;
			p1 = mad(w, mad(w, mad(w, mad(w, t12, t9), t6), t3), t0);
			p2 = mad(w, mad(w, mad(w, mad(w, t13, t10), t7), t4), t1);
			p3 = mad(w, mad(w, mad(w, mad(w, t14, t11), t8), t5), t2);
			p = mad(p1, z, mad(w, mad(y, p3, p2), -tt));
			r += (tf + p);
			break;
		case 2:
			p1 = y * mad(y, mad(y, mad(y, mad(y, mad(y, u5, u4), u3), u2), u1), u0);
			p2 = mad(y, mad(y, mad(y, mad(y, mad(y, v5, v4), v3), v2), v1), one);
			r += (-(float) 0.5 * y + p1 / p2);
		}
	} else if (ix < 0x41000000) {
		i = (int) x;
		t = zero;
		y = x - (float) i;

		p =y * mad(y, mad(y, mad(y, mad(y, mad(y, mad(y, s6, s5), s4), s3), s2), s1), s0);
		q = mad(y, mad(y, mad(y, mad(y, mad(y, mad(y, r6, r5), r4), r3), r2), r1), one);
		r = .5f * y + p / q;
		z = one;

		switch (i) {
		case 7:
			z *= (y + 6.0f);
		case 6:
			z *= (y + 5.0f);
		case 5:
			z *= (y + 4.0f);
		case 4:
			z *= (y + 3.0f);
		case 3:
			z *= (y + 2.0f);
			r += native_log(z);
			break;
		}

	} else if (ix < 0x5c800000) {
		t = native_log(x);
		z = one / x;
		y = z * z;
		w = mad(z, mad(y, mad(y, mad(y, mad(y, mad(y, w6, w5), w4), w3), w2), w1), w0);
		r = (x - .5f) * (t - one) + w;
	} else
		r = x * (native_log(x) - one);
	if (hx < 0)
		r = nadj - r;
	return r;
}

OVERLOADABLE float log1p(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_log1p(x);
/*
 *  Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
  const float
  ln2_hi =   6.9313812256e-01,  /* 0x3f317180 */
  ln2_lo =   9.0580006145e-06,  /* 0x3717f7d1 */
  two25 =    3.355443200e+07, /* 0x4c000000 */
  Lp1 = 6.6666668653e-01, /* 3F2AAAAB */
  Lp2 = 4.0000000596e-01, /* 3ECCCCCD */
  Lp3 = 2.8571429849e-01, /* 3E924925 */
  Lp4 = 2.2222198546e-01; /* 3E638E29 */
  const float zero = 0.0;
  float hfsq,f,c,s,z,R,u;
  int k,hx,hu,ax;
  union {float f; unsigned i;} un;
  un.f = x;  hx = un.i;
  ax = hx&0x7fffffff;

  k = 1;
  if (hx < 0x3ed413d7) {      /* x < 0.41422  */
      if(ax>=0x3f800000) {    /* x <= -1.0 */
    if(x==(float)-1.0) return -two25/zero; /* log1p(-1)=+inf */
    else return (x-x)/(x-x);  /* log1p(x<-1)=NaN */
      }
      if(ax<0x31000000) {     /* |x| < 2**-29 */
    if(two25+x>zero     /* raise inexact */
              &&ax<0x24800000)    /* |x| < 2**-54 */
        return x;
    else
        return x - x*x*(float)0.5;
      }
      if(hx>0||hx<=((int)0xbe95f61f)) {
    k=0;f=x;hu=1;}  /* -0.2929<x<0.41422 */
  }
  if (hx >= 0x7f800000) return x+x;
  if(k!=0) {
      if(hx<0x5a000000) {
    u  = (float)1.0+x;

    un.f = u; hu = un.i;
          k  = (hu>>23)-127;
    /* correction term */
          c  = (k>0)? (float)1.0-(u-x):x-(u-(float)1.0);
    c /= u;
      } else {
    u  = x;
    un.f = u; hu = un.i;
          k  = (hu>>23)-127;
    c  = 0;
      }
      hu &= 0x007fffff;
      if(hu<0x3504f7) {
          un.i = hu|0x3f800000; u = un.f;/* normalize u */
      } else {
          k += 1;
          un.i = hu|0x3f000000; u = un.f;  /* normalize u/2 */
          hu = (0x00800000-hu)>>2;
      }
      f = u-(float)1.0;
  }
  hfsq=(float)0.5*f*f;
  if(hu==0)
  { /* |f| < 2**-20 */
      if(f==zero)
      {
    	  if(k==0) return zero;
    	  else {c = mad(k , ln2_lo, c); return mad(k, ln2_hi, c);}
      }
      R = mad(hfsq, 1.0f, -0.66666666666666666f * f);
      if(k==0) return f-R; else
    	  return k * ln2_hi - (R - mad(k, ln2_lo, c) - f);
  }
  s = f/((float)2.0+f);
  z = s*s;
  R = z * mad(z, mad(z, mad(z, Lp4, Lp3), Lp2), Lp1);
  if(k==0)
	  return f + mad(hfsq + R, s, -hfsq);
  else
	  return k*ln2_hi-( (hfsq - mad(s, hfsq + R, mad(k, ln2_lo, c))) - f);
}

OVERLOADABLE float logb(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_logb(x);

  union {float f; unsigned i;} u;
  u.f = x;
  int e =  ((u.i & 0x7f800000) >> 23);
  float r1 = e-127;
  float r2 = -INFINITY;
  float r3 = x*x;
    /* sub normal or +/-0 */
  float r = e == 0 ? r2 : r1;
    /* inf & nan */
  return e == 0xff ? r3 : r;
}

OVERLOADABLE int ilogb(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_ilogb(x);

  union { int i; float f; } u;
  if (isnan(x))
    return FP_ILOGBNAN;
  if (isinf(x))
    return 0x7FFFFFFF;
  u.f = x;
  u.i &= 0x7fffffff;
  if (u.i == 0)
    return FP_ILOGB0;
  if (u.i >= 0x800000)
    return (u.i >> 23) - 127;
  int r = -126;
  int a = u.i & 0x7FFFFF;
  while(a < 0x800000) {
    a <<= 1;
    r --;
  }
  return r;
}
OVERLOADABLE float nan(uint code) {
  return NAN;
}
OVERLOADABLE float __gen_ocl_internal_tanpi(float x) {
  float sign = 1.0f;
  int ix;
  if(isinf(x)) return NAN;
  if(x < 0.0f) { x = -x; sign = -1.0f; }
  GEN_OCL_GET_FLOAT_WORD(ix, x);
  if(x> 0x1.0p24) return 0.0f;
  float m = __gen_ocl_internal_floor(x);
  ix = (int)m;
  m = x-m;
  int n = __gen_ocl_internal_floor(m*4.0f);
  if(m == 0.5f) {
    return (ix&0x1) == 0 ? sign*INFINITY : sign*-INFINITY;
  }
  if(m == 0.0f) {
    return (ix&0x1) == 0 ? 0.0f : -0.0f;
  }

  switch(n) {
    case 0:
      return sign * __kernel_tanf(m*M_PI_F, 0.0f, 1);
    case 1:
      return sign * 1.0f/__kernel_tanf((0.5f-m)*M_PI_F, 0.0f, 1);
    case 2:
      return sign * 1.0f/__kernel_tanf((0.5f-m)*M_PI_F, 0.0f, 1);
    default:
      return sign * -1.0f*__kernel_tanf((1.0f-m)*M_PI_F, 0.0f, 1);
  }
}
OVERLOADABLE float __gen_ocl_internal_cbrt(float x) {
  /* copied from fdlibm */
  const unsigned
  B1 = 709958130, /* B1 = (84+2/3-0.03306235651)*2**23 */
  B2 = 642849266; /* B2 = (76+2/3-0.03306235651)*2**23 */

  const float
  C =  5.4285717010e-01, /* 19/35     = 0x3f0af8b0 */
  D = -7.0530611277e-01, /* -864/1225 = 0xbf348ef1 */
  E =  1.4142856598e+00, /* 99/70     = 0x3fb50750 */
  F =  1.6071428061e+00, /* 45/28     = 0x3fcdb6db */
  G =  3.5714286566e-01; /* 5/14      = 0x3eb6db6e */

  float r,s,t, w;
  int hx;
  uint sign;
  uint high;

  GEN_OCL_GET_FLOAT_WORD(hx,x);
  sign=hx&0x80000000;     /* sign= sign(x) */
  hx  ^=sign;
  if(hx>=0x7f800000) return(x+x); /* cbrt(NaN,INF) is itself */
  if(hx==0)
      return(x);    /* cbrt(0) is itself */

  GEN_OCL_SET_FLOAT_WORD(x,hx); /* x <- |x| */
    /* rough cbrt to 5 bits */
  if(hx<0x00800000)     /* subnormal number */
    {
    //SET_FLOAT_WORD(t,0x4b800000); /* set t= 2**24 */
     //t*=x; GET_FLOAT_WORD(high,t); SET_FLOAT_WORD(t,high/3+B2);
      t = (sign = 0) ? 0.0f : -0.0f;
      return t;
    }
  else
    GEN_OCL_SET_FLOAT_WORD(t,hx/3+B1);


    /* new cbrt to 23 bits */
  r=t*t/x;
  s=mad(r, t, C);
  t*=G+F/(s+E+D/s);
    /* one step newton iteration to 53 bits with error less than 0.667 ulps */
  s=t*t;    /* t*t is exact */
  r=x/s;
  w=t+t;
  r=(r-t)/(w+r);  /* r-s is exact */
  t=mad(t, r, t);

    /* retore the sign bit */
  GEN_OCL_GET_FLOAT_WORD(high,t);
  GEN_OCL_SET_FLOAT_WORD(t,high|sign);
  return(t);
}

INLINE float __gen_ocl_asin_util(float x) {
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
  float
  pS0 =  1.66666666666666657415e-01,
  pS1 = -3.25565818622400915405e-01,
  pS2 =  2.01212532134862925881e-01,
  pS3 = -4.00555345006794114027e-02,
  pS4 =  7.91534994289814532176e-04,
  qS1 = -2.40339491173441421878e+00,
  qS2 =  2.02094576023350569471e+00,
  qS3 = -6.88283971605453293030e-01,
  qS4 =  7.70381505559019352791e-02;

  float t = x*x;
  float p = t * mad(t, mad(t, mad(t, mad(t, pS4, pS3), pS2), pS1), pS0);
  float q = mad(t, mad(t, mad(t, mad(t, qS4, qS3), qS2), qS1), 1.0f);
  float w = p / q;
  return mad(x, w, x);
}

OVERLOADABLE float __gen_ocl_internal_asin(float x) {
    float asinX2 =__gen_ocl_asin_util(x);
    float absX = fabs(x);
    float asinX1 = mad(2.0f , __gen_ocl_asin_util(native_sqrt(mad(-0.5f, absX, 0.5f))) , -M_PI_2_F);

    float retVal = (x < 0.0f)?asinX1:-asinX1;
    retVal = (absX > 0.5f)?retVal:asinX2;
    retVal = (absX > 1.0f)?NAN:retVal;
    return retVal;
}
OVERLOADABLE float __gen_ocl_internal_asinpi(float x) {
  return __gen_ocl_internal_asin(x) / M_PI_F;
}
OVERLOADABLE float __gen_ocl_internal_acos(float x) {
    float absX = fabs(x);
    float asinX2 =__gen_ocl_asin_util(x);
    float tmp = __gen_ocl_asin_util(native_sqrt(mad(-0.5f, absX, 0.5f)));
    float asinX1 = mad(2.0f ,tmp, -M_PI_2_F);

    float retVal = (x < 0.0f)?asinX1:-asinX1;
    retVal = (absX > 0.5f)?retVal:asinX2;
    retVal = (x <= 0.5f) ? M_PI_2_F - retVal:2.0f*tmp;
    return retVal;
}
OVERLOADABLE float __gen_ocl_internal_acospi(float x) {
  return __gen_ocl_internal_acos(x) / M_PI_F;
}
__constant float atanhi[4] = {
  4.6364760399e-01, /* atan(0.5)hi 0x3eed6338 */
  7.8539812565e-01, /* atan(1.0)hi 0x3f490fda */
  9.8279368877e-01, /* atan(1.5)hi 0x3f7b985e */
  1.5707962513e+00, /* atan(inf)hi 0x3fc90fda */
};
__constant float atanlo[4] = {
  5.0121582440e-09, /* atan(0.5)lo 0x31ac3769 */
  3.7748947079e-08, /* atan(1.0)lo 0x33222168 */
  3.4473217170e-08, /* atan(1.5)lo 0x33140fb4 */
  7.5497894159e-08, /* atan(inf)lo 0x33a22168 */
};

OVERLOADABLE float __gen_ocl_internal_atan(float x) {
	/* copied from fdlibm */
	 float aT[11];
	 aT[0] = 3.3333334327e-01; /* 0x3eaaaaaa */
	 aT[1] =  -2.0000000298e-01; /* 0xbe4ccccd */
	 aT[2] =   1.4285714924e-01; /* 0x3e124925 */
	 aT[3] =  -1.1111110449e-01; /* 0xbde38e38 */
	 aT[4] =   9.0908870101e-02; /* 0x3dba2e6e */
	 aT[5] =  -7.6918758452e-02; /* 0xbd9d8795 */
	 aT[6] =   6.6610731184e-02; /* 0x3d886b35 */
	 const float one = 1.0f, huge = 1.0e30;

	 float w,s1,s2,z;
	 int ix,hx;
	 float extraVal = 0.0f;
	 GEN_OCL_GET_FLOAT_WORD(hx,x);
	 ix = hx&0x7fffffff;

	 if (ix >= 0x3ee00000)
	 {
		 x = __gen_ocl_fabs(x);
		 if (ix < 0x3f980000)
		 {	  /* |x| < 1.1875 */
			 if (ix < 0x3f300000)
			 {	/* 7/16 <=|x|<11/16 */
				 extraVal = 0.4636476040f;
				 x = mad(2.0f, x,  -1.0f)/(2.0f+x);
			 }
			 else
			 {		/* 11/16<=|x|< 19/16 */
				 extraVal = 0.7853981853f;
				 x	= (x-one)/(x+one);
			 }
		 }
		 else
		 {
			 if (ix < 0x401c0000)
			 {	/* |x| < 2.4375 */
				 extraVal = 0.9827937484f;
				 x	= (x-1.5f)/mad(1.5f, x, one);
			 }
			 else
			 {		/* 2.4375 <= |x| < 2^66 */
				 extraVal = 1.5707963705f;
				 x	= -1.0f/x;
			 }
		 }
	 }

	 /* end of argument reduction */
	 z = x*x;
	 w = z*z;
	 /* break sum from i=0 to 10 aT[i]z**(i+1) into odd and even poly */
	 s1 = z * mad(w, mad(w, mad(w, aT[6], aT[4]), aT[2]), aT[0]);
	 s2 = w * mad(w, mad(w, aT[5], aT[3]), aT[1]);

	 float retVal = mad(x, (-s1-s2), extraVal + x);
	 float retVal1 =  (hx<0)? -retVal:retVal;
	 return (extraVal == 0.0) ? retVal:retVal1;
}
OVERLOADABLE float __gen_ocl_internal_atanpi(float x) {
  return __gen_ocl_internal_atan(x) / M_PI_F;
}

// XXX work-around PTX profile
OVERLOADABLE float sqrt(float x) { return native_sqrt(x); }
OVERLOADABLE float rsqrt(float x) { return native_rsqrt(x); }

/*
 * Copyright (c) 2014 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
OVERLOADABLE float __gen_ocl_internal_atan2(float y, float x) {
    const float pi = 0x1.921fb6p+1f;
    const float piby2 = 0x1.921fb6p+0f;
    const float piby4 = 0x1.921fb6p-1f;
    const float threepiby4 = 0x1.2d97c8p+1f;

    float ax = fabs(x);
    float ay = fabs(y);
    float v = min(ax, ay);
    float u = max(ax, ay);

    // Scale since u could be large, as in "regular" divide
    float s = u > 0x1.0p+96f ? 0x1.0p-32f : 1.0f;
    float vbyu = s * v/ (s*u);

    float vbyu2 = vbyu * vbyu;

    float p = mad(vbyu2, mad(vbyu2, -0x1.7e1f78p-9f, -0x1.7d1b98p-3f), -0x1.5554d0p-2f) * vbyu2 * vbyu;
    float q = mad(vbyu2, mad(vbyu2, 0x1.1a714cp-2f, 0x1.287c56p+0f), 1.0f);

    // Octant 0 result
    float a = mad(p, 1.0f/q, vbyu);

    // Fix up 3 other octants
    float at = piby2 - a;
    a = ay > ax ? at : a;
    at = pi - a;
    a = x < 0.0F ? at : a;

    // y == 0 => 0 for x >= 0, pi for x < 0
    int hx = as_int(x);
    at = (hx < 0) ? pi : 0.0f;
    a = y == 0.0f ? at : a;

    at = x > 0.0f ? piby4 : threepiby4;
    a = ax == INFINITY & ay == INFINITY ? at : a;

    // x or y is NaN
    a = isnan(x) | isnan(y) ? NAN : a;

    // Fixup sign and return
    return copysign(a, y);
}

OVERLOADABLE float __gen_ocl_internal_atan2pi(float y, float x) {
  return __gen_ocl_internal_atan2(y, x) / M_PI_F;
}
OVERLOADABLE float __gen_ocl_internal_fabs(float x)  { return __gen_ocl_fabs(x); }
OVERLOADABLE float __gen_ocl_internal_trunc(float x) { return __gen_ocl_rndz(x); }
OVERLOADABLE float __gen_ocl_internal_round(float x) {
  float y = __gen_ocl_rndz(x);
  if (__gen_ocl_fabs(x - y) >= 0.5f)
    y += __gen_ocl_internal_copysign(1.f, x);
  return y;
}
OVERLOADABLE float __gen_ocl_internal_ceil(float x)  { return __gen_ocl_rndu(x); }
OVERLOADABLE float __gen_ocl_internal_rint(float x) {
  return __gen_ocl_rnde(x);
}

OVERLOADABLE float __gen_ocl_internal_exp(float x) {
  float o_threshold = 8.8721679688e+01,  /* 0x42b17180 */
  u_threshold = -1.0397208405e+02,  /* 0xc2cff1b5 */
  twom100 = 7.8886090522e-31, 	 /* 2**-100=0x0d800000 */
  ivln2	 =	1.4426950216e+00, /* 0x3fb8aa3b =1/ln2 */
  one = 1.0,
  huge = 1.0e+30,
  P1 = 1.6666667163e-01, /* 0x3e2aaaab */
  P2 = -2.7777778450e-03; /* 0xbb360b61 */
  float y,hi=0.0,lo=0.0,c,t;
  int k=0,xsb;
  unsigned hx;
  float ln2HI_0 = 6.9313812256e-01;	/* 0x3f317180 */
  float ln2HI_1 = -6.9313812256e-01;	/* 0xbf317180 */
  float ln2LO_0 = 9.0580006145e-06;  	/* 0x3717f7d1 */
  float ln2LO_1 = -9.0580006145e-06; /* 0xb717f7d1 */
  float half_0 = 0.5;
  float half_1 =	-0.5;

  GEN_OCL_GET_FLOAT_WORD(hx,x);
  xsb = (hx>>31)&1;		/* sign bit of x */
  hx &= 0x7fffffff;		/* high word of |x| */

  /* filter out non-finite argument */
  if(hx >= 0x42b17218) {			/* if |x|>=88.721... */
    if(hx>0x7f800000)
      return x+x;			/* NaN */
    if(hx==0x7f800000)
      return (xsb==0)? x:0.0; 	/* exp(+-inf)={inf,0} */
    if(x > o_threshold) return huge*huge; /* overflow */
    if(x < u_threshold) return twom100*twom100; /* underflow */
  }
  /* argument reduction */
  if(hx > 0x3eb17218) {		/* if  |x| > 0.5 ln2 */
    if(hx < 0x3F851592) {	/* and |x| < 1.5 ln2 */
      hi = x-(xsb ==1 ? ln2HI_1 : ln2HI_0); lo= xsb == 1? ln2LO_1 : ln2LO_0; k = 1-xsb-xsb;
    } else {
      float tmp = xsb == 1 ? half_1 : half_0;
      k  = ivln2*x+tmp;
      t  = k;
      hi = x - t*ln2HI_0;	/* t*ln2HI is exact here */
      lo = t*ln2LO_0;
    }
    x  = hi - lo;
  }
  else if(hx < 0x31800000)  { /* when |x|<2**-28 */
    if(huge+x>one) return one+x;/* trigger inexact */
  }
  else k = 0;

  /* x is now in primary range */
  t  = x*x;
  c  = x - t*(P1+t*P2);
  if(k==0)
    return one-((x*c)/(c-(float)2.0)-x);
  else
    y = one-((lo-(x*c)/((float)2.0-c))-hi);
  if(k >= -125) {
    unsigned hy;
    GEN_OCL_GET_FLOAT_WORD(hy,y);
    GEN_OCL_SET_FLOAT_WORD(y,hy+(k<<23));	/* add k to y's exponent */
    return y;
  } else {
    unsigned hy;
    GEN_OCL_GET_FLOAT_WORD(hy,y);
    GEN_OCL_SET_FLOAT_WORD(y,hy+((k+100)<<23)); /* add k to y's exponent */
    return y*twom100;
  }
}

OVERLOADABLE float __gen_ocl_internal_simple_exp(float x) {
  float o_threshold = 8.8721679688e+01, /* 0x42b17180 */
      u_threshold = -1.0397208405e+02,  /* 0xc2cff1b5 */
      twom100 = 7.8886090522e-31,       /* 2**-100=0x0d800000 */
      ivln2 = 1.4426950216e+00,         /* 0x3fb8aa3b =1/ln2 */
      one = 1.0, P1 = 1.6666667163e-01, /* 0x3e2aaaab */
      P2 = -2.7777778450e-03;           /* 0xbb360b61 */
  float y, hi = 0.0, lo = 0.0, c, t;
  int k = 0;
  unsigned hx;
  float ln2HI_0 = 6.9313812256e-01;  /* 0x3f317180 */
  float ln2HI_1 = -6.9313812256e-01; /* 0xbf317180 */
  float ln2LO_0 = 9.0580006145e-06;  /* 0x3717f7d1 */
  float ln2LO_1 = -9.0580006145e-06; /* 0xb717f7d1 */
  float half_0 = 0.5;
  float half_1 = -0.5;
  float retVal = -1.0f;
  hx = as_uint(fabs(x));

  /* filter out non-finite argument */
  /* if |x|>=88.721... */
  if (hx >= 0x42b17218) {
    float tmp = (x > 0) ? x : 0.0;
    retVal = (x > 0) ? INFINITY : retVal; /* overflow */
    retVal = (hx > 0x7f800000) ? NAN : retVal;
    retVal = (hx == 0x7f800000) ? tmp : retVal;
    retVal = (x < u_threshold) ? 0.0 : retVal; /* underflow */

    if (retVal != -1.0f)
      return retVal;
  }

  /* argument reduction */
  float tmp = (x < 0) ? half_1 : half_0;
  k = mad(ivln2, x, tmp);
  t = k;
  hi = mad(t, -ln2HI_0, x); /* t*ln2HI is exact here */
  lo = t * ln2LO_0;
  x = hi - lo;

  /* x is now in primary range */
  t = x * x;
  c = mad(t, mad(t, -P2, -P1), x);
  y = one - ((lo + (x * c) / (c - 2.0f)) - hi);

  unsigned hy;
  GEN_OCL_GET_FLOAT_WORD(hy, y);

  float factor = (k >= -125) ? 1.0f : twom100;
  k = (k >= -125) ? k : k + 100;
  GEN_OCL_SET_FLOAT_WORD(y, hy + (k << 23)); /* add k to y's exponent */
  return y * factor;
}

/* erf,erfc from glibc s_erff.c -- float version of s_erf.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

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

INLINE_OVERLOADABLE float __gen_ocl_internal_erf(float x) {
/*...*/
const float
tiny = 1.0e-30,
half_val=  5.0000000000e-01, /* 0x3F000000 */
one =  1.0000000000e+00, /* 0x3F800000 */
two =  2.0000000000e+00, /* 0x40000000 */
	/* c = (subfloat)0.84506291151 */
erx =  8.4506291151e-01, /* 0x3f58560b */
/*
 * Coefficients for approximation to  erf on [0,0.84375]
 */
efx =  1.2837916613e-01, /* 0x3e0375d4 */
efx8=  1.0270333290e+00, /* 0x3f8375d4 */
pp0  =  1.2837916613e-01, /* 0x3e0375d4 */
pp1  = -3.2504209876e-01, /* 0xbea66beb */
pp2  = -2.8481749818e-02, /* 0xbce9528f */
pp3  = -5.7702702470e-03, /* 0xbbbd1489 */
pp4  = -2.3763017452e-05, /* 0xb7c756b1 */
qq1  =  3.9791721106e-01, /* 0x3ecbbbce */
qq2  =  6.5022252500e-02, /* 0x3d852a63 */
qq3  =  5.0813062117e-03, /* 0x3ba68116 */
qq4  =  1.3249473704e-04, /* 0x390aee49 */
qq5  = -3.9602282413e-06, /* 0xb684e21a */
/*
 * Coefficients for approximation to  erf  in [0.84375,1.25]
 */
pa0  = -2.3621185683e-03, /* 0xbb1acdc6 */
pa1  =  4.1485610604e-01, /* 0x3ed46805 */
pa2  = -3.7220788002e-01, /* 0xbebe9208 */
pa3  =  3.1834661961e-01, /* 0x3ea2fe54 */
pa4  = -1.1089469492e-01, /* 0xbde31cc2 */
pa5  =  3.5478305072e-02, /* 0x3d1151b3 */
pa6  = -2.1663755178e-03, /* 0xbb0df9c0 */
qa1  =  1.0642088205e-01, /* 0x3dd9f331 */
qa2  =  5.4039794207e-01, /* 0x3f0a5785 */
qa3  =  7.1828655899e-02, /* 0x3d931ae7 */
qa4  =  1.2617121637e-01, /* 0x3e013307 */
qa5  =  1.3637083583e-02, /* 0x3c5f6e13 */
qa6  =  1.1984500103e-02, /* 0x3c445aa3 */
 /*
 * Coefficients for approximation to  erfc in [1.25,1/0.35]
 */ra0  = -9.8649440333e-03, /* 0xbc21a093 */
ra1  = -6.9385856390e-01, /* 0xbf31a0b7 */
ra2  = -1.0558626175e+01, /* 0xc128f022 */
ra3  = -6.2375331879e+01, /* 0xc2798057 */
ra4  = -1.6239666748e+02, /* 0xc322658c */
ra5  = -1.8460508728e+02, /* 0xc3389ae7 */
ra6  = -8.1287437439e+01, /* 0xc2a2932b */
ra7  = -9.8143291473e+00, /* 0xc11d077e */
sa1  =  1.9651271820e+01, /* 0x419d35ce */
sa2  =  1.3765776062e+02, /* 0x4309a863 */
sa3  =  4.3456588745e+02, /* 0x43d9486f */
sa4  =  6.4538726807e+02, /* 0x442158c9 */
sa5  =  4.2900814819e+02, /* 0x43d6810b */
sa6  =  1.0863500214e+02, /* 0x42d9451f */
sa7  =  6.5702495575e+00, /* 0x40d23f7c */
sa8  = -6.0424413532e-02, /* 0xbd777f97 */
/*
 * Coefficients for approximation to  erfc in [1/.35,28]
 */
rb0  = -9.8649431020e-03, /* 0xbc21a092 */
rb1  = -7.9928326607e-01, /* 0xbf4c9dd4 */
rb2  = -1.7757955551e+01, /* 0xc18e104b */
rb3  = -1.6063638306e+02, /* 0xc320a2ea */
rb4  = -6.3756646729e+02, /* 0xc41f6441 */
rb5  = -1.0250950928e+03, /* 0xc480230b */
rb6  = -4.8351919556e+02, /* 0xc3f1c275 */
sb1  =  3.0338060379e+01, /* 0x41f2b459 */
sb2  =  3.2579251099e+02, /* 0x43a2e571 */
sb3  =  1.5367296143e+03, /* 0x44c01759 */
sb4  =  3.1998581543e+03, /* 0x4547fdbb */
sb5  =  2.5530502930e+03, /* 0x451f90ce */
sb6  =  4.7452853394e+02, /* 0x43ed43a7 */
sb7  = -2.2440952301e+01; /* 0xc1b38712 */

	int hx,ix,i;
	float R,S,P,Q,s,y,z,r;
	GEN_OCL_GET_FLOAT_WORD(hx,x);
	ix = hx&0x7fffffff;
	if(ix>=0x7f800000) {		/* erf(nan)=nan */
	    i = ((unsigned int)hx>>31)<<1;
	    return (float)(1-i)+one/x;	/* erf(+-inf)=+-1 */
	}

	if(ix < 0x3f580000) {		/* |x|<0.84375 */
	    if(ix < 0x31800000) { 	/* |x|<2**-28 */
	        if (ix < 0x04000000)
		    /*avoid underflow */
		    return (float)0.125*((float)8.0*x+efx8*x);
		return x + efx*x;
	    }
	    z = x*x;
	    r = mad(z, mad(z, mad(z, mad(z, pp4, pp3), pp2), pp1), pp0);
	    s = mad(z, mad(z, mad(z, mad(z, mad(z, qq5,qq4), qq3), qq2), qq1), one);
	    y = r / s;
	    return mad(x, y, x);
	}
	if(ix < 0x3fa00000) {		/* 0.84375 <= |x| < 1.25 */
	    s = __gen_ocl_internal_fabs(x)-one;
	    P = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, pa6, pa5), pa4), pa3), pa2), pa1), pa0);
	    Q = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, qa6, qa5), qa4), qa3), qa2), qa1), one);
	    if(hx>=0) return erx + P/Q; else return -erx - P/Q;
	}
	if (ix >= 0x40c00000) {		/* inf>|x|>=6 */
	    if(hx>=0) return one-tiny; else return tiny-one;
	}
	x = __gen_ocl_internal_fabs(x);
    s = one/(x*x);
	if(ix< 0x4036DB6E) {	/* |x| < 1/0.35 */
	    R = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, mad(s,
	    		ra7, ra6), ra5), ra4), ra3), ra2), ra1), ra0);
	    S = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, mad(s,
	    		sa8, sa7), sa6), sa5), sa4), sa3), sa2), sa1), one);
	} else {	/* |x| >= 1/0.35 */
	    R = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s,
	    		rb6, rb5), rb4), rb3), rb2), rb1), rb0);
	    S = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, mad(s,
	    		sb7, sb6), sb5), sb4), sb3), sb2), sb1), one);
	}
	GEN_OCL_GET_FLOAT_WORD(ix,x);
	GEN_OCL_SET_FLOAT_WORD(z,ix&0xfffff000);
	r  =  __gen_ocl_internal_exp(-z*z-(float)0.5625)*__gen_ocl_internal_exp((z-x)*(z+x)+R/S);
	if(hx>=0) return one-r/x; else return  r/x-one;
}
INLINE_OVERLOADABLE float __gen_ocl_internal_erfc(float x) {
/*...*/
const float
tiny = 1.0e-30,
half_val=  5.0000000000e-01, /* 0x3F000000 */
one =  1.0000000000e+00, /* 0x3F800000 */
two =  2.0000000000e+00, /* 0x40000000 */
	/* c = (subfloat)0.84506291151 */
erx =  8.4506291151e-01, /* 0x3f58560b */
/*
 * Coefficients for approximation to  erf on [0,0.84375]
 */
efx =  1.2837916613e-01, /* 0x3e0375d4 */
efx8=  1.0270333290e+00, /* 0x3f8375d4 */
pp0  =  1.2837916613e-01, /* 0x3e0375d4 */
pp1  = -3.2504209876e-01, /* 0xbea66beb */
pp2  = -2.8481749818e-02, /* 0xbce9528f */
pp3  = -5.7702702470e-03, /* 0xbbbd1489 */
pp4  = -2.3763017452e-05, /* 0xb7c756b1 */
qq1  =  3.9791721106e-01, /* 0x3ecbbbce */
qq2  =  6.5022252500e-02, /* 0x3d852a63 */
qq3  =  5.0813062117e-03, /* 0x3ba68116 */
qq4  =  1.3249473704e-04, /* 0x390aee49 */
qq5  = -3.9602282413e-06, /* 0xb684e21a */
/*
 * Coefficients for approximation to  erf  in [0.84375,1.25]
 */
pa0  = -2.3621185683e-03, /* 0xbb1acdc6 */
pa1  =  4.1485610604e-01, /* 0x3ed46805 */
pa2  = -3.7220788002e-01, /* 0xbebe9208 */
pa3  =  3.1834661961e-01, /* 0x3ea2fe54 */
pa4  = -1.1089469492e-01, /* 0xbde31cc2 */
pa5  =  3.5478305072e-02, /* 0x3d1151b3 */
pa6  = -2.1663755178e-03, /* 0xbb0df9c0 */
qa1  =  1.0642088205e-01, /* 0x3dd9f331 */
qa2  =  5.4039794207e-01, /* 0x3f0a5785 */
qa3  =  7.1828655899e-02, /* 0x3d931ae7 */
qa4  =  1.2617121637e-01, /* 0x3e013307 */
qa5  =  1.3637083583e-02, /* 0x3c5f6e13 */
qa6  =  1.1984500103e-02, /* 0x3c445aa3 */
 /*
 * Coefficients for approximation to  erfc in [1.25,1/0.35]
 */ra0  = -9.8649440333e-03, /* 0xbc21a093 */
ra1  = -6.9385856390e-01, /* 0xbf31a0b7 */
ra2  = -1.0558626175e+01, /* 0xc128f022 */
ra3  = -6.2375331879e+01, /* 0xc2798057 */
ra4  = -1.6239666748e+02, /* 0xc322658c */
ra5  = -1.8460508728e+02, /* 0xc3389ae7 */
ra6  = -8.1287437439e+01, /* 0xc2a2932b */
ra7  = -9.8143291473e+00, /* 0xc11d077e */
sa1  =  1.9651271820e+01, /* 0x419d35ce */
sa2  =  1.3765776062e+02, /* 0x4309a863 */
sa3  =  4.3456588745e+02, /* 0x43d9486f */
sa4  =  6.4538726807e+02, /* 0x442158c9 */
sa5  =  4.2900814819e+02, /* 0x43d6810b */
sa6  =  1.0863500214e+02, /* 0x42d9451f */
sa7  =  6.5702495575e+00, /* 0x40d23f7c */
sa8  = -6.0424413532e-02, /* 0xbd777f97 */
/*
 * Coefficients for approximation to  erfc in [1/.35,28]
 */
rb0  = -9.8649431020e-03, /* 0xbc21a092 */
rb1  = -7.9928326607e-01, /* 0xbf4c9dd4 */
rb2  = -1.7757955551e+01, /* 0xc18e104b */
rb3  = -1.6063638306e+02, /* 0xc320a2ea */
rb4  = -6.3756646729e+02, /* 0xc41f6441 */
rb5  = -1.0250950928e+03, /* 0xc480230b */
rb6  = -4.8351919556e+02, /* 0xc3f1c275 */
sb1  =  3.0338060379e+01, /* 0x41f2b459 */
sb2  =  3.2579251099e+02, /* 0x43a2e571 */
sb3  =  1.5367296143e+03, /* 0x44c01759 */
sb4  =  3.1998581543e+03, /* 0x4547fdbb */
sb5  =  2.5530502930e+03, /* 0x451f90ce */
sb6  =  4.7452853394e+02, /* 0x43ed43a7 */
sb7  = -2.2440952301e+01; /* 0xc1b38712 */
	int hx,ix;
	float R,S,P,Q,s,y,z,r;
	GEN_OCL_GET_FLOAT_WORD(hx,x);
	ix = hx&0x7fffffff;
	if(ix>=0x7f800000) {			/* erfc(nan)=nan */
						/* erfc(+-inf)=0,2 */
	    return (float)(((unsigned int)hx>>31)<<1)+one/x;
	}

	if(ix < 0x3f580000) {		/* |x|<0.84375 */
	    if(ix < 0x23800000)  	/* |x|<2**-56 */
		return one-x;
	    z = x*x;
	    r = mad(z, mad(z, mad(z, mad(z, pp4, pp3), pp2), pp1), pp0);
	    s = mad(z, mad(z, mad(z, mad(z, mad(z, qq5, qq4), qq3), qq2), qq1), one);
	    y = r/s;
	    if(hx < 0x3e800000) {  	/* x<1/4 */
		return one-(x+x*y);
	    } else {
		r = x*y;
		r += (x-half_val);
	        return half_val - r ;
	    }
	}
	if(ix < 0x3fa00000) {		/* 0.84375 <= |x| < 1.25 */
	    s = __gen_ocl_internal_fabs(x)-one;
	    P = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, pa6, pa5), pa4), pa3), pa2), pa1), pa0);
	    Q = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, qa6, qa5), qa4), qa3), qa2), qa1), one);
	    if(hx>=0) {
	        z  = one-erx; return z - P/Q;
	    } else {
		z = erx+P/Q; return one+z;
	    }
	}
	if (ix < 0x41e00000) {		/* |x|<28 */
	    x = __gen_ocl_internal_fabs(x);
        s = one/(x*x);
	    if(ix< 0x4036DB6D) {	/* |x| < 1/.35 ~ 2.857143*/
		    R = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, mad(s,
		    		ra7, ra6), ra5), ra4), ra3), ra2), ra1), ra0);
		    S = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, mad(s,
		    		sa8, sa7), sa6), sa5), sa4), sa3), sa2), sa1), one);
	    } else {			/* |x| >= 1/.35 ~ 2.857143 */
		if(hx<0&&ix>=0x40c00000) return two-tiny;/* x < -6 */
		    R = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s,
		    		rb6, rb5), rb4), rb3), rb2), rb1), rb0);
		    S = mad(s, mad(s, mad(s, mad(s, mad(s, mad(s, mad(s,
		    		sb7, sb6), sb5), sb4), sb3), sb2), sb1), one);
	    }
	    GEN_OCL_GET_FLOAT_WORD(ix,x);
	    GEN_OCL_SET_FLOAT_WORD(z,ix&0xffffe000);
	    r  =  __gen_ocl_internal_exp(-z*z-(float)0.5625)*
			__gen_ocl_internal_exp((z-x)*(z+x)+R/S);
	    if(hx>0) {
		float ret = r/x;
		return ret;
	    } else
		return two-r/x;
	} else {
	    if(hx>0) {
		return tiny*tiny;
	    } else
		return two-tiny;
	}
}

OVERLOADABLE float __gen_ocl_internal_fmod (float x, float y) {
  //return x-y*__gen_ocl_rndz(x/y);
  float one = 1.0;
  float Zero[2];
  int n,hx,hy,hz,ix,iy,sx,i;
  Zero[0] = 0.0;
  Zero[1] = -0.0;
  GEN_OCL_GET_FLOAT_WORD(hx,x);
  GEN_OCL_GET_FLOAT_WORD(hy,y);
  sx = hx&0x80000000;		/* sign of x */
  hx ^=sx;		/* |x| */
  hy &= 0x7fffffff;	/* |y| */
  /* purge off exception values */
  if(hy==0||(hx>=0x7f800000)||		/* y=0,or x not finite */
  (hy>0x7f800000))			/* or y is NaN */
    return (x*y)/(x*y);
  if(hx<hy) return x;			/* |x|<|y| return x */
  if(hx==hy)
    return Zero[(unsigned)sx>>31];	/* |x|=|y| return x*0*/

  /* determine ix = ilogb(x) */
  if(hx<0x00800000) {	/* subnormal x */
    for (ix = -126,i=(hx<<8); i>0; i<<=1) ix -=1;
  } else ix = (hx>>23)-127;

  /* determine iy = ilogb(y) */
  if(hy<0x00800000) {	/* subnormal y */
    for (iy = -126,i=(hy<<8); i>=0; i<<=1) iy -=1;
  } else iy = (hy>>23)-127;

  /* set up {hx,lx}, {hy,ly} and align y to x */
  if(ix >= -126)
    hx = 0x00800000|(0x007fffff&hx);
  else {		/* subnormal x, shift x to normal */
    n = -126-ix;
    hx = hx<<n;
  }
  if(iy >= -126)
    hy = 0x00800000|(0x007fffff&hy);
  else {		/* subnormal y, shift y to normal */
    n = -126-iy;
    hy = hy<<n;
  }
  /* fix point fmod */
  n = ix - iy;
  while(n--) {
    hz=hx-hy;
    if(hz<0){hx = hx+hx;}
    else {
      if(hz==0)		/* return sign(x)*0 */
        return Zero[(unsigned)sx>>31];
      hx = hz+hz;
    }
  }
  hz=hx-hy;
  if(hz>=0) {hx=hz;}

    /* convert back to floating value and restore the sign */
  if(hx==0)			/* return sign(x)*0 */
    return Zero[(unsigned)sx>>31];
  while(hx<0x00800000) {		/* normalize x */
    hx = hx+hx;
    iy -= 1;
  }
  if(iy>= -126) {		/* normalize output */
    hx = ((hx-0x00800000)|((iy+127)<<23));
	GEN_OCL_SET_FLOAT_WORD(x,hx|sx);
   } else {		/* subnormal output */
     n = -126 - iy;
     hx >>= n;
     GEN_OCL_SET_FLOAT_WORD(x,hx|sx);
     x *= one;		/* create necessary signal */
  }
  return x;		/* exact output */
}

OVERLOADABLE float __gen_ocl_internal_expm1(float x) {
  //return __gen_ocl_pow(M_E_F, x) - 1;
  float	Q1 = -3.3333335072e-02, /* 0xbd088889 */
  ln2_hi = 6.9313812256e-01,	/* 0x3f317180 */
  ln2_lo = 9.0580006145e-06,	/* 0x3717f7d1 */
  Q2 = 1.5873016091e-03, /* 0x3ad00d01 */
  huge = 1.0e30,
  tiny = 1.0e-30,
  ivln2 = 1.4426950216e+00, /* 0x3fb8aa3b =1/ln2 */
  one	=  1.0,
  o_threshold=  8.8721679688e+01;  /* 0x42b17180 */
  float y,hi,lo,c,t,e,hxs,hfx,r1;
  int k,xsb;
  int hx;
  GEN_OCL_GET_FLOAT_WORD(hx,x);
  xsb = hx&0x80000000;
  /* sign bit of x */
  //if(xsb==0)
  //y=x;
  //else
  //y= -x; /* y = |x| */
  y = __gen_ocl_internal_fabs(x);
  hx &= 0x7fffffff;		/* high word of |x| */
  /* filter out huge and non-finite argument */
  if(hx >= 0x4195b844) {			/* if |x|>=27*ln2 */
    if(hx >= 0x42b17218) {		/* if |x|>=88.721... */
      if(hx>0x7f800000)
        return x+x; 	 /* NaN */
      if(hx==0x7f800000)
        return (xsb==0)? x:-1.0;/* exp(+-inf)={inf,-1} */
      if(x > o_threshold)
        return huge*huge; /* overflow */
    }
    if(xsb!=0) { /* x < -27*ln2, return -1.0 with inexact */
      if(x+tiny<(float)0.0)	/* raise inexact */
        return tiny-one;	/* return -1 */
    }
  }
  /* argument reduction */
  if(hx > 0x3eb17218) {/* if  |x| > 0.5 ln2 */
    if(hx < 0x3F851592) {/* and |x| < 1.5 ln2 */
      if(xsb==0){
        hi = x - ln2_hi; lo = ln2_lo;  k =  1;
      }	else {
        hi = x + ln2_hi; lo = -ln2_lo;  k = -1;
      }
    } else {
      k  = ivln2*x+((xsb==0)?(float)0.5:(float)-0.5);
      t  = k;
      hi = x - t*ln2_hi;/* t*ln2_hi is exact here */
      lo = t*ln2_lo;
    }
    x  = hi - lo;
    c  = (hi-x)-lo;
  } else if(hx < 0x33000000) {	/* when |x|<2**-25, return x */
    //t = huge+x; /* return x with inexact flags when x!=0 */
    //return x - (t-(huge+x));
    return x;
  } else k = 0;
  /* x is now in primary range */
  hfx = (float)0.5*x;
  hxs = x*hfx;
  r1 = one+hxs*(Q1+hxs*Q2);
  t = (float)3.0-r1*hfx;
  e = hxs*((r1-t)/((float)6.0 - x*t));
  if(k==0)
    return x - (x*e-hxs);		/* c is 0 */
  else{
    e = (x*(e-c)-c);
    e -= hxs;
    if(k== -1)return (float)0.5*(x-e)-(float)0.5;
    if(k==1){
      if(x < (float)-0.25)
        return -(float)2.0*(e-(x+(float)0.5));
      else
        return  (one+(float)2.0*(x-e));
    }
    if (k <= -2 || k>56) {	 /* suffice to return exp(x)-1 */
      int i;
      y = one-(e-x);
      GEN_OCL_GET_FLOAT_WORD(i,y);
      GEN_OCL_SET_FLOAT_WORD(y,i+(k<<23));	/* add k to y's exponent */
      return y-one;
    }
    t = one;
    if(k<23) {
      int i;
      GEN_OCL_SET_FLOAT_WORD(t,0x3f800000 - (0x1000000>>k)); /* t=1-2^-k */
      y = t-(e-x);
      GEN_OCL_GET_FLOAT_WORD(i,y);
      GEN_OCL_SET_FLOAT_WORD(y,i+(k<<23));	/* add k to y's exponent */
    } else {
      int i;
      GEN_OCL_SET_FLOAT_WORD(t,((0x7f-k)<<23));	/* 2^-k */
      y = x-(e+t);
      y += one;
      GEN_OCL_GET_FLOAT_WORD(i,y);
      GEN_OCL_SET_FLOAT_WORD(y,i+(k<<23));	/* add k to y's exponent */
    }
  }
  return y;
}

OVERLOADABLE float __gen_ocl_internal_acosh(float x) {
  //return native_log(x + native_sqrt(x + 1) * native_sqrt(x - 1));
  float one	= 1.0,
  ln2	= 6.9314718246e-01;/* 0x3f317218 */
  float t;
  int hx;
  GEN_OCL_GET_FLOAT_WORD(hx,x);
  if(hx<0x3f800000) {	/* x < 1 */
    return (x-x)/(x-x);
  } else if(hx >=0x4d800000) {	/* x > 2**28 */
    if(hx >=0x7f800000) {/* x is inf of NaN */
      return x+x;
    } else
      return __gen_ocl_internal_log(x)+ln2;/* acosh(huge)=log(2x) */
  } else if (hx==0x3f800000) {
    return 0.0;			/* acosh(1) = 0 */
  } else if (hx > 0x40000000) {	/* 2**28 > x > 2 */
    t=x*x;
    return __gen_ocl_internal_log((float)2.0*x-one/(x+__gen_ocl_sqrt(t-one)));
  } else {			/* 1<x<2 */
    t = x-one;
    return log1p(t+__gen_ocl_sqrt((float)2.0*t+t*t));
  }
}

OVERLOADABLE float __gen_ocl_internal_asinh(float x){
  //return native_log(x + native_sqrt(x * x + 1));
  float one =  1.0000000000e+00, /* 0x3F800000 */
  ln2 =  6.9314718246e-01, /* 0x3f317218 */
  huge=  1.0000000000e+30;
  float w;
  int hx,ix;
  GEN_OCL_GET_FLOAT_WORD(hx,x);
  ix = hx&0x7fffffff;
  if(ix< 0x38000000) {	/* |x|<2**-14 */
    if(huge+x>one) return x;	/* return x inexact except 0 */
  }
  if(ix>0x47000000) {/* |x| > 2**14 */
    if(ix>=0x7f800000) return x+x;/* x is inf or NaN */
    w = __gen_ocl_internal_log(__gen_ocl_internal_fabs(x))+ln2;
  } else {
    float xa = __gen_ocl_internal_fabs(x);
    if (ix>0x40000000) {/* 2**14 > |x| > 2.0 */
      w = __gen_ocl_internal_log(mad(xa, 2.0f, one / (__gen_ocl_sqrt(mad(xa, xa, one)) + xa)));
    } else {		/* 2.0 > |x| > 2**-14 */
      float t = xa*xa;
      w =log1p(xa+t/(one+__gen_ocl_sqrt(one+t)));
    }
  }
  return __gen_ocl_internal_copysign(w, x);
}

OVERLOADABLE float __gen_ocl_internal_sinh(float x){
  //return (1 - native_exp(-2 * x)) / (2 * native_exp(-x));
  float one = 1.0,
  shuge = 1.0e37;
  float t,w,h;
  int ix,jx;
  GEN_OCL_GET_FLOAT_WORD(jx,x);
  ix = jx&0x7fffffff;
  /* x is INF or NaN */
  if(ix>=0x7f800000) return x+x;
  h = 0.5;
  if (jx<0) h = -h;
  /* |x| in [0,22], return sign(x)*0.5*(E+E/(E+1))) */
  if (ix < 0x41b00000) {		/* |x|<22 */
    if (ix<0x31800000)	/* |x|<2**-28 */
      if(shuge+x>one) return x;/* sinh(tiny) = tiny with inexact */
    t = __gen_ocl_internal_expm1(__gen_ocl_internal_fabs(x));
    if(ix<0x3f800000) return h*((float)2.0*t-t*t/(t+one));
      return h*(t+t/(t+one));
  }
  /* |x| in [22, log(maxdouble)] return 0.5*exp(|x|) */
  if (ix < 0x42b17180)  return h*__gen_ocl_internal_exp(__gen_ocl_internal_fabs(x));
  /* |x| in [log(maxdouble), overflowthresold] */
  if (ix<=0x42b2d4fc) {
    w = __gen_ocl_internal_exp((float)0.5*__gen_ocl_internal_fabs(x));
    t = h*w;
    return t*w;
  }
  /* |x| > overflowthresold, sinh(x) overflow */
  return x*shuge;
}

OVERLOADABLE float __gen_ocl_internal_tanh(float x) {
  //float y = native_exp(-2 * x);
  //return (1 - y) / (1 + y);
  float one=1.0, two=2.0, tiny = 1.0e-30;
  float t,z;
  int jx,ix;
  GEN_OCL_GET_FLOAT_WORD(jx,x);
  ix = jx&0x7fffffff;
  /* x is INF or NaN */
  if(ix>=0x7f800000) {
    if (jx>=0)
      return one/x+one; /* tanh(+-inf)=+-1 */
    else
      return one/x-one; /* tanh(NaN) = NaN */
  }

  if (ix < 0x41b00000) { /* |x|<22 */
    if (ix == 0)
      return x;		/* x == +-0 */
    if (ix<0x24000000) 	/* |x|<2**-55 */
      return x*(one+x);    	/* tanh(small) = small */
    if (ix>=0x3f800000) {	/* |x|>=1  */
      t = __gen_ocl_internal_expm1(two*__gen_ocl_internal_fabs(x));
      z = one - two/(t+two);
    } else {
      t = __gen_ocl_internal_expm1(-two*__gen_ocl_internal_fabs(x));
      z= -t/(t+two);
    }
  } else { /* |x| > 22, return +-1 */
    z = one - tiny;		/* raised inexact flag */
  }
  return (jx>=0)? z: -z;
}

OVERLOADABLE float __gen_ocl_internal_cosh(float x) {
  //return (1 + native_exp(-2 * x)) / (2 * native_exp(-x));
  float halF = 0.5,
  huge = 1.0e+30,
  tiny = 1.0e-30,
  one = 1.0;
  float t,w;
  int ix;
  GEN_OCL_GET_FLOAT_WORD(ix,x);
  ix &= 0x7fffffff;
  /* |x| in [0,22] */
  if (ix < 0x41b00000) {
    /* |x| in [0,0.5*ln2], return 1+expm1(|x|)^2/(2*exp(|x|)) */
    if(ix<0x3eb17218) {
      t = __gen_ocl_internal_expm1(__gen_ocl_fabs(x));
      w = one+t;
      if (ix<0x24000000) return w;	/* cosh(tiny) = 1 */
      return one+(t*t)/(w+w);
    }
    /* |x| in [0.5*ln2,22], return (exp(|x|)+1/exp(|x|)/2; */
    t = __gen_ocl_internal_exp(__gen_ocl_fabs(x));
    return halF*t+halF/t;
  }
  /* |x| in [22, log(maxdouble)] return half*exp(|x|) */
  if (ix < 0x42b17180)  return halF*__gen_ocl_internal_exp(__gen_ocl_fabs(x));
  /* |x| in [log(maxdouble), overflowthresold] */
  if (ix<=0x42b2d4fc) {
    w = __gen_ocl_internal_exp(halF*__gen_ocl_fabs(x));
    t = halF*w;
    return t*w;
  }
  /* x is INF or NaN */
  if(ix>=0x7f800000) return x*x;
  /* |x| > overflowthresold, cosh(x) overflow */
  return huge*huge;
}

OVERLOADABLE float __gen_ocl_internal_remainder(float x, float p){
  //return x-y*__gen_ocl_rnde(x/y);
  float zero = 0.0;
  int hx,hp;
  unsigned sx;
  float p_half;
  GEN_OCL_GET_FLOAT_WORD(hx,x);
  GEN_OCL_GET_FLOAT_WORD(hp,p);
  sx = hx&0x80000000;
  hp &= 0x7fffffff;
  hx &= 0x7fffffff;
  /* purge off exception values */
  if(hp==0) return (x*p)/(x*p);	        /* p = 0 */
  if((hx>=0x7f800000)||               /* x not finite */
    ((hp>0x7f800000)))	               /* p is NaN */
    return (x*p)/(x*p);
  if (hp<=0x7effffff) x = __gen_ocl_internal_fmod(x,p+p); /* now x < 2p */
  if ((hx-hp)==0) return zero*x;
  x = __gen_ocl_fabs(x);
  p = __gen_ocl_fabs(p);
  if (hp<0x01000000) {
    if(x+x>p) {
      x-=p;
      if(x+x>=p) x -= p;
    }
  } else {
    p_half = (float)0.5*p;
    if(x>p_half) {
      x-=p;
      if(x>=p_half) x -= p;
    }
  }
  GEN_OCL_GET_FLOAT_WORD(hx,x);
  GEN_OCL_SET_FLOAT_WORD(x,hx^sx);
  return x;
}

OVERLOADABLE float __gen_ocl_internal_ldexp(float x, int n) {
  x = __gen_ocl_scalbnf(x,n);
  return x;
}

OVERLOADABLE float __gen_ocl_internal_atanh(float x) {
  //return 0.5f * native_sqrt((1 + x) / (1 - x));
  float xa = __gen_ocl_fabs (x);
  float t;
  if (isless (xa, 0.5f)){
    if (xa < 0x1.0p-28f) return x;
    t = xa + xa;
    t = 0.5f * log1p (t + t * xa / (1.0f - xa));
  } else if (isless (xa, 1.0f)){
    t = 0.5f * log1p ((xa + xa) / (1.0f - xa));
  } else{
    if (isgreater (xa, 1.0f)) return (x - x) / (x - x);
    return x / 0.0f;
  }
  return __gen_ocl_internal_copysign(t, x);
}

OVERLOADABLE float __gen_ocl_internal_exp10(float x){
  float px, qx,ans;
  short n;
  int i;
  float*p;
  float MAXL10 = 38.230809449325611792;
  float LOG210 = 3.32192809488736234787e0;
  float LG102A = 3.00781250000000000000E-1;
  float LG102B = 2.48745663981195213739E-4;
  float P[6];
  P[0] = 2.063216740311022E-001;
  P[1] = 5.420251702225484E-001;
  P[2] = 1.171292686296281E+000;
  P[3] = 2.034649854009453E+000;
  P[4] = 2.650948748208892E+000;
  P[5] = 2.302585167056758E+000;

  if( x < -MAXL10 ) return 0.0;

  if( isinf(x))  return INFINITY;
  /* The following is necessary because range reduction blows up: */
  if( x == 0 )return 1.0;

  /* Express 10**x = 10**g 2**n
    *	 = 10**g 10**( n log10(2) )
    *	 = 10**( g + n log10(2) )
    */
  px = x * LOG210;
  qx = __gen_ocl_internal_floor( px + 0.5 );
  n = qx;
  x -= qx * LG102A;
  x -= qx * LG102B;

  /* rational approximation for exponential
    * of the fractional part:
    * 10**x - 1  =  2x P(x**2)/( Q(x**2) - P(x**2) )
    */
  p = P;
  ans = *p++;
  i = 5;
  do{
    ans = ans * x  +  *p++;
  }
  while( --i );
  px = 1.0 + x * ans;

  /* multiply by power of 2 */
  x = __gen_ocl_internal_ldexp( px, n );
  return x;
}

OVERLOADABLE float cospi(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_cospi(x);

  return __gen_ocl_internal_cospi(x);
}

OVERLOADABLE float cosh(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_cosh(x);

  return  __gen_ocl_internal_cosh(x);
}

OVERLOADABLE float acos(float x) {
  return __gen_ocl_internal_acos(x);
}

OVERLOADABLE float acospi(float x) {
  return __gen_ocl_internal_acospi(x);
}

OVERLOADABLE float acosh(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_acosh(x);

  return __gen_ocl_internal_acosh(x);
}

OVERLOADABLE float sinpi(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_sinpi(x);

  return __gen_ocl_internal_sinpi(x);
}

OVERLOADABLE float sinh(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_sinh(x);

  return __gen_ocl_internal_sinh(x);
}

OVERLOADABLE float asin(float x) {
  return __gen_ocl_internal_asin(x);
}

OVERLOADABLE float asinpi(float x) {
  return __gen_ocl_internal_asinpi(x);
}

OVERLOADABLE float asinh(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_asinh(x);

  return __gen_ocl_internal_asinh(x);
}

OVERLOADABLE float tanpi(float x) {
  return __gen_ocl_internal_tanpi(x);
}

OVERLOADABLE float tanh(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_tanh(x);

  return __gen_ocl_internal_tanh(x);
}

OVERLOADABLE float atan(float x) {
  return __gen_ocl_internal_atan(x);
}

OVERLOADABLE float atan2(float y, float x) {
  return __gen_ocl_internal_atan2(y, x);
}

OVERLOADABLE float atan2pi(float y, float x) {
  return __gen_ocl_internal_atan2pi(y, x);
}

OVERLOADABLE float atanpi(float x) {
  return __gen_ocl_internal_atanpi(x);
}

OVERLOADABLE float atanh(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_atanh(x);

  return __gen_ocl_internal_atanh(x);
}

OVERLOADABLE float cbrt(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_cbrt(x);

  return __gen_ocl_internal_cbrt(x);
}

OVERLOADABLE float rint(float x) {
  return __gen_ocl_internal_rint(x);
}

OVERLOADABLE float copysign(float x, float y) {
  return __gen_ocl_internal_copysign(x, y);
}

OVERLOADABLE float erf(float x) {
  return __gen_ocl_internal_erf(x);
}

OVERLOADABLE float erfc(float x) {
  return __gen_ocl_internal_erfc(x);
}

OVERLOADABLE float fmod (float x, float y) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_fmod(x, y);

  return __gen_ocl_internal_fmod(x, y);
}

OVERLOADABLE float remainder(float x, float p) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_remainder(x, p);

  return __gen_ocl_internal_remainder(x, p);
}

OVERLOADABLE float ldexp(float x, int n) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_ldexp(x, n);

  if (x == (float)0.0f) x = 0.0f;
  return __gen_ocl_internal_ldexp(x, n);
}

CONST OVERLOADABLE float __gen_ocl_mad(float a, float b, float c) __asm("llvm.fma" ".f32");
CONST OVERLOADABLE half __gen_ocl_mad(half a, half b, half c) __asm("llvm.fma" ".f16");
PURE CONST float __gen_ocl_fmax(float a, float b);
PURE CONST float __gen_ocl_fmin(float a, float b);

OVERLOADABLE float mad(float a, float b, float c) {
  return __gen_ocl_mad(a, b, c);
}

OVERLOADABLE float __gen_ocl_internal_fmax(float a, float b) { return max(a,b); }
OVERLOADABLE float __gen_ocl_internal_fmin(float a, float b) { return min(a,b); }
OVERLOADABLE float __gen_ocl_internal_fmax(half a, half b) { return max(a,b); }
OVERLOADABLE float __gen_ocl_internal_fmin(half a, half b) { return min(a,b); }
OVERLOADABLE float __gen_ocl_internal_maxmag(float x, float y) {
  float a = __gen_ocl_fabs(x), b = __gen_ocl_fabs(y);
  return a > b ? x : b > a ? y : max(x, y);
}
OVERLOADABLE float __gen_ocl_internal_minmag(float x, float y) {
  float a = __gen_ocl_fabs(x), b = __gen_ocl_fabs(y);
  return a < b ? x : b < a ? y : min(x, y);
}
OVERLOADABLE float __gen_ocl_internal_fdim(float x, float y) {
  if(isnan(x))
    return x;
  if(isnan(y))
    return y;
  return x > y ? (x - y) : +0.f;
}
/*
 * the pow/pown high precision implementation are copied from msun library.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

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

OVERLOADABLE float __gen_ocl_internal_pow(float x, float y) {
  float z,ax,z_h,z_l,p_h,p_l;
  float y1,t1,t2,r,s,sn,t,u,v,w;
  int i,j,k,yisint,n;
  int hy,ix,iy,is;
  unsigned int hx;
  float bp,dp_h,dp_l,
  zero    =  0.0,
  one	=  1.0,
  two	=  2.0,
  two24	=  16777216.0,	/* 0x4b800000 */
  huge	=  1.0e30,
  tiny    =  1.0e-30,
  /* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
  L1  =  6.0000002384e-01, /* 0x3f19999a */
  L2  =  4.2857143283e-01, /* 0x3edb6db7 */
  P1   =  1.6666667163e-01, /* 0x3e2aaaab */
  P2   = -2.7777778450e-03, /* 0xbb360b61 */
  lg2  =  6.9314718246e-01, /* 0x3f317218 */
  lg2_h  =  6.93145752e-01, /* 0x3f317200 */
  lg2_l  =  1.42860654e-06, /* 0x35bfbe8c */
  ovt =  4.2995665694e-08, /* -(128-log2(ovfl+.5ulp)) */
  cp    =  9.6179670095e-01, /* 0x3f76384f =2/(3ln2) */
  cp_h  =  9.6179199219e-01, /* 0x3f763800 =head of cp */
  cp_l  =  4.7017383622e-06, /* 0x369dc3a0 =tail of cp_h */
  ivln2    =  1.4426950216e+00, /* 0x3fb8aa3b =1/ln2 */
  ivln2_h  =  1.4426879883e+00, /* 0x3fb8aa00 =16b 1/ln2*/
  ivln2_l  =  7.0526075433e-06; /* 0x36eca570 =1/ln2 tail*/
  bp = 1.0;//,bp1 = 1.5,
  dp_h = 0.0;//,dp_h[1] = 5.84960938e-01,
  dp_l = 0.0;//,dp_l[1] = 1.56322085e-06;

  float retVal = 0.0f;
  bool bRet = false;

  hx = as_uint(x);
  GEN_OCL_GET_FLOAT_WORD(hy,y);
  ax   = __gen_ocl_fabs(x);
  ix = as_int(ax);  iy = as_int(fabs(y));

  if(iy < 0x00800000)
  {
     bRet = true;
     retVal = one;
  }
  else if (iy > 0x7f800000)
  {
       bRet = true;
       retVal = NAN;
  }

  /* determine if y is an odd int when x < 0
     * yisint = 0	... y is not an integer
     * yisint = 1	... y is an odd int
     * yisint = 2	... y is an even int
     */
  sn = one; /* s (sign of result -ve**odd) = -1 else = 1 */

  if(hx >= 0x7f800000)
  {
    yisint  = 0;
    n = (hx>>31)-1;

    if (!retVal && ix > 0x7f800000)
    {
      bRet = true;
      retVal = NAN;
    }

    if(hx >= 0x80000000) {
      k = (iy>>23)-0x7f;		 /* exponent */
      j = iy>>(23-k);
      yisint = (iy>=0x3f800000 && (j<<(23-k))==iy)? 2-(j&1):yisint;
      yisint = (iy>=0x4b800000) ? 2:yisint;
    }

      /* special value of x */
    if(ix==0x7f800000||ix==0||ix==0x3f800000){
      z = ax;     /*x is +-0,+-inf,+-1*/
      z = (hy < 0)? one/z:z;
      z = (((ix-0x3f800000)|yisint)==0)? NAN:z;
      z = (yisint==1)? -z:z;
      retVal = (bRet)? retVal:z;
      bRet = true;
    }

    /* (x<0)**(non-int) is NaN */
    if(!bRet && (n|yisint)==0)
    {
       bRet= true;
       retVal = NAN;
    }

    if((n|(yisint-1))==0) sn = -one;/* (-ve)**(odd int) */
  }

    /* special value of x */
  if((ix&0x7fffff) == 0) {
    if(hx == 0x3f800000)
    {
      retVal = one;
      bRet = true;
    }

    if(ix==0x7f800000||ix==0) {
      z = ax;			/*x is +0,+inf*/
      z = (hy < 0)? one/z:z;
      retVal = (bRet)? retVal:z;
      bRet = true;
    }
  }

  /* |y| is not huge */
  if(iy <= 0x4d000000)
  {
    float s2,s_h,s_l,t_h,t_l;
    n  = ((ix)>>23)-0x7f;
    j  = ix&0x007fffff;
    /* determine interval */
    ix = j|0x3f800000; /* normalize ix */

    if(x > 0x1.0p-6 && x < 0x1.2p7 && fabs(y) < 0x1.0p4)
    {
      GEN_OCL_SET_FLOAT_WORD(ax,ix);
      t1 = n;
      t2 = native_log2(ax);
    }
    else
    {
      n = (j >= 0x5db3d7)?n+1:n;
      ix = (j >= 0x5db3d7)? ix - 0x00800000:ix;
      k = (j<=0x1cc471 || j >= 0x5db3d7)? 0:1;

      GEN_OCL_SET_FLOAT_WORD(ax,ix);

      bp = k? 1.5:bp;
      dp_h = k? 5.84960938e-01:dp_h;
      dp_l = k? 1.56322085e-06:dp_l;

      /* compute s = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
      u = ax-bp;		/* bp[0]=1.0, bp[1]=1.5 */
      v = one/(ax+bp);
      s = u*v;
      s_h = s;
      GEN_OCL_GET_FLOAT_WORD(is,s_h);
      GEN_OCL_SET_FLOAT_WORD(s_h,is&0xfffff000);
      /* t_h=ax+bp[k] High */
      is = ((ix>>1)&0xfffff000)|0x20000000;
      GEN_OCL_SET_FLOAT_WORD(t_h,is+0x00400000+(k<<21));
      t_l = ax - (t_h-bp);
      s_l = v*(mad(-s_h, t_l, mad(-s_h, t_h, u)));

      /* compute log(ax) */
      s2 = s*s;
      r = s2*s2*(mad(s2, L2, L1));
      r = mad(s_l, (s_h+s), r);
      t_h = mad(s_h, s_h, 3.0f)+r;
      GEN_OCL_GET_FLOAT_WORD(is,t_h);
      GEN_OCL_SET_FLOAT_WORD(t_h,is&0xffffe000);
      t_l = r-(mad(-s_h, s_h, (t_h-3.0f)));
      /* u+v = s*(1+...) */
      u = s_h*t_h;
      v = mad(s_l, t_h, t_l*s);
      /* 2/(3log2)*(s+...) */
      p_h = mad(s_h, t_h, v);
      GEN_OCL_GET_FLOAT_WORD(is,p_h);
      GEN_OCL_SET_FLOAT_WORD(p_h,is&0xffffe000);
      p_l = v-(mad(-s_h, t_h, p_h));
      z_l = mad(cp_l, p_h, mad(p_l, cp, dp_l));
      /* log2(ax) = (s+..)*2/(3*log2) = n + dp_h + z_h + z_l */
      t = (float)n;
      t1 = ((mad(cp_h, p_h, z_l)+dp_h)+t);
      GEN_OCL_GET_FLOAT_WORD(is,t1);
      GEN_OCL_SET_FLOAT_WORD(t1,is&0xffffe000);
      t2 = z_l-mad(-cp_h, p_h, ((t1-t)-dp_h));
    }
  }
  else
  { /* if |y| > 2**27 */
    /* over/underflow if x is not close to one */
    /* special value of y */
    float b1 = (hy>=0)? y: zero;
    float b2 = (hy<0)?-y: zero;
    b1 = (ix > 0x3f800000)? b1:b2;
    retVal = (iy==0x7f800000 && !bRet)? b1:retVal;
    bRet = (iy==0x7f800000 && !bRet)? true: bRet;


    b1 = (hy>0)? sn*huge*huge:0;
    retVal = (ix>0x3f800007 && !bRet)? b1:retVal;
    bRet = (ix>0x3f800007 && !bRet)? true:bRet;

    /* now |1-x| is tiny <= 2**-20, suffice to compute
          log(x) by x-x^2/2+x^3/3-x^4/4 */
    t = ax-1;		/* t has 20 trailing zeros */
    w = (t*t)*((float)0.5-t*(0.333333333333f-t*0.25f));
    u = ivln2_h*t;	/* ivln2_h has 16 sig. bits */
    v = t*ivln2_l-w*ivln2;
    t1 = u+v;
    GEN_OCL_GET_FLOAT_WORD(is,t1);
    GEN_OCL_SET_FLOAT_WORD(t1,is&0xfffff000);
    t2 = v-(t1-u);
  }

  /* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
  GEN_OCL_GET_FLOAT_WORD(is,y);
  GEN_OCL_SET_FLOAT_WORD(y1,is&0xffffe000);
  p_l = mad((y-y1), t1, y*t2);
  p_h = y1*t1;

  z = mad(y1, t1, p_l);
  if(fabs(z) >= 128.0f)
  {
    if(!bRet)
    {
       retVal = (fabs(z) > 150.0f)? 0.0:retVal;
       retVal = (z > 128.0f) ? sn*INFINITY:retVal;
       retVal = (z == 128.0f && p_l+ovt>z-p_h)? sn*INFINITY:retVal;
       retVal = (z == -150.0f && p_l<=z-p_h)? 0.0:retVal;
       bRet = ((fabs(z) > 150.0f) || (z == -150.0f && p_l<=z-p_h) || retVal == sn*INFINITY)? true:false;
    }
  }

  z = exp2(p_l)*exp2(p_h);

  return bRet ? retVal:sn*z;
}

OVERLOADABLE float tgamma (float x)
{
  /* based on glibc __ieee754_gammaf_r by Ulrich Drepper <drepper@cygnus.com> */

  unsigned int hx;
  GEN_OCL_GET_FLOAT_WORD(hx,x);
  if (hx == 0xff800000)
    {
      /* x == -Inf.  According to ISO this is NaN.  */
      return NAN;
    }
  if ((hx & 0x7f800000) == 0x7f800000)
    {
      /* Positive infinity (return positive infinity) or NaN (return
	 NaN).  */
      return x;
    }
  if (x < 0.0f && __gen_ocl_internal_floor (x) == x)
    {
      /* integer x < 0 */
      return NAN;
    }

  if (x >= 36.0f)
    {
      /* Overflow.  */
      return INFINITY;
    }
  else if (x <= 0.0f && x >= -FLT_EPSILON / 4.0f)
    {
      return 1.0f / x;
    }
  else
    {
      float sinpix = __gen_ocl_internal_sinpi(x);
      if (x <= -42.0f)
	/* Underflow.  */
	{return 0.0f * sinpix /*for sign*/;}
      int exp2_adj = 0;
      float x_abs = __gen_ocl_fabs(x);
      float gam0;

      if (x_abs < 4.0f) {
        /* gamma = exp(lgamma) is only accurate for small lgamma */
        float prod,x_adj;
        if (x_abs < 0.5f) {
          prod = 1.0f / x_abs;
          x_adj = x_abs + 1.0f;
        } else if (x_abs <= 1.5f) {
          prod = 1.0f;
          x_adj = x_abs;
        } else if (x_abs < 2.5f) {
          x_adj = x_abs - 1.0f;
          prod = x_adj;
        } else {
          x_adj = x_abs - 2.0f;
          prod = x_adj * (x_abs - 1.0f);
        }
        gam0 = __gen_ocl_internal_exp (lgamma (x_adj)) * prod;
      }
      else {
        /* Compute gamma (X) using Stirling's approximation,
  	 starting by computing pow (X, X) with a power of 2
  	 factored out to avoid intermediate overflow.  */
        float x_int = __gen_ocl_internal_round (x_abs);
        float x_frac = x_abs - x_int;
        int x_log2;
	 float x_mant;
	 bool skip = false;
	 if (isnan(x_abs) || isinf(x_abs)) {
	  x_log2 = 0;
	  x_mant = x_abs;
	  skip = true;
	 }

	 uint u = as_uint(x_abs);
	 uint a = u & 0x7FFFFFFFu;
	 if (a == 0) {
	  x_log2 = 0;
	  x_mant = x_abs;
	  skip = true;
	 }
	 if (a >= 0x800000 && !skip) {
	  x_log2 = (a >> 23) - 126;
	  x_mant = as_float((u & (0x807FFFFFu)) | 0x3F000000);
	  skip = true;
	 }

	 int e = -126;
	 if(!skip)
	 {
		while (a < 0x400000) {
		  e --;
		  a <<= 1;
		}
		a <<= 1;
		x_log2 = e;
		x_mant = as_float((a & (0x807FFFFFu)) | (u & 0x80000000u) | 0x3F000000);
	 }

        if (x_mant < M_SQRT1_2_F)
          {
          x_log2--;
          x_mant *= 2.0f;
          }
        exp2_adj = x_log2 * (int) x_int;
        float ret = (__gen_ocl_internal_pow(x_mant, x_abs)
  		   * exp2 (x_log2 * x_frac)
  		   * __gen_ocl_internal_exp (-x_abs)
  		   * sqrt (2.0f * M_PI_F / x_abs) );

        float x2 = x_abs * x_abs;
        float bsum = (0x3.403404p-12f / x2 -0xb.60b61p-12f) / x2 + 0x1.555556p-4f;
        gam0 = ret + ret * __gen_ocl_internal_expm1 (bsum / x_abs);
      }
      if (x > 0.0f) {return __gen_ocl_internal_ldexp (gam0, exp2_adj);}
      float gam1 = M_PI_F / (-x * sinpix * gam0);
      return __gen_ocl_internal_ldexp (gam1, -exp2_adj);
    }
}
#undef BODY

float __gen_ocl_internal_pown(float x, int y) {
  const float
  bp[] = {1.0, 1.5,},
  dp_h[] = { 0.0, 5.84960938e-01,}, /* 0x3f15c000 */
  dp_l[] = { 0.0, 1.56322085e-06,}, /* 0x35d1cfdc */
  zero    =  0.0,
  one =  1.0,
  two =  2.0,
  two24 =  16777216.0,  /* 0x4b800000 */
  huge  =  1.0e30,
  tiny    =  1.0e-30,
    /* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
  L1  =  6.0000002384e-01, /* 0x3f19999a */
  L2  =  4.2857143283e-01, /* 0x3edb6db7 */
  P1   =  1.6666667163e-01, /* 0x3e2aaaab */
  P2   = -2.7777778450e-03, /* 0xbb360b61 */
  lg2  =  6.9314718246e-01, /* 0x3f317218 */
  lg2_h  =  0x1.62ep-1,
  lg2_l  =  0x1.0bfbe8p-15,
  ovt =  4.2995665694e-08, /* -(128-log2(ovfl+.5ulp)) */
  cp    =  9.6179670095e-01, /* 0x3f76384f =2/(3ln2) */
  cp_h  =  9.6179199219e-01, /* 0x3f763800 =head of cp */
  cp_l  =  4.7017383622e-06, /* 0x369dc3a0 =tail of cp_h */
  ivln2    =  1.4426950216e+00, /* 0x3fb8aa3b =1/ln2 */
  ivln2_h  =  1.4426879883e+00, /* 0x3fb8aa00 =16b 1/ln2*/
  ivln2_l  =  7.0526075433e-06; /* 0x36eca570 =1/ln2 tail*/

  float z,ax,z_h,z_l,p_h,p_l;
  float y1,t1,t2,r,s,t,u,v,w;
  int i,j,k,yisint,n;
  int hx,ix,iy,is;

  GEN_OCL_GET_FLOAT_WORD(hx,x);
  ix = hx&0x7fffffff;
  iy = y > 0 ? y&0x7fffffff : (-y)&0x7fffffff;
    /* y==zero: x**0 = 1 */
  if(y==0) return one;

    /* +-NaN return NAN */
  if(ix > 0x7f800000)
    return NAN;

    /* determine if y is an odd int
     * yisint = 1 ... y is an odd int
     * yisint = 2 ... y is an even int
     */
    yisint = y&1 ? 1 : 2;

  if (y == 1) return x;
  if (y == -1) return one/x;
  if (y == 2) return x*x;

  ax   = __gen_ocl_fabs(x);

   /* special value of x */
  if(ix==0x7f800000||ix==0||ix==0x3f800000){
      z = ax;     /*x is +-0,+-inf,+-1*/
      if(y<0) z = one/z; /* z = (1/|x|) */
      if(hx<0) {
      if(yisint==1)
        z = -z;   /* (x<0)**odd = -(|x|**odd) */
      }
      return z;
  }

  float sn = one; /* s (sign of result -ve**odd) = -1 else = 1 */
  if(((((unsigned)hx>>31)-1)|(yisint-1))==0)
      sn = -one; /* (-ve)**(odd int) */

    /* |y| is huge */
  if(iy>0x08000000) { /* if |y| > 2**27 */
    /* over/underflow if x is not close to one */
      if(ix<0x3f7ffff8) return (y<0)? sn*huge*huge:tiny*tiny;
      if(ix>0x3f800007) return (y>0)? sn*huge*huge:tiny*tiny;
    /* now |1-x| is tiny <= 2**-20, suffice to compute
     log(x) by x-x^2/2+x^3/3-x^4/4 */
      t = ax-1;   /* t has 20 trailing zeros */
      w = (t*t)*((float)0.5-t*((float)0.333333333333-t*(float)0.25));
      u = ivln2_h*t;  /* ivln2_h has 16 sig. bits */
      v = t*ivln2_l-w*ivln2;
      t1 = u+v;
      GEN_OCL_GET_FLOAT_WORD(is,t1);
      GEN_OCL_SET_FLOAT_WORD(t1,is&0xfffff000);
      t2 = v-(t1-u);
  } else {
    float s2,s_h,s_l,t_h,t_l;
    n = 0;
    /* take care subnormal number */
//      if(ix<0x00800000)
//    {ax *= two24; n -= 24; GEN_OCL_GET_FLOAT_WORD(ix,ax); }
    n  += ((ix)>>23)-0x7f;
    j  = ix&0x007fffff;
    /* determine interval */
    ix = j|0x3f800000;    /* normalize ix */
    if(j<=0x1cc471) k=0;  /* |x|<sqrt(3/2) */
    else if(j<0x5db3d7) k=1;  /* |x|<sqrt(3)   */
    else {k=0;n+=1;ix -= 0x00800000;}
    GEN_OCL_SET_FLOAT_WORD(ax,ix);

    /* compute s = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
    u = ax-bp[k];   /* bp[0]=1.0, bp[1]=1.5 */
    v = one/(ax+bp[k]);
    s = u*v;
    s_h = s;
    GEN_OCL_GET_FLOAT_WORD(is,s_h);
    GEN_OCL_SET_FLOAT_WORD(s_h,is&0xfffff000);

    /* t_h=ax+bp[k] High */
    GEN_OCL_SET_FLOAT_WORD(t_h, (((ix>>1)|0x20000000)+0x00400000+(k<<21)) &0xfffff000);
    t_l = ax - (t_h-bp[k]);
    s_l = v*((u-s_h*t_h)-s_h*t_l);


    /* compute log(ax) */
    s2 = s*s;
    r = s2*s2*(L1+s2*L2);
    r += s_l*(s_h+s);
    s2  = s_h*s_h;
    t_h = (float)3.0+s2+r;
    GEN_OCL_GET_FLOAT_WORD(is,t_h);
    GEN_OCL_SET_FLOAT_WORD(t_h,is&0xffffe000);
    t_l = r-((t_h-(float)3.0)-s2);
    /* u+v = s*(1+...) */
    u = s_h*t_h;
    v = s_l*t_h+t_l*s;
    /* 2/(3log2)*(s+...) */
    p_h = u+v;
    GEN_OCL_GET_FLOAT_WORD(is,p_h);
    GEN_OCL_SET_FLOAT_WORD(p_h,is&0xffffe000);
    p_l = v-(p_h-u);
    z_h = cp_h*p_h;   /* cp_h+cp_l = 2/(3*log2) */
    z_l = cp_l*p_h+p_l*cp+dp_l[k];
    /* log2(ax) = (s+..)*2/(3*log2) = n + dp_h + z_h + z_l */
    t = (float)n;
    t1 = (((z_h+z_l)+dp_h[k])+t);
    GEN_OCL_GET_FLOAT_WORD(is,t1);
    GEN_OCL_SET_FLOAT_WORD(t1,is&0xffffe000);
    t2 = z_l-(((t1-t)-dp_h[k])-z_h);
  }

  /* split up y into y1+y2+y3 and compute (y1+y2+y3)*(t1+t2) */

  float fy = (float)y;
  float y3 = (float)(y-(int)fy);
  GEN_OCL_GET_FLOAT_WORD(is,fy);
  GEN_OCL_SET_FLOAT_WORD(y1,is&0xfffff000);

  p_l = (fy-y1)*t1 + y3*t1 + fy*t2 + y3*t2;
  p_h = y1*t1;
  z = p_l+p_h;

  GEN_OCL_GET_FLOAT_WORD(j,z);
  if (j>0x43000000)       /* if z > 128 */
      return sn*huge*huge;       /* overflow */
  else if (j==0x43000000) {     /* if z == 128 */
      if(p_l+ovt>z-p_h) return sn*huge*huge; /* overflow */
  }
  else if ((j&0x7fffffff)>0x43160000)   /* z <= -150 */
      return sn*tiny*tiny;       /* underflow */
  else if (j==0xc3160000){      /* z == -150 */
      if(p_l<=z-p_h) return sn*tiny*tiny;    /* underflow */
  }
    /*
     * compute 2**(p_h+p_l)
     */
  i = j&0x7fffffff;
  k = (i>>23)-0x7f;
  n = 0;
  if(i>0x3f000000) {    /* if |z| > 0.5, set n = [z+0.5] */
      n = j+(0x00800000>>(k+1));
      k = ((n&0x7fffffff)>>23)-0x7f;  /* new k for n */
      GEN_OCL_SET_FLOAT_WORD(t,n&~(0x007fffff>>k));
      n = ((n&0x007fffff)|0x00800000)>>(23-k);
      if(j<0) n = -n;
      p_h -= t;

      z -= n;
  }

  t = z;
  GEN_OCL_GET_FLOAT_WORD(is,t);
  GEN_OCL_SET_FLOAT_WORD(t,is&0xfffff000);
  u = t*lg2_h;
  v = (p_l-(t-p_h))*lg2+t*lg2_l;
  z = u+v;
  w = v-(z-u);
  t  = z*z;
  t1  = z - t*(P1+t*P2);
  r  = (z*t1)/(t1-two)-(w+z*w);
  z  = one-(r-z);
  GEN_OCL_GET_FLOAT_WORD(j,z);
  j += (n<<23);
  if((j>>23)<=0) z = __gen_ocl_scalbnf(z,n);  /* subnormal output */
  else GEN_OCL_SET_FLOAT_WORD(z,j);
  return sn*z;
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
float __gen_ocl_internal_frexp(float x, int *exp) { BODY; }

float __fast_scalbnf(float x, int n) {
  /* copy from fdlibm */
  float two25 = 3.355443200e+07, /* 0x4c000000 */
      twom25 = 2.9802322388e-08, /* 0x33000000 */
      huge = 1.0e+30, tiny = 1.0e-30;
  int k, ix, t, tmp;
  float retVal;

  GEN_OCL_GET_FLOAT_WORD(ix, x);
  k = (ix & 0x7f800000) >> 23; /* extract exponent */
  t = k;
  k = k + n;
  tmp = (ix & 0x807fffff);
  x = as_float(tmp | (k << 23));
  retVal = (k > 0) ? x : 0.0f;
  retVal = (k > 0xfe) ? INFINITY : retVal;
  retVal = (k <= -25) ? 0.0f : retVal;
  x = as_float(tmp | ((k + 25) << 23));
  retVal = ((k > 0) && (k <= 25)) ? x * twom25 : retVal;
  retVal = (t == 0) ? 0.0f : retVal;

  return retVal;
}

OVERLOADABLE float hypot(float x, float y) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_hypot(x, y);

  float a, b, an, bn, cn, retVal;
  int e;
  if (isfinite (x) && isfinite (y)){      /* Determine absolute values.  */
  x = __gen_ocl_fabs (x);
  y = __gen_ocl_fabs (y);
  /* Find the bigger and the smaller one.  */
  a = max(x,y);
  b = min(x,y);

  bool skip = false;
  uint u = as_uint(a);
  uint x = u;
  if (x == 0) {
    e = 0;
    an = x;
    skip = true;
  }

  if (x >= 0x800000) {
    e = (x >> 23) - 126;
    an = as_float((u & (0x807FFFFFu)) | 0x3F000000);
    skip = true;
  }

  if (!skip) {
    int msbOne = clz(x);
    x <<= (msbOne - 8);
    e = -117 - msbOne;
    an = as_float((x & (0x807FFFFFu)) | 0x3F000000);
  }

  bn = __fast_scalbnf(b, -e);
  /* Through the normalization, no unneeded overflow or underflow will occur
   * here.  */
  cn = __gen_ocl_sqrt(mad(an, an, bn * bn));
  retVal = __fast_scalbnf(cn, e);
  } else {
    retVal = NAN; /* x or y is NaN.  Return NaN.  */
    retVal = (isinf(x) || isinf(y))
                 ? INFINITY
                 : retVal; /* x or y is infinite.  Return +Infinity.  */
  }

  return retVal;
}

OVERLOADABLE float powr(float x, float y) {
  unsigned int hx, sx, hy, sy;

  if (__ocl_math_fastpath_flag)
    return __gen_ocl_pow(x,y);
  else {
    if (isnan(x) || isnan(y)) return NAN;
    GEN_OCL_GET_FLOAT_WORD(hx,x);
    GEN_OCL_GET_FLOAT_WORD(hy,y);
    sx = (hx & 0x80000000) >> 31;
    sy = (hy & 0x80000000) >> 31;

    if ((hx&0x7fffffff) < 0x00800000) {	   /* x < 2**-126  */
      x = 0.0f;/* Gen does not support subnormal number now */
      hx = hx &0x80000000;
    }
    if ((hy&0x7fffffff) < 0x00800000) {	  /* y < 2**-126  */
      y = 0.0;/* Gen does not support subnormal number now */
      hy = hy &0x80000000;
    }

    // (x < 0) ** y = NAN (y!=0)
    if ((sx && (hx & 0x7fffffff))) return NAN;

    // +/-0 ** +/-0 = NAN
    if ( !(hx&0x7fffffff) && !(hy&0x7fffffff)) return NAN;

    // +inf ** +/-0 = NAN
    if ( ((hx & 0x7f800000) ==0x7f800000) && !(hy&0x7fffffff)) return NAN;

    // others except nan/inf/0 ** 0 = 1.0
    if (!(hy&0x7fffffff)) return 1.0f;

    // +1 ** inf = NAN; +1 ** finite = 1;
    if (hx == 0x3f800000) {
      return isinf(y) ? NAN : 1.0f;
    }

    if ( !(hx & 0x7fffffff)) {
        // +/-0 ** y<0 = +inf
        // +/-0 ** y>0 = +0
      return sy ? INFINITY : 0.0f;
    }

    return __gen_ocl_internal_pow(x,y);
  }
}

OVERLOADABLE float pown(float x, int n) {
  if (__ocl_math_fastpath_flag) {
    if (x == 0.f && n == 0)
      return 1.f;
    if (x < 0.f && (n&1) )
      return -powr(-x, n);
    return powr(x, n);
  } else {
    int ix;
    GEN_OCL_GET_FLOAT_WORD(ix, x);
    float sign = ix < 0 ? -1.0f : 1.0f;
    if (x == 0.0f) x = sign * 0.0f;

    return __gen_ocl_internal_pown(x, n);
  }
}

OVERLOADABLE float pow(float x, float y) {
  if (!__ocl_math_fastpath_flag)
    return __gen_ocl_internal_pow(x,y);
  else {
    int n;
    if (x == 0.f && y == 0.f)
      return 1.f;
    if (x >= 0.f)
      return powr(x, y);
    n = y;
    if ((float)n == y)//is exact integer
      return pown(x, n);
    return NAN;
  }
}

OVERLOADABLE float rootn(float x, int n) {
  float ax,re;
  int sign = 0;
  int hx;
  if( n == 0 )return NAN;

  GEN_OCL_GET_FLOAT_WORD(hx, x);
  // Gen does not support denorm, flush to zero
  if ((hx & 0x7fffffff) < 0x00800000) {
    x = hx < 0 ? -0.0f : 0.0f;
  }

  //rootn ( x, n )  returns a NaN for x < 0 and n is even.
  if( x < 0 && 0 == (n&1) )
    return NAN;
  if( x == 0.0 ){
    switch( n & 0x80000001 ){
      //rootn ( +-0,  n ) is +0 for even n > 0.
      case 0:
        return 0.0f;
      //rootn ( +-0,  n ) is +-0 for odd n > 0.
      case 1:
        return x;
      //rootn ( +-0,  n ) is +inf for even n < 0.
      case 0x80000000:
        return INFINITY;

      //rootn ( +-0,  n ) is +-inf for odd n < 0.
      case 0x80000001:
        return __gen_ocl_internal_copysign(INFINITY, x);
    }
  }
  ax = __gen_ocl_fabs(x);
  if(x <0.0f && (n&1))
    sign = 1;
  if (__ocl_math_fastpath_flag)
    re = __gen_ocl_pow(ax, 1.f/n);
  else
    re = __gen_ocl_internal_pow(ax,1.f/n);
  if(sign)
    re = -re;
  return re;
}

OVERLOADABLE float fabs(float x) {
  return __gen_ocl_internal_fabs(x);
}

OVERLOADABLE float trunc(float x) {
  return  __gen_ocl_internal_trunc(x);
}

OVERLOADABLE float round(float x) {
  return __gen_ocl_internal_round(x);
}

OVERLOADABLE float floor(float x) {
  return __gen_ocl_internal_floor(x);
}

OVERLOADABLE float ceil(float x) {
  return __gen_ocl_internal_ceil(x);
}

OVERLOADABLE float log(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_log(x);

  /* Use native instruction when it has enough precision */
  if((x > 0x1.1p0) || (x <= 0))
    return __gen_ocl_internal_fastpath_log(x);

  return  __gen_ocl_internal_log(x);
}

OVERLOADABLE float log2(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_log2(x);

  /* Use native instruction when it has enough precision */
  if((x > 0x1.1p0) || (x <= 0))
    return __gen_ocl_internal_fastpath_log2(x);

  return  __gen_ocl_internal_log2(x);
}

OVERLOADABLE float log10(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_log10(x);

  /* Use native instruction when it has enough precision */
  if((x > 0x1.1p0) || (x <= 0))
    return __gen_ocl_internal_fastpath_log10(x);

  return  __gen_ocl_internal_log10(x);
}

OVERLOADABLE float exp(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_exp(x);

  /* Use native instruction when it has enough precision */
  if (fabs(x) < 0x1.6p1)
    return __gen_ocl_internal_fastpath_exp(x);

  return __gen_ocl_internal_simple_exp(x);
}

OVERLOADABLE float exp2(float x) {
  /* Use native instruction when it has enough precision, exp2 always */
  return native_exp2(x);
}

OVERLOADABLE float exp10(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_exp10(x);

  return  __gen_ocl_internal_exp10(x);
}

OVERLOADABLE float expm1(float x) {
  if (__ocl_math_fastpath_flag)
    return __gen_ocl_internal_fastpath_expm1(x);

  return  __gen_ocl_internal_expm1(x);
}

OVERLOADABLE float fmin(float a, float b) {
  return __gen_ocl_internal_fmin(a, b);
}

OVERLOADABLE float fmax(float a, float b) {
  return __gen_ocl_internal_fmax(a, b);
}

OVERLOADABLE float fma(float a, float b, float c) {
  return mad(a, b, c);
}

OVERLOADABLE float fdim(float x, float y) {
  return __gen_ocl_internal_fdim(x, y);
}

OVERLOADABLE float maxmag(float x, float y) {
  return __gen_ocl_internal_maxmag(x, y);
}

OVERLOADABLE float minmag(float x, float y) {
  return __gen_ocl_internal_minmag(x, y);
}

OVERLOADABLE float nextafter(float x, float y) {
  int hx, hy, ix, iy;
  hx = as_int(x);
  hy = as_int(y);
  ix = hx & 0x7fffffff;
  iy = hy & 0x7fffffff;
  if(ix == 0)
    ix = hx & 0x7fffff;
  if(iy == 0)
    iy = hy & 0x7fffff;
  if(ix>0x7f800000 || iy>0x7f800000)
    return x+y;
  if(hx == hy)
    return y;
  if(ix == 0) {
    if(iy == 0)
      return y;
    else
      return as_float((hy&0x80000000) | 1);
  }
  if(hx >= 0) {
    if(hx > hy) {
      hx -= 1;
    } else {
      hx += 1;
    }
  } else {
    if(hy >= 0 || hx > hy){
      hx -= 1;
    } else {
      hx += 1;
    }
  }
  return as_float(hx);
}

/* So far, the HW do not support half float math function.
   We just do the conversion and call the float version here. */
OVERLOADABLE half cospi(half x) {
  float _x = (float)x;
  return (half)cospi(_x);
}
OVERLOADABLE half cosh(half x) {
  float _x = (float)x;
  return (half)cosh(_x);
}
OVERLOADABLE half acos(half x) {
  float _x = (float)x;
  return (half)acos(_x);
}
OVERLOADABLE float half_cos(float x) {
  return (float)cos(x);
}
OVERLOADABLE float half_divide(float x, float y) {
  return (float)native_divide(x, y);
}
OVERLOADABLE float half_exp(float x) {
  return (float)native_exp(x);
}
OVERLOADABLE float half_exp2(float x){
  return (float)native_exp2(x);
}
OVERLOADABLE float half_exp10(float x){
  return (float)native_exp10(x);
}
OVERLOADABLE float half_log(float x){
  return (float)native_log(x);
}
OVERLOADABLE float half_log2(float x){
  return (float)native_log2(x);
}
OVERLOADABLE float half_log10(float x){
  return (float)native_log10(x);
}
OVERLOADABLE float half_powr(float x, float y){
  return (float)powr(x, y);
}
OVERLOADABLE float half_recip(float x){
  return (float)native_recip(x);
}
OVERLOADABLE float half_rsqrt(float x){
  return (float)native_rsqrt(x);
}
OVERLOADABLE float half_sin(float x){
  return (float)sin(x);
}
OVERLOADABLE float half_sqrt(float x){
  return (float)native_sqrt(x);
}
OVERLOADABLE float half_tan(float x){
  return (float)tan(x);
}
OVERLOADABLE half acospi(half x) {
  float _x = (float)x;
  return (half)acospi(_x);
}
OVERLOADABLE half acosh(half x) {
  float _x = (float)x;
  return (half)acosh(_x);
}
OVERLOADABLE half sinpi(half x) {
  float _x = (float)x;
  return (half)sinpi(_x);
}
OVERLOADABLE half sinh(half x) {
  float _x = (float)x;
  return (half)sinh(_x);
}
OVERLOADABLE half asin(half x) {
  float _x = (float)x;
  return (half)asin(_x);
}
OVERLOADABLE half asinpi(half x) {
  float _x = (float)x;
  return (half)asinpi(_x);
}
OVERLOADABLE half asinh(half x) {
  float _x = (float)x;
  return (half)asinh(_x);
}
OVERLOADABLE half tanpi(half x) {
  float _x = (float)x;
  return (half)tanpi(_x);
}
OVERLOADABLE half tanh(half x) {
  float _x = (float)x;
  return (half)tanh(_x);
}
OVERLOADABLE half atan(half x) {
  float _x = (float)x;
  return (half)atan(_x);
}
OVERLOADABLE half atan2(half y, half x) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)atan2(_x, _y);
}
OVERLOADABLE half atan2pi(half y, half x) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)atan2pi(_x, _y);
}
OVERLOADABLE half atanpi(half x) {
  float _x = (float)x;
  return (half)atanpi(_x);
}
OVERLOADABLE half atanh(half x) {
  float _x = (float)x;
  return (half)atanh(_x);
}
OVERLOADABLE half cbrt(half x) {
  float _x = (float)x;
  return (half)cbrt(_x);
}
OVERLOADABLE half rint(half x) {
  float _x = (float)x;
  return (half)rint(_x);
}
OVERLOADABLE half copysign(half x, half y) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)copysign(_x, _y);
}
OVERLOADABLE half erf(half x) {
  float _x = (float)x;
  return (half)erf(_x);
}
OVERLOADABLE half erfc(half x) {
  float _x = (float)x;
  return (half)erfc(_x);
}
OVERLOADABLE half fmod(half x, half y) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)fmod(_x, _y);
}
OVERLOADABLE half remainder(half x, half p) {
  float _x = (float)x;
  float _p = (float)p;
  return (half)remainder(_x, _p);
}
OVERLOADABLE half ldexp(half x, int n) {
  float _x = (float)x;
  return (half)ldexp(_x, n);
}
OVERLOADABLE half powr(half x, half y) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)powr(_x, _y);
}
OVERLOADABLE half pow(half x, half y) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)pow(_x, _y);
}
//no pow, we use powr instead
OVERLOADABLE half fabs(half x) {
  float _x = (float)x;
  return (half)fabs(_x);
}
OVERLOADABLE half trunc(half x) {
  float _x = (float)x;
  return (half)trunc(_x);
}
OVERLOADABLE half round(half x) {
  float _x = (float)x;
  return (half)round(_x);
}
OVERLOADABLE half floor(half x) {
  float _x = (float)x;
  return (half)floor(_x);
}
OVERLOADABLE half ceil(half x) {
  float _x = (float)x;
  return (half)ceil(_x);
}
OVERLOADABLE half log(half x) {
  float _x = (float)x;
  return (half)log(_x);
}
OVERLOADABLE half log2(half x) {
  float _x = (float)x;
  return (half)log2(_x);
}
OVERLOADABLE half log10(half x) {
  float _x = (float)x;
  return (half)log10(_x);
}
OVERLOADABLE half exp(half x) {
  float _x = (float)x;
  return (half)exp(_x);
}
OVERLOADABLE half exp10(half x) {
  float _x = (float)x;
  return (half)exp10(_x);
}
OVERLOADABLE half expm1(half x) {
  float _x = (float)x;
  return (half)expm1(_x);
}
OVERLOADABLE half fmin(half a, half b) {
  return __gen_ocl_internal_fmin(a, b);
}
OVERLOADABLE half fmax(half a, half b) {
  return __gen_ocl_internal_fmax(a, b);
}
OVERLOADABLE half fma(half a, half b, half c) {
  float _a = (float)a;
  float _b = (float)b;
  float _c = (float)c;
  return (half)fma(_a, _b, _c);
}
OVERLOADABLE half fdim(half x, half y) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)fdim(_x, _y);
}
OVERLOADABLE half maxmag(half x, half y) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)maxmag(_x, _y);
}
OVERLOADABLE half minmag(half x, half y) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)minmag(_x, _y);
}
OVERLOADABLE half exp2(half x) {
  float _x = (float)x;
  return (half)exp2(_x);
}
OVERLOADABLE half mad(half a, half b, half c) {
  return __gen_ocl_mad(a,b,c);
}
OVERLOADABLE half sin(half x) {
  float _x = (float)x;
  return (half)sin(_x);
}
OVERLOADABLE half cos(half x) {
  float _x = (float)x;
  return (half)cos(_x);
}
OVERLOADABLE half tan(half x) {
  float _x = (float)x;
  return (half)tan(_x);
}
OVERLOADABLE half tgamma(half x) {
  float _x = (float)x;
  return (half)tgamma(_x);
}
OVERLOADABLE half lgamma(half x) {
  float _x = (float)x;
  return (half)lgamma(_x);
}

OVERLOADABLE half log1p(half x) {
  float _x = (float)x;
  return (half)log1p(_x);
}
OVERLOADABLE half logb(half x) {
  float _x = (float)x;
  return (half)logb(_x);
}
OVERLOADABLE int ilogb(half x) {
  float _x = (float)x;
  return ilogb(_x);
}
OVERLOADABLE half nan(ushort code) {
  return (half)NAN;
}

OVERLOADABLE half sqrt(half x) {
  float _x = (float)x;
  return (half)sqrt(_x);
}
OVERLOADABLE half rsqrt(half x) {
  float _x = (float)x;
  return (half)rsqrt(_x);
}

OVERLOADABLE half nextafter(half x, half y) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)nextafter(_x, _y);
}

OVERLOADABLE half hypot(half x, half y) {
  float _x = (float)x;
  float _y = (float)y;
  return (half)hypot(_x, _y);
}

OVERLOADABLE half pown(half x, int n) {
  float _x = (float)x;
  return (half)pown(_x, n);
}
OVERLOADABLE half rootn(half x, int n) {
  float _x = (float)x;
  return (half)rootn(_x, n);
}

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

OVERLOADABLE double acos(double x)
{
    double one=  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
    pi =  3.14159265358979311600e+00, /* 0x400921FB, 0x54442D18 */
    pio2_hi =  1.57079632679489655800e+00, /* 0x3FF921FB, 0x54442D18 */
    pio2_lo =  6.12323399573676603587e-17, /* 0x3C91A626, 0x33145C07 */
    pS0 =  1.66666666666666657415e-01, /* 0x3FC55555, 0x55555555 */
    pS1 = -3.25565818622400915405e-01, /* 0xBFD4D612, 0x03EB6F7D */
    pS2 =  2.01212532134862925881e-01, /* 0x3FC9C155, 0x0E884455 */
    pS3 = -4.00555345006794114027e-02, /* 0xBFA48228, 0xB5688F3B */
    pS4 =  7.91534994289814532176e-04, /* 0x3F49EFE0, 0x7501B288 */
    pS5 =  3.47933107596021167570e-05, /* 0x3F023DE1, 0x0DFDF709 */
    qS1 = -2.40339491173441421878e+00, /* 0xC0033A27, 0x1C8A2D4B */
    qS2 =  2.02094576023350569471e+00, /* 0x40002AE5, 0x9C598AC8 */
    qS3 = -6.88283971605453293030e-01, /* 0xBFE6066C, 0x1B8D0159 */
    qS4 =  7.70381505559019352791e-02; /* 0x3FB3B8C5, 0xB12E9282 */

    double z,p,q,r,w,s,c,df;
    int hx,ix;
    hx = __HI(x);
    ix = hx&0x7fffffff;
    if(ix>=0x3ff00000) {    /* |x| >= 1 */
        if(((ix-0x3ff00000)|__LO(x))==0) {  /* |x|==1 */
        if(hx>0) return 0.0;        /* acos(1) = 0  */
        else return pi+2.0*pio2_lo; /* acos(-1)= pi */
        }
        return (x-x)/(x-x);     /* acos(|x|>1) is NaN */
    }
    if(ix<0x3fe00000) { /* |x| < 0.5 */
        if(ix<=0x3c600000) return pio2_hi+pio2_lo;/*if|x|<2**-57*/
        z = x*x;
        p = z*(pS0+z*(pS1+z*(pS2+z*(pS3+z*(pS4+z*pS5)))));
        q = one+z*(qS1+z*(qS2+z*(qS3+z*qS4)));
        r = p/q;
        return pio2_hi - (x - (pio2_lo-x*r));
    } else  if (hx<0) {     /* x < -0.5 */
        z = (one+x)*0.5;
        p = z*(pS0+z*(pS1+z*(pS2+z*(pS3+z*(pS4+z*pS5)))));
        q = one+z*(qS1+z*(qS2+z*(qS3+z*qS4)));
        s = sqrt(z);
        r = p/q;
        w = r*s-pio2_lo;
        return pi - 2.0*(s+w);
    } else {            /* x > 0.5 */
        z = (one-x)*0.5;
        s = sqrt(z);
        df = s;
        __setLow(&df, 0);
        c  = (z-df*df)/(s+df);
        p = z*(pS0+z*(pS1+z*(pS2+z*(pS3+z*(pS4+z*pS5)))));
        q = one+z*(qS1+z*(qS2+z*(qS3+z*qS4)));
        r = p/q;
        w = r*s+c;
        return 2.0*(df+w);
    }
}

OVERLOADABLE double acospi(double x)
{
	return acos(x)/M_PI;
}

OVERLOADABLE double acosh(double x)
{
	double one = 1.0,
	ln2 = 6.93147180559945286227e-01;  /* 0x3FE62E42, 0xFEFA39EF */

	double t;
	int hx;
	hx = __HI(x);
	if(hx<0x3ff00000) {		/* x < 1 */
		return (x-x)/(x-x);
	} else if(hx >=0x41b00000) {	/* x > 2**28 */
		if(hx >=0x7ff00000) {	/* x is inf of NaN */
			return x+x;
		} else
		return log(x)+ln2;	/* acosh(huge)=log(2x) */
	} else if(((hx-0x3ff00000)|__LO(x))==0) {
		return 0.0;			/* acosh(1) = 0 */
	} else if (hx > 0x40000000) {	/* 2**28 > x > 2 */
		t=x*x;
		return log(2.0*x-one/(x+sqrt(t-one)));
	} else {			/* 1<x<2 */
		t = x-one;
		return log1p(t+sqrt(2.0*t+t*t));
	}
}

OVERLOADABLE double asin(double x)
{
    double one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
    huge =  1.000e+300,
    pio2_hi =  1.57079632679489655800e+00, /* 0x3FF921FB, 0x54442D18 */
    pio2_lo =  6.12323399573676603587e-17, /* 0x3C91A626, 0x33145C07 */
    pio4_hi =  7.85398163397448278999e-01, /* 0x3FE921FB, 0x54442D18 */
        /* coefficient for R(x^2) */
    pS0 =  1.66666666666666657415e-01, /* 0x3FC55555, 0x55555555 */
    pS1 = -3.25565818622400915405e-01, /* 0xBFD4D612, 0x03EB6F7D */
    pS2 =  2.01212532134862925881e-01, /* 0x3FC9C155, 0x0E884455 */
    pS3 = -4.00555345006794114027e-02, /* 0xBFA48228, 0xB5688F3B */
    pS4 =  7.91534994289814532176e-04, /* 0x3F49EFE0, 0x7501B288 */
    pS5 =  3.47933107596021167570e-05, /* 0x3F023DE1, 0x0DFDF709 */
    qS1 = -2.40339491173441421878e+00, /* 0xC0033A27, 0x1C8A2D4B */
    qS2 =  2.02094576023350569471e+00, /* 0x40002AE5, 0x9C598AC8 */
    qS3 = -6.88283971605453293030e-01, /* 0xBFE6066C, 0x1B8D0159 */
    qS4 =  7.70381505559019352791e-02; /* 0x3FB3B8C5, 0xB12E9282 */

    double t,w,p,q,c,r,s;
    int hx,ix;
    hx = __HI(x);
    ix = hx&0x7fffffff;
    if(ix>= 0x3ff00000) {       /* |x|>= 1 */
        if(((ix-0x3ff00000)|__LO(x))==0)
            /* asin(1)=+-pi/2 with inexact */
        return x*pio2_hi+x*pio2_lo;
        return (x-x)/(x-x);     /* asin(|x|>1) is NaN */
    } else if (ix<0x3fe00000) { /* |x|<0.5 */
        if(ix<0x3e400000) {     /* if |x| < 2**-27 */
        if(huge+x>one) return x;/* return x with inexact if x!=0*/
        } else
        t = x*x;
        p = t*(pS0+t*(pS1+t*(pS2+t*(pS3+t*(pS4+t*pS5)))));
        q = one+t*(qS1+t*(qS2+t*(qS3+t*qS4)));
        w = p/q;
        return x+x*w;
    }
    /* 1> |x|>= 0.5 */
    w = one-fabs(x);
    t = w*0.5;
    p = t*(pS0+t*(pS1+t*(pS2+t*(pS3+t*(pS4+t*pS5)))));
    q = one+t*(qS1+t*(qS2+t*(qS3+t*qS4)));
    s = sqrt(t);
    if(ix>=0x3FEF3333) {    /* if |x| > 0.975 */
        w = p/q;
        t = pio2_hi-(2.0*(s+s*w)-pio2_lo);
    } else {
        w  = s;
        __setLow(&w, 0);
        c  = (t-w*w)/(s+w);
        r  = p/q;
        p  = 2.0*s*r-(pio2_lo-2.0*c);
        q  = pio4_hi-2.0*w;
        t  = pio4_hi-(p-q);
    }
    if(hx>0) return t; else return -t;
}

OVERLOADABLE double asinpi(double x)
{
	return asin(x)/M_PI;
}

OVERLOADABLE double asinh(double x)
{
	double one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
	ln2 =  6.93147180559945286227e-01, /* 0x3FE62E42, 0xFEFA39EF */
	huge=  1.00000000000000000000e+300;
	double t,w;

	int hx,ix;
	hx = __HI(x);
	ix = hx&0x7fffffff;
	if(ix>=0x7ff00000) return x+x;	/* x is inf or NaN */
	if(ix< 0x3e300000) {	/* |x|<2**-28 */
		if(huge+x>one) return x;	/* return x inexact except 0 */
	}
	if(ix>0x41b00000) {	/* |x| > 2**28 */
		w = log(fabs(x))+ln2;
	} else if (ix>0x40000000) {	/* 2**28 > |x| > 2.0 */
		t = fabs(x);
		w = log(2.0*t+one/(sqrt(x*x+one)+t));
	} else {		/* 2.0 > |x| > 2**-28 */
		t = x*x;
		w =log1p(fabs(x)+t/(one+sqrt(one+t)));
	}
	if(hx>0) return w; else return -w;
}

OVERLOADABLE double atan(double x)
{
	double atanhi[] = {
	  4.63647609000806093515e-01, /* atan(0.5)hi 0x3FDDAC67, 0x0561BB4F */
	  7.85398163397448278999e-01, /* atan(1.0)hi 0x3FE921FB, 0x54442D18 */
	  9.82793723247329054082e-01, /* atan(1.5)hi 0x3FEF730B, 0xD281F69B */
	  1.57079632679489655800e+00, /* atan(inf)hi 0x3FF921FB, 0x54442D18 */
	};

	double atanlo[] = {
	  2.26987774529616870924e-17, /* atan(0.5)lo 0x3C7A2B7F, 0x222F65E2 */
	  3.06161699786838301793e-17, /* atan(1.0)lo 0x3C81A626, 0x33145C07 */
	  1.39033110312309984516e-17, /* atan(1.5)lo 0x3C700788, 0x7AF0CBBD */
	  6.12323399573676603587e-17, /* atan(inf)lo 0x3C91A626, 0x33145C07 */
	};

	double aT[] = {
	  3.33333333333329318027e-01, /* 0x3FD55555, 0x5555550D */
	 -1.99999999998764832476e-01, /* 0xBFC99999, 0x9998EBC4 */
	  1.42857142725034663711e-01, /* 0x3FC24924, 0x920083FF */
	 -1.11111104054623557880e-01, /* 0xBFBC71C6, 0xFE231671 */
	  9.09088713343650656196e-02, /* 0x3FB745CD, 0xC54C206E */
	 -7.69187620504482999495e-02, /* 0xBFB3B0F2, 0xAF749A6D */
	  6.66107313738753120669e-02, /* 0x3FB10D66, 0xA0D03D51 */
	 -5.83357013379057348645e-02, /* 0xBFADDE2D, 0x52DEFD9A */
	  4.97687799461593236017e-02, /* 0x3FA97B4B, 0x24760DEB */
	 -3.65315727442169155270e-02, /* 0xBFA2B444, 0x2C6A6C2F */
	  1.62858201153657823623e-02, /* 0x3F90AD3A, 0xE322DA11 */
	};

	double one   = 1.0,
	huge   = 1.0e300;
	double w,s1,s2,z;
	int ix,hx,id;

	hx = __HI(x);
	ix = hx&0x7fffffff;
	if(ix>=0x44100000) {	/* if |x| >= 2^66 */
		if(ix>0x7ff00000 ||(ix==0x7ff00000 && (__LO(x)!=0)))
		return x+x;		/* NaN */
		if(hx>0) return  atanhi[3]+atanlo[3];
		else	 return -atanhi[3]-atanlo[3];
	} if (ix < 0x3fdc0000) {	/* |x| < 0.4375 */
		if (ix < 0x3e200000) {	/* |x| < 2^-29 */
		if(huge+x>one) return x;	/* raise inexact */
		}
		id = -1;
	} else {
	x = fabs(x);
	if (ix < 0x3ff30000) {		/* |x| < 1.1875 */
		if (ix < 0x3fe60000) {	/* 7/16 <=|x|<11/16 */
		id = 0; x = (2.0*x-one)/(2.0+x);
		} else {			/* 11/16<=|x|< 19/16 */
		id = 1; x  = (x-one)/(x+one);
		}
	} else {
		if (ix < 0x40038000) {	/* |x| < 2.4375 */
		id = 2; x  = (x-1.5)/(one+1.5*x);
		} else {			/* 2.4375 <= |x| < 2^66 */
		id = 3; x  = -1.0/x;
		}
	}}
	/* end of argument reduction */
	z = x*x;
	w = z*z;
	/* break sum from i=0 to 10 aT[i]z**(i+1) into odd and even poly */
	s1 = z*(aT[0]+w*(aT[2]+w*(aT[4]+w*(aT[6]+w*(aT[8]+w*aT[10])))));
	s2 = w*(aT[1]+w*(aT[3]+w*(aT[5]+w*(aT[7]+w*aT[9]))));
	if (id<0) return x - x*(s1+s2);
	else {
		z = atanhi[id] - ((x*(s1+s2) - atanlo[id]) - x);
		return (hx<0)? -z:z;
	}
}

OVERLOADABLE double atan2(double y, double x)
{
	double tiny  = 1.0e-300,
	zero  = 0.0,
	pi_o_4  = 7.8539816339744827900E-01, /* 0x3FE921FB, 0x54442D18 */
	pi_o_2  = 1.5707963267948965580E+00, /* 0x3FF921FB, 0x54442D18 */
	pi	  = 3.1415926535897931160E+00, /* 0x400921FB, 0x54442D18 */
	pi_lo   = 1.2246467991473531772E-16; /* 0x3CA1A626, 0x33145C07 */

	double z;
	int k,m,hx,hy,ix,iy;
	unsigned lx,ly;

	hx = __HI(x); ix = hx&0x7fffffff;
	lx = __LO(x);
	hy = __HI(y); iy = hy&0x7fffffff;
	ly = __LO(y);
	if(((ix|((lx|-lx)>>31))>0x7ff00000)||
		((iy|((ly|-ly)>>31))>0x7ff00000))	/* x or y is NaN */
			return x+y;

	m = ((hy>>31)&1)|((hx>>30)&2);  /* 2*sign(x)+sign(y) */

	/* when y = 0 */
	if((iy|ly)==0) {
		switch(m) {
		case 0:
		case 1: return y;   /* atan(+-0,+anything)=+-0 */
		case 2: return  pi+tiny;/* atan(+0,-anything) = pi */
		case 3: return -pi-tiny;/* atan(-0,-anything) =-pi */
		}
	}
	/* when x = 0 */
	if((ix|lx)==0) return (hy<0)?  -pi_o_2-tiny: pi_o_2+tiny;

	/* when x is INF */
	if(ix==0x7ff00000)
	{
		if(iy==0x7ff00000)
		{
			switch(m)
			{
				case 0: return  pi_o_4+tiny;/* atan(+INF,+INF) */
				case 1: return -pi_o_4-tiny;/* atan(-INF,+INF) */
				case 2: return  3.0*pi_o_4+tiny;/*atan(+INF,-INF)*/
				case 3: return -3.0*pi_o_4-tiny;/*atan(-INF,-INF)*/
			}
		}
		else
		{
			switch(m)
			{
				case 0: return  zero  ; /* atan(+...,+INF) */
				case 1: return -zero; /* atan(-...,+INF) */
				case 2: return  pi+tiny  ;  /* atan(+...,-INF) */
				case 3: return -pi -tiny  ;  /* atan(-...,-INF) */
			}
		}
	}
	/* when y is INF */
	if(iy==0x7ff00000) return (hy<0)? -pi_o_2-tiny: pi_o_2+tiny;

	/* compute y/x */
	k = (iy-ix)>>20;
	if(k > 60) z=pi_o_2+0.5*pi_lo;  /* |y/x| >  2**60 */
	else if(hx<0&&k<-60) z=0.0;	 /* |y|/x < -2**60 */
	else z=atan(fabs(y/x));	 /* safe to do y/x */
	switch (m) {
		case 0: return	   z  ;   /* atan(+,+) */
		case 1: __setHigh(&z, __HI(z) ^ 0x80000000);
			return	z  ;   /* atan(-,+) */
		case 2: return  pi-(z-pi_lo);/* atan(+,-) */
		default: /* case 3 */
				return  (z-pi_lo)-pi;/* atan(-,-) */
	}
}

OVERLOADABLE double atanpi(double x)
{
	return atan(x)/M_PI;
}

OVERLOADABLE double atan2pi(double x, double y)
{
	return atan2(x, y)/M_PI;
}

OVERLOADABLE double atanh(double x)
{
		double one = 1.0, huge = 1e300;

	double t, zero = 0;
	int hx,ix;
	unsigned lx;
	hx = __HI(x);		/* high word */
	lx = __LO(x);		/* low word */
	ix = hx&0x7fffffff;
	if ((ix|((lx|(-lx))>>31))>0x3ff00000) /* |x|>1 */
		return (x-x)/(x-x);
	if(ix==0x3ff00000)
		return x/zero;
	if(ix<0x3e300000&&(huge+x)>zero) return x;	/* x<2**-28 */
	__setHigh(&x, ix);		/* x <- |x| */
	if(ix<0x3fe00000) {		/* x < 0.5 */
		t = x+x;
		t = 0.5*log1p(t+t*x/(one-x));
	} else
		t = 0.5*log1p((x+x)/(one-x));
	if(hx>=0) return t; else return -t;

		return 0.0;
}

OVERLOADABLE double exp(double x)
{
	double one	= 1.0,
	halF[2]	= {0.5,-0.5,},
	huge	= 1.0e+300,
	twom1000= 9.33263618503218878990e-302,	 /* 2**-1000=0x01700000,0*/
	o_threshold=  7.09782712893383973096e+02,  /* 0x40862E42, 0xFEFA39EF */
	u_threshold= -7.45133219101941108420e+02,  /* 0xc0874910, 0xD52D3051 */
	ln2HI[2]   ={ 6.93147180369123816490e-01,  /* 0x3fe62e42, 0xfee00000 */
			 -6.93147180369123816490e-01,},/* 0xbfe62e42, 0xfee00000 */
	ln2LO[2]   ={ 1.90821492927058770002e-10,  /* 0x3dea39ef, 0x35793c76 */
			 -1.90821492927058770002e-10,},/* 0xbdea39ef, 0x35793c76 */
	invln2 =  1.44269504088896338700e+00, /* 0x3ff71547, 0x652b82fe */
	P1   =  1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
	P2   = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
	P3   =  6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
	P4   = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
	P5   =  4.13813679705723846039e-08; /* 0x3E663769, 0x72BEA4D0 */

	double y,hi,lo,c,t;
	int k,xsb;
	unsigned hx;

	hx	= __HI(x);	/* high word of x */
	xsb = (hx>>31)&1;		/* sign bit of x */
	hx &= 0x7fffffff;		/* high word of |x| */

	/* filter out non-finite argument */
	if(hx >= 0x40862E42) {			/* if |x|>=709.78... */
			if(hx>=0x7ff00000) {
		if(((hx&0xfffff)|__LO(x))!=0)
			 return x+x;		/* NaN */
		else return (xsb==0)? x:0.0;	/* exp(+-inf)={inf,0} */
		}
		if(x > o_threshold) return huge*huge; /* overflow */
		if(x < u_threshold) return twom1000*twom1000; /* underflow */
	}

	/* argument reduction */
	if(hx > 0x3fd62e42) {		/* if  |x| > 0.5 ln2 */
		if(hx < 0x3FF0A2B2) {	/* and |x| < 1.5 ln2 */
		hi = x-ln2HI[xsb]; lo=ln2LO[xsb]; k = 1-xsb-xsb;
		} else {
		k  = (int)(invln2*x+halF[xsb]);
		t  = k;
		hi = x - t*ln2HI[0];	/* t*ln2HI is exact here */
		lo = t*ln2LO[0];
		}
		x  = hi - lo;
	}
	else if(hx < 0x3e300000)  { /* when |x|<2**-28 */
		if(huge+x>one) return one+x;/* trigger inexact */
	}
	else k = 0;

	/* x is now in primary range */
	t  = x*x;
	c  = x - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
	if(k==0)	return one-((x*c)/(c-2.0)-x);
	else		y = one-((lo-(x*c)/(2.0-c))-hi);
	if(k >= -1021) {
		__setHigh(&y, __HI(y) + (k<<20)); /* add k to y's exponent */
		return y;
	} else {
		__setHigh(&y, __HI(y) + ((k+1000)<<20));/* add k to y's exponent */
		return y*twom1000;
	}

}

OVERLOADABLE double expm1(double x)
{
	double one	= 1.0,
	huge		= 1.0e+300,
	tiny		= 1.0e-300,
	o_threshold = 7.09782712893383973096e+02,/* 0x40862E42, 0xFEFA39EF */
	ln2_hi		= 6.93147180369123816490e-01,/* 0x3fe62e42, 0xfee00000 */
	ln2_lo		= 1.90821492927058770002e-10,/* 0x3dea39ef, 0x35793c76 */
	invln2		= 1.44269504088896338700e+00,/* 0x3ff71547, 0x652b82fe */
		/* scaled coefficients related to expm1 */
	Q1	=  -3.33333333333331316428e-02, /* BFA11111 111110F4 */
	Q2	=	1.58730158725481460165e-03, /* 3F5A01A0 19FE5585 */
	Q3	=  -7.93650757867487942473e-05, /* BF14CE19 9EAADBB7 */
	Q4	=	4.00821782732936239552e-06, /* 3ED0CFCA 86E65239 */
	Q5	=  -2.01099218183624371326e-07; /* BE8AFDB7 6E09C32D */

	double y,hi,lo,c,t,e,hxs,hfx,r1;
	int k,xsb;
	unsigned hx;

	hx	= __HI(x);	/* high word of x */
	xsb = hx&0x80000000;		/* sign bit of x */
	if(xsb==0) y=x; else y= -x; /* y = |x| */
	hx &= 0x7fffffff;		/* high word of |x| */

	/* filter out huge and non-finite argument */
	if(hx >= 0x4043687A) {			/* if |x|>=56*ln2 */
		if(hx >= 0x40862E42) {		/* if |x|>=709.78... */
				if(hx>=0x7ff00000) {
			if(((hx&0xfffff)|__LO(x))!=0)
				 return x+x;	 /* NaN */
			else return (xsb==0)? x:-1.0;/* exp(+-inf)={inf,-1} */
			}
			if(x > o_threshold) return huge*huge; /* overflow */
		}
		if(xsb!=0) { /* x < -56*ln2, return -1.0 with inexact */
		if(x+tiny<0.0)		/* raise inexact */
		return tiny-one;	/* return -1 */
		}
	}

	/* argument reduction */
	if(hx > 0x3fd62e42) {		/* if  |x| > 0.5 ln2 */
		if(hx < 0x3FF0A2B2) {	/* and |x| < 1.5 ln2 */
		if(xsb==0)
			{hi = x - ln2_hi; lo =	ln2_lo;  k =  1;}
		else
			{hi = x + ln2_hi; lo = -ln2_lo;  k = -1;}
		} else {
		k  = invln2*x+((xsb==0)?0.5:-0.5);
		t  = k;
		hi = x - t*ln2_hi;	/* t*ln2_hi is exact here */
		lo = t*ln2_lo;
		}
		x  = hi - lo;
		c  = (hi-x)-lo;
	}
	else if(hx < 0x3c900000) {		/* when |x|<2**-54, return x */
		t = huge+x; /* return x with inexact flags when x!=0 */
		return x - (t-(huge+x));
	}
	else k = 0;

	/* x is now in primary range */
	hfx = 0.5*x;
	hxs = x*hfx;
	r1 = one+hxs*(Q1+hxs*(Q2+hxs*(Q3+hxs*(Q4+hxs*Q5))));
	t  = 3.0-r1*hfx;
	e  = hxs*((r1-t)/(6.0 - x*t));
	if(k==0) return x - (x*e-hxs);		/* c is 0 */
	else {
		e  = (x*(e-c)-c);
		e -= hxs;
		if(k== -1) return 0.5*(x-e)-0.5;
		if(k==1)
	{
			if(x < -0.25)
			return -2.0*(e-(x+0.5));
			else
			return	one+2.0*(x-e);
		}

		if (k <= -2 || k>56) {	 /* suffice to return exp(x)-1 */
			y = one-(e-x);
			__setHigh(&y, __HI(y) + (k<<20));;	/* add k to y's exponent */
			return y-one;
		}
		t = one;
		if(k<20) {
			__setHigh(&t, 0x3ff00000 - (0x200000>>k));	/* t=1-2^-k */
			y = t-(e-x);
			__setHigh(&y, __HI(y) + (k<<20));	/* add k to y's exponent */
	   } else {
			__setHigh(&t, ((0x3ff-k)<<20)); /* 2^-k */
			y = x-(e+t);
			y += one;
			__setHigh(&y, __HI(y) + (k<<20));	/* add k to y's exponent */
		}
	}
	return y;
}

OVERLOADABLE double exp2(double x)
{
	return pow(2, x);
}

OVERLOADABLE double exp10(double x)
{
	return pow(10, x);
}

OVERLOADABLE double erf(double x)
{
	double erx =  8.45062911510467529297e-01, /* 0x3FEB0AC1, 0x60000000 */
	efx =  1.28379167095512586316e-01, /* 0x3FC06EBA, 0x8214DB69 */
	efx8=  1.02703333676410069053e+00, /* 0x3FF06EBA, 0x8214DB69 */
	pp0  =  1.28379167095512558561e-01, /* 0x3FC06EBA, 0x8214DB68 */
	pp1  = -3.25042107247001499370e-01, /* 0xBFD4CD7D, 0x691CB913 */
	pp2  = -2.84817495755985104766e-02, /* 0xBF9D2A51, 0xDBD7194F */
	pp3  = -5.77027029648944159157e-03, /* 0xBF77A291, 0x236668E4 */
	pp4  = -2.37630166566501626084e-05, /* 0xBEF8EAD6, 0x120016AC */
	qq1  =  3.97917223959155352819e-01, /* 0x3FD97779, 0xCDDADC09 */
	qq2  =  6.50222499887672944485e-02, /* 0x3FB0A54C, 0x5536CEBA */
	qq3  =  5.08130628187576562776e-03, /* 0x3F74D022, 0xC4D36B0F */
	qq4  =  1.32494738004321644526e-04, /* 0x3F215DC9, 0x221C1A10 */
	qq5  = -3.96022827877536812320e-06, /* 0xBED09C43, 0x42A26120 */
	pa0  = -2.36211856075265944077e-03, /* 0xBF6359B8, 0xBEF77538 */
	pa1  =  4.14856118683748331666e-01, /* 0x3FDA8D00, 0xAD92B34D */
	pa2  = -3.72207876035701323847e-01, /* 0xBFD7D240, 0xFBB8C3F1 */
	pa3  =  3.18346619901161753674e-01, /* 0x3FD45FCA, 0x805120E4 */
	pa4  = -1.10894694282396677476e-01, /* 0xBFBC6398, 0x3D3E28EC */
	pa5  =  3.54783043256182359371e-02, /* 0x3FA22A36, 0x599795EB */
	pa6  = -2.16637559486879084300e-03, /* 0xBF61BF38, 0x0A96073F */
	qa1  =  1.06420880400844228286e-01, /* 0x3FBB3E66, 0x18EEE323 */
	qa2  =  5.40397917702171048937e-01, /* 0x3FE14AF0, 0x92EB6F33 */
	qa3  =  7.18286544141962662868e-02, /* 0x3FB2635C, 0xD99FE9A7 */
	qa4  =  1.26171219808761642112e-01, /* 0x3FC02660, 0xE763351F */
	qa5  =  1.36370839120290507362e-02, /* 0x3F8BEDC2, 0x6B51DD1C */
	qa6  =  1.19844998467991074170e-02, /* 0x3F888B54, 0x5735151D */
	ra0  = -9.86494403484714822705e-03, /* 0xBF843412, 0x600D6435 */
	ra1  = -6.93858572707181764372e-01, /* 0xBFE63416, 0xE4BA7360 */
	ra2  = -1.05586262253232909814e+01, /* 0xC0251E04, 0x41B0E726 */
	ra3  = -6.23753324503260060396e+01, /* 0xC04F300A, 0xE4CBA38D */
	ra4  = -1.62396669462573470355e+02, /* 0xC0644CB1, 0x84282266 */
	ra5  = -1.84605092906711035994e+02, /* 0xC067135C, 0xEBCCABB2 */
	ra6  = -8.12874355063065934246e+01, /* 0xC0545265, 0x57E4D2F2 */
	ra7  = -9.81432934416914548592e+00, /* 0xC023A0EF, 0xC69AC25C */
	sa1  =  1.96512716674392571292e+01, /* 0x4033A6B9, 0xBD707687 */
	sa2  =  1.37657754143519042600e+02, /* 0x4061350C, 0x526AE721 */
	sa3  =  4.34565877475229228821e+02, /* 0x407B290D, 0xD58A1A71 */
	sa4  =  6.45387271733267880336e+02, /* 0x40842B19, 0x21EC2868 */
	sa5  =  4.29008140027567833386e+02, /* 0x407AD021, 0x57700314 */
	sa6  =  1.08635005541779435134e+02, /* 0x405B28A3, 0xEE48AE2C */
	sa7  =  6.57024977031928170135e+00, /* 0x401A47EF, 0x8E484A93 */
	sa8  = -6.04244152148580987438e-02, /* 0xBFAEEFF2, 0xEE749A62 */
	rb0  = -9.86494292470009928597e-03, /* 0xBF843412, 0x39E86F4A */
	rb1  = -7.99283237680523006574e-01, /* 0xBFE993BA, 0x70C285DE */
	rb2  = -1.77579549177547519889e+01, /* 0xC031C209, 0x555F995A */
	rb3  = -1.60636384855821916062e+02, /* 0xC064145D, 0x43C5ED98 */
	rb4  = -6.37566443368389627722e+02, /* 0xC083EC88, 0x1375F228 */
	rb5  = -1.02509513161107724954e+03, /* 0xC0900461, 0x6A2E5992 */
	rb6  = -4.83519191608651397019e+02, /* 0xC07E384E, 0x9BDC383F */
	sb1  =  3.03380607434824582924e+01, /* 0x403E568B, 0x261D5190 */
	sb2  =  3.25792512996573918826e+02; /* 0x40745CAE, 0x221B9F0A */
	double sb3  =  1.53672958608443695994e+03, /* 0x409802EB, 0x189D5118 */
	sb4  =  3.19985821950859553908e+03, /* 0x40A8FFB7, 0x688C246A */
	sb5  =  2.55305040643316442583e+03, /* 0x40A3F219, 0xCEDF3BE6 */
	sb6  =  4.74528541206955367215e+02, /* 0x407DA874, 0xE79FE763 */
	sb7  = -2.24409524465858183362e+01; /* 0xC03670E2, 0x42712D62 */

	double tiny = 1e-300;
	double halfD =  5.00000000000000000000e-01;/* 0x3FE00000, 0x00000000 */
	double one =  1.00000000000000000000e+00; /* 0x3FF00000, 0x00000000 */
	double two =  2.00000000000000000000e+00; /* 0x40000000, 0x00000000 */

	int hx,ix,i;
	double R,S,P,Q,s,y,z,r;
	hx = __HI(x);
	ix = hx&0x7fffffff;
	if(ix>=0x7ff00000) {		/* erf(nan)=nan */
		i = ((unsigned)hx>>31)<<1;
		return (double)(1-i)+one/x;	/* erf(+-inf)=+-1 */
	}

	if(ix < 0x3feb0000) {		/* |x|<0.84375 */
		if(ix < 0x3e300000) { 	/* |x|<2**-28 */
			if (ix < 0x00800000)
			return 0.125*(8.0*x+efx8*x);  /*avoid underflow */
		return x + efx*x;
		}
		z = x*x;
		r = pp0+z*(pp1+z*(pp2+z*(pp3+z*pp4)));
		s = one+z*(qq1+z*(qq2+z*(qq3+z*(qq4+z*qq5))));
		y = r/s;
		return x + x*y;
	}
	if(ix < 0x3ff40000) {		/* 0.84375 <= |x| < 1.25 */
		s = fabs(x)-one;
		P = pa0+s*(pa1+s*(pa2+s*(pa3+s*(pa4+s*(pa5+s*pa6)))));
		Q = one+s*(qa1+s*(qa2+s*(qa3+s*(qa4+s*(qa5+s*qa6)))));
		if(hx>=0) return erx + P/Q; else return -erx - P/Q;
	}
	if (ix >= 0x40180000) {		/* inf>|x|>=6 */
		if(hx>=0) return one-tiny; else return tiny-one;
	}
	x = fabs(x);
	s = one/(x*x);
	if(ix< 0x4006DB6E) {	/* |x| < 1/0.35 */
		R=ra0+s*(ra1+s*(ra2+s*(ra3+s*(ra4+s*(
				ra5+s*(ra6+s*ra7))))));
		S=one+s*(sa1+s*(sa2+s*(sa3+s*(sa4+s*(
				sa5+s*(sa6+s*(sa7+s*sa8)))))));
	} else {	/* |x| >= 1/0.35 */
		R=rb0+s*(rb1+s*(rb2+s*(rb3+s*(rb4+s*(
				rb5+s*rb6)))));
		S=one+s*(sb1+s*(sb2+s*(sb3+s*(sb4+s*(
				sb5+s*(sb6+s*sb7))))));
	}
	z  = x;
	__setLow(&z, 0);
	r  = exp(-z*z-0.5625)*exp((z-x)*(z+x)+R/S);
	if(hx>=0) return one-r/x; else return  r/x-one;

}

OVERLOADABLE double erfc(double x)
{
	double erx =  8.45062911510467529297e-01, /* 0x3FEB0AC1, 0x60000000 */
	efx =  1.28379167095512586316e-01, /* 0x3FC06EBA, 0x8214DB69 */
	efx8=  1.02703333676410069053e+00, /* 0x3FF06EBA, 0x8214DB69 */
	pp0  =  1.28379167095512558561e-01, /* 0x3FC06EBA, 0x8214DB68 */
	pp1  = -3.25042107247001499370e-01, /* 0xBFD4CD7D, 0x691CB913 */
	pp2  = -2.84817495755985104766e-02, /* 0xBF9D2A51, 0xDBD7194F */
	pp3  = -5.77027029648944159157e-03, /* 0xBF77A291, 0x236668E4 */
	pp4  = -2.37630166566501626084e-05, /* 0xBEF8EAD6, 0x120016AC */
	qq1  =  3.97917223959155352819e-01, /* 0x3FD97779, 0xCDDADC09 */
	qq2  =  6.50222499887672944485e-02, /* 0x3FB0A54C, 0x5536CEBA */
	qq3  =  5.08130628187576562776e-03, /* 0x3F74D022, 0xC4D36B0F */
	qq4  =  1.32494738004321644526e-04, /* 0x3F215DC9, 0x221C1A10 */
	qq5  = -3.96022827877536812320e-06, /* 0xBED09C43, 0x42A26120 */
	pa0  = -2.36211856075265944077e-03, /* 0xBF6359B8, 0xBEF77538 */
	pa1  =  4.14856118683748331666e-01, /* 0x3FDA8D00, 0xAD92B34D */
	pa2  = -3.72207876035701323847e-01, /* 0xBFD7D240, 0xFBB8C3F1 */
	pa3  =  3.18346619901161753674e-01, /* 0x3FD45FCA, 0x805120E4 */
	pa4  = -1.10894694282396677476e-01, /* 0xBFBC6398, 0x3D3E28EC */
	pa5  =  3.54783043256182359371e-02, /* 0x3FA22A36, 0x599795EB */
	pa6  = -2.16637559486879084300e-03, /* 0xBF61BF38, 0x0A96073F */
	qa1  =  1.06420880400844228286e-01, /* 0x3FBB3E66, 0x18EEE323 */
	qa2  =  5.40397917702171048937e-01, /* 0x3FE14AF0, 0x92EB6F33 */
	qa3  =  7.18286544141962662868e-02, /* 0x3FB2635C, 0xD99FE9A7 */
	qa4  =  1.26171219808761642112e-01, /* 0x3FC02660, 0xE763351F */
	qa5  =  1.36370839120290507362e-02, /* 0x3F8BEDC2, 0x6B51DD1C */
	qa6  =  1.19844998467991074170e-02, /* 0x3F888B54, 0x5735151D */
	ra0  = -9.86494403484714822705e-03, /* 0xBF843412, 0x600D6435 */
	ra1  = -6.93858572707181764372e-01, /* 0xBFE63416, 0xE4BA7360 */
	ra2  = -1.05586262253232909814e+01, /* 0xC0251E04, 0x41B0E726 */
	ra3  = -6.23753324503260060396e+01, /* 0xC04F300A, 0xE4CBA38D */
	ra4  = -1.62396669462573470355e+02, /* 0xC0644CB1, 0x84282266 */
	ra5  = -1.84605092906711035994e+02, /* 0xC067135C, 0xEBCCABB2 */
	ra6  = -8.12874355063065934246e+01, /* 0xC0545265, 0x57E4D2F2 */
	ra7  = -9.81432934416914548592e+00, /* 0xC023A0EF, 0xC69AC25C */
	sa1  =  1.96512716674392571292e+01, /* 0x4033A6B9, 0xBD707687 */
	sa2  =  1.37657754143519042600e+02, /* 0x4061350C, 0x526AE721 */
	sa3  =  4.34565877475229228821e+02, /* 0x407B290D, 0xD58A1A71 */
	sa4  =  6.45387271733267880336e+02, /* 0x40842B19, 0x21EC2868 */
	sa5  =  4.29008140027567833386e+02, /* 0x407AD021, 0x57700314 */
	sa6  =  1.08635005541779435134e+02, /* 0x405B28A3, 0xEE48AE2C */
	sa7  =  6.57024977031928170135e+00, /* 0x401A47EF, 0x8E484A93 */
	sa8  = -6.04244152148580987438e-02, /* 0xBFAEEFF2, 0xEE749A62 */
	rb0  = -9.86494292470009928597e-03, /* 0xBF843412, 0x39E86F4A */
	rb1  = -7.99283237680523006574e-01, /* 0xBFE993BA, 0x70C285DE */
	rb2  = -1.77579549177547519889e+01, /* 0xC031C209, 0x555F995A */
	rb3  = -1.60636384855821916062e+02, /* 0xC064145D, 0x43C5ED98 */
	rb4  = -6.37566443368389627722e+02, /* 0xC083EC88, 0x1375F228 */
	rb5  = -1.02509513161107724954e+03, /* 0xC0900461, 0x6A2E5992 */
	rb6  = -4.83519191608651397019e+02, /* 0xC07E384E, 0x9BDC383F */
	sb1  =  3.03380607434824582924e+01, /* 0x403E568B, 0x261D5190 */
	sb2  =  3.25792512996573918826e+02; /* 0x40745CAE, 0x221B9F0A */
	double sb3  =  1.53672958608443695994e+03, /* 0x409802EB, 0x189D5118 */
	sb4  =  3.19985821950859553908e+03, /* 0x40A8FFB7, 0x688C246A */
	sb5  =  2.55305040643316442583e+03, /* 0x40A3F219, 0xCEDF3BE6 */
	sb6  =  4.74528541206955367215e+02, /* 0x407DA874, 0xE79FE763 */
	sb7  = -2.24409524465858183362e+01; /* 0xC03670E2, 0x42712D62 */

	double tiny = 1e-300;
	double halfD =  5.00000000000000000000e-01;/* 0x3FE00000, 0x00000000 */
	double one =  1.00000000000000000000e+00; /* 0x3FF00000, 0x00000000 */
	double two =  2.00000000000000000000e+00; /* 0x40000000, 0x00000000 */

	int hx,ix;
	double R,S,P,Q,s,y,z,r;
	hx = __HI(x);
	ix = hx&0x7fffffff;
	if(ix>=0x7ff00000) {			/* erfc(nan)=nan */
						/* erfc(+-inf)=0,2 */
		return (double)(((unsigned)hx>>31)<<1)+one/x;
	}

	if(ix < 0x3feb0000) {		/* |x|<0.84375 */
		if(ix < 0x3c700000)  	/* |x|<2**-56 */
		return one-x;
		z = x*x;
		r = pp0+z*(pp1+z*(pp2+z*(pp3+z*pp4)));
		s = one+z*(qq1+z*(qq2+z*(qq3+z*(qq4+z*qq5))));
		y = r/s;
		if(hx < 0x3fd00000) {  	/* x<1/4 */
		return one-(x+x*y);
		} else {
		r = x*y;
		r += (x-halfD);
			return halfD - r ;
		}
	}
	if(ix < 0x3ff40000) {		/* 0.84375 <= |x| < 1.25 */
		s = fabs(x)-one;
		P = pa0+s*(pa1+s*(pa2+s*(pa3+s*(pa4+s*(pa5+s*pa6)))));
		Q = one+s*(qa1+s*(qa2+s*(qa3+s*(qa4+s*(qa5+s*qa6)))));
		if(hx>=0) {
			z  = one-erx; return z - P/Q;
		} else {
		z = erx+P/Q; return one+z;
		}
	}
	if (ix < 0x403c0000) {		/* |x|<28 */
		x = fabs(x);
		s = one/(x*x);
		if(ix< 0x4006DB6D) {	/* |x| < 1/.35 ~ 2.857143*/
			R=ra0+s*(ra1+s*(ra2+s*(ra3+s*(ra4+s*(
				ra5+s*(ra6+s*ra7))))));
			S=one+s*(sa1+s*(sa2+s*(sa3+s*(sa4+s*(
				sa5+s*(sa6+s*(sa7+s*sa8)))))));
		} else {			/* |x| >= 1/.35 ~ 2.857143 */
		if(hx<0&&ix>=0x40180000) return two-tiny;/* x < -6 */
			R=rb0+s*(rb1+s*(rb2+s*(rb3+s*(rb4+s*(
				rb5+s*rb6)))));
			S=one+s*(sb1+s*(sb2+s*(sb3+s*(sb4+s*(
				sb5+s*(sb6+s*sb7))))));
		}
		z  = x;
		__setLow(&z, 0);
		r  =  exp(-z*z-0.5625)*
			exp((z-x)*(z+x)+R/S);
		if(hx>0) return r/x; else return two-r/x;
	} else {
		if(hx>0) return tiny*tiny; else return two-tiny;
	}
}

OVERLOADABLE double cbrt(double x)
{
	double B1 = 715094163, /* B1 = (682-0.03306235651)*2**20 */
	B2 = 696219795; /* B2 = (664-0.03306235651)*2**20 */

	double C =  5.42857142857142815906e-01, /* 19/35	 = 0x3FE15F15, 0xF15F15F1 */
	D = -7.05306122448979611050e-01, /* -864/1225 = 0xBFE691DE, 0x2532C834 */
	E =  1.41428571428571436819e+00, /* 99/70	 = 0x3FF6A0EA, 0x0EA0EA0F */
	F =  1.60714285714285720630e+00, /* 45/28	 = 0x3FF9B6DB, 0x6DB6DB6E */
	G =  3.57142857142857150787e-01; /* 5/14	  = 0x3FD6DB6D, 0xB6DB6DB7 */

	int	hx;
	double r,s,t=0.0,w;
	unsigned sign;


	hx = __HI(x);		/* high word of x */
	sign=hx&0x80000000; 		/* sign= sign(x) */
	hx  ^=sign;
	if(hx>=0x7ff00000) return(x+x); /* cbrt(NaN,INF) is itself */
	if((hx|__LO(x))==0)
		return(x);		/* cbrt(0) is itself */

	__setHigh(&x, hx);	/* x <- |x| */
	/* rough cbrt to 5 bits */
	if(hx<0x00100000) 		/* subnormal number */
	  {
		__setHigh(&t, 0x43500000); 		/* set t= 2**54 */
		t*=x;
		__setHigh(&t, __HI(t)/3+B2);
	  }
	else
	  __setHigh(&t, hx/3+B1);


	/* new cbrt to 23 bits, may be implemented in single precision */
	r=t*t/x;
	s=C+r*t;
	t*=G+F/(s+E+D/s);

	/* chopped to 20 bits and make it larger than cbrt(x) */
	__setLow(&t, 0); __setHigh(&t, __HI(t)+0x00000001);


	/* one step newton iteration to 53 bits with error less than 0.667 ulps */
	s=t*t;		/* t*t is exact */
	r=x/s;
	w=t+t;
	r=(r-t)/(w+r);	/* r-s is exact */
	t=t+t*r;

	/* retore the sign bit */
	__setHigh(&t, __HI(t)|sign);
	return(t);
}

OVERLOADABLE double ceil(double x)
{
    double ret;
    ulong lval = as_ulong(x);
    int exp = ((lval >> 52) & 0x7FF) - 1023;
    int sign = (lval >> 63) & 0x1;

    long i = (1L << (52 - exp));
    long mask = 0x10000000000000 - i;
    unsigned long uv = 0xFFF0000000000000 + mask;
    ulong vv = lval & uv;
    double	dp = as_double(vv);
    ret = ((vv != lval) && !sign) ? dp +1.0:dp;

    ret = ((exp < 0) && sign) ? 0:ret;
    double tmp = (lval & DF_ABS_MASK) ? 1.0:0.0;
    ret = ((exp < 0) && !sign) ? tmp:ret;
    ret = (exp >= 52) ? x:ret;

     return ret;
}

OVERLOADABLE double copysign(double x, double y)
{
	ulong uy = as_ulong(y);
	ulong sign = uy & DF_SIGN_MASK;
	ulong ux = as_ulong(x);
	ux = (ux & DF_ABS_MASK) | sign;
	return as_double(ux);
}

double __scalbn (double x, int n)
{
	double two54   =  1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
	twom54  =  5.55111512312578270212e-17, /* 0x3C900000, 0x00000000 */
	huge   = 1.0e+300,
	tiny   = 1.0e-300;

	int  k,hx,lx;
	hx = __HI(x);
	lx = __LO(x);
		k = (hx&0x7ff00000)>>20;		/* extract exponent */
		if (k==0) {				/* 0 or subnormal x */
			if ((lx|(hx&0x7fffffff))==0) return x; /* +-0 */
		x *= two54;
		hx = __HI(x);
		k = ((hx&0x7ff00000)>>20) - 54;
			if (n< -50000) return tiny*x; 	/*underflow*/
		}
		if (k==0x7ff) return x+x;		/* NaN or Inf */
		k = k+n;
		if (k >  0x7fe) return huge*copysign(huge,x); /* overflow  */
		if (k > 0) 				/* normal result */
		{__setHigh(&x, (hx&0x800fffff)|(k<<20)); return x;}
		if (k <= -54)
			{
			if (n > 50000) 	/* in case integer overflow in n+k */
		return huge*copysign(huge,x);	/*overflow*/
		else return tiny*copysign(tiny,x); 	/*underflow*/
		}
		k += 54;				/* subnormal result */
		__setHigh(&x, (hx&0x800fffff)|(k<<20));
		return x*twom54;
}

int __kernel_rem_pio2(double *x, double *y, int e0, int nx, int prec, const int *ipio2)
{
	double zero   = 0.0,
	one	= 1.0,
	two24   =  1.67772160000000000000e+07, /* 0x41700000, 0x00000000 */
	twon24  =  5.96046447753906250000e-08; /* 0x3E700000, 0x00000000 */

	double PIo2[] = {
	  1.57079625129699707031e+00, /* 0x3FF921FB, 0x40000000 */
	  7.54978941586159635335e-08, /* 0x3E74442D, 0x00000000 */
	  5.39030252995776476554e-15, /* 0x3CF84698, 0x80000000 */
	  3.28200341580791294123e-22, /* 0x3B78CC51, 0x60000000 */
	  1.27065575308067607349e-29, /* 0x39F01B83, 0x80000000 */
	  1.22933308981111328932e-36, /* 0x387A2520, 0x40000000 */
	  2.73370053816464559624e-44, /* 0x36E38222, 0x80000000 */
	  2.16741683877804819444e-51, /* 0x3569F31D, 0x00000000 */
	};

	const int init_jk[] = {2,3,4,6}; /* initial value for jk */

	int jz,jx,jv,jp,jk,carry,n,iq[20],i,j,k,m,q0,ih;
	double z,fw,f[20],fq[20],q[20];

	/* initialize jk*/
	jk = init_jk[prec];
	jp = jk;

	/* determine jx,jv,q0, note that 3>q0 */
	jx =  nx-1;
	jv = (e0-3)/24; if(jv<0) jv=0;
	q0 =  e0-24*(jv+1);

	/* set up f[0] to f[jx+jk] where f[jx+jk] = ipio2[jv+jk] */
	j = jv-jx; m = jx+jk;
	for(i=0;i<=m;i++,j++) f[i] = (j<0)? zero : (double) ipio2[j];

	/* compute q[0],q[1],...q[jk] */
	for (i=0;i<=jk;i++) {
		for(j=0,fw=0.0;j<=jx;j++) fw += x[j]*f[jx+i-j]; q[i] = fw;
	}

	jz = jk;
recompute:
	/* distill q[] into iq[] reversingly */
	for(i=0,j=jz,z=q[jz];j>0;i++,j--) {
		fw	=  (double)((int)(twon24* z));
		iq[i] =  (int)(z-two24*fw);
		z	 =  q[j-1]+fw;
	}

	/* compute n */
	z  = __scalbn(z,q0);		/* actual value of z */
	z -= 8.0*floor(z*0.125);		/* trim off integer >= 8 */
	n  = (int) z;
	z -= (double)n;
	ih = 0;
	if(q0>0) {	/* need iq[jz-1] to determine n */
		i  = (iq[jz-1]>>(24-q0)); n += i;
		iq[jz-1] -= i<<(24-q0);
		ih = iq[jz-1]>>(23-q0);
	}
	else if(q0==0) ih = iq[jz-1]>>23;
	else if(z>=0.5) ih=2;

	if(ih>0) {	/* q > 0.5 */
		n += 1; carry = 0;
		for(i=0;i<jz ;i++) {	/* compute 1-q */
		j = iq[i];
		if(carry==0) {
			if(j!=0) {
			carry = 1; iq[i] = 0x1000000- j;
			}
		} else  iq[i] = 0xffffff - j;
		}
		if(q0>0) {		/* rare case: chance is 1 in 12 */
			switch(q0) {
			case 1:
			   iq[jz-1] &= 0x7fffff; break;
			case 2:
			   iq[jz-1] &= 0x3fffff; break;
			}
		}
		if(ih==2) {
		z = one - z;
		if(carry!=0) z -= __scalbn(one,q0);
		}
	}

	/* check if recomputation is needed */
	if(z==zero) {
		j = 0;
		for (i=jz-1;i>=jk;i--) j |= iq[i];
		if(j==0) { /* need recomputation */
		for(k=1;iq[jk-k]==0;k++);   /* k = no. of terms needed */

		for(i=jz+1;i<=jz+k;i++) {   /* add q[jz+1] to q[jz+k] */
			f[jx+i] = (double) ipio2[jv+i];
			for(j=0,fw=0.0;j<=jx;j++) fw += x[j]*f[jx+i-j];
			q[i] = fw;
		}
		jz += k;
		goto recompute;
		}
	}

	/* chop off zero terms */
	if(z==0.0) {
		jz -= 1; q0 -= 24;
		while(iq[jz]==0) { jz--; q0-=24;}
	} else { /* break z into 24-bit if necessary */
		z = __scalbn(z,-q0);
		if(z>=two24) {
		fw = (double)((int)(twon24*z));
		iq[jz] = (int)(z-two24*fw);
		jz += 1; q0 += 24;
		iq[jz] = (int) fw;
		} else iq[jz] = (int) z ;
	}

	/* convert integer "bit" chunk to floating-point value */
	fw = __scalbn(one,q0);
	for(i=jz;i>=0;i--) {
		q[i] = fw*(double)iq[i]; fw*=twon24;
	}

	/* compute PIo2[0,...,jp]*q[jz,...,0] */
	for(i=jz;i>=0;i--) {
		for(fw=0.0,k=0;k<=jp&&k<=jz-i;k++) fw += PIo2[k]*q[i+k];
		fq[jz-i] = fw;
	}

	/* compress fq[] into y[] */
	switch(prec) {
		case 0:
		fw = 0.0;
		for (i=jz;i>=0;i--) fw += fq[i];
		y[0] = (ih==0)? fw: -fw;
		break;
		case 1:
		case 2:
		fw = 0.0;
		for (i=jz;i>=0;i--) fw += fq[i];
		y[0] = (ih==0)? fw: -fw;
		fw = fq[0]-fw;
		for (i=1;i<=jz;i++) fw += fq[i];
		y[1] = (ih==0)? fw: -fw;
		break;
		case 3:	/* painful */
		for (i=jz;i>0;i--) {
			fw	  = fq[i-1]+fq[i];
			fq[i]  += fq[i-1]-fw;
			fq[i-1] = fw;
		}
		for (i=jz;i>1;i--) {
			fw	  = fq[i-1]+fq[i];
			fq[i]  += fq[i-1]-fw;
			fq[i-1] = fw;
		}
		for (fw=0.0,i=jz;i>=2;i--) fw += fq[i];
		if(ih==0) {
			y[0] =  fq[0]; y[1] =  fq[1]; y[2] =  fw;
		} else {
			y[0] = -fq[0]; y[1] = -fq[1]; y[2] = -fw;
		}
	}
	return n&7;
}

int __ieee754_rem_pio2(double x, double *y)
{
	int two_over_pi[] = {
	0xA2F983, 0x6E4E44, 0x1529FC, 0x2757D1, 0xF534DD, 0xC0DB62,
	0x95993C, 0x439041, 0xFE5163, 0xABDEBB, 0xC561B7, 0x246E3A,
	0x424DD2, 0xE00649, 0x2EEA09, 0xD1921C, 0xFE1DEB, 0x1CB129,
	0xA73EE8, 0x8235F5, 0x2EBB44, 0x84E99C, 0x7026B4, 0x5F7E41,
	0x3991D6, 0x398353, 0x39F49C, 0x845F8B, 0xBDF928, 0x3B1FF8,
	0x97FFDE, 0x05980F, 0xEF2F11, 0x8B5A0A, 0x6D1F6D, 0x367ECF,
	0x27CB09, 0xB74F46, 0x3F669E, 0x5FEA2D, 0x7527BA, 0xC7EBE5,
	0xF17B3D, 0x0739F7, 0x8A5292, 0xEA6BFB, 0x5FB11F, 0x8D5D08,
	0x560330, 0x46FC7B, 0x6BABF0, 0xCFBC20, 0x9AF436, 0x1DA9E3,
	0x91615E, 0xE61B08, 0x659985, 0x5F14A0, 0x68408D, 0xFFD880,
	0x4D7327, 0x310606, 0x1556CA, 0x73A8C9, 0x60E27B, 0xC08C6B,
	};

	int npio2_hw[] = {
	0x3FF921FB, 0x400921FB, 0x4012D97C, 0x401921FB, 0x401F6A7A, 0x4022D97C,
	0x4025FDBB, 0x402921FB, 0x402C463A, 0x402F6A7A, 0x4031475C, 0x4032D97C,
	0x40346B9C, 0x4035FDBB, 0x40378FDB, 0x403921FB, 0x403AB41B, 0x403C463A,
	0x403DD85A, 0x403F6A7A, 0x40407E4C, 0x4041475C, 0x4042106C, 0x4042D97C,
	0x4043A28C, 0x40446B9C, 0x404534AC, 0x4045FDBB, 0x4046C6CB, 0x40478FDB,
	0x404858EB, 0x404921FB,
	};

	double zero =  0.00000000000000000000e+00, /* 0x00000000, 0x00000000 */
	halfD =  5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
	two24 =  1.67772160000000000000e+07, /* 0x41700000, 0x00000000 */
	invpio2 =  6.36619772367581382433e-01, /* 0x3FE45F30, 0x6DC9C883 */
	pio2_1  =  1.57079632673412561417e+00, /* 0x3FF921FB, 0x54400000 */
	pio2_1t =  6.07710050650619224932e-11, /* 0x3DD0B461, 0x1A626331 */
	pio2_2  =  6.07710050630396597660e-11, /* 0x3DD0B461, 0x1A600000 */
	pio2_2t =  2.02226624879595063154e-21, /* 0x3BA3198A, 0x2E037073 */
	pio2_3  =  2.02226624871116645580e-21, /* 0x3BA3198A, 0x2E000000 */
	pio2_3t =  8.47842766036889956997e-32; /* 0x397B839A, 0x252049C1 */

	double z,w,t,r,fn;
	double tx[3];
	int e0,i,j,nx,n,ix,hx;

	hx = __HI(x);		/* high word of x */
	ix = hx&0x7fffffff;
	if(ix<=0x3fe921fb)   /* |x| ~<= pi/4 , no need for reduction */
		{y[0] = x; y[1] = 0; return 0;}
	if(ix<0x4002d97c) {  /* |x| < 3pi/4, special case with n=+-1 */
		if(hx>0) {
		z = x - pio2_1;
		if(ix!=0x3ff921fb) { 	/* 33+53 bit pi is good enough */
			y[0] = z - pio2_1t;
			y[1] = (z-y[0])-pio2_1t;
		} else {		/* near pi/2, use 33+33+53 bit pi */
			z -= pio2_2;
			y[0] = z - pio2_2t;
			y[1] = (z-y[0])-pio2_2t;
		}
		return 1;
		} else {	/* negative x */
		z = x + pio2_1;
		if(ix!=0x3ff921fb) { 	/* 33+53 bit pi is good enough */
			y[0] = z + pio2_1t;
			y[1] = (z-y[0])+pio2_1t;
		} else {		/* near pi/2, use 33+33+53 bit pi */
			z += pio2_2;
			y[0] = z + pio2_2t;
			y[1] = (z-y[0])+pio2_2t;
		}
		return -1;
		}
	}
	if(ix<=0x413921fb) { /* |x| ~<= 2^19*(pi/2), medium size */
		t  = fabs(x);
		n  = (int) (t*invpio2+halfD);
		fn = (double)n;
		r  = t-fn*pio2_1;
		w  = fn*pio2_1t;	/* 1st round good to 85 bit */
		if(n<32&&ix!=npio2_hw[n-1]) {
		y[0] = r-w;	/* quick check no cancellation */
		} else {
			j  = ix>>20;
			y[0] = r-w;
			i = j-(((__HI(y[0]))>>20)&0x7ff);
			if(i>16) {  /* 2nd iteration needed, good to 118 */
			t  = r;
			w  = fn*pio2_2;
			r  = t-w;
			w  = fn*pio2_2t-((t-r)-w);
			y[0] = r-w;
			i = j-(((__HI(y[0]))>>20)&0x7ff);
			if(i>49)  {	/* 3rd iteration need, 151 bits acc */
				t  = r;	/* will cover all possible cases */
				w  = fn*pio2_3;
				r  = t-w;
				w  = fn*pio2_3t-((t-r)-w);
				y[0] = r-w;
			}
		}
		}
		y[1] = (r-y[0])-w;
		if(hx<0) 	{y[0] = -y[0]; y[1] = -y[1]; return -n;}
		else	 return n;
	}
	/*
	 * all other (large) arguments
	 */
	if(ix>=0x7ff00000) {		/* x is inf or NaN */
		y[0]=y[1]=x-x; return 0;
	}
	/* set z = scalbn(|x|,ilogb(x)-23) */
	__setLow(&z,  __LO(x));
	e0 	= (ix>>20)-1046;	/* e0 = ilogb(z)-23; */
	__setHigh(&z, ix - (e0<<20));
	for(i=0;i<2;i++) {
		tx[i] = (double)((int)(z));
		z	 = (z-tx[i])*two24;
	}
	tx[2] = z;
	nx = 3;
	while(tx[nx-1]==zero) nx--;	/* skip zero term */
	n  =  __kernel_rem_pio2(tx,y,e0,nx,2,two_over_pi);
	if(hx<0) {y[0] = -y[0]; y[1] = -y[1]; return -n;}
	return n;
}

double __kernel_cos(double x, double y)
{
	double one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
	C1  =  4.16666666666666019037e-02, /* 0x3FA55555, 0x5555554C */
	C2  = -1.38888888888741095749e-03, /* 0xBF56C16C, 0x16C15177 */
	C3  =  2.48015872894767294178e-05, /* 0x3EFA01A0, 0x19CB1590 */
	C4  = -2.75573143513906633035e-07, /* 0xBE927E4F, 0x809C52AD */
	C5  =  2.08757232129817482790e-09, /* 0x3E21EE9E, 0xBDB4B1C4 */
	C6  = -1.13596475577881948265e-11; /* 0xBDA8FAE9, 0xBE8838D4 */

	double a,hz,z,r,qx;
	int ix;
	ix = __HI(x)&0x7fffffff;	/* ix = |x|'s high word*/
	if(ix<0x3e400000) {			/* if x < 2**27 */
		if(((int)x)==0) return one;		/* generate inexact */
	}
	z  = x*x;
	r  = z*(C1+z*(C2+z*(C3+z*(C4+z*(C5+z*C6)))));
	if(ix < 0x3FD33333) 			/* if |x| < 0.3 */
		return one - (0.5*z - (z*r - x*y));
	else {
		if(ix > 0x3fe90000) {		/* x > 0.78125 */
		qx = 0.28125;
		} else {
			__setHigh(&qx,  ix-0x00200000);	/* x/4 */
			__setLow(&qx, 0);
		}
		hz = 0.5*z-qx;
		a  = one-qx;
		return a - (hz - (z*r-x*y));
	}
}

double __kernel_sin(double x, double y, int iy)
{
	double halfD =  5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
	S1  = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */
	S2  =  8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */
	S3  = -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */
	S4  =  2.75573137070700676789e-06, /* 0x3EC71DE3, 0x57B1FE7D */
	S5  = -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */
	S6  =  1.58969099521155010221e-10; /* 0x3DE5D93A, 0x5ACFD57C */

	double z,r,v;
	int ix;
	ix = __HI(x)&0x7fffffff;	/* high word of x */
	if(ix<0x3e400000)			/* |x| < 2**-27 */
	   {if((int)x==0) return x;}		/* generate inexact */
	z	=  x*x;
	v	=  z*x;
	r	=  S2+z*(S3+z*(S4+z*(S5+z*S6)));
	if(iy==0) return x+v*(S1+z*r);
	else	  return x-((z*(halfD*y-v*r)-y)-v*S1);
}

OVERLOADABLE double cos(double x)
{
	double y[2],z=0.0;
	int n, ix;

	/* High word of x. */
	ix = __HI(x);

	/* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if(ix <= 0x3fe921fb) return __kernel_cos(x,z);

	/* cos(Inf or NaN) is NaN */
	else if (ix>=0x7ff00000) return x-x;

	/* argument reduction needed */
	else {
		n = __ieee754_rem_pio2(x,y);
		switch(n&3) {
		case 0: return  __kernel_cos(y[0],y[1]);
		case 1: return -__kernel_sin(y[0],y[1],1);
		case 2: return -__kernel_cos(y[0],y[1]);
		default:
				return  __kernel_sin(y[0],y[1],1);
		}
	}
}

OVERLOADABLE double cosh(double x)
{
	double one = 1.0, dHalf=0.5, huge = 1.0e300;

	double t,w;
	int ix;
	unsigned lx;

	/* High word of |x|. */
	ix = __HI(x);
	ix &= 0x7fffffff;

	/* x is INF or NaN */
	if(ix>=0x7ff00000) return x*x;

	/* |x| in [0,0.5*ln2], return 1+expm1(|x|)^2/(2*exp(|x|)) */
	if(ix<0x3fd62e43) {
		t = expm1(fabs(x));
		w = one+t;
		if (ix<0x3c800000) return w;	/* cosh(tiny) = 1 */
		return one+(t*t)/(w+w);
	}

	/* |x| in [0.5*ln2,22], return (exp(|x|)+1/exp(|x|)/2; */
	if (ix < 0x40360000) {
		t = exp(fabs(x));
		return dHalf*t+dHalf/t;
	}

	/* |x| in [22, log(maxdouble)] return half*exp(|x|) */
	if (ix < 0x40862E42)  return dHalf*exp(fabs(x));

	/* |x| in [log(maxdouble), overflowthresold] */
	lx = *( (((*(unsigned*)&one)>>29)) + (unsigned*)&x);
	if (ix<0x408633CE ||
		  ((ix==0x408633ce)&&(lx<=(unsigned)0x8fb9f87d))) {
		w = exp(dHalf*fabs(x));
		t = dHalf*w;
		return t*w;
	}

	/* |x| > overflowthresold, cosh(x) overflow */
	return huge*huge;

}

OVERLOADABLE double cospi(double x)
{
	double y,z, signValue = 1.0;
	int ix;

	ix = 0x7fffffff&__HI(x);

	if(ix<0x3fd00000) return __kernel_cos(M_PI*x,0);
	y = fabs(x);

	if(ix >=0x7ff00000)return as_double(DF_POSITIVE_INF + 1);

	z = floor(y);

	if(z > 0)
	{
		ulong uz = as_ulong(z);
		ulong expValue = ((uz & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
		ulong manValue = ((uz & DF_MAN_MASK) | DF_IMPLICITE_ONE);
		
		if(expValue > 52) return 1.0;
		manValue = (manValue >> (52 - expValue));
		
		if(manValue & 1)
			signValue = -1.0;
	}

	if(z!=y)
	{				/* inexact anyway */
		y = y -z;

		if(y > 0.5)
		{
			y = y - 0.5;
			signValue *= -1.0;
			return signValue * sin(y*M_PI);
		}
	}
	else
	{
		return signValue;
	}

	//the precison of sin is better than cos
	y = signValue * sin((0.5 - y)*M_PI);
	return y;
}

OVERLOADABLE double fabs(double x)
{
    long  qw = as_ulong(x);
    qw &= 0x7FFFFFFFFFFFFFFF;
    return as_double(qw);
}

OVERLOADABLE double fdim(double x, double y)
{
	if(isnan(x))
		return x;
	if(isnan(y))
		return y;
	return x > y ? (x - y) : +0.f;
}

OVERLOADABLE double maxmag(double x, double y)
{
	if(fabs(x) > fabs(y)) return x;
	if(fabs(y) > fabs(x)) return y;

	return fmax(x, y);
}

OVERLOADABLE double minmag(double x, double y)
{
	if(fabs(x) < fabs(y)) return x;
	if(fabs(y) < fabs(x)) return y;

	return fmin(x, y);
}

OVERLOADABLE double ldexp(double x, int n)
{
	ulong ux = as_ulong(x);

	if(((ux&DF_ABS_MASK) >= DF_POSITIVE_INF) ||x==0.0) return x;
		x = __scalbn(x,n);
		return x;
}

OVERLOADABLE double floor(double x)
{
    ulong lval = as_ulong(x);
    int exp = ((lval >> 52) & 0x7FF) - 1023;
    int sign = (lval >> 63) & 0x1;
    if(exp < 0)
    {
        if(sign)
	{
		if(lval & DF_ABS_MASK)
			return -1L;
		else
			return 0.0;
	}
        else return 0.0;
    }
    else
    {
        if(exp >= 52)
            return x;
         long i = (1L << (52 - exp));
         i = 0x10000000000000 - i;
         unsigned long uv = 0xFFF0000000000000 + i;
         ulong vv = lval & uv;
         double  dp = as_double(vv);
         if(vv != lval)
            dp -= sign;

         return dp;
    }
}

/*-
 * Copyright (c) 2005-2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
struct dd {
    double hi;
    double lo;
};

static inline struct dd
dd_add(double a, double b)
{
    struct dd ret;
    double s;

    ret.hi = a + b;
    s = ret.hi - a;
    ret.lo = (a - (ret.hi - s)) + (b - s);
    return (ret);
}

static inline double
add_adjusted(double a, double b)
{
    struct dd sum;
    ulong hibits, lobits;

    sum = dd_add(a, b);
    if (sum.lo != 0) {
        hibits = as_long(sum.hi);
        if ((hibits & 1) == 0) {
            /* hibits += (int)copysign(1.0, sum.hi * sum.lo) */
            lobits = as_long(sum.lo);
            hibits += 1 - ((hibits ^ lobits) >> 62);
            sum.hi = as_double(hibits);
        }
    }
    return (sum.hi);
}

static inline double
add_and_denormalize(double a, double b, int scale)
{
    struct dd sum;
    long hibits, lobits;
    int bits_lost;

    sum = dd_add(a, b);

    if (sum.lo != 0) {
        hibits = as_long(sum.hi);
        bits_lost = -((int)(hibits >> 52) & 0x7ff) - scale + 1;
        if ((bits_lost != 1) ^ (int)(hibits & 1)) {
            /* hibits += (int)copysign(1.0, sum.hi * sum.lo) */
            lobits = as_long(sum.lo);
            hibits += 1 - (((hibits ^ lobits) >> 62) & 2);
            sum.hi = as_double(hibits);
        }
    }
    return (ldexp(sum.hi, scale));
}

static inline struct dd
dd_mul(double a, double b)
{
    const double split = 0x1p27 + 1.0;
    struct dd ret;
    double ha, hb, la, lb, p, q;

    p = a * split;
    ha = a - p;
    ha += p;
    la = a - ha;

    p = b * split;
    hb = b - p;
    hb += p;
    lb = b - hb;

    p = ha * hb;
    q = ha * lb + la * hb;

    ret.hi = p + q;
    ret.lo = p - ret.hi + q + la * lb;
    return (ret);
}

double __frexp(double x, int *exp)
{
    double two54 =  1.80143985094819840000e+16; /* 0x43500000, 0x00000000 */
    int  hx, ix, lx;
    hx = __HI(x);
    ix = 0x7fffffff&hx;
    lx = __LO(x);
    *exp = 0;
    if(ix>=0x7ff00000||((ix|lx)==0)) return x;  /* 0,inf,nan */
    if (ix<0x00100000) {        /* subnormal */
        x *= two54;
        hx = __HI(x);
        ix = hx&0x7fffffff;
        *exp = -54;
    }
    *exp += (ix>>20)-1022;
    hx = (hx&0x800fffff)|0x3fe00000;
    __setHigh(&x, hx);
    return x;
}

OVERLOADABLE double fma(double x, double y, double z)
{
    double xs, ys, zs, adj;
    struct dd xy, r;
    int oround;
    int ex, ey, ez;
    int spread;

    if (x == 0.0 || y == 0.0)
        return (x * y + z);
    if (z == 0.0)
        return (x * y);
    if (!isfinite(x) || !isfinite(y))
        return (x * y + z);
    if (!isfinite(z))
        return (z);

    xs = __frexp(x, &ex);
    ys = __frexp(y, &ey);
    zs = __frexp(z, &ez);
    spread = ex + ey - ez;

    if (spread < -53) {
            return (z);
    }

    if (spread <= 53 * 2)
        zs = ldexp(zs, -spread);
    else
        zs = copysign(2.225073858507201383090e-308, zs);


    xy = dd_mul(xs, ys);
    r = dd_add(xy.hi, zs);

    spread = ex + ey;

    if (r.hi == 0.0) {
        return (xy.hi + zs + ldexp(xy.lo, spread));
    }

    adj = add_adjusted(r.lo, xy.lo);
    double base = r.hi + adj;
    if (spread + ilogb(r.hi) > -1023)
        return (ldexp(base, spread));
    else
        return (add_and_denormalize(r.hi, adj, spread));
}

OVERLOADABLE double hypot(double x, double y)
{
	double a=x,b=y,t1,t2,y1,y2,w;
	int j,k,ha,hb;

	ha = __HI(x)&0x7fffffff;	/* high word of  x */
	hb = __HI(y)&0x7fffffff;	/* high word of  y */
	if(hb > ha) {a=y;b=x;j=ha; ha=hb;hb=j;} else {a=x;b=y;}
	__setHigh(&a, ha);   /* a <- |a| */
	__setHigh(&b, hb);   /* b <- |b| */
	if((ha-hb)>0x3c00000) {return a+b;} /* x/y > 2**60 */
	k=0;
	if(ha > 0x5f300000) {   /* a>2**500 */
	   if(ha >= 0x7ff00000) {   /* Inf or NaN */
		   w = a+b;		 /* for sNaN */
		   if(((ha&0xfffff)|__LO(a))==0) w = a;
		   if(((hb^0x7ff00000)|__LO(b))==0) w = b;
		   return w;
	   }
	   /* scale a and b by 2**-600 */
	   ha -= 0x25800000; hb -= 0x25800000;  k += 600;
	   __setHigh(&a, ha);
	   __setHigh(&b, hb);
	}
	if(hb < 0x20b00000) {   /* b < 2**-500 */
		if(hb <= 0x000fffff) {  /* subnormal b or 0 */
		if((hb|(__LO(b)))==0) return a;
		t1=0;
		__setHigh(&t1, 0x7fd00000); /* t1=2^1022 */
		b *= t1;
		a *= t1;
		k -= 1022;
		} else {		/* scale a and b by 2^600 */
			ha += 0x25800000;   /* a *= 2^600 */
		hb += 0x25800000;   /* b *= 2^600 */
		k -= 600;
		__setHigh(&a, ha);
		__setHigh(&b, hb);
		}
	}
	/* medium size a and b */
	w = a-b;
	if (w>b) {
		t1 = 0;
		__setHigh(&t1, ha);
		t2 = a-t1;
		w  = sqrt(t1*t1-(b*(-b)-t2*(a+t1)));
	} else {
		a  = a+a;
		y1 = 0;
		__setHigh(&y1, hb);
		y2 = b - y1;
		t1 = 0;
		__setHigh(&t1, ha+0x00100000);
		t2 = a - t1;
		w  = sqrt(t1*y1-(w*(-w)-(t1*y2+t2*b)));
	}
	if(k!=0) {
		t1 = 1.0;
		__setHigh(&t1, __HI(t1) + (k<<20));
		return t1*w;
	} else return w;

}

OVERLOADABLE double log(double x)
{
	double ln2_hi	=  6.93147180369123816490e-01,	/* 3fe62e42 fee00000 */
	ln2_lo	=  1.90821492927058770002e-10,	/* 3dea39ef 35793c76 */
	two54	=  1.80143985094819840000e+16,	/* 43500000 00000000 */
	Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
	Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
	Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
	Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
	Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
	Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
	Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

	double zero = 0;
	double hfsq,f,s,z,R,w,t1,t2,dk;
	int k,hx,i,j;
	unsigned lx;

	hx = __HI(x);		/* high word of x */
	lx = __LO(x);		/* low  word of x */

	k=0;
	if (hx < 0x00100000)
	{			/* x < 2**-1022  */
		if (((hx&0x7fffffff)|lx)==0)
		return -two54/zero;		/* log(+-0)=-inf */
		if (hx<0)
			return (x-x)/zero;	/* log(-#) = NaN */
		k -= 54; x *= two54; /* subnormal number, scale up x */
		hx = __HI(x);		/* high word of x */
	}
	if (hx >= 0x7ff00000) return x+x;
	k += (hx>>20)-1023;
	hx &= 0x000fffff;
	i = (hx+0x95f64)&0x100000;
	__setHigh(&x, (hx|(i^0x3ff00000)));	/* normalize x or x/2 */
	k += (i>>20);
	f = x-1.0;
	if((0x000fffff&(2+hx))<3) {	/* |f| < 2**-20 */
		if(f==zero)
		{
			if(k==0) return zero;
			else
			{
				dk=(double)k;
				return dk*ln2_hi+dk*ln2_lo;
			}
		}

		R = f*f*(0.5-0.33333333333333333*f);
		if(k==0)
			return f-R;
		else {dk=(double)k;
			return dk*ln2_hi-((R-dk*ln2_lo)-f);}
	}
	s = f/(2.0+f);
	dk = (double)k;
	z = s*s;
	i = hx-0x6147a;
	w = z*z;
	j = 0x6b851-hx;
	t1= w*(Lg2+w*(Lg4+w*Lg6));
	t2= z*(Lg1+w*(Lg3+w*(Lg5+w*Lg7)));
	i |= j;
	R = t2+t1;
	if(i>0) {
	hfsq=0.5*f*f;
	if(k==0)
		return f-(hfsq-s*(hfsq+R));
	else
		return dk*ln2_hi-((hfsq-(s*(hfsq+R)+dk*ln2_lo))-f);
	}
	else
	{
		if(k==0)
			return f-s*(f-R);
		else
			return dk*ln2_hi-((s*(f-R)-dk*ln2_lo)-f);
	}

}

OVERLOADABLE double log2(double x)
{
	double ln2 = 0.69314718055994530942,
	zero = 0,
	two54   =  1.80143985094819840000e+16,  /* 43500000 00000000 */
	Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
	Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
	Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
	Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
	Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
	Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
	Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

	double hfsq,f,s,z,R,w,t1,t2,dk;
	int k,hx,i,j;
	uint lx;

	hx = __HI(x);
	lx = __LO(x);

	k=0;
	if (hx < 0x00100000)
	{			/* x < 2**-1022  */
		if (((hx&0x7fffffff)|lx)==0)
			return -two54/(x-x);		/* log(+-0)=-inf */

		if (hx<0) return (x-x)/(x-x);	/* log(-#) = NaN */

		k -= 54; x *= two54; /* subnormal number, scale up x */
		hx = __HI(x);
	}

	if (hx >= 0x7ff00000) return x+x;
	k += (hx>>20)-1023;
	hx &= 0x000fffff;
	i = (hx+0x95f64)&0x100000;
	__setHigh(&x,hx|(i^0x3ff00000));	/* normalize x or x/2 */
	k += (i>>20);
	dk = (double) k;
	f = x-1.0;

	if((0x000fffff&(2+hx))<3)
	{	/* |f| < 2**-20 */
		if(f==zero) return dk;
		R = f*f*(0.5-0.33333333333333333*f);
		return dk-(R-f)/ln2;
	}

	s = f/(2.0+f);
	z = s*s;
	i = hx-0x6147a;
	w = z*z;
	j = 0x6b851-hx;
	t1= w*(Lg2+w*(Lg4+w*Lg6));
	t2= z*(Lg1+w*(Lg3+w*(Lg5+w*Lg7)));
	i |= j;
	R = t2+t1;
	if(i>0)
	{
		hfsq=0.5*f*f;
		return dk-((hfsq-(s*(hfsq+R)))-f)/ln2;
	}
	else
	{
		return dk-((s*(f-R))-f)/ln2;
	}
}

OVERLOADABLE double log10(double x)
{
	double zero = 0.0,
	two54	   =  1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
	ivln10	   =  4.34294481903251816668e-01, /* 0x3FDBCB7B, 0x1526E50E */
	log10_2hi  =  3.01029995663611771306e-01, /* 0x3FD34413, 0x509F6000 */
	log10_2lo  =  3.69423907715893078616e-13; /* 0x3D59FEF3, 0x11F12B36 */

	double y,z;
	int i,k,hx;
	unsigned lx;

	hx = __HI(x);	/* high word of x */
	lx = __LO(x);	/* low word of x */

	k=0;
	if (hx < 0x00100000)
	{  /* x < 2**-1022  */
		if (((hx&0x7fffffff)|lx)==0)
			return -two54/zero; /* log(+-0)=-inf */

		if (hx<0)
			return (x-x)/zero;/* log(-#) = NaN */

		k -= 54; x *= two54; /* subnormal number, scale up x */
		hx = __HI(x);/* high word of x */
	}

	if (hx >= 0x7ff00000) return x+x;
	k += (hx>>20)-1023;
	i  = ((unsigned)k&0x80000000)>>31;
	hx = (hx&0x000fffff)|((0x3ff-i)<<20);
	y  = (double)(k+i);
	__setHigh(&x, hx);
	z  = y*log10_2lo + ivln10*log(x);
	return  z+y*log10_2hi;
}

OVERLOADABLE double log1p(double x)
{
	double ln2_hi  =  6.93147180369123816490e-01,	/* 3fe62e42 fee00000 */
	ln2_lo	=  1.90821492927058770002e-10,	/* 3dea39ef 35793c76 */
	two54	=  1.80143985094819840000e+16,	/* 43500000 00000000 */
	Lp1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
	Lp2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
	Lp3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
	Lp4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
	Lp5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
	Lp6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
	Lp7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

	double hfsq,f,c,s,z,R,u,zero = 0;
	int k,hx,hu,ax;

	hx = __HI(x);		/* high word of x */
	ax = hx&0x7fffffff;

	k = 1;
	if (hx < 0x3FDA827A) {			/* x < 0.41422	*/
		if(ax>=0x3ff00000) {		/* x <= -1.0 */
		if(x==-1.0) return -two54/zero; /* log1p(-1)=+inf */
		else return (x-x)/(x-x);	/* log1p(x<-1)=NaN */
		}
		if(ax<0x3e200000) { 		/* |x| < 2**-29 */
		if(two54+x>zero 		/* raise inexact */
				&&ax<0x3c900000)		/* |x| < 2**-54 */
			return x;
		else
			return x - x*x*0.5;
		}
		if(hx>0||hx<=((int)0xbfd2bec3)) {
		k=0;f=x;hu=1;}	/* -0.2929<x<0.41422 */
	}
	if (hx >= 0x7ff00000) return x+x;
	if(k!=0) {
		if(hx<0x43400000) {
		u  = 1.0+x;
			hu = __HI(u);		/* high word of u */
			k  = (hu>>20)-1023;
			c  = (k>0)? 1.0-(u-x):x-(u-1.0);/* correction term */
		c /= u;
		} else {
		u  = x;
			hu = __HI(u);		/* high word of u */
			k  = (hu>>20)-1023;
		c  = 0;
		}
		hu &= 0x000fffff;
		if(hu<0x6a09e) {
			__setHigh(&u, hu|0x3ff00000);	/* normalize u */
		} else {
			k += 1;
			__setHigh(&u, hu|0x3fe00000);	/* normalize u/2 */
			hu = (0x00100000-hu)>>2;
		}
		f = u-1.0;
	}
	hfsq=0.5*f*f;
	if(hu==0) { /* |f| < 2**-20 */
		if(f==zero)
		{
			if(k==0) return zero;
			else {c += k*ln2_lo; return k*ln2_hi+c;}
		}
		R = hfsq*(1.0-0.66666666666666666*f);
		if(k==0) return f-R; else
				 return k*ln2_hi-((R-(k*ln2_lo+c))-f);
	}
	s = f/(2.0+f);
	z = s*s;
	R = z*(Lp1+z*(Lp2+z*(Lp3+z*(Lp4+z*(Lp5+z*(Lp6+z*Lp7))))));
	if(k==0) return f-(hfsq-s*(hfsq+R)); else
		return k*ln2_hi-((hfsq-(s*(hfsq+R)+(k*ln2_lo+c)))-f);

}

OVERLOADABLE double logb(double x)
{
	int lx,ix;
	ix = (__HI(x))&0x7fffffff;	/* high |x| */
	lx = __LO(x);			/* low x */
	if((ix|lx)==0) return -1.0/fabs(x);
	if(ix>=0x7ff00000) return x*x;
	if((ix>>=20)==0) 			/* IEEE 754 logb */
	{
		long qx = as_long(x);
		qx = qx & DF_ABS_MASK;
		int msbOne = clz(qx);
		return (double)(-1022 - (53 -(64 -msbOne)));
	}
	else
		return (double) (ix-1023);
}

OVERLOADABLE int ilogb(double x)
{
	int hx,lx,ix;

	hx  = (__HI(x))&0x7fffffff;	/* high word of x */
	if(hx == 0x7ff00000 && __LO(x) == 0) return 0x7fffffff;

	if(hx<0x00100000) {
		lx = __LO(x);
		if((hx|lx)==0)
		return 0x80000000;	/* ilogb(0) = 0x80000000 */
		else			/* subnormal x */
		if(hx==0) {
			for (ix = -1043; lx>0; lx<<=1) ix -=1;
		} else {
			for (ix = -1022,hx<<=11; hx>0; hx<<=1) ix -=1;
		}
		return ix;
	}
	else if (hx<0x7ff00000) return (hx>>20)-1023;
	else return 0x80000000;
}

OVERLOADABLE double lgamma(double x)
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
	ix = hx&0x7fffffff;
	if(ix>=0x7ff00000) return x*x;
	if((ix|lx)==0) return one/zero;
	if(ix<0x3b900000) {	/* |x|<2**-70, return -log(|x|) */
		if(hx<0) {
			return -log(-x);
		} else return -log(x);
	}
	if(hx<0) {
		if(ix>=0x43300000) 	/* |x|>=2**52, must be -integer */
		return one/zero;
		t = sinpi(x);
		if(t==zero) return one/zero; /* -integer */
		nadj = log(pi/fabs(t*x));
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

CONST OVERLOADABLE double __gen_ocl_mad(double a, double b, double c) __asm("llvm.fma" ".f64");
OVERLOADABLE double mad(double a, double b, double c)
{
	return __gen_ocl_mad(a, b, c);
}

OVERLOADABLE double nan(ulong code)
{
	return as_double(DF_POSITIVE_INF + (code&DF_MAN_MASK));
}

OVERLOADABLE double nextafter(double x, double y)
{
	long hx, hy, ix, iy;
	hx = as_long(x);
	hy = as_long(y);
	ix = hx & DF_ABS_MASK;
	iy = hy & DF_ABS_MASK;

	if(ix>DF_POSITIVE_INF|| iy>DF_POSITIVE_INF)
	  return x+y;
	if(hx == hy)
	  return y;
	if(ix == 0) {
	  if(iy == 0)
		return y;
	  else
		return as_double((hy&DF_SIGN_MASK) | 1);
	}
	if(hx >= 0) {
	  if(hx > hy) {
		hx -= 1;
	  } else {
		hx += 1;
	  }
	} else {
	  if(hy >= 0 || hx > hy){
		hx -= 1;
	  } else {
		hx += 1;
	  }
	}
	return as_double(hx);
}

OVERLOADABLE double fmax(double a, double b)
{
	ulong ua = as_ulong(a);
	ulong ub =as_ulong(b);

	if((ua & DF_ABS_MASK) > DF_MAX_NORMAL) return b;
	if((ub & DF_ABS_MASK) > DF_MAX_NORMAL) return a;
	if(ua == DF_POSITIVE_INF) return a;
	if(ub == DF_POSITIVE_INF) return b;

	double c = a - b;
	return (c >= 0) ? a:b;
}

OVERLOADABLE double fmin(double a, double b)
{
	ulong ua = as_ulong(a);
	ulong ub =as_ulong(b);

	if((ua & DF_ABS_MASK) > DF_MAX_NORMAL) return b;
	if((ub & DF_ABS_MASK) > DF_MAX_NORMAL) return a;
	if(ua == DF_NEGTIVE_INF) return a;
	if(ub == DF_NEGTIVE_INF) return b;

	double c = a - b;
	return (c <= 0) ? a:b;
}

OVERLOADABLE double fmod (double x, double y)
{
	const double one = 1.0, Zero[] = {0.0, -0.0,};
	int n,hx,hy,hz,ix,iy,sx,i;
	uint lx,ly,lz;

	hx = __HI(x);
	lx = __LO(x);
	hy = __HI(y);
	ly = __LO(y);
	sx = hx&0x80000000;		/* sign of x */
	hx ^=sx;		/* |x| */
	hy &= 0x7fffffff;	/* |y| */

    /* purge off exception values */
	if((hy|ly)==0||(hx>=0x7ff00000)||	/* y=0,or x not finite */
	  ((hy|((ly|-ly)>>31))>0x7ff00000))	/* or y is NaN */
	    return (x*y)/(x*y);
	if(hx<=hy) {
	    if((hx<hy)||(lx<ly)) return x;	/* |x|<|y| return x */
	    if(lx==ly) 
		return Zero[(uint)sx>>31];	/* |x|=|y| return x*0*/
	}

    /* determine ix = ilogb(x) */
	if(hx<0x00100000) {	/* subnormal x */
	    if(hx==0) {
		for (ix = -1043, i=lx; i>0; i<<=1) ix -=1;
	    } else {
		for (ix = -1022,i=(hx<<11); i>0; i<<=1) ix -=1;
	    }
	} else ix = (hx>>20)-1023;

    /* determine iy = ilogb(y) */
	if(hy<0x00100000) {	/* subnormal y */
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
	while(n--) {
	    hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	    if(hz<0){hx = hx+hx+(lx>>31); lx = lx+lx;}
	    else {
	    	if((hz|lz)==0) 		/* return sign(x)*0 */
		    return Zero[(uint)sx>>31];
	    	hx = hz+hz+(lz>>31); lx = lz+lz;
	    }
	}
	hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	if(hz>=0) {hx=hz;lx=lz;}

    /* convert back to floating value and restore the sign */
	if((hx|lx)==0) 			/* return sign(x)*0 */
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

OVERLOADABLE double __ocl_internal_pow(double x, double y)
{
	double bp[] = {1.0, 1.5,},
	dp_h[] = { 0.0, 5.84962487220764160156e-01,}, /* 0x3FE2B803, 0x40000000 */
	dp_l[] = { 0.0, 1.35003920212974897128e-08,}, /* 0x3E4CFDEB, 0x43CFD006 */
	zero	=  0.0,
	one =  1.0,
	two =  2.0,
	two53   =  9007199254740992.0,  /* 0x43400000, 0x00000000 */
	huge	=  1.0e300,
	tiny	=  1.0e-300,
		/* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
	L1  =  5.99999999999994648725e-01, /* 0x3FE33333, 0x33333303 */
	L2  =  4.28571428578550184252e-01, /* 0x3FDB6DB6, 0xDB6FABFF */
	L3  =  3.33333329818377432918e-01, /* 0x3FD55555, 0x518F264D */
	L4  =  2.72728123808534006489e-01, /* 0x3FD17460, 0xA91D4101 */
	L5  =  2.30660745775561754067e-01, /* 0x3FCD864A, 0x93C9DB65 */
	L6  =  2.06975017800338417784e-01, /* 0x3FCA7E28, 0x4A454EEF */
	P1   =  1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
	P2   = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
	P3   =  6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
	P4   = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
	P5   =  4.13813679705723846039e-08, /* 0x3E663769, 0x72BEA4D0 */
	lg2  =  6.93147180559945286227e-01, /* 0x3FE62E42, 0xFEFA39EF */
	lg2_h  =  6.93147182464599609375e-01, /* 0x3FE62E43, 0x00000000 */
	lg2_l  = -1.90465429995776804525e-09, /* 0xBE205C61, 0x0CA86C39 */
	ovt =  8.0085662595372944372e-0017, /* -(1024-log2(ovfl+.5ulp)) */
	cp	=  9.61796693925975554329e-01, /* 0x3FEEC709, 0xDC3A03FD =2/(3ln2) */
	cp_h  =  9.61796700954437255859e-01, /* 0x3FEEC709, 0xE0000000 =(float)cp */
	cp_l  = -7.02846165095275826516e-09, /* 0xBE3E2FE0, 0x145B01F5 =tail of cp_h*/
	ivln2	=  1.44269504088896338700e+00, /* 0x3FF71547, 0x652B82FE =1/ln2 */
	ivln2_h  =  1.44269502162933349609e+00, /* 0x3FF71547, 0x60000000 =24b 1/ln2*/
	ivln2_l  =  1.92596299112661746887e-08; /* 0x3E54AE0B, 0xF85DDF44 =1/ln2 tail*/

	double z,ax,z_h,z_l,p_h,p_l;
	double y1,t1,t2,r,s,t,u,v,w;
	int i0,i1,i,j,k,yisint,n;
	int hx,hy,ix,iy;
	unsigned lx,ly;

	hx = __HI(x); lx = __LO(x);
	hy = __HI(y); ly = __LO(y);
	ix = hx&0x7fffffff;  iy = hy&0x7fffffff;

	i0 = ((*(int*)&one)>>29)^1; i1=1-i0;

	/* determine if y is an odd int when x < 0
	 * yisint = 0   ... y is not an integer
	 * yisint = 1   ... y is an odd int
	 * yisint = 2   ... y is an even int
	 */
	yisint  = 0;
	if(hx<0) {
		if(iy>=0x43400000) yisint = 2; /* even integer y */
		else if(iy>=0x3ff00000) {
		k = (iy>>20)-0x3ff;	/* exponent */
		if(k>20) {
			j = ly>>(52-k);
			if((j<<(52-k))==ly) yisint = 2-(j&1);
		} else if(ly==0) {
			j = iy>>(20-k);
			if((j<<(20-k))==iy) yisint = 2-(j&1);
		}
		}
	}

	/* special value of y */
	if(ly==0) {
		if (iy==0x7ff00000) {   /* y is +-inf */
			if(((ix-0x3ff00000)|lx)==0)
			return  1.0;  /* inf**+-1 is NaN */
			else if (ix >= 0x3ff00000)/* (|x|>1)**+-inf = inf,0 */
			return (hy>=0)? y: zero;
			else			/* (|x|<1)**-,+inf = inf,0 */
			return (hy<0)?-y: zero;
		}
		if(iy==0x3ff00000) {	/* y is  +-1 */
		if(hy<0) return one/x; else return x;
		}
		if(hy==0x40000000) return x*x; /* y is  2 */
		if(hy==0x3fe00000) {	/* y is  0.5 */
		if(hx>=0)   /* x >= +0 */
		return sqrt(x);
		}
	}

	ax   = fabs(x);
	/* special value of x */
	if(lx==0) {
		if(ix==0x7ff00000||ix==0||ix==0x3ff00000){
		z = ax;		 /*x is +-0,+-inf,+-1*/
		if(hy<0) z = one/z; /* z = (1/|x|) */
		if(hx<0) {
			if(((ix-0x3ff00000)|yisint)==0) {
			z = (z-z)/(z-z); /* (-1)**non-int is NaN */
			} else if(yisint==1)
			z = -z;	 /* (x<0)**odd = -(|x|**odd) */
		}
		return z;
		}
	}

	n = (hx>>31)+1;

	/* (x<0)**(non-int) is NaN */
	if((n|yisint)==0) return (x-x)/(x-x);

	s = one; /* s (sign of result -ve**odd) = -1 else = 1 */
	if((n|(yisint-1))==0) s = -one;/* (-ve)**(odd int) */

	/* |y| is huge */
	if(iy>0x41e00000) { /* if |y| > 2**31 */
		if(iy>0x43f00000){  /* if |y| > 2**64, must o/uflow */
		if(ix<=0x3fefffff) return (hy<0)? huge*huge:tiny*tiny;
		if(ix>=0x3ff00000) return (hy>0)? huge*huge:tiny*tiny;
		}
	/* over/underflow if x is not close to one */
		if(ix<0x3fefffff) return (hy<0)? s*huge*huge:s*tiny*tiny;
		if(ix>0x3ff00000) return (hy>0)? s*huge*huge:s*tiny*tiny;
	/* now |1-x| is tiny <= 2**-20, suffice to compute
	   log(x) by x-x^2/2+x^3/3-x^4/4 */
		t = ax-one;	 /* t has 20 trailing zeros */
		w = (t*t)*(0.5-t*(0.3333333333333333333333-t*0.25));
		u = ivln2_h*t;  /* ivln2_h has 21 sig. bits */
		v = t*ivln2_l-w*ivln2;
		t1 = u+v;
		__setLow(&t1, 0);
		t2 = v-(t1-u);
	} else {
		double ss,s2,s_h,s_l,t_h,t_l;
		n = 0;
	/* take care subnormal number */
		if(ix<0x00100000)
		{ax *= two53; n -= 53; ix = __HI(ax); }
		n  += ((ix)>>20)-0x3ff;
		j  = ix&0x000fffff;
	/* determine interval */
		ix = j|0x3ff00000;	  /* normalize ix */
		if(j<=0x3988E) k=0;	 /* |x|<sqrt(3/2) */
		else if(j<0xBB67A) k=1; /* |x|<sqrt(3)   */
		else {k=0;n+=1;ix -= 0x00100000;}
		__setHigh(&ax, ix);

	/* compute ss = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
		u = ax-bp[k];	   /* bp[0]=1.0, bp[1]=1.5 */
		v = one/(ax+bp[k]);
		ss = u*v;
		s_h = ss;
		__setLow(&s_h, 0);
	/* t_h=ax+bp[k] High */
		t_h = zero;
		__setHigh(&t_h, ((ix>>1)|0x20000000)+0x00080000+(k<<18));
		t_l = ax - (t_h-bp[k]);
		s_l = v*((u-s_h*t_h)-s_h*t_l);
	/* compute log(ax) */
		s2 = ss*ss;
		r = s2*s2*(L1+s2*(L2+s2*(L3+s2*(L4+s2*(L5+s2*L6)))));
		r += s_l*(s_h+ss);
		s2  = s_h*s_h;
		t_h = 3.0+s2+r;
		__setLow(&t_h, 0);
		t_l = r-((t_h-3.0)-s2);
	/* u+v = ss*(1+...) */
		u = s_h*t_h;
		v = s_l*t_h+t_l*ss;
	/* 2/(3log2)*(ss+...) */
		p_h = u+v;
		__setLow(&p_h, 0);
		p_l = v-(p_h-u);
		z_h = cp_h*p_h;	 /* cp_h+cp_l = 2/(3*log2) */
		z_l = cp_l*p_h+p_l*cp+dp_l[k];
	/* log2(ax) = (ss+..)*2/(3*log2) = n + dp_h + z_h + z_l */
		t = (double)n;
		t1 = (((z_h+z_l)+dp_h[k])+t);
		__setLow(&t1, 0);
		t2 = z_l-(((t1-t)-dp_h[k])-z_h);
	}

	/* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
	y1  = y;
	__setLow(&y1, 0);
	p_l = (y-y1)*t1+y*t2;
	p_h = y1*t1;
	z = p_l+p_h;
	j = __HI(z);
	i = __LO(z);
	if (j>=0x40900000) {				/* z >= 1024 */
		if(((j-0x40900000)|i)!=0)		   /* if z > 1024 */
		return s*huge*huge;		 /* overflow */
		else {
		if(p_l+ovt>z-p_h) return s*huge*huge;   /* overflow */
		}
	} else if((j&0x7fffffff)>=0x4090cc00 ) {	/* z <= -1075 */
		if(((j-0xc090cc00)|i)!=0)	   /* z < -1075 */
		return s*tiny*tiny;	 /* underflow */
		else {
		if(p_l<=z-p_h) return s*tiny*tiny;  /* underflow */
		}
	}
	/*
	 * compute 2**(p_h+p_l)
	 */
	i = j&0x7fffffff;
	k = (i>>20)-0x3ff;
	n = 0;
	if(i>0x3fe00000) {	  /* if |z| > 0.5, set n = [z+0.5] */
		n = j+(0x00100000>>(k+1));
		k = ((n&0x7fffffff)>>20)-0x3ff; /* new k for n */
		t = zero;
		__setHigh(&t, (n&~(0x000fffff>>k)));
		n = ((n&0x000fffff)|0x00100000)>>(20-k);
		if(j<0) n = -n;
		p_h -= t;
	}
	t = p_l+p_h;
	__setLow(&t, 0);
	u = t*lg2_h;
	v = (p_l-(t-p_h))*lg2+t*lg2_l;
	z = u+v;
	w = v-(z-u);
	t  = z*z;
	t1  = z - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
	r  = (z*t1)/(t1-two)-(w+z*w);
	z  = one-(r-z);
	j  = __HI(z);
	j += (n<<20);
	if((j>>20)<=0) z = __scalbn(z,n); /* subnormal output */
	else __setHigh(&z, __HI(z)+(n<<20));
	return s*z;
}


OVERLOADABLE double pow(double x, double y)
{
	int hx,hy,ix,iy;
	unsigned lx,ly;

	hx = __HI(x); lx = __LO(x);
	hy = __HI(y); ly = __LO(y);
	ix = hx&0x7fffffff;  iy = hy&0x7fffffff;

	/* y==zero: x**0 = 1 */
	if((iy|ly)==0) return 1.0;

	/* +-NaN return x+y */
	if(ix > 0x7ff00000 || ((ix==0x7ff00000)&&(lx!=0)))
		return x+y;

	if(iy > 0x7ff00000 || ((iy==0x7ff00000)&&(ly!=0)))
	{
		if(x == 1.0)
			return x;
		else
		return x + y;
	}

	return __ocl_internal_pow(x, y);
}

OVERLOADABLE double pown(double x, int n)
{
	int hx,hy,ix,iy;
	unsigned lx,ly;

	hx = __HI(x); lx = __LO(x);
	ix = hx&0x7fffffff;

	/* y==zero: x**0 = 1 */
	if(n ==0) return 1.0;

	/* +-NaN return x+y */
	if(ix > 0x7ff00000 || ((ix==0x7ff00000)&&(lx!=0)))
		return x+n;

	return __ocl_internal_pow(x, n);
}

OVERLOADABLE double powr(double x, double y)
{
    if( x < 0.0L )
        return as_double(DF_POSITIVE_INF + 1);

    if( isnan(x) || isnan(y) )
        return x + y;

    if( x == 1.0L )
    {
        if(fabs(y) == INFINITY )
            return as_double(DF_POSITIVE_INF + 1);

        return 1.0L;
    }


    if( y == 0.0L )
    {
        if( x == 0.0L || x == INFINITY )
            return as_double(DF_POSITIVE_INF + 1);
        return 1.0L;
    }

    if( x == 0.0L )
    {
        if( y < 0.0L )
            return INFINITY;
        return 0.0L;
    }

    return __ocl_internal_pow(x, y);
}

OVERLOADABLE double remainder(double x, double p)
{
	int hx,hp;
	unsigned sx,lx,lp;
	double p_half, zero = 0.0;

	hx = __HI(x);		/* high word of x */
	lx = __LO(x);		/* low  word of x */
	hp = __HI(p);		/* high word of p */
	lp = __LO(p);		/* low  word of p */
	sx = hx&0x80000000;
	hp &= 0x7fffffff;
	hx &= 0x7fffffff;

	/* purge off exception values */
	if((hp|lp)==0) return (x*p)/(x*p); 	/* p = 0 */
	if((hx>=0x7ff00000)||			/* x not finite */
	  ((hp>=0x7ff00000)&&			/* p is NaN */
	  (((hp-0x7ff00000)|lp)!=0)))
		return (x*p)/(x*p);


	if (hp<=0x7fdfffff) x = fmod(x,p+p);	/* now x < 2p */
	if (((hx-hp)|(lx-lp))==0) return zero*x;
	x  = fabs(x);
	p  = fabs(p);
	if (hp<0x00200000) {
		if(x+x>p) {
		x-=p;
		if(x+x>=p) x -= p;
		}
	} else {
		p_half = 0.5*p;
		if(x>p_half) {
		x-=p;
		if(x>=p_half) x -= p;
		}
	}
	__setHigh(&x,  __HI(x) ^sx);
	return x;
}

OVERLOADABLE double rint(double x)
{
	long ret;
	long lval = as_long(x);
	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	long sign = (lval & DF_SIGN_MASK)?1:0;
	long ma = (lval &DF_MAN_MASK);

	if((lval & DF_ABS_MASK) == 0)
		return as_double(sign << 63);

	if(exp < -1)
	{
		ret = ((sign << 63)) ;
		return as_double(ret);
	}

	if(exp > 51) return x;

	long i = (1L << (52 - exp));
	i = 0x10000000000000 - i;
	unsigned long uv = 0xFFF0000000000000 + i;
	ulong vv = lval & uv;
	double	dp = as_double(vv);
	if(exp == -1) dp = 0;

	long fval = ma | DF_IMPLICITE_ONE;
	long lastBit = (fval & (1L << (52 -exp)));
	long roundBits = (fval & ( (1L << (52 -exp)) -1));

	if(roundBits > (1L << (51 -exp)))
		dp = (sign) ? dp-1.0:dp+1.0;
	else if((roundBits == (1L << (51 -exp))) && lastBit)
		dp = (sign) ? dp-1.0:dp+1.0;

	return dp;
}

OVERLOADABLE double round(double x)
{
	long ret;
	long lval = as_long(x);
	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	long sign = (lval & DF_SIGN_MASK)?1:0;
	long ma = (lval &DF_MAN_MASK);

	if((lval & DF_ABS_MASK) == 0)
		return as_double(sign << 63);

	if(exp < -1)
	{
		ret = ((sign << 63)) ;
		return as_double(ret);
	}

	if(exp > 51) return x;

	long i = (1L << (52 - exp));
	i = 0x10000000000000 - i;
	unsigned long uv = 0xFFF0000000000000 + i;
	ulong vv = lval & uv;
	double	dp = as_double(vv);
	if(exp == -1) dp = 0;

	long fval = ma | DF_IMPLICITE_ONE;
	long roundBits = (fval & ( (1L << (52 -exp)) -1));

	if(roundBits > (1L << (51 -exp)))
		dp = (sign) ? dp-1.0:dp+1.0;
	else if(roundBits == (1L << (51 -exp)))
		dp = (sign) ? dp-1.0:dp+1.0;

	return dp;
}

OVERLOADABLE double rootn(double x, int n)
{
	double ax,re;
	ulong ux = as_ulong(x);
	ulong sign = ux&DF_SIGN_MASK;

	if( n == 0 )return as_double(DF_POSITIVE_INF+1);

	if((ux&DF_ABS_MASK)== 0)
	{
		int neg = (n & 0x80000000);
		int odd = (n & 1);

		if(neg && odd)
			return as_double(sign|DF_POSITIVE_INF);

		if(neg && !odd)
			return as_double(DF_POSITIVE_INF);

		if(!neg && !odd)
			return 0.0;

		if(!neg && odd)
			return as_double(sign);
	}

	if( x < 0 && ! (n&1) )
		return as_double(DF_POSITIVE_INF+1);

	if((ux&DF_ABS_MASK) == DF_POSITIVE_INF)
	{
		if(n < 0)
			return 0.0;

		return x;
	}

	if(n == 1)
		return x;
	if(n == 2)
		return sqrt(x);
	if(n == 3)
		return cbrt(x);

	if(n == -1)
		return 1.0/x;
	if(n == -2)
		return rsqrt(x);
	if(n == -3)
		return 1.0/cbrt(x);

	if(x > 0.0f)
		re = exp10(log10(x)/(double)n);
	else
		re = -exp10(log10(-x)/(double)n);

	return re;
}

OVERLOADABLE double rsqrt(double x)
{
    return 1.0/sqrt(x);
}

OVERLOADABLE double sin(double x)
{
	double y[2],z=0.0;
	int n, ix;

	/* High word of x. */
	ix = __HI(x);

	/* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if(ix <= 0x3fe921fb) return __kernel_sin(x,z,0);

	/* sin(Inf or NaN) is NaN */
	else if (ix>=0x7ff00000) return x-x;

	/* argument reduction needed */
	else {
		n = __ieee754_rem_pio2(x,y);
		switch(n&3) {
		case 0: return  __kernel_sin(y[0],y[1],1);
		case 1: return  __kernel_cos(y[0],y[1]);
		case 2: return -__kernel_sin(y[0],y[1],1);
		default:
			return -__kernel_cos(y[0],y[1]);
		}
	}
}

OVERLOADABLE double sinh(double x)
{
	double one = 1.0, shuge = 1.0e307;
	double t,w,h;
	int ix,jx;
	unsigned lx;

	/* High word of |x|. */
	jx = __HI(x);
	ix = jx&0x7fffffff;

	/* x is INF or NaN */
	if(ix>=0x7ff00000) return x+x;

	h = 0.5;
	if (jx<0) h = -h;
	/* |x| in [0,22], return sign(x)*0.5*(E+E/(E+1))) */
	if (ix < 0x40360000) {		/* |x|<22 */
		if (ix<0x3e300000) 		/* |x|<2**-28 */
		if(shuge+x>one) return x;/* sinh(tiny) = tiny with inexact */
		t = expm1(fabs(x));
		if(ix<0x3ff00000) return h*(2.0*t-t*t/(t+one));
		return h*(t+t/(t+one));
	}

	/* |x| in [22, log(maxdouble)] return 0.5*exp(|x|) */
	if (ix < 0x40862E42)  return h*exp(fabs(x));

	/* |x| in [log(maxdouble), overflowthresold] */
	lx = *( (((*(unsigned*)&one)>>29)) + (unsigned*)&x);
	if (ix<0x408633CE || ((ix==0x408633ce)&&(lx<=(unsigned)0x8fb9f87d))) {
		w = exp(0.5*fabs(x));
		t = h*w;
		return t*w;
	}

	/* |x| > overflowthresold, sinh(x) overflow */
	return x*shuge;
}

OVERLOADABLE double sinpi(double x)
{
	double y,z, signVaule = 1.0;
	int ix;

	ix = 0x7fffffff&__HI(x);

	if(ix<0x3fd00000) return __kernel_sin(M_PI*x,0, 0);
	y = fabs(x);

	if(ix >=0x7ff00000)return as_double(DF_POSITIVE_INF + 1);

	z = floor(y);

	if(z!=y)
	{				/* inexact anyway */
		y = y -z;

		if(y > 0.5)
		{
			y = y - 0.5;
			y = 0.5 - y;
		}

		if(z > 0)
		{
			ulong uz = as_ulong(z);
			int expValue = ((uz & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
			ulong manValue = ((uz & DF_MAN_MASK) | DF_IMPLICITE_ONE);
			
			if(expValue > 52) return 1.0;
			manValue = (manValue >> (52 - expValue));
			
			if(manValue & 1)
				signVaule =  -1.0;
		}
	}
	else
	{
		return copysign(0.0, x);
	}

	y = sin(y*M_PI);
	return signVaule*copysign(y, x);
}

OVERLOADABLE double sqrt(double x)
{
    double z;
    int     sign = (int)0x80000000;
    unsigned r,t1,s1,ix1,q1;
    int ix0,s0,q,m,t,i;
    const double    one = 1.0, tiny=1.0e-300;

    ix0 = __HI(x);          /* high word of x */
    ix1 = __LO(x);      /* low word of x */

    /* take care of Inf and NaN */
    if((ix0&0x7ff00000)==0x7ff00000) {
        return x*x+x;       /* sqrt(NaN)=NaN, sqrt(+inf)=+inf
                       sqrt(-inf)=sNaN */
    }
    /* take care of zero */
    if(ix0<=0) {
        if(((ix0&(~sign))|ix1)==0) return x;/* sqrt(+-0) = +-0 */
        else if(ix0<0)
        return (x-x)/(x-x);     /* sqrt(-ve) = sNaN */
    }
    /* normalize x */
    m = (ix0>>20);
    if(m==0) {              /* subnormal x */
        while(ix0==0) {
        m -= 21;
        ix0 |= (ix1>>11); ix1 <<= 21;
        }
        for(i=0;(ix0&0x00100000)==0;i++) ix0<<=1;
        m -= i-1;
        ix0 |= (ix1>>(32-i));
        ix1 <<= i;
    }
    m -= 1023;  /* unbias exponent */
    ix0 = (ix0&0x000fffff)|0x00100000;
    if(m&1){    /* odd m, double x to make it even */
        ix0 += ix0 + ((ix1&sign)>>31);
        ix1 += ix1;
    }
    m >>= 1;    /* m = [m/2] */

    /* generate sqrt(x) bit by bit */
    ix0 += ix0 + ((ix1&sign)>>31);
    ix1 += ix1;
    q = q1 = s0 = s1 = 0;   /* [q,q1] = sqrt(x) */
    r = 0x00200000;     /* r = moving bit from right to left */

    while(r!=0) {
        t = s0+r;
        if(t<=ix0) {
        s0   = t+r;
        ix0 -= t;
        q   += r;
        }
        ix0 += ix0 + ((ix1&sign)>>31);
        ix1 += ix1;
        r>>=1;
    }

    r = sign;
    while(r!=0) {
        t1 = s1+r;
        t  = s0;
        if((t<ix0)||((t==ix0)&&(t1<=ix1))) {
        s1  = t1+r;
        if(((t1&sign)==sign)&&(s1&sign)==0) s0 += 1;
        ix0 -= t;
        if (ix1 < t1) ix0 -= 1;
        ix1 -= t1;
        q1  += r;
        }
        ix0 += ix0 + ((ix1&sign)>>31);
        ix1 += ix1;
        r>>=1;
    }

    /* use floating add to find out rounding direction */
    if((ix0|ix1)!=0) {
        z = one-tiny; /* trigger inexact flag */
        if (z>=one) {
            z = one+tiny;
            if (q1==(unsigned)0xffffffff) { q1=0; q += 1;}
        else if (z>one) {
            if (q1==(unsigned)0xfffffffe) q+=1;
            q1+=2;
        } else
                q1 += (q1&1);
        }
    }
    ix0 = (q>>1)+0x3fe00000;
    ix1 =  q1>>1;
    if ((q&1)==1) ix1 |= sign;
    ix0 += (m <<20);
    __setHigh(&z, ix0);
    __setLow(&z, ix1);
    return z;
}

double __kernel_tan(double x, double y, int iy)
{
		 const double xxx[] = {
			 3.33333333333334091986e-01,	/* 3FD55555, 55555563 */
			 1.33333333333201242699e-01,	/* 3FC11111, 1110FE7A */
			 5.39682539762260521377e-02,	/* 3FABA1BA, 1BB341FE */
			 2.18694882948595424599e-02,	/* 3F9664F4, 8406D637 */
			 8.86323982359930005737e-03,	/* 3F8226E3, E96E8493 */
			 3.59207910759131235356e-03,	/* 3F6D6D22, C9560328 */
			 1.45620945432529025516e-03,	/* 3F57DBC8, FEE08315 */
			 5.88041240820264096874e-04,	/* 3F4344D8, F2F26501 */
			 2.46463134818469906812e-04,	/* 3F3026F7, 1A8D1068 */
			 7.81794442939557092300e-05,	/* 3F147E88, A03792A6 */
			 7.14072491382608190305e-05,	/* 3F12B80F, 32F0A7E9 */
			-1.85586374855275456654e-05,	/* BEF375CB, DB605373 */
			 2.59073051863633712884e-05,	/* 3EFB2A70, 74BF7AD4 */
	/* one */	1.00000000000000000000e+00,	/* 3FF00000, 00000000 */
	/* pio4 */   7.85398163397448278999e-01,	/* 3FE921FB, 54442D18 */
	/* pio4lo */	 3.06161699786838301793e-17 /* 3C81A626, 33145C07 */
	};
#define	one	xxx[13]
#define	pio4	xxx[14]
#define	pio4lo	xxx[15]
#define	T	xxx

	double z, r, v, w, s;
	int ix, hx;

	hx = __HI(x);		/* high word of x */
	ix = hx & 0x7fffffff;			/* high word of |x| */
	if (ix < 0x3e300000) {			/* x < 2**-28 */
		if ((int) x == 0) {		/* generate inexact */
			if (((ix | __LO(x)) | (iy + 1)) == 0)
				return one / fabs(x);
			else {
				if (iy == 1)
					return x;
				else {	/* compute -1 / (x+y) carefully */
					double a, t;

					z = w = x + y;
					__setLow(&z, 0);
					v = y - (z - x);
					t = a = -one / w;
					__setLow(&t, 0);
					s = one + t * z;
					return t + a * (s + t * v);
				}
			}
		}
	}
	if (ix >= 0x3FE59428) {	/* |x| >= 0.6744 */
		if (hx < 0) {
			x = -x;
			y = -y;
		}
		z = pio4 - x;
		w = pio4lo - y;
		x = z + w;
		y = 0.0;
	}
	z = x * x;
	w = z * z;
	/*
	 * Break x^5*(T[1]+x^2*T[2]+...) into
	 * x^5(T[1]+x^4*T[3]+...+x^20*T[11]) +
	 * x^5(x^2*(T[2]+x^4*T[4]+...+x^22*[T12]))
	 */
	r = T[1] + w * (T[3] + w * (T[5] + w * (T[7] + w * (T[9] +
		w * T[11]))));
	v = z * (T[2] + w * (T[4] + w * (T[6] + w * (T[8] + w * (T[10] +
		w * T[12])))));
	s = z * x;
	r = y + z * (s * (r + v) + y);
	r += T[0] * s;
	w = x + r;
	if (ix >= 0x3FE59428) {
		v = (double) iy;
		return (double) (1 - ((hx >> 30) & 2)) *
			(v - 2.0 * (x - (w * w / (w + v) - r)));
	}
	if (iy == 1)
		return w;
	else {
		/*
		 * if allow error up to 2 ulp, simply return
		 * -1.0 / (x+r) here
		 */
		/* compute -1.0 / (x+r) accurately */
		double a, t;
		z = w;
		__setLow(&z, 0);
		v = r - (z - x);	/* z+v = r+x */
		t = a = -1.0 / w;	/* a = -1.0/w */
		__setLow(&t, 0);
		s = 1.0 + t * z;
		return t + a * (s + t * v);
	}

#undef	one
#undef	pio4
#undef	pio4lo
#undef	T
}

OVERLOADABLE double tan(double x)
{
	double y[2],z=0.0;
	int n, ix;

	/* High word of x. */
	ix = __HI(x);

	/* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if(ix <= 0x3fe921fb) return __kernel_tan(x,z,1);

	/* tan(Inf or NaN) is NaN */
	else if (ix>=0x7ff00000) return x-x;		/* NaN */

	/* argument reduction needed */
	else {
		n = __ieee754_rem_pio2(x,y);
		return __kernel_tan(y[0],y[1],1-((n&1)<<1)); /*   1 -- n even
							-1 -- n odd */
	}
}

OVERLOADABLE double tanh(double x)
{
	double one=1.0, two=2.0, tiny = 1.0e-300;
	double t,z;
	int jx,ix;

	/* High word of |x|. */
	jx = __HI(x);
	ix = jx&0x7fffffff;

	/* x is INF or NaN */
	if(ix>=0x7ff00000) {
		if (jx>=0) return one/x+one;	/* tanh(+-inf)=+-1 */
		else	   return one/x-one;	/* tanh(NaN) = NaN */
	}

	/* |x| < 22 */
	if (ix < 0x40360000) {		/* |x|<22 */
		if (ix<0x3c800000) 		/* |x|<2**-55 */
		return x*(one+x);		/* tanh(small) = small */
		if (ix>=0x3ff00000) {	/* |x|>=1  */
		t = expm1(two*fabs(x));
		z = one - two/(t+two);
		} else {
			t = expm1(-two*fabs(x));
			z= -t/(t+two);
		}
	/* |x| > 22, return +-1 */
	} else {
		z = one - tiny;		/* raised inexact flag */
	}
	return (jx>=0)? z: -z;
}

OVERLOADABLE double tanpi(double x)
{
	double y,z, signValue, infsign;
	ulong lx;

	lx = as_ulong(x);
	y = fabs(x);
	signValue = (lx&DF_SIGN_MASK) ? -1.0:1.0;
	infsign = signValue;
	if((lx&DF_ABS_MASK) >=DF_POSITIVE_INF)return as_double(DF_POSITIVE_INF + 1);

	z = floor(y);
	if(z > 0)
	{
		ulong uz = as_ulong(z);
		ulong expValue = ((uz & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
		ulong manValue = ((uz & DF_MAN_MASK) | DF_IMPLICITE_ONE);
		
		if(expValue > 52) copysign(0.0, x);
		manValue = (manValue >> (52 - expValue));
		
		if(manValue & 1)
			infsign = signValue * -1.0;
	}

	if(z!=y)
	{
		y = y -z;

		if(y > 0.5)
		{
			y = y -0.5;
			if(y < 0.25)
				return -1.0*signValue /tan(y*M_PI);

			y = 0.5 - y;
			return -1.0*signValue*tan(y*M_PI);
		}

		if((y < 0.5) && (y > 0.25))
		{
			y = 0.5 - y;
			return signValue/tan(y*M_PI);
		}

		if(y == 0.5)
		{
			if(infsign == 1.0)
				return as_double(DF_POSITIVE_INF);
			else
				return as_double(DF_NEGTIVE_INF);
		}
	}
	else
	{
		return copysign(0.0, x);
	}

	y = tan(y*M_PI);
	return copysign(y, x);
}

OVERLOADABLE double tgamma(double x)
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
	ix = hx&0x7fffffff;
	if(ix>=0x7ff00000) return x*x;
	if((ix|lx)==0) return one/zero;
	if(ix<0x3b900000) {	/* |x|<2**-70, return -log(|x|) */
		if(hx<0) {
			return -log(-x);
		} else return -log(x);
	}
	if(hx<0) {
		if(ix>=0x43300000) 	/* |x|>=2**52, must be -integer */
		return one/zero;
		t = sinpi(x);
		if(t==zero) return one/zero; /* -integer */
		nadj = log(pi/fabs(t*x));
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

	return exp(r);
}

OVERLOADABLE double trunc(double x)
{
	double ret = floor(fabs(x));
	return copysign(ret, x);
}



