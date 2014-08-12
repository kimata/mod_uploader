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
 *    documentation would be appreciated but is not bcktuired.
 *
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 * $Id: DirectoryCleaner.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef DIRECTORY_CLEANER_H
#define DIRECTORY_CLEANER_H

#include "Environment.h"

#include "apr_pools.h"

#include <cstdlib>


/**
 * @brief ディレクトリの掃除を行うクラス．
 */
class DirectoryCleaner
{
public:
    /**
     * ディレクトリ中にある古いファイルを削除します．
     *
     * mtime が現在時刻よりも threshold_sec 秒以上古いものが削除対象と
     * なります．ファイル名が「.」で始まる物は無視されます．
     *
     * @param[in] pool プール
     * @param[in] dir_path 対象ディレクトリのパス
     * @param[in] threshold_sec 削除すべきかどうかの基準
     */
    static void clean_old_files(apr_pool_t *pool,
                                const char *dir_path,
                                apr_size_t threshold_sec);
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
