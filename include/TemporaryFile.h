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
 * $Id: TemporaryFile.h 2889 2008-05-10 16:36:45Z svn $
 *****************************************************************************/

#ifndef TEMPORARY_FILE_H
#define TEMPORARY_FILE_H

#include "Environment.h"

#include "apr_strings.h"

#include "File.h"
#include "Message.h"
#include "Uncopyable.h"
#include "Macro.h"

/**
 * @brief 一時的なファイルを表すクラス．
 */
class TemporaryFile: public File, public Uncopyable
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] pool 書き込み時刻
     * @param[in] file_path ファイルのパス
     * @param[in] need_remove 必要なくなったらファイルを削除するかどうか
     */
    TemporaryFile(apr_pool_t *pool, const char *file_path,
                  bool need_remove=true)
      : File(pool, file_path),
        need_remove_(need_remove)
    {
        temp_file_path_ = apr_pstrcat(pool, file_path, TEMP_NAME_SUFFIX, NULL);
    };
    TemporaryFile(const TemporaryFile& file)
      : File(file),
        Uncopyable(),
        temp_file_path_(file.temp_file_path_),
        need_remove_(file.need_remove_)
    {

    };
    TemporaryFile& operator=(const TemporaryFile& file)
    {
        if (this == &file) {
            return *this;
        }

        this->~TemporaryFile();

        pool_       = file.pool_;
        file_path_  = file.file_path_;
        file_       = file.file_;
        mmap_       = file.mmap_;
        ref_count_  = file.ref_count_;

        temp_file_path_ = file.temp_file_path_;
        need_remove_    = file.need_remove_;

        (*ref_count_)++;

        return *this;
    };


    /**
     * デストラクタです．
     */
    ~TemporaryFile()
    {
        if (*ref_count_ != 1) {
            return;
        }

        if (!need_remove_) {
            return;
        }

        apr_file_remove(temp_file_path_, pool_);
    };

    /**
     * ファイルを open します．
     *
     * @param[in] flag フラグ
     */
    void open(apr_int32_t flag=0)
    {
        flag |= APR_READ | APR_CREATE | APR_EXCL;

        is_buffered_ = (flag & APR_BUFFERED) != 0;

        if (apr_file_mktemp(&file_, temp_file_path_, flag, pool_)
            != APR_SUCCESS) {
            THROW(MESSAGE_FILE_OPEN_FAILED);
        }
    };
    /**
     * 一時的なファイルを使ってファイルの更新を行います．
     */
    void commit()
    {
        close();

        if (apr_file_rename(temp_file_path_, file_path_, pool_)
            != APR_SUCCESS) {
            THROW(MESSAGE_FILE_RENAME_FAILED);
        }

        need_remove_ = false;
    };
    const char *get_temp_path() const
    {
        return temp_file_path_;
    };

private:
    /** 一時ファイルのサフィックス */
    static const char TEMP_NAME_SUFFIX[];
    /** 一時ファイルのパス */
    char *temp_file_path_;
    /** 必要なくなったらファイルを削除するかどうか */
    bool need_remove_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
