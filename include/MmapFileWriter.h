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
 * $Id: MmapFileWriter.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef MMAP_FILE_WRITER_H
#define MMAP_FILE_WRITER_H

#include "Environment.h"

#include <cstdlib>

#include "apr_mmap.h"

#include "FileWriter.h"


/**
 * @brief mmap を使って，ファイルへデータを書き出すクラス．
 */
class MmapFileWriter: public FileWriter
{
public:
    static const apr_int32_t OPEN_FLAG  = APR_READ|APR_WRITE|APR_BINARY;
    /**
     * コンストラクタ．
     *
     * offset が指定されている場合は，その位置以降に書き出しを行います．
     *
     * @param[in] pool プール
     * @param[in] file ファイルハンドラ
     * @param[in] offset 書き出しを開始する位置
     */
    MmapFileWriter(apr_pool_t *pool, apr_file_t *file, apr_size_t offset=0);
    /**
     * ファイルを閉じます．
     *
     * @exception const char * 閉じるのに失敗した場合
     */
    void close();
    /**
     * デストラクタ．
     *
     * ファイルを閉じます．
     */
    ~MmapFileWriter();

private:
    /**
     * ファイルへの書き出しを行います．
     *
     * @param[in] buffer 書き出すデータ
     * @param[in] size 書き出すサイズ
     * @return 実際に書き出したサイズ
     * @exception const char * 書き出しに失敗した場合
     */
    apr_size_t write_impl(const void *buffer, apr_size_t size);
    /**
     * OS のページサイズ
     *
     * getpagesize(2) が使えればその返値が用いられる．
     */
    static const apr_size_t PAGE_SIZE;
    /**
     * mmap によるメモリ割り当てのブロックサイズ候補
     */
    static const apr_size_t BLOCK_SIZE;

    /**
     * 現在のマッピングを解除して，次のブロック位置をマップします．
     */
    void expand();

    /** mmap の情報 */
    apr_mmap_t *file_map_;
    /** 現在マップしているブロックのインデックス */
    apr_uint64_t block_index_;
    /** 次に書き込むブロック中の位置 */
    apr_size_t block_offset_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
