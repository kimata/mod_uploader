#!/usr/bin/env zsh

unsetopt FUNCTION_ARGZERO

PACKAGE=`grep AC_INIT configure.ac | perl -pe 's/^[^\(]+[^\[]+\[([^\]]+)\].*$/\1/'`
VERSION=`grep AC_INIT configure.ac | perl -pe 's/^[^,]+\D+([\d.]+)\D.*$/\1/' | tr '.' '_'`

WEB_BASE=/var/www/acapulco.dyndns.org/${PACKAGE}
CVS_BASE=${HOME}/cvs/${PACKAGE}

echo -n 'windows pass: '
read -s WIN_PASS
echo
echo -n 'sourceforge pass: '
read -s SF_PASS
echo

export WIN_PASS
export SF_PASS

run() {
    echo "$0: running \`$@'"
    $@ || exit -1
}

run ./util/rpm_build.pl
run ./util/deb_build.pl
run ./util/win_build.pl

run make -f GNUmakefile.dist dist-sf
run make -f GNUmakefile.dist dist-sf-rpm
run make -f GNUmakefile.dist dist-sf-srpm
run make -f GNUmakefile.dist dist-sf-deb
run make -f GNUmakefile.dist dist-sf-win

run cp src/mod_uploader-ap2.0.so ${WEB_BASE}/file
run cp src/mod_uploader-ap2.2.so ${WEB_BASE}/file

(
    run cd doc
    run make doc-update
    run make doc-api
    run cp apache.htm apache.*.htm apache-win.htm ${WEB_BASE}
    run cp -r api ${WEB_BASE}
)

(
    run rsync -avLq --exclude '.*' --max-size=400K . ${CVS_BASE}
    run cd ${CVS_BASE}
    cvs commit -m ""
    run cvs tag release-${VERSION}_`date "+%y%m%d%H%M"`
)

echo -e "\033[1;32mOK\033[0m";
