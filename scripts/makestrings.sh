#!/bin/bash
BASEDIR=$(dirname $(realpath $0))/..
PODIR=${BASEDIR}/po

for subdir in ${PODIR}/*/; do
    pofile=${subdir}hexbed.po
    if [ -f $pofile ]; then
        mofile=${subdir}hexbed.mo
        msgfmt --output-file=$mofile $pofile || exit 1
    fi
done
