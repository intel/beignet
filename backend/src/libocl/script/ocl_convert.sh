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
'
fi

# convert_DSTTYPE_ROUNDING function
for vector_length in $VECTOR_LENGTHS; do
    for ftype in $TYPES; do
	fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
	if test $fbasetype = "double"; then continue; fi

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

# convert_DSTTYPE_sat_ROUNDING function
for vector_length in $VECTOR_LENGTHS; do
    for ftype in $TYPES; do
	fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
	if test $fbasetype = "double"; then continue; fi

	for ttype in $TYPES; do
	    tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
	    if test $tbasetype = "double" -o $tbasetype = "float" -o $tbasetype = "half" ; then continue; fi

	    if test $vector_length -eq 1; then
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

if [ $1"a" = "-pa" ]; then
    echo "#endif /* __OCL_CONVERT_H__ */"
fi
