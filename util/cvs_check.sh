#!/usr/bin/env zsh

unsetopt FUNCTION_ARGZERO

BASE_DIR=`mktemp -d`

run() {
    echo "$0: running \`$@'"
    $@ || exit -1
}

pushd $BASE_DIR

run cvs -z3 -d:pserver:anonymous@cvs.sourceforge.jp:/cvsroot/mod-uploader co mod_uploader
cd mod_uploader
run ./configure
run make

popd
rm -rf $BASE_DIR

echo -e "\033[1;32mOK\033[0m";
