#! /bin/sh -e

STDLIB_HEADER=ocl_stdlib.h

exec >$STDLIB_HEADER.tmp
sed -n -e '1,/##BEGIN_AS##/p' $STDLIB_HEADER
./gen_as.sh
sed -n -e '/##END_AS##/,$p' $STDLIB_HEADER
exec >&2

mv $STDLIB_HEADER.tmp $STDLIB_HEADER
