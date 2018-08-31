#! /bin/sh -e

echo '
/*
 * Copyright © 2012 - 2014 Intel Corporatio
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
'

if [ $1"a" = "-pa" ]; then
    echo "#ifndef __OCL_CONVERT_H__"
    echo "#define __OCL_CONVERT_H__"
    echo "#include \"ocl_types.h\""
    echo
else
    echo "#include \"ocl_convert.h\""
    echo "#include \"ocl_float.h\""
    echo "#include \"ocl_as.h\""
    echo "#include \"ocl_integer.h\""
    echo
fi

# Supported base types and their lengths
TYPES="long:8 ulong:8 int:4 uint:4 short:2 ushort:2 char:1 uchar:1 double:8 float:4 half:2"
# Supported vector lengths
VECTOR_LENGTHS="1 2 3 4 8 16"
ROUNDING_MODES="rte rtz rtp rtn"

# For all vector lengths and types, generate conversion functions
for vector_length in $VECTOR_LENGTHS; do
    if test $vector_length -eq 1; then
	for ftype in $TYPES; do
	    fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
	    for ttype in $TYPES; do
		tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
		if [ $1"a" = "-pa" ]; then
		    echo "OVERLOADABLE $tbasetype convert_$tbasetype($fbasetype v);"
		else
		    echo "OVERLOADABLE $tbasetype convert_$tbasetype($fbasetype v) {"
		    echo "  return ($tbasetype)v;"
		    echo "}"
		    echo
		fi
	    done
	done
    else
	for ftype in $TYPES; do
	    fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
	    for ttype in $TYPES; do
		tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
		if test $fbasetype = $tbasetype; then
		    if test $vector_length -gt 1; then
			fvectortype=$fbasetype$vector_length
			tvectortype=$tbasetype$vector_length
			if [ $1"a" = "-pa" ]; then
			    echo "OVERLOADABLE $tvectortype convert_$tvectortype($fvectortype v);"
			else
			    echo "OVERLOADABLE $tvectortype convert_$tvectortype($fvectortype v) { return v; }"
			fi
		    else
			if [ $1"a" = "-pa" ]; then
			    echo "OVERLOADABLE $tbasetype convert_$tbasetype($fbasetype v);"
			else
			    echo "OVERLOADABLE $tbasetype convert_$tbasetype($fbasetype v) { return v; }"
			fi
		    fi
		    continue
		fi
		fvectortype=$fbasetype$vector_length
		tvectortype=$tbasetype$vector_length
		construct="($tbasetype)(v.s0)"
		if test $vector_length -gt 1; then
		    construct="$construct, ($tbasetype)(v.s1)"
		fi
		if test $vector_length -gt 2; then
		    construct="$construct, ($tbasetype)(v.s2)"
		fi
		if test $vector_length -gt 3; then
		    construct="$construct, ($tbasetype)(v.s3)"
		fi
		if test $vector_length -gt 4; then
		    construct="$construct, ($tbasetype)(v.s4)"
		    construct="$construct, ($tbasetype)(v.s5)"
		    construct="$construct, ($tbasetype)(v.s6)"
		    construct="$construct, ($tbasetype)(v.s7)"
		fi
		if test $vector_length -gt 8; then
		    construct="$construct, ($tbasetype)(v.s8)"
		    construct="$construct, ($tbasetype)(v.s9)"
		    construct="$construct, ($tbasetype)(v.sA)"
		    construct="$construct, ($tbasetype)(v.sB)"
		    construct="$construct, ($tbasetype)(v.sC)"
		    construct="$construct, ($tbasetype)(v.sD)"
		    construct="$construct, ($tbasetype)(v.sE)"
		    construct="$construct, ($tbasetype)(v.sF)"
		fi

		if [ $1"a" = "-pa" ]; then
		    echo "OVERLOADABLE $tvectortype convert_$tvectortype($fvectortype v);"
		else
		    echo "OVERLOADABLE $tvectortype convert_$tvectortype($fvectortype v) {"
		    echo "  return ($tvectortype)($construct);"
		    echo "}"
		    echo
		fi
	    done
	done
    fi
done

echo '
/* The sat cvt supported by HW. */
#define DEF(DSTTYPE, SRCTYPE) \
OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x);
DEF(char, uchar);
DEF(char, short);
DEF(char, ushort);
DEF(char, int);
DEF(char, uint);
DEF(char, float);
DEF(uchar, char);
DEF(uchar, short);
DEF(uchar, ushort);
DEF(uchar, int);
DEF(uchar, uint);
DEF(uchar, float);
DEF(short, ushort);
DEF(short, int);
DEF(short, uint);
DEF(short, float);
DEF(short, double);
DEF(ushort, short);
DEF(ushort, int);
DEF(ushort, uint);
DEF(ushort, float);
DEF(ushort, double);
DEF(int, uint);
DEF(int, float);
DEF(int, double);
DEF(uint, int);
DEF(uint, float);
DEF(uint, double);
DEF(char, half);
DEF(uchar, half);
DEF(short, half);
DEF(ushort, half);
DEF(int, half);
DEF(uint, half);
#undef DEF
'

if [ $1"a" = "-pa" ]; then
    echo "#define DEF(DSTTYPE, SRCTYPE, MIN, MAX)  OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x);"
else
    echo '
#define DEF(DSTTYPE, SRCTYPE, MIN, MAX) \
OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x) { \
  x = x >= MAX ? MAX : x; \
  return x <= MIN ? (DSTTYPE)MIN : (DSTTYPE)x; \
}
'
fi

echo '
DEF(char, long, -128, 127);
DEF(uchar, long, 0, 255);
DEF(short, long, -32768, 32767);
DEF(ushort, long, 0, 65535);
DEF(int, long, -0x7fffffff-1, 0x7fffffff);
DEF(uint, long, 0, 0xffffffffu);
#undef DEF
'

if [ $1"a" = "-pa" ]; then
    echo "
#define DEF(DSTTYPE, SRCTYPE, SRC_MIN, SRC_MAX, DST_MIN, DST_MAX) \
OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x);"
else
    echo '
//convert float to long/ulong must take care of overflow, if overflow the value is undef.
#define DEF(DSTTYPE, SRCTYPE, SRC_MIN, SRC_MAX, DST_MIN, DST_MAX) \
OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x) { \
  DSTTYPE y = x >= SRC_MAX ? DST_MAX : (DSTTYPE)x; \
  return x <= SRC_MIN ? DST_MIN : y; \
}
'
fi

