#!/bin/bash

SOURCE_DIR=$1
FILE=$2

touch ${SOURCE_DIR}/${FILE}_tmp
if test -d ${SOURCE_DIR}/../.git; then
        if which git > /dev/null; then
            git --git-dir=${SOURCE_DIR}/../.git log -n 1 --oneline | \
                sed 's/^\([^ ]*\) .*/#define BEIGNET_GIT_SHA1 "git-\1"/' \
                > ${SOURCE_DIR}/${FILE}_tmp
        fi
fi

#updating ${SOURCE_DIR}/${FILE}
if ! cmp -s ${SOURCE_DIR}/${FILE}_tmp ${SOURCE_DIR}/${FILE}; then
                mv  ${SOURCE_DIR}/${FILE}_tmp ${SOURCE_DIR}/${FILE}
else
                rm  ${SOURCE_DIR}/${FILE}_tmp
fi
