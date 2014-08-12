/******************************************************************************
 * Copyright (C) 2007 Tetsuya Kimata <kimata@acapulco.dyndns.org>
 *
 * All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 *    not claim that you wrote the original software. If you use this
 *    software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 * $Id: TestRunner.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include "Environment.h"

#include <string>
#include <iostream>
#include <iomanip>

#include "apr_network_io.h"
#include "apr_file_info.h"
#include "AtomicWrapper.h"

#include "Auxiliary.h"
#include "SourceInfo.h"

// MEMO:
// TestRunner.h は一番最後に include するようにしているので，
// ここで using を使っても問題ない
using namespace std;

/**
 * テストを実行します．
 *
 * @param[in] pool プール
 * @param[in] argc 引数の数
 * @param[in] argv 引数
 */
static void run_all(apr_pool_t *pool, int argc, const char * const *argv);
/**
 * テストコードの実行方法を表示します．
 *
 * @param[in] prog_name プログラム名
 */
static void show_usage(const char *prog_name);
/**
 * 区切り線を表示します．
 */
void show_line()
{
    cerr << "\033[0;37m";
    cerr << "------------------------------------------------------------";
    cerr << "\033[0m" << endl;
}
/**
 * 区切り行を表示します．
 */
void show_spacer()
{
    cerr << endl;
}
/**
 * ソースの情報を表示します．
 */
void show_source_info()
{
    show_line();
    cerr << "\033[1;36m*\033[0m \033[1;37m" << "SOURCE" << "\033[0m" << endl;
    for (apr_size_t i = 0 ; i < SourceInfo::instance()->count(); i++) {
        cerr << SourceInfo::instance()->get(i) <<endl;
    }
}
/**
 * 入力情報のラベルを表示します．
 */
void show_input_label()
{
    show_line();
    cerr << "\033[1;36m*\033[0m \033[1;37m" << "INPUT" << "\033[0m" << endl;
}
/**
 * テストの名前を表示します．
 *
 * @param[in] test_name テストの名前
 */
void show_test_name(const char *test_name)
{
    cerr << "\033[0;36m*\033[0m \033[1;37m" << test_name << "\033[0m" << endl;
}
/**
 * アイテムを表示します．
 *
 * @param[in] key アイテムのキー
 * @param[in] value アイテムの値
 */
void show_item(std::string key, std::string value)
{
    cerr << setw(24) << setiosflags(ios::left) << key << ": ";
    cerr << value << endl;
}
/**
 * アイテムを表示します．
 *
 * @param[in] key アイテムのキー
 * @param[in] value アイテムの値
 */
void show_item(const char *key, int value)
{
    cerr << setw(24) << setiosflags(ios::left) << key << ": ";
    cerr.setf(ios::fixed, ios::floatfield);
    cerr << value << endl;
}
/**
 * アイテムを表示します．
 *
 * @param[in] key アイテムのキー
 * @param[in] value アイテムの値
 */
void show_item(const char *key, apr_size_t value)
{
    cerr << setw(24) << setiosflags(ios::left) << key << ": ";
    cerr.setf(ios::fixed, ios::floatfield);
    cerr << static_cast<unsigned int>(value) << endl;
}
/**
 * アイテムを表示します．
 *
 * @param[in] key アイテムのキー
 * @param[in] value アイテムの値
 */
void show_item(const char *key, double value)
{
    cerr << setw(24) << setiosflags(ios::left) << key << ": ";
    cerr.setf(ios::fixed, ios::floatfield);
    cerr << value << endl;
}
/**
 * アイテムを表示します．
 *
 * @param[in] key アイテムのキー
 * @param[in] value アイテムの値
 * @param[in] unit アイテムの単位
 */
void show_item(const char *key, double value, const char *unit)
{
    cerr << setw(24) << setiosflags(ios::left) << key << ": ";
    cerr.setf(ios::fixed, ios::floatfield);
    cerr << value << unit << endl;
}

/**
 * アドレスを適当な構造体に変換します．
 *
 * @param[in] pool プール
 * @param[in] address アドレス
 */
apr_sockaddr_t *get_sockaddr(apr_pool_t *pool, const char *address)
{
    apr_sockaddr_t *sockaddr;


    if (apr_sockaddr_info_get(&sockaddr, address, APR_UNSPEC, 80,
                              APR_IPV4_ADDR_OK, pool) != APR_SUCCESS) {
        THROW(MESSAGE_BUG_FOUND);
    }

    return sockaddr;
}

/**
 * テストを実行するメイン関数．
 *
 * @param[in] argc 引数の数
 * @param[in] argv 引数
 */
int main(int argc, const char * const *argv)
{
    apr_pool_t *pool;
    apr_app_initialize(&argc, &argv, NULL);
    apr_pool_create(&pool, NULL);

    try {
        init_atomic(pool);

        show_source_info();
        show_input_label();

        run_all(pool, argc, argv);

        show_line();

        cerr << "\033[1;32mOK.\033[0m" <<endl;
    } catch(const char *message) {
        cerr << "\033[1;31mNG.\033[0m" <<endl;
        cerr << "Error: " << message << endl;
        show_usage(argv[0]);

        apr_terminate();

        return EXIT_FAILURE;
    }

    apr_terminate();

    return EXIT_SUCCESS;
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