echo '
DEF(long, float, -0x1.0p63, 0x1.0p63, 0x8000000000000000, 0x7fffffffffffffff);
DEF(ulong, float, 0, 0x1.0p64, 0, 0xffffffffffffffff);
DEF(char, double, -0x1.0p7, 0x1.0p7,  0x80, 0x7F);
DEF(uchar, double, 0, 0x1.0p8, 0, 0xFF);
DEF(long, double, -0x1.0p63, 0x1.0p63, 0x8000000000000000, 0x7FFFFFFFFFFFFFFF);
DEF(ulong, double, 0, 0x1.0p64, 0, 0xFFFFFFFFFFFFFFFF);
#undef DEF
'

if [ $1"a" = "-pa" ]; then
    echo "#define DEF(DSTTYPE, SRCTYPE, MAX) OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x);"
else
    echo '
#define DEF(DSTTYPE, SRCTYPE, MAX) \
OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x) { \
  return x >= MAX ? (DSTTYPE)MAX : x; \
}
'
fi

echo '
DEF(char, ulong, 127);
DEF(uchar, ulong, 255);
DEF(short, ulong, 32767);
DEF(ushort, ulong, 65535);
DEF(int, ulong, 0x7fffffff);
DEF(uint, ulong, 0xffffffffu);
#undef DEF
'

if [ $1"a" = "-pa" ]; then
    echo  "OVERLOADABLE long convert_long_sat(ulong x);"
else
    echo '
OVERLOADABLE long convert_long_sat(ulong x) {
  ulong MAX = 0x7ffffffffffffffful;
  return x >= MAX ? MAX : x;
}
'
fi

if [ $1"a" = "-pa" ]; then
    echo "#define DEF(DSTTYPE, SRCTYPE) OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x);"
else
    echo '
#define DEF(DSTTYPE, SRCTYPE) \
  OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x) { \
  return x <= 0 ? 0 : x; \
}
'
fi

echo '
  DEF(ushort, char);
  DEF(uint, char);
  DEF(uint, short);
  DEF(ulong, char);
  DEF(ulong, short);
  DEF(ulong, int);
  DEF(ulong, long);
  #undef DEF
'

if [ $1"a" = "-pa" ]; then
    echo "#define DEF(DSTTYPE, SRCTYPE) OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x);"
else
    echo '
#define DEF(DSTTYPE, SRCTYPE) \
  OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x) { \
  return x; \
}
'
fi

echo '
DEF(char, char);
DEF(uchar, uchar);
DEF(short, char);
DEF(short, uchar);
DEF(short, short);
DEF(ushort, uchar);
DEF(ushort, ushort);
DEF(int, char);
DEF(int, uchar);
DEF(int, short);
DEF(int, ushort);
DEF(int, int);
DEF(uint, uchar);
DEF(uint, ushort);
DEF(uint, uint);
DEF(long, char);
DEF(long, uchar);
DEF(long, short);
DEF(long, ushort);
DEF(long, int);
DEF(long, uint);
DEF(long, long);
DEF(ulong, uchar);
DEF(ulong, ushort);
DEF(ulong, uint);
DEF(ulong, ulong);
#undef DEF
'

# for half to long
if [ $1"a" = "-pa" ]; then
    echo '
       OVERLOADABLE long convert_long_sat(half x);
       OVERLOADABLE ulong convert_ulong_sat(half x);
       '
else
    echo '
union _type_half_and_ushort {
  half hf;
  ushort us;
};
OVERLOADABLE long convert_long_sat(half x) {
  union _type_half_and_ushort u;
  u.hf = x;
  if (u.us == 0x7C00) // +inf
    return 0x7FFFFFFFFFFFFFFF;
  if (u.us == 0xFC00) // -inf
    return 0x8000000000000000;

  return (long)x;
}
OVERLOADABLE ulong convert_ulong_sat(half x) {
  union _type_half_and_ushort u;
  u.hf = x;
  if (u.us == 0x7C00) // +inf
    return 0xFFFFFFFFFFFFFFFF;

  if (x < (half)0.0) {
    return 0;
  }
  return (ulong)x;
}'
fi


# vector convert_DSTTYPE_sat function
for vector_length in $VECTOR_LENGTHS; do
    if test $vector_length -eq 1; then continue; fi

    for ftype in $TYPES; do
	fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
	#if test $fbasetype = "double"; then continue; fi

	for ttype in $TYPES; do
	    tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
	    if test $tbasetype = "double" -o $tbasetype = "float" -o $tbasetype = "half" ; then continue; fi

	    fvectortype=$fbasetype$vector_length
	    tvectortype=$tbasetype$vector_length
	    conv="convert_${tbasetype}_sat"

	    construct="$conv(v.s0)"
	    if test $vector_length -gt 1; then
		construct="$construct, $conv(v.s1)"
	    fi
	    if test $vector_length -gt 2; then
		construct="$construct, $conv(v.s2)"
	    fi
	    if test $vector_length -gt 3; then
		construct="$construct, $conv(v.s3)"
	    fi
	    if test $vector_length -gt 4; then
		construct="$construct, $conv(v.s4)"
		construct="$construct, $conv(v.s5)"
		construct="$construct, $conv(v.s6)"
		construct="$construct, $conv(v.s7)"
	    fi
	    if test $vector_length -gt 8; then
		construct="$construct, $conv(v.s8)"
		construct="$construct, $conv(v.s9)"
		construct="$construct, $conv(v.sA)"
		construct="$construct, $conv(v.sB)"
		construct="$construct, $conv(v.sC)"
		construct="$construct, $conv(v.sD)"
		construct="$construct, $conv(v.sE)"
		construct="$construct, $conv(v.sF)"
	    fi

	    if [ $1"a" = "-pa" ]; then
		echo "OVERLOADABLE $tvectortype convert_${tvectortype}_sat($fvectortype v);"
	    else
		echo "OVERLOADABLE $tvectortype convert_${tvectortype}_sat($fvectortype v) {"
		echo "  return ($tvectortype)($construct);"
		echo "}"
		echo
	    fi
	done
    done
done

