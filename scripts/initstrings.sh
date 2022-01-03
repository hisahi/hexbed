#!/bin/bash
BASEDIR=$(dirname $(realpath $0))/..
PODIR=${BASEDIR}/po

LANGUAGE=$1
if [ "$#" -lt 1 ]; then
    echo "$0 <langcode> [locale]"
    exit 2
fi

LOCALE=${2:-${1}}
POSUBDIR=${PODIR}/${LANGUAGE}

mkdir -p ${POSUBDIR}
msginit --input=${PODIR}/hexbed.pot --locale=$LOCALE --output=${POSUBDIR}/hexbed.po
