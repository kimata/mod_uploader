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
 * $Id: BasicFileWriter.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef BASIC_FILE_WRITER_H
#define BASIC_FILE_WRITER_H

#include "Environment.h"

#include <cstdlib>

#include "apr_mmap.h"

#include "FileWriter.h"


/**
 * @brief read と write を使って，ファイルへデータを書き出すクラス．
 */
class BasicFileWriter: public FileWriter
{
public:
    static const apr_int32_t OPEN_FLAG  = APR_READ|APR_WRITE|APR_BINARY|APR_BUFFERED;
    /**
     * コンストラクタ．
     *
     * offset が指定されている場合は，その位置以降に書き出しを行います．
     *
     * @param[in] pool プール
     * @param[in] file ファイルハンドラ
     * @param[in] offset 書き出しを開始する位置
     */
    BasicFileWriter(apr_pool_t *pool, apr_file_t *file, apr_size_t offset=0);
    /**
     * ファイルへの書き出しを行います．
     *
     * @param[in] buffer 書き出すデータ
     * @param[in] size 書き出すサイズ
     * @return 実際に書き出したサイズ
     */
    apr_size_t write_impl(const void *buffer, apr_size_t size);
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