if [ $1"a" != "-pa" ]; then
echo '
CONST float __gen_ocl_rndz(float x) __asm("llvm.trunc" ".f32");
CONST float __gen_ocl_rnde(float x) __asm("llvm.rint" ".f32");
CONST float __gen_ocl_rndu(float x) __asm("llvm.ceil" ".f32");
CONST float __gen_ocl_rndd(float x) __asm("llvm.floor" ".f32");
OVERLOADABLE float __convert_float_rtz(long x)
{
  union {
    uint u;
    float f;
  } u;
  u.f = x;
  long l = u.f;
  if((l > x && x > 0) || x >= 0x7fffffc000000000 ||
    (l < x && x < 0)) {
    u.u -= 1;
  }
  return u.f;
}
OVERLOADABLE float __convert_float_rtp(long x)
{
  union {
    uint u;
    float f;
  } u;
  u.f = x;
  long l = u.f;  //can not use u.f < x
  if(l < x && x < 0x7fffffc000000000) {
    if(x > 0)
      u.u = u.u + 1;
    else
      u.u = u.u - 1;
  }
  return u.f;
}
OVERLOADABLE float __convert_float_rtn(long x)
{
  union {
    uint u;
    float f;
  } u;
  u.f = x;
  long l = u.f;  //avoid overflow
  if(l > x || x >= 0x7fffffc000000000) {
    if(x > 0)
      u.u = u.u - 1;
    else
      u.u = u.u + 1;
  }
  return u.f;
}
OVERLOADABLE float __convert_float_rtz(ulong x)
{
  union {
    uint u;
    float f;
  } u;
  u.f = x;
  ulong l = u.f;
  if(l > x  || x >= 0xffffff8000000000)
    u.u -= 1;
  return u.f;
}
OVERLOADABLE float __convert_float_rtp(ulong x)
{
  union {
    uint u;
    float f;
  } u;
  u.f = x;
  ulong l = u.f;  //can not use u.f < x
  if(l < x && x < 0xffffff8000000000)
    u.u = u.u + 1;
  return u.f;
}
OVERLOADABLE float __convert_float_rtn(ulong x)
{
  return __convert_float_rtz(x);
}
OVERLOADABLE float __convert_float_rtz(int x)
{
  union {
    uint u;
    float f;
  } u;
  u.f = x;
  long i = u.f;
  if((i > x && x > 0) ||
    (i < x && x < 0)) {
    u.u -= 1;
  }
  return u.f;
}
OVERLOADABLE float __convert_float_rtp(int x)
{
  union {
    uint u;
    float f;
  } u;
  u.f = x;
  int i = u.f;
  if(i < x) {
    if(x > 0)
      u.u += 1;
    else
      u.u -= 1;
  }
  return u.f;
}
OVERLOADABLE float __convert_float_rtn(int x)
{
  union {
    uint u;
    float f;
  } u;
  u.f = x;
  long i = u.f;  //avoid overflow
  if(i > x) {
    if(x > 0)
      u.u = u.u - 1;
    else
      u.u = u.u + 1;
  }
  return u.f;
}
OVERLOADABLE float __convert_float_rtz(uint x)
{
  union {
    uint u;
    float f;
  } u;
  u.f = x;
  ulong i = u.f;
  if(i > x)
    u.u -= 1;
  return u.f;
}
OVERLOADABLE float __convert_float_rtp(uint x)
{
  union {
    uint u;
    float f;
  } u;
  u.f = x;
  uint i = u.f;
  if(i < x)
    u.u += 1;
  return u.f;
}
OVERLOADABLE float __convert_float_rtn(uint x)
{
    return __convert_float_rtz(x);
}

OVERLOADABLE char convert_char_rtn(double x)
{
	return (char)convert_int_rtn(x);
}

OVERLOADABLE uchar convert_uchar_rtn(double x)
{
	return (uchar)convert_uint_rtn(x);
}

OVERLOADABLE short convert_short_rtn(double x)
{
	return (short)convert_int_rtn(x);
}

OVERLOADABLE ushort convert_ushort_rtn(double x)
{
	return (ushort)convert_uint_rtn(x);
}

OVERLOADABLE int convert_int_rtn(double x)
{
	int ret, iexp ;
	long lval = as_long(x);
	if((lval & DF_ABS_MASK) == 0) return 0;

	uint sign = (lval & DF_SIGN_MASK) >> DF_SIGN_OFFSET;
	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	ulong ma = (lval &DF_MAN_MASK);

	long intPart = (ma |DF_IMPLICITE_ONE)>> (DF_EXP_OFFSET -exp);
	ret = (int)intPart;
	iexp = (exp < 0) ? 0:exp;
	ret = (exp < 0) ? 0:ret;
	ret = sign ? -ret:ret;
	long mask = (1L << (DF_EXP_OFFSET -iexp)) - 1;
	ret = ((ma & mask) || (exp < 0)) ? ret -sign:ret;

	return ret;
}

OVERLOADABLE uint convert_uint_rtn(double x)
{
	uint ret, iexp ;
	long lval = as_long(x);
	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	long ma = (lval &DF_MAN_MASK);

	long intPart = (ma |DF_IMPLICITE_ONE)>> (DF_EXP_OFFSET -exp);
	ret = (int)intPart;
	ret = (exp < 0) ? 0:ret;

	return ret;
}

OVERLOADABLE long convert_long_rtn(double x)
{
	int iexp;
	long ret;
	ulong lval = as_ulong(x);
	uint sign = (lval & DF_SIGN_MASK) >> DF_SIGN_OFFSET;
	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	ulong ma = (lval &DF_MAN_MASK);

	ulong ldata = ma |DF_IMPLICITE_ONE;
	uint shift = abs(exp -DF_EXP_OFFSET);
	ulong ldataL = ldata << shift;
	ulong ldataR = ldata >> shift;
	ret    = (exp > DF_EXP_OFFSET) ? ldataL:ldataR;
	ret    = (exp >= 0) ? ret:0;
	iexp = (exp >= 0) ? exp:0;

	ret = sign ? -ret:ret;
	shift = convert_uint_sat(DF_EXP_OFFSET - iexp);
	long mask = (1L << shift) - 1;
	ret = ((ma & mask) || (exp < 0)) ? ret -sign:ret;
	ret = (lval & DF_ABS_MASK) ? ret:0;

	return ret;
}

