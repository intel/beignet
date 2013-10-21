#! /bin/sh -e

. ./genconfig.sh

# For all vector lengths and types, generate conversion functions
for vector_length in $VECTOR_LENGTHS; do
        if test $vector_length -eq 1; then
          for ftype in $TYPES; do
            fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
            for ttype in $TYPES; do
              tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
              if test $fbasetype = $tbasetype; then
                continue
              fi
              echo "INLINE OVERLOADABLE $tbasetype convert_$tbasetype($fbasetype v) {"
              echo "  return ($tbasetype)v;"
              echo "}"
              echo
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
                            echo "INLINE OVERLOADABLE $tvectortype convert_$tvectortype($fvectortype v) { return v; }"
                          else
                            echo "INLINE OVERLOADABLE $tbasetype convert_$tbasetype($fbasetype v) { return v; }"
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

                        echo "INLINE OVERLOADABLE $tvectortype convert_$tvectortype($fvectortype v) {"
                        echo "  return ($tvectortype)($construct);"
                        echo "}"
                        echo
                done
          done
        fi
done

echo '
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
DEF(ushort, short);
DEF(ushort, int);
DEF(ushort, uint);
DEF(ushort, float);
DEF(int, uint);
DEF(int, float);
DEF(uint, int);
DEF(uint, float);
#undef DEF

#define DEF(DSTTYPE, SRCTYPE, MIN, MAX) \
  INLINE_OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x) { \
    return x > MAX ? (DSTTYPE)MAX : x < MIN ? (DSTTYPE)MIN : x; \
  }
DEF(char, long, -128, 127);
DEF(uchar, long, 0, 255);
DEF(short, long, -32768, 32767);
DEF(ushort, long, 0, 65535);
DEF(int, long, -0x7fffffff-1, 0x7fffffff);
DEF(uint, long, 0, 0xffffffffu);
DEF(long, float, -9.223372036854776e+18f, 9.223372036854776e+18f);
DEF(ulong, float, 0, 1.8446744073709552e+19f);
#undef DEF

#define DEF(DSTTYPE, SRCTYPE, MAX) \
  INLINE_OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x) { \
    return x > MAX ? (DSTTYPE)MAX : x; \
  }
DEF(char, ulong, 127);
DEF(uchar, ulong, 255);
DEF(short, ulong, 32767);
DEF(ushort, ulong, 65535);
DEF(int, ulong, 0x7fffffff);
DEF(uint, ulong, 0xffffffffu);
#undef DEF

INLINE_OVERLOADABLE long convert_long_sat(ulong x) {
  ulong MAX = 0x7ffffffffffffffful;
  return x > MAX ? MAX : x;
}

INLINE_OVERLOADABLE ulong convert_ulong_sat(long x) {
  return x < 0 ? 0 : x;
}

#define DEF(DSTTYPE, SRCTYPE) \
  INLINE_OVERLOADABLE DSTTYPE convert_ ## DSTTYPE ## _sat(SRCTYPE x) { \
    return x; \
  }
DEF(char, char);
DEF(uchar, uchar);
DEF(short, char);
DEF(short, uchar);
DEF(short, short);
DEF(ushort, char);
DEF(ushort, uchar);
DEF(ushort, ushort);
DEF(int, char);
DEF(int, uchar);
DEF(int, short);
DEF(int, ushort);
DEF(int, int);
DEF(uint, char);
DEF(uint, uchar);
DEF(uint, short);
DEF(uint, ushort);
DEF(uint, uint);
DEF(long, char);
DEF(long, uchar);
DEF(long, short);
DEF(long, ushort);
DEF(long, int);
DEF(long, uint);
DEF(long, long);
DEF(ulong, char);
DEF(ulong, uchar);
DEF(ulong, short);
DEF(ulong, ushort);
DEF(ulong, int);
DEF(ulong, uint);
DEF(ulong, ulong);
#undef DEF
'

