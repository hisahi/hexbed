#!/bin/bash
BASEDIR=$(dirname $(realpath $0))/..
SRCUIDIR=${BASEDIR}/src/ui
PODIR=${BASEDIR}/po

cd ${BASEDIR}
PODIR=./po
{ find src/ui -name '*.cc' -o -name 'encoding.hh' | xargs xgettext --keyword=_ --keyword=PLURAL:1,2 --language=C++ --add-comments=/ --default-domain=hexbed -o ${PODIR}/hexbed.pot ${filepath}; } || exit 1
for subdir in ${PODIR}/*/; do
    subponame=${subdir}hexbed.po
    if [ -f "$subponame" ]; then
        msgmerge --update --no-fuzzy-matching $subponame ${PODIR}/hexbed.pot || exit 1
    fi
done