OVERLOADABLE ulong convert_ulong_rtn(double x)
{
	int iexp;
	long ret;
	long lval = as_long(x);
	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	long ma = (lval &DF_MAN_MASK);

	ulong ldata = ma |DF_IMPLICITE_ONE;
	int shift = abs(exp -DF_EXP_OFFSET);
	ulong ldataL = ldata << shift;
	ulong ldataR = ldata >> shift;
	ret    = (exp > DF_EXP_OFFSET) ? ldataL:ldataR;
	ret    = (exp >= 0) ? ret:0;
	iexp = (exp >= 0) ? exp:0;
	ret    = (lval & DF_ABS_MASK) ? ret:0;

	return ret;
}

OVERLOADABLE float  convert_float_rtn(double x)
{
	int ret, tmp;
	long lval = as_long(x);
	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	int sign = (lval & DF_SIGN_MASK)?1:0;
	long ma = (lval &DF_MAN_MASK);

	int fval = ((exp + 127) << 23) |convert_int(ma >> 29);
	if((ma & 0x1FFFFFFF) && sign) fval += 1;
	ret = fval;

	ma |= DF_IMPLICITE_ONE;
	tmp = ma >> (-exp - 97);
	long shift = (1UL << (-exp - 97)) -1UL;
	if((ma & shift) && sign) tmp += 1;
	ret = (exp < -126) ? tmp:ret;

	tmp = sign;
	ret = (exp < -149) ? tmp:ret;

	tmp = sign ? 0x7F800000:0x7F7FFFFF;
	ret = (exp > 127) ? tmp:ret;

	tmp = (lval & DF_MAN_MASK) ? 0x7FFFFFFF:0x7F800000;
	ret = (exp == 1024) ? tmp:ret;

	tmp = sign ? (0xFF800000) : (0x7F7FFFFF);
	ret = ((exp == 1023) && (ma == DF_MAN_MASK)) ? tmp:ret;

	ret = ((lval & DF_ABS_MASK) == 0) ? 0:ret;

	ret |= (sign << 31);
	float ftemp = as_float(ret);
	return ftemp;
}

OVERLOADABLE float  convert_float_rtz(double x)
{
	int ret, tmp;
	long lval = as_long(x);
	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	int sign = (lval & DF_SIGN_MASK)?1:0;
	long ma = (lval &DF_MAN_MASK);

	int fval = ((exp + 127) << 23) |convert_int(ma >> 29);
	ret = fval;

	ma |= DF_IMPLICITE_ONE;
	tmp = ma >> (-exp - 97);
	ret = (exp < -126) ? tmp:ret;
	ret = (exp < -149) ? 0:ret;

	tmp = 0x7F7FFFFF;
	ret = (exp > 127) ? tmp:ret;

	tmp = (lval &DF_MAN_MASK) ? 0x7FFFFFFF:0x7F800000;
	ret = (exp == 1024) ? tmp:ret;

	ret = ((lval & DF_ABS_MASK) == 0) ? 0:ret;

	ret |= (sign << 31);
	float ftemp = as_float(ret);
	return ftemp;
}

OVERLOADABLE float  convert_float_rtp(double x)
{
	int ret, tmp;
	long lval = as_long(x);
	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	int sign = (lval & DF_SIGN_MASK)?1:0;
	long ma = (lval &DF_MAN_MASK);

	int fval = ((exp + 127) << 23) |convert_int(ma >> 29);
	if((ma & 0x1FFFFFFF) && !sign) fval += 1;
	ret = fval;

	ma |= DF_IMPLICITE_ONE;
	tmp = ma >> (-exp - 97);
	long shift = (1UL << (-exp - 97)) -1UL;
	if((ma & shift) && !sign) tmp += 1;
	ret = (exp < -126) ? tmp:ret;

	tmp = 0 ;
	if(!sign) tmp |= 1;
	ret = (exp < -149) ? tmp:ret;

	tmp = 0x7F7FFFFF;
	if(!sign) tmp = 0x7F800000;
	ret = (exp > 127) ? tmp:ret;

	tmp = (lval & DF_MAN_MASK) ? 0x7FFFFFFF:0x7F800000;
	ret = (exp == 1024) ? tmp:ret;

	ret = ((lval & DF_ABS_MASK) == 0) ? 0:ret;

	ret |= (sign << 31);
	float ftemp = as_float(ret);
	return ftemp;
}

OVERLOADABLE float  convert_float_rte(double x)
{
	int ret, tmp;
	long lval = as_long(x);
	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	int sign = (lval & DF_SIGN_MASK)?1:0;
	long ma = (lval &DF_MAN_MASK);

	int fval = ((exp + 127) << 23) |convert_int(ma >> 29);
	int lastBit = (ma & 0x20000000);
	int roundBits32 = (ma & 0x1FFFFFFF);
	if(roundBits32 > 0x10000000) fval += 1;
	else if((roundBits32 == 0x10000000) && lastBit) fval += 1;
	ret = fval;

	ma |= DF_IMPLICITE_ONE;
	int rshift = -exp -97;
	tmp = ma >> rshift;
	long shift = (1UL << rshift) -1UL;
	long roundBits = (ma & shift);
	lastBit = tmp & 0x1;
	if(roundBits > (1L << (rshift - 1))) tmp += 1;
	if(roundBits == (1L << (rshift - 1)) && lastBit) tmp += 1;
	ret = (exp < -126) ? tmp:ret;
	ret = (exp < -149) ? 0:ret;

	tmp = 0x7F800000;
	if(!sign) tmp = 0x7F800000;
	ret = (exp > 127) ? tmp:ret;

	tmp = (lval & DF_MAN_MASK) ? 0x7FFFFFFF:0x7F800000;
	ret = (exp == 1024) ? tmp:ret;

	tmp = 0x7F800000;
	ret = ((exp == 1023) && (ma == DF_MAN_MASK)) ? tmp:ret;

	ret = ((lval & DF_ABS_MASK) == 0) ? 0:ret;

	ret |= (sign << 31);
	float ftemp = as_float(ret);
	return ftemp;
}

OVERLOADABLE long convert_long_rte(double x)
{
	long lval = as_long(x);
	long ret = convert_ulong_rte(x);
	ret = (lval & DF_SIGN_MASK) ? -ret:ret;

	return ret;
}

