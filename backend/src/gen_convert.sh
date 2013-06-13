#! /bin/sh -e

. ./genconfig.sh

# For all vector lengths and types, generate conversion functions
for vector_length in $VECTOR_LENGTHS; do
        if test $vector_length -eq 1; then
                continue;
        fi
        for ftype in $TYPES; do
                fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
                for ttype in $TYPES; do
                        tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
                        if test $fbasetype = $tbasetype; then
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
done
