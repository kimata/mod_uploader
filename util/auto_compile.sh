#!/usr/bin/env zsh

unsetopt FUNCTION_ARGZERO

BASE_DIR=${1:-$PWD}
WAIT_SEC=10

run() {
    echo "$0: running \`$@'"
    $@
}

while true; do
    echo "\033[1;32mwait for changing $BASE_DIR/include and $BASE_DIR/src\033[0m"

    run inotifywait $BASE_DIR/include $BASE_DIR/src

    # 変更が一定期間発生しなくなるのを待つ
    while true; do
        run inotifywait -t $WAIT_SEC $BASE_DIR/include $BASE_DIR/src
        [ $? -eq 2 ] && break
    done

    run make -C $BASE_DIR
    [ $? -eq 0 ] || continue
    run sudo make -C $BASE_DIR -f GNUmakefile.apache install
    run sudo /etc/init.d/apache2 stop
    run sudo /etc/init.d/apache2 start
done