OVERLOADABLE ulong convert_ulong_rte(double x)
{
	ulong ret;
	long lval = as_long(x);
	if((lval & DF_ABS_MASK) == 0) return 0;

	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	long ma = (lval & DF_MAN_MASK);
	uint shift = abs(exp -DF_EXP_OFFSET);
	long absVal = (ma |DF_IMPLICITE_ONE);
	ret = absVal << shift;
	long tmp = absVal >> shift;
	int lastBit = tmp & 0x1;
	long mask = (1L << shift) - 1;
	long roundBit = (1L << (shift -1));
	long diff = (mask & absVal) - roundBit;
	tmp = (diff == 0) ? tmp + lastBit:tmp;
	tmp = (diff > 0) ? tmp+1:tmp;

	ret = (exp < 52) ? tmp:ret;
	ret = (exp < -1) ? 0:ret;

	return ret;
}

OVERLOADABLE ulong convert_ulong_rtp(double x)
{
	int iexp ;
	ulong ret = 1;
	long lval = as_long(x);

	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	long ma = (lval & DF_MAN_MASK);
	int shift = abs(exp - 52);
	ret = (ma |DF_IMPLICITE_ONE)<< shift;
	ulong tmp = (ma |DF_IMPLICITE_ONE)>> shift;
	long mask = (1L << shift) - 1;
	tmp = (mask & ma) ? tmp+1:tmp;

	ret = (exp > 52) ? ret:tmp;
	ret = (exp < 0) ? 1:ret;
	ret = (lval & DF_SIGN_MASK) ? 0:ret;
	ret = (lval & DF_ABS_MASK) ? ret:0;

	return ret;
}

OVERLOADABLE long convert_long_rtp(double x)
{
	int iexp ;
	ulong ret;
	long lval = as_long(x);

	int exp = ((lval & DF_EXP_MASK) >> DF_EXP_OFFSET) - DF_EXP_BIAS;
	long ma = (lval & DF_MAN_MASK);
	int shift = abs(exp - 52);
	ret = (ma |DF_IMPLICITE_ONE)<< shift;
	ulong tmp = (ma |DF_IMPLICITE_ONE)>> shift;
	long mask = (1L << shift) - 1;
	int sign = ((lval & DF_SIGN_MASK) >> DF_SIGN_OFFSET);
	tmp = ((mask & ma) && !sign) ? tmp+1:tmp;

	ret = (exp > 52) ? ret:tmp;
	ret = (exp < 0) ? 1 -sign:ret;
	ret = (lval & DF_ABS_MASK) ? ret:0;
	ret = sign ? -ret:ret;

	return ret;
}
'
fi

# convert_u|char|short|int_rte(double x)
ITYPES="int:4 uint:4 short:2 ushort:2 char:1 uchar:1"
for ttype in $ITYPES; do
	tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
	if [ $1"a" != "-pa" ]; then
	echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rte(double x)"
	echo "{ return ($tbasetype)convert_long_rte(x);}"
	fi
done

# convert_u|char|short|int_rte(double x)
IUTYPES="ulong:8 uint:4 ushort:2 uchar:1"
for ttype in $IUTYPES; do
	tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
	if [ $1"a" != "-pa" ]; then
	echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rtz(double x)"
	echo "{ double lx = (x < 0) ? 0:x;"
	echo " return convert_${tbasetype}_rtn(lx);}"
	fi
done

ITYPES="long:8 int:4 short:2 char:1"
for ttype in $ITYPES; do
	tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
	if [ $1"a" != "-pa" ]; then
	echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rtz(double x)"
	echo "{ double lx = (x < 0) ? -x:x;"
	echo " $tbasetype tmp = convert_u${tbasetype}_rtn(lx);"
	echo "return (x < 0) ? -tmp:tmp;}"
	fi
done

# convert_u|char|short|int_rtp(double x)
IUTYPES="uint:4 ushort:2 uchar:1"
for ttype in $IUTYPES; do
	tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
	if [ $1"a" != "-pa" ]; then
	echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rtp(double x)"
	echo "{ return ($tbasetype)convert_ulong_rtp(x);}"
	fi
done

ITYPES="int:4 short:2 char:1"
for ttype in $ITYPES; do
	tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
	if [ $1"a" != "-pa" ]; then
	echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rtp(double x)"
	echo " { return ($tbasetype)convert_long_rtp(x);}"
	fi
done

# convert_DSTTYPE_ROUNDING function
for vector_length in $VECTOR_LENGTHS; do
    for ftype in $TYPES; do
	fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
	#if test $fbasetype = "double"; then continue; fi

	for ttype in $TYPES; do
	    tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
	    if test $tbasetype = "double"; then continue; fi

	    if test $vector_length -eq 1; then
		if [ $1"a" = "-pa" ]; then
		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rte($fbasetype x);"
		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rtz($fbasetype x);"
		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rtp($fbasetype x);"
		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rtn($fbasetype x);"
		else
		    if [ "$fbasetype" = "double" ]; then
		        continue;
		    fi

		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rte($fbasetype x)"
		    if test $fbasetype = "float" -a $tbasetype != "float"; then
			echo "{ return __gen_ocl_rnde(x); }"
		    else
			echo "{ return x; }"
		    fi

		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rtz($fbasetype x)"
		    if test $fbasetype = "float" -a $tbasetype != "float"; then
			echo "{ return __gen_ocl_rndz(x); }"
		    elif [ "$fbasetype" = "int" -o "$fbasetype" = "uint" -o "$fbasetype" = "long" -o "$fbasetype" = "ulong" ] && [ "$tbasetype" = "float" ]; then
			echo "{ return __convert_${tbasetype}_rtz(x); }"
		    else
			echo "{ return x; }"
		    fi

		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rtp($fbasetype x)"
		    if test $fbasetype = "float" -a $tbasetype != "float"; then
			echo "{ return __gen_ocl_rndu(x); }"
		    elif [ "$fbasetype" = "int" -o "$fbasetype" = "uint" -o "$fbasetype" = "long" -o "$fbasetype" = "ulong" ] && [ "$tbasetype" = "float" ]; then
			echo "{ return __convert_${tbasetype}_rtp(x); }"
		    else
			echo "{ return x; }"
		    fi

		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_rtn($fbasetype x)"
		    if test $fbasetype = "float" -a $tbasetype != "float"; then
			echo "{ return __gen_ocl_rndd(x); }"
		    elif [ "$fbasetype" = "int" -o "$fbasetype" = "uint" -o "$fbasetype" = "long" -o "$fbasetype" = "ulong" ] && [ "$tbasetype" = "float" ]; then
			echo "{ return __convert_${tbasetype}_rtn(x); }"
		    else
			echo "{ return x; }"
		    fi
		fi

		continue
	    fi

	    for rounding in $ROUNDING_MODES; do
		fvectortype=$fbasetype$vector_length
		tvectortype=$tbasetype$vector_length
		conv="convert_${tbasetype}_${rounding}"

		construct="$conv(v.s0)"
		if test $vector_length -gt 1; then
		    construct="$construct, $conv(v.s1)"
		fi
		if test $vector_length -gt 2; then
		    construct="$construct, $conv(v.s2)"
		fi
		if test $vector_length -gt 3; then
		    construct="$construct, $conv(v.s3)"
		fi
		if test $vector_length -gt 4; then
		    construct="$construct, $conv(v.s4)"
		    construct="$construct, $conv(v.s5)"
		    construct="$construct, $conv(v.s6)"
		    construct="$construct, $conv(v.s7)"
		fi
		if test $vector_length -gt 8; then
		    construct="$construct, $conv(v.s8)"
		    construct="$construct, $conv(v.s9)"
		    construct="$construct, $conv(v.sA)"
		    construct="$construct, $conv(v.sB)"
		    construct="$construct, $conv(v.sC)"
		    construct="$construct, $conv(v.sD)"
		    construct="$construct, $conv(v.sE)"
		    construct="$construct, $conv(v.sF)"
		fi

		if [ $1"a" = "-pa" ]; then
		    echo "OVERLOADABLE $tvectortype convert_${tvectortype}_${rounding}($fvectortype v);"
		else
		    echo "OVERLOADABLE $tvectortype convert_${tvectortype}_${rounding}($fvectortype v) {"
		    echo "  return ($tvectortype)($construct);"
		    echo "}"
		    echo
		fi
	    done
	done
    done
