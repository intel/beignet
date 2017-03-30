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


OVERLOADABLE double trunc(double x)
{
	double ret = floor(fabs(x));
	return copysign(ret, x);
}



