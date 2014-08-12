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
 * $Id: cgi_uploader.cpp 2768 2007-12-16 13:03:37Z svn $
 *****************************************************************************/

/**
 * @file
 * @brief CGI のエントリ関数群．
 */

#ifndef UPLOADER_TYPE_CGI
#define UPLOADER_TYPE_CGI
#endif

#include "Environment.h"

#include <iostream>

#ifdef __linux__
#include <sched.h>
#endif

#include "cgi_uploader.h"
#include "uploader_handler.h"

#include "CGIResponse.h"
#include "CGIConfigReader.h"
#include "TemporaryPool.h"
#include "SourceInfo.h"

#define TEMPLATE_INSTANTIATION
#include "uploader_handler.cpp"

SOURCE_INFO_ADD("$Id: cgi_uploader.cpp 2768 2007-12-16 13:03:37Z svn $");

// strace したときに，表示処理の範囲を見分けるための印
#if defined(DEBUG) && defined(__linux__)
#define STRACE_MARKER() sched_yield()
#else
#define STRACE_MARKER()
#endif

int main(int argc, const char * const *argv)
{
    apr_pool_t *pool;
    const char *arg;
    UploaderConfig *config;
    int status;
    apr_size_t bench_count;

    apr_app_initialize(&argc, &argv, NULL);
    apr_pool_create(&pool, NULL);

    config = NULL;
    status = 0;
    try {
        arg = getenv("PATH_INFO");
        if (arg == NULL) {
            std::cout << "Location: http://";
            std::cout << CGIResponse::get_env(pool, "SERVER_NAME");

            if ((strcmp(CGIResponse::get_env(pool, "SERVER_PORT"),
                        "80") != 0)) {
                std::cout << ":" << CGIResponse::get_env(pool, "SERVER_PORT");
            }

            std::cout << CGIResponse::get_env(pool, "SCRIPT_NAME") << "/\r\n\r\n";
            return EXIT_SUCCESS;
        }

        config = CGIConfigReader::read(pool, CONF_FILE_NAME);
        config->child_init();

        // ベンチマーク用
        bench_count = atoi(getenv("UPLOADER_BENCH"));
        if (bench_count == 0) {
            bench_count++;
        }

        STRACE_MARKER();
        for (apr_size_t i = 0; i < bench_count; i++) {
            TemporaryPool temp_pool(pool);
            CGIResponse::Handle handle(temp_pool.get());

            status = uploader_command_handler<CGIResponse>
                (&handle, config, arg);
        }
        STRACE_MARKER();

        config->finalize();
        apr_pool_destroy(pool);
        apr_terminate();

        return status;
    } catch(const char *message) {
        // できれば Internal Server Error にしたいけど，問い合わせが多
        // くなりそうなので...
        std::cout << "Content-type: text/html; charset=EUC_JP\r\n\r\n";
        std::cout << "Fatal Error: " << message << std::endl;

        if (config != NULL) {
            config->finalize();
        }
        apr_pool_destroy(pool);
        apr_terminate();

        return EXIT_FAILURE;
    }
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