done

if [ $1"a" = "-pa" ]; then
        echo "#define DEF(DSTTYPE, SRCTYPE, SRC_MIN, SRC_MAX, DST_MIN, DST_MAX) \
            OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat_rte(SRCTYPE x); \
            OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat_rtz(SRCTYPE x); \
            OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat_rtn(SRCTYPE x); \
            OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat_rtp(SRCTYPE x);"
else
        echo '
            #define DEF(DSTTYPE, SRCTYPE, SRC_MIN, SRC_MAX, DST_MIN, DST_MAX) \
            OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat_rte(SRCTYPE x) { \
              DSTTYPE y = convert_##DSTTYPE##_rte(x); \
              y = (x >= SRC_MAX) ? DST_MAX : y; \
              y = (x <= SRC_MIN) ? DST_MIN : y; \
              return y; \
            } \
            OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat_rtz(SRCTYPE x) { \
              DSTTYPE y = convert_##DSTTYPE##_rtz(x); \
              y = (x >= SRC_MAX) ? DST_MAX : y; \
              y = (x <= SRC_MIN) ? DST_MIN : y; \
              return y; \
            } \
            OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat_rtn(SRCTYPE x) { \
              DSTTYPE y = convert_##DSTTYPE##_rtn(x); \
              y = (x >= SRC_MAX) ? DST_MAX : y; \
              y = (x <= SRC_MIN) ? DST_MIN : y; \
              return y; \
            } \
            OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat_rtp(SRCTYPE x) { \
              DSTTYPE y = convert_##DSTTYPE##_rtp(x); \
              y = (x >= SRC_MAX) ? DST_MAX : y; \
              y = (x <= SRC_MIN) ? DST_MIN : y; \
              return y; \
            }'
fi

echo '
    DEF(char, double, -0x1.0p7, 0x1.0p7-1,  0x80, 0x7F);
    DEF(uchar, double, 0, 0x1.0p8-1, 0, 0xFF);
    DEF(short, double, -0x1.0p15, 0x1.0p15-1, 0x8000, 0x7FFF);
    DEF(ushort, double, 0, 0x1.0p16-1, 0, 0xFFFF);
    DEF(int, double, -0x1.0p31, 0x1.0p31-1, 0x80000000, 0x7FFFFFFF);
    DEF(uint, double, 0, 0x1.0p32-1, 0, 0xFFFFFFFF);
    DEF(long, double, -0x1.0p63, 0x1.0p63, 0x8000000000000000, 0x7FFFFFFFFFFFFFFF);
    DEF(ulong, double, 0, 0x1.0p64, 0, 0xFFFFFFFFFFFFFFFF);
    #undef DEF
    '

# convert_DSTTYPE_sat_ROUNDING function
for vector_length in $VECTOR_LENGTHS; do
    for ftype in $TYPES; do
	fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
	#if test $fbasetype = "double"; then continue; fi

	for ttype in $TYPES; do
	    tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
	    if test $tbasetype = "double" -o $tbasetype = "float" -o $tbasetype = "half" ; then continue; fi

	    if test $vector_length -eq 1; then
	        if test $fbasetype = "double"; then continue; fi
		if [ $1"a" = "-pa" ]; then
		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rte($fbasetype x);"
		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rtz($fbasetype x);"
		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rtp($fbasetype x);"
		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rtn($fbasetype x);"
		else
		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rte($fbasetype x)"
		    if test $fbasetype = "float"; then
			echo "{ return convert_${tbasetype}_sat(__gen_ocl_rnde(x)); }"
		    else
			echo "{ return convert_${tbasetype}_sat(x); }"
		    fi

		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rtz($fbasetype x)"
		    if test $fbasetype = "float"; then
			echo "{ return convert_${tbasetype}_sat(__gen_ocl_rndz(x)); }"
		    else
			echo "{ return convert_${tbasetype}_sat(x); }"
		    fi

		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rtp($fbasetype x)"
		    if test $fbasetype = "float"; then
			echo "{ return convert_${tbasetype}_sat(__gen_ocl_rndu(x)); }"
		    else
			echo "{ return convert_${tbasetype}_sat(x); }"
		    fi

		    echo "OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rtn($fbasetype x)"
		    if test $fbasetype = "float"; then
			echo "{ return convert_${tbasetype}_sat(__gen_ocl_rndd(x)); }"
		    else
			echo "{ return convert_${tbasetype}_sat(x); }"
		    fi
		fi
		continue
	    fi

	    for rounding in $ROUNDING_MODES; do
		fvectortype=$fbasetype$vector_length
		tvectortype=$tbasetype$vector_length
		conv="convert_${tbasetype}_sat_${rounding}"

		construct="$conv(v.s0)"
		if test $vector_length -gt 1; then
		    construct="$construct, $conv(v.s1)"
		fi
		if test $vector_length -gt 2; then
		    construct="$construct, $conv(v.s2)"
		fi
		if test $vector_length -gt 3; then
		    construct="$construct, $conv(v.s3)"
		fi
		if test $vector_length -gt 4; then
		    construct="$construct, $conv(v.s4)"
		    construct="$construct, $conv(v.s5)"
		    construct="$construct, $conv(v.s6)"
		    construct="$construct, $conv(v.s7)"
		fi
		if test $vector_length -gt 8; then
		    construct="$construct, $conv(v.s8)"
		    construct="$construct, $conv(v.s9)"
		    construct="$construct, $conv(v.sA)"
		    construct="$construct, $conv(v.sB)"
		    construct="$construct, $conv(v.sC)"
		    construct="$construct, $conv(v.sD)"
		    construct="$construct, $conv(v.sE)"
		    construct="$construct, $conv(v.sF)"
		fi

		if [ $1"a" = "-pa" ]; then
		    echo "OVERLOADABLE $tvectortype convert_${tvectortype}_sat_${rounding}($fvectortype v);"
		else
		    echo "OVERLOADABLE $tvectortype convert_${tvectortype}_sat_${rounding}($fvectortype v) {"
		    echo "  return ($tvectortype)($construct);"
		    echo "}"
		    echo
		fi
	    done
	done
    done
