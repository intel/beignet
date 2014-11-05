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
    echo "#ifndef __OCL_AS_H__"
    echo "#define __OCL_AS_H__"
    echo "#include \"ocl_types.h\""
    echo
else
    echo "#include \"ocl_as.h\""
    echo
fi

# Supported base types and their lengths
TYPES="long:8 ulong:8 int:4 uint:4 short:2 ushort:2 char:1 uchar:1 double:8 float:4"
# Supported vector lengths
VECTOR_LENGTHS="1 2 3 4 8 16"
ROUNDING_MODES="rte rtz rtp rtn"

# Generate list of union sizes
for type in $TYPES; do
        size=`IFS=:; set -- dummy $type; echo $3`
        for vector_length in $VECTOR_LENGTHS; do
                if test $vector_length -eq 3; then
                      continue;
                fi
                union_sizes="$union_sizes `expr $vector_length \* $size`"
        done
done
union_sizes="`echo $union_sizes | tr ' ' '\n' | sort -n | uniq`"

# For each union size
for union_size in $union_sizes; do

    if [ $1"a" != "-pa" ]; then
	# Define an union that contains all vector types that have the same size as the union
	unionname="union _type_cast_${union_size}_b"
	echo "$unionname {"
	for type in $TYPES; do
	    basetype=`IFS=:; set -- dummy $type; echo $2`
	    basesize=`IFS=:; set -- dummy $type; echo $3`
	    for vector_length in $VECTOR_LENGTHS; do
		if test $vector_length -eq 3; then
		    vector_size_length="4"
		else
		    vector_size_length=$vector_length;
		fi
		vector_size_in_union="`expr $vector_size_length \* $basesize`"
		if test $union_size -ne $vector_size_in_union; then
		    continue
		fi
		if test $vector_length -eq 1; then
		    vectortype=$basetype
		else
		    vectortype=$basetype$vector_length
		fi
		echo "  $vectortype _$vectortype;"
	    done

	done
	echo "};"
	echo
    fi

    # For each tuple of vector types that has the same size as the current union size,
    # define an as_* function that converts types without changing binary representation.
    for ftype in $TYPES; do
	fbasetype=`IFS=:; set -- dummy $ftype; echo $2`
	fbasesize=`IFS=:; set -- dummy $ftype; echo $3`
	for fvector_length in $VECTOR_LENGTHS; do
	    if test $fvector_length -eq 3; then
		fvector_size_length="4"
	    else
		fvector_size_length=$fvector_length;
	    fi
	    fvector_size_in_union="`expr $fvector_size_length \* $fbasesize`"
	    if test $union_size -ne $fvector_size_in_union; then
		continue
	    fi
	    if test $fvector_length -eq 1; then
		fvectortype=$fbasetype
	    else
		fvectortype=$fbasetype$fvector_length
	    fi
	    for ttype in $TYPES; do
		tbasetype=`IFS=:; set -- dummy $ttype; echo $2`
		tbasesize=`IFS=:; set -- dummy $ttype; echo $3`
		if test $fbasetype = $tbasetype; then
		    continue
		fi
		for tvector_length in $VECTOR_LENGTHS; do
		    if test $tvector_length -eq 3; then
			tvector_size_length="4"
		    else
			tvector_size_length=$tvector_length;
		    fi
		    tvector_size_in_union="`expr $tvector_size_length \* $tbasesize`"
		    if test $union_size -ne $tvector_size_in_union; then
			continue
		    fi
		    if test $tvector_length -eq 1; then
			tvectortype=$tbasetype
		    else
			tvectortype=$tbasetype$tvector_length
		    fi

		    if [ $1"a" = "-pa" ]; then
			echo "OVERLOADABLE $tvectortype as_$tvectortype($fvectortype v);"
		    else
			echo "OVERLOADABLE $tvectortype as_$tvectortype($fvectortype v) {"
			echo "  $unionname u;"
			echo "  u._$fvectortype = v;"
			echo "  return u._$tvectortype;"
			echo "}"
			echo
		    fi
		done
	    done
	done

    done

done


if [ $1"a" = "-pa" ]; then
    echo "#endif /* __OCL_AS_H__ */"
fi