# vector convert_DSTTYPE_sat function
for vector_length in $VECTOR_LENGTHS; do
  if test $vector_length -eq 1; then continue; fi

  for ftype in $TYPES; do
    fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
    if test $fbasetype = "double"; then continue; fi

    for ttype in $TYPES; do
      tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
      if test $tbasetype = "double" -o $tbasetype = "float"; then continue; fi

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

      echo "INLINE OVERLOADABLE $tvectortype convert_${tvectortype}_sat($fvectortype v) {"
      echo "  return ($tvectortype)($construct);"
      echo "}"
      echo
    done
  done
done

echo '
float __gen_ocl_rndz(float x);
float __gen_ocl_rnde(float x);
float __gen_ocl_rndu(float x);
float __gen_ocl_rndd(float x);
'

# convert_DSTTYPE_ROUNDING function
for vector_length in $VECTOR_LENGTHS; do
  for ftype in $TYPES; do
    fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
    if test $fbasetype = "double"; then continue; fi

    for ttype in $TYPES; do
      tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
      if test $tbasetype = "double"; then continue; fi

      if test $vector_length -eq 1; then
        echo "INLINE_OVERLOADABLE $tbasetype convert_${tbasetype}_rte($fbasetype x)"
        if test $fbasetype = "float" -a $tbasetype != "float"; then
          echo "{ return __gen_ocl_rnde(x); }"
        else
          echo "{ return x; }"
        fi

        echo "INLINE_OVERLOADABLE $tbasetype convert_${tbasetype}_rtz($fbasetype x)"
        if test $fbasetype = "float" -a $tbasetype != "float"; then
          echo "{ return __gen_ocl_rndz(x); }"
        else
          echo "{ return x; }"
        fi

        echo "INLINE_OVERLOADABLE $tbasetype convert_${tbasetype}_rtp($fbasetype x)"
        if test $fbasetype = "float" -a $tbasetype != "float"; then
          echo "{ return __gen_ocl_rndu(x); }"
        else
          echo "{ return x; }"
        fi

        echo "INLINE_OVERLOADABLE $tbasetype convert_${tbasetype}_rtn($fbasetype x)"
        if test $fbasetype = "float" -a $tbasetype != "float"; then
          echo "{ return __gen_ocl_rndd(x); }"
        else
          echo "{ return x; }"
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

        echo "INLINE OVERLOADABLE $tvectortype convert_${tvectortype}_${rounding}($fvectortype v) {"
        echo "  return ($tvectortype)($construct);"
        echo "}"
        echo
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
      if test $tbasetype = "double" -o $tbasetype = "float"; then continue; fi

      if test $vector_length -eq 1; then
        echo "INLINE_OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rte($fbasetype x)"
        if test $fbasetype = "float"; then
          echo "{ return convert_${tbasetype}_sat(__gen_ocl_rnde(x)); }"
        else
          echo "{ return convert_${tbasetype}_sat(x); }"
        fi

        echo "INLINE_OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rtz($fbasetype x)"
        if test $fbasetype = "float"; then
          echo "{ return convert_${tbasetype}_sat(__gen_ocl_rndz(x)); }"
        else
          echo "{ return convert_${tbasetype}_sat(x); }"
        fi

        echo "INLINE_OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rtp($fbasetype x)"
        if test $fbasetype = "float"; then
          echo "{ return convert_${tbasetype}_sat(__gen_ocl_rndu(x)); }"
        else
          echo "{ return convert_${tbasetype}_sat(x); }"
        fi

        echo "INLINE_OVERLOADABLE $tbasetype convert_${tbasetype}_sat_rtn($fbasetype x)"
        if test $fbasetype = "float"; then
          echo "{ return convert_${tbasetype}_sat(__gen_ocl_rndd(x)); }"
        else
          echo "{ return convert_${tbasetype}_sat(x); }"
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

        echo "INLINE OVERLOADABLE $tvectortype convert_${tvectortype}_sat_${rounding}($fvectortype v) {"
        echo "  return ($tvectortype)($construct);"
        echo "}"
        echo
      done
    done
  done
done