done

# convert_double_roundingmode( double, float ,int32, int16 ,int8)
TTYPES=" double:8 float:4 int:4 uint:4 short:2 ushort:2 char:1 uchar:1"
for vector_length in $VECTOR_LENGTHS; do
    for ftype in $TTYPES; do
	fbasetype=`IFS=:; set -- dummy $ftype; echo $2`

	    if test $vector_length -eq 1; then
		if [ $1"a" = "-pa" ]; then
		    echo "OVERLOADABLE double convert_double_rte($fbasetype x);"
		    echo "OVERLOADABLE double convert_double_rtz($fbasetype x);"
		    echo "OVERLOADABLE double convert_double_rtp($fbasetype x);"
		    echo "OVERLOADABLE double convert_double_rtn($fbasetype x);"
		else
		    echo "OVERLOADABLE double convert_double_rte($fbasetype x)"
			echo "{ return convert_double(x); }"

		    echo "OVERLOADABLE double convert_double_rtz($fbasetype x)"
			echo "{ return convert_double(x); }"

		    echo "OVERLOADABLE double convert_double_rtp($fbasetype x)"
			echo "{ return convert_double(x); }"

		    echo "OVERLOADABLE double convert_double_rtn($fbasetype x)"
			echo "{ return convert_double(x); }"
		fi
		continue
	    fi

	    for rounding in $ROUNDING_MODES; do
		fvectortype=$fbasetype$vector_length
		tvectortype=double$vector_length
		conv="convert_double_${rounding}"

		construct="$conv(v.s0)"
		if test $vector_length -gt 1; then
		    construct="$construct, $conv(v.s1)"
		fi
		if test $vector_length -gt 2; then
		    construct="$construct, $conv(v.s2)"
		fi
		if test $vector_length -gt 3; then
		    construct="$construct, $conv(v.s3)"
		fi
		if test $vector_length -gt 4; then
		    construct="$construct, $conv(v.s4)"
		    construct="$construct, $conv(v.s5)"
		    construct="$construct, $conv(v.s6)"
		    construct="$construct, $conv(v.s7)"
		fi
		if test $vector_length -gt 8; then
		    construct="$construct, $conv(v.s8)"
		    construct="$construct, $conv(v.s9)"
		    construct="$construct, $conv(v.sA)"
		    construct="$construct, $conv(v.sB)"
		    construct="$construct, $conv(v.sC)"
		    construct="$construct, $conv(v.sD)"
		    construct="$construct, $conv(v.sE)"
		    construct="$construct, $conv(v.sF)"
		fi

		if [ $1"a" = "-pa" ]; then
		    echo "OVERLOADABLE $tvectortype convert_${tvectortype}_${rounding}($fvectortype v);"
		else
		    echo "OVERLOADABLE $tvectortype convert_${tvectortype}_${rounding}($fvectortype v) {"
		    echo "  return ($tvectortype)($construct);"
		    echo "}"
		    echo
		fi
	done
    done
done

if [ $1"a" != "-pa" ]; then
echo '
OVERLOADABLE double convert_double_rtp(ulong x)
{
	long exp;
	long ret, ma, tmp;
	int msbOne = 64 -clz(x);

	exp = msbOne + DF_EXP_BIAS - 1;
	ret = (exp << 52);
	int shift = abs(53 - msbOne);
	tmp = ret | ((x << shift) &DF_MAN_MASK);

	ma = (x & ((0x1 << shift) - 1));
	ret |= (x >> shift) &DF_MAN_MASK;
	if(ma) ret += 1;

	ret = (msbOne < 54) ? tmp:ret;
	ret = (msbOne == 0) ? 0:ret;

	return as_double(ret);
}

OVERLOADABLE double convert_double_rtz(ulong x)
{
	long exp;
	long ret, ma, tmp;
	int msbOne = 64 -clz(x);
	exp = msbOne + DF_EXP_BIAS - 1;
	ret = (exp << 52);

	int shift = abs(53 - msbOne);
	tmp = ret | ((x << shift) &DF_MAN_MASK);
	ret |= (x >> shift) &DF_MAN_MASK;
	ret = (msbOne < 54) ? tmp:ret;
	ret = (!msbOne) ? 0:ret;

	return as_double(ret);
}

OVERLOADABLE double convert_double_rtn(ulong x)
{
	long exp;
	long ret, ma, tmp;
	int msbOne = 64 -clz(x);
	exp = msbOne + DF_EXP_BIAS - 1;
	ret = (exp << 52);

	int shift = abs(53 - msbOne);
	tmp = ret | ((x << shift) &DF_MAN_MASK);
	ret |= (x >> shift) &DF_MAN_MASK;
	ret = (msbOne < 54) ? tmp:ret;
	ret = (!msbOne) ? 0:ret;

	return as_double(ret);
}

