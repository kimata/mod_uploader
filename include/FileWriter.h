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
 * $Id: FileWriter.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef FILE_WRITER_H
#define FILE_WRITER_H

#include "Environment.h"

#include <cstdlib>

#include "apr_file_io.h"

#include "Uncopyable.h"
#include "Message.h"
#include "Macro.h"


class File;

/**
 * @brief ファイルへデータを書き出すクラス．
 */
class FileWriter: public Uncopyable
{
public:
    /**
     * コンストラクタ．
     *
     * @param[in] pool プール
     * @param[in] file ファイルハンドラ
     */
    FileWriter(apr_pool_t *pool, apr_file_t *file);
    /**
     * ファイルへの書き出しを行います．
     * 実際の書き込み処理は write_impl に委譲します．
     *
     * @param[in] buffer 書き出すデータ
     * @param[in] size 書き出すサイズ
     * @exception const char * 書き出しに失敗した場合
     */
    void write(const void *buffer, apr_size_t size)
    {
        apr_size_t write_size;

        if (size == 0) {
            return;
        }

        write_size = size;
        write(buffer, &write_size);
        if (write_size != size) {
            THROW(MESSAGE_FILE_WRITE_FAILED);
        }
    };
    /**
     * ファイルへの書き出しを行います．
     * 実際の書き込み処理は write_impl に委譲します．
     *
     * @param[in] buffer 書き出すデータ
     * @param[in,out] size 書き出すサイズ
     * @exception const char * 書き出しに失敗した場合
     */
    void write(const void *buffer, apr_size_t *size)
    {
        *size = write_impl(buffer, *size);
        write_size_ += *size;
    };
    /**
     * ファイルを閉じます．
     *
     * @exception const char * 閉じるのに失敗した場合
     */
    apr_uint64_t get_write_size() const
    {
        return write_size_;
    };
    virtual void close();
    /**
     * デストラクタ．
     *
     * ファイルを閉じます．
     */
    virtual ~FileWriter();

protected:
    /**
     * ファイルへの書き出しを実際に行います．
     *
     * @param[in] buffer 書き出すデータ
     * @param[in] size 書き出すサイズ
     * @return 実際に書き出したサイズ
     * @exception const char * 書き出しに失敗した場合
     */
    virtual apr_size_t write_impl(const void *buffer, apr_size_t size) = 0;

    /** プール */
    apr_pool_t *pool_;
    /** ファイルハンドラ */
    File *file_;
    /** 書き込んだサイズ */
    apr_uint64_t write_size_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