OVERLOADABLE double convert_double_rte(ulong x)
{
	long exp;
	long ret, ma, tmp;
	int msbOne = 64 -clz(x);

	exp = msbOne + DF_EXP_BIAS - 1;
	ret = (exp << 52);
	int shift = abs(53 - msbOne);
	tmp = ret | ((x << shift) &DF_MAN_MASK);

	ma = (x & ((0x1L << shift) - 1));
	int lastBit = (x >> shift) & 0x1;
	ret |= (x >> shift) &DF_MAN_MASK;
	ret = (ma > (0x1L << (msbOne -54))) ? ret+1:ret;
	ret = (ma == (0x1L << (msbOne -54)) && lastBit) ? ret+1:ret;

	ret = (msbOne < 54) ? tmp:ret;
	ret = (!msbOne) ? 0:ret;

	return as_double(ret);
}

OVERLOADABLE double convert_double_rtp(long x)
{
	long exp;
	long ret, ma, tmp;
	int sign = (x & DF_SIGN_MASK) ? 1:0;

	long absX = abs(x);
	int msbOne = 64 -clz(absX);
	exp = msbOne + DF_EXP_BIAS - 1;
	ret = (exp << 52);
	int shift = abs(53 - msbOne);
	tmp = ret | ((absX << shift) &DF_MAN_MASK);

	ma = (absX & ((0x1 << shift) - 1));
	ret |= (absX >> shift) &DF_MAN_MASK;
	if(ma && !sign) ret += 1;

	ret = (msbOne < 54) ? tmp:ret;
	ret = (msbOne == 0) ? 0:ret;

	ret |= (x & DF_SIGN_MASK);
	return as_double(ret);
}

OVERLOADABLE double convert_double_rtn(long x)
{
	long exp;
	long ret, ma, tmp;
	int sign = (x & DF_SIGN_MASK) ? 1:0;

	long absX = abs(x);
	int msbOne = 64 -clz(absX);
	exp = msbOne + DF_EXP_BIAS - 1;
	ret = (exp << 52);
	int shift = abs(53 - msbOne);
	tmp = ret | ((absX << shift) &DF_MAN_MASK);

	ma = (absX & ((0x1 << shift) - 1));
	ret |= (absX >> shift) &DF_MAN_MASK;
	if(ma && sign) ret += 1;

	ret = (msbOne < 54) ? tmp:ret;
	ret = (msbOne == 0) ? 0:ret;

	ret |= (x & DF_SIGN_MASK);
	return as_double(ret);
}

OVERLOADABLE double convert_double_rtz(long x)
{
	long exp;
	long ret, ma, tmp;
	int sign = (x & DF_SIGN_MASK) ? 1:0;

	long absX = abs(x);
	int msbOne = 64 -clz(absX);
	exp = msbOne + DF_EXP_BIAS - 1;
	ret = (exp << 52);
	int shift = abs(53 - msbOne);
	tmp = ret | ((absX << shift) &DF_MAN_MASK);

	ma = (absX & ((0x1 << shift) - 1));
	ret |= (absX >> shift) &DF_MAN_MASK;

	ret = (msbOne < 54) ? tmp:ret;
	ret = (msbOne == 0) ? 0:ret;

	ret |= (x & DF_SIGN_MASK);
	return as_double(ret);
}

OVERLOADABLE double convert_double_rte(long x)
{
	long exp;
	long ret, ma, tmp;

	long absX = abs(x);
	int msbOne = 64 -clz(absX);
	exp = msbOne + DF_EXP_BIAS - 1;
	ret = (exp << 52);
	int shift = abs(53 - msbOne);
	tmp = ret | ((absX << shift) &DF_MAN_MASK);

	ma = (absX& ((0x1L << shift) - 1));
	int lastBit = (absX >> shift) & 0x1;
	ret |= (absX>> shift) &DF_MAN_MASK;
	ret = (ma > (0x1L << (msbOne -54))) ? ret+1:ret;
	ret = (ma == (0x1L << (msbOne -54)) && lastBit) ? ret+1:ret;

	ret = (msbOne < 54) ? tmp:ret;
	ret = (!msbOne) ? 0:ret;

	ret |= (x & DF_SIGN_MASK);
	return as_double(ret);
}
'
fi

TTYPES=" ulong:8 long:8"
for vector_length in $VECTOR_LENGTHS; do
    for ftype in $TTYPES; do
	fbasetype=`IFS=:; set -- dummy $ftype; echo $2`

	    if test $vector_length -eq 1; then
		if [ $1"a" = "-pa" ]; then
		    echo "OVERLOADABLE double convert_double_rte($fbasetype x);"
		    echo "OVERLOADABLE double convert_double_rtz($fbasetype x);"
		    echo "OVERLOADABLE double convert_double_rtp($fbasetype x);"
		    echo "OVERLOADABLE double convert_double_rtn($fbasetype x);"
		fi
		continue
	    fi

	    for rounding in $ROUNDING_MODES; do
		fvectortype=$fbasetype$vector_length
		tvectortype=double$vector_length
		conv="convert_double_${rounding}"

		construct="$conv(v.s0)"
		if test $vector_length -gt 1; then
		    construct="$construct, $conv(v.s1)"
		fi
		if test $vector_length -gt 2; then
		    construct="$construct, $conv(v.s2)"
		fi
		if test $vector_length -gt 3; then
		    construct="$construct, $conv(v.s3)"
		fi
		if test $vector_length -gt 4; then
		    construct="$construct, $conv(v.s4)"
		    construct="$construct, $conv(v.s5)"
		    construct="$construct, $conv(v.s6)"
		    construct="$construct, $conv(v.s7)"
		fi
		if test $vector_length -gt 8; then
		    construct="$construct, $conv(v.s8)"
		    construct="$construct, $conv(v.s9)"
		    construct="$construct, $conv(v.sA)"
		    construct="$construct, $conv(v.sB)"
		    construct="$construct, $conv(v.sC)"
		    construct="$construct, $conv(v.sD)"
		    construct="$construct, $conv(v.sE)"
		    construct="$construct, $conv(v.sF)"
		fi

		if [ $1"a" = "-pa" ]; then
		    echo "OVERLOADABLE $tvectortype convert_${tvectortype}_${rounding}($fvectortype v);"
		else
		    echo "OVERLOADABLE $tvectortype convert_${tvectortype}_${rounding}($fvectortype v) {"
		    echo "  return ($tvectortype)($construct);"
		    echo "}"
		    echo
		fi
	done
    done
done

if [ $1"a" = "-pa" ]; then
    echo "#endif /* __OCL_CONVERT_H__ */"
fi
