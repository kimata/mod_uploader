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
 * $Id: File.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef FILE_H
#define FILE_H

#include "Environment.h"

#include <cstdlib>
#ifdef DEBUG
#include <iostream>
#endif

#ifndef WIN32
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#endif

#include "apr_mmap.h"
#include "apr_strings.h"

#include "Message.h"
#include "Auxiliary.h"
#include "Macro.h"


#ifdef DEBUG
#define REQUIRE_FILE_OPENDED                                    \
    do {                                                        \
        if (is_closed()) { \
            THROW(MESSAGE_FILE_STATE_INVALID);                  \
        }                                                       \
    } while (0)
#else
#define REQUIRE_FILE_OPENDED
#endif

/**
 * @brief ファイルを表すクラス．
 *
 * 例外チェックをクラス内に隠匿することを目的に作られています．インター
 * フェースはかなり適当です．
 */
class File
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] file_path ファイルのパス
     */
    File(apr_pool_t *pool, const char *file_path="")
      : pool_(pool),
        is_buffered_(false),
        file_path_(file_path),
        file_(NULL),
        mmap_(NULL),
        ref_count_(NULL)
    {
        APR_PALLOC(ref_count_, int *, pool_, sizeof(int));
        *ref_count_ = 1;
    };
    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] file ファイルハンドル
     */
    File(apr_pool_t *pool, apr_file_t *file)
      : pool_(pool),
        is_buffered_((apr_file_flags_get(file) & APR_BUFFERED) != 0),
        file_path_(NULL),
        file_(file),
        mmap_(NULL),
        ref_count_(NULL)
    {
        APR_PALLOC(ref_count_, int *, pool_, sizeof(int));
        *ref_count_ = 1;
    };
    File(const File& file)
      : pool_(file.pool_),
        file_path_(file.file_path_),
        file_(file.file_),
        mmap_(file.mmap_),
        ref_count_(file.ref_count_)
    {
        (*ref_count_)++;
    };
    File& operator=(const File& file)
    {
        if (this == &file) {
            return *this;
        }

        this->~File();

        pool_       = file.pool_;
        file_path_  = file.file_path_;
        file_       = file.file_;
        mmap_       = file.mmap_;
        ref_count_  = file.ref_count_;

        (*ref_count_)++;

        return *this;
    };
    /**
     * デストラクタです．
     */
    virtual ~File()
    {
        if (--(*ref_count_) != 0) {
            return;
        }
#ifdef DEBUG
        if (*ref_count_ < 0) {
            THROW(MESSAGE_BUG_FOUND);
        }
#endif
        close();
    };
    /**
     * ファイルを open します．
     *
     * @param[in] flag フラグ
     */
    virtual void open(apr_int32_t flag)
    {
        is_buffered_ = (flag & APR_BUFFERED) != 0;

        if (apr_file_open(&file_, file_path_, flag, APR_OS_DEFAULT,
                          pool_) != APR_SUCCESS) {
            throw apr_pstrcat(pool_,
                              MESSAGE_FILE_OPEN_FAILED,
                              " [", file_path_, "]",
                              " (" __FILE__ ":" APR_STRINGIFY(__LINE__) ")",
                              NULL);
        }
    };
    /**
     * ファイルを close します．
     */
    void close()
    {
        munmap();
        if (file_ == NULL) {
            return;
        }

#ifdef DEBUG_File
        apr_status_t status;
        if ((status = apr_file_close(file_)) != APR_SUCCESS) {
            cerr << get_error_message(pool_, status) << endl;
            exit(EXIT_FAILURE);
        }
#else
        apr_file_close(file_);
#endif

        file_ = NULL;
    };
    /**
     * ファイルに対して mmap を行います．
     */
    apr_mmap_t *mmap(apr_uint64_t offset, apr_uint64_t size, apr_int32_t flag,
                     bool is_madvise)
    {
        REQUIRE_FILE_OPENDED;

#ifdef DEBUG
        if (is_buffered_) {
            THROW(MESSAGE_FILE_MMAP_INVALID_FILE);
        }

        if (mmap_ != NULL) {
            THROW(MESSAGE_FILE_MMAP_DOUBLED);
        }
#endif
        if (size == 0) {
            size = get_size();
        }

        if (apr_mmap_create(&mmap_, file_, offset,
                            static_cast<apr_size_t>(size), flag, pool_)
            != APR_SUCCESS) {
            THROW(MESSAGE_FILE_MMAP_FAILED);
        }

#ifndef WIN32
#ifdef HAVE_MADVISE
        // read の時のみ madvise
        if (is_madvise) {
            madvise(mmap_->mm, mmap_->size, MADV_SEQUENTIAL);
        }
#endif
#endif
        return mmap_;
    };
    /**
     * ファイルに対して mmap を行います．
     */
    apr_mmap_t *mmap(apr_uint64_t offset, apr_size_t size, apr_int32_t flag)
    {
        REQUIRE_FILE_OPENDED;

        return mmap(offset, size, flag, false);
    }
    /**
     * ファイルに対して mmap を行います．
     */
    apr_mmap_t *mmap(apr_int32_t flag=0)
    {
        REQUIRE_FILE_OPENDED;

        flag |= APR_MMAP_READ;

        munmap();

        return mmap(0, static_cast<apr_size_t>(get_size()), flag);
    };
    /**
     * ファイルに対する mmap を解除します．
     */
    void munmap()
    {
        if (mmap_ == NULL) {
            return;
        }
        apr_mmap_delete(mmap_);
        mmap_ = NULL;
    };
    /**
     * ファイルを指定されたサイズに拡張します．
     *
     * @param[in] size サイズ
     */
    void expand(apr_uint64_t size)
    {
        REQUIRE_FILE_OPENDED;

        munmap();
        seek(size - 1);

        if (apr_file_putc('*', file_) != APR_SUCCESS) {
            THROW(MESSAGE_FILE_WRITE_FAILED);
        }
    };
    /**
     * ファイルを指定された位置まで seek します．
     *
     * @param[in] offset オフセット
     */
    void seek(apr_uint64_t offset)
    {
        apr_off_t seek_offset;

        REQUIRE_FILE_OPENDED;

        seek_offset = offset;
        if (apr_file_seek(file_, APR_SET, &seek_offset) != APR_SUCCESS) {
            THROW(MESSAGE_FILE_SEEK_FAILED);
        }
    };
    /**
     * ファイルサイズを取得します．
     *
     * @return ファイルサイズ
     */
    apr_uint64_t get_size()
    {
        bool is_not_opened;
        apr_finfo_t info;

        is_not_opened = (file_ == NULL);
        if (is_not_opened) {
            open(APR_READ);
        }

        if (apr_file_info_get(&info, APR_FINFO_SIZE, file_) != APR_SUCCESS) {
            THROW(MESSAGE_FILE_STAT_FAILED);
        }

        if (is_not_opened) {
            close();
        }

        return info.size;
    };
    /**
     * 更新日時を取得します．
     *
     * @return 更新日時
     */
    apr_time_t get_mtime()
    {
        bool is_not_opened;
        apr_finfo_t info;

        is_not_opened = (file_ == NULL);
        if (is_not_opened) {
            open(APR_READ);
        }

        if (apr_file_info_get(&info, APR_FINFO_MTIME, file_) != APR_SUCCESS) {
            THROW(MESSAGE_FILE_STAT_FAILED);
        }

        if (is_not_opened) {
            close();
        }

        return info.mtime;
    };
    /**
     * ファイルに書き込みます．
     *
     * @param[in] buffer バッファ
     * @param[in] size サイズ
     */
    void write(const void *buffer, apr_size_t *size)
    {
        REQUIRE_FILE_OPENDED;

        if (apr_file_write_full(file_, buffer, *size, size) != APR_SUCCESS) {
            THROW(MESSAGE_FILE_WRITE_FAILED);
        }
    };
    /**
     * ファイルから読み出します．
     *
     * @param[in] buffer バッファ
     * @param[in] size サイズ
     */
    void read(void *buffer, apr_size_t *size)
    {
        REQUIRE_FILE_OPENDED;

        if (apr_file_read(file_, buffer, size) != APR_SUCCESS) {
            THROW(MESSAGE_FILE_READ_FAILED);
        }
    };
    /**
     * ファイルを指定されたサイズに切り詰めます．
     *
     * @param[in] size サイズ
     */
    void trunc(apr_size_t size)
    {
        REQUIRE_FILE_OPENDED;

        munmap();
        if (apr_file_trunc(file_, size) != APR_SUCCESS) {
            THROW(MESSAGE_FILE_TRUNC_FAILED);
        }
    };
    const char *get_path() const
    {
        return file_path_;
    };
    apr_file_t *get_handle()
    {
        apr_file_t *file;

        file = file_;

        // ハンドルの所有権を手放す
        file_ = NULL;

        return file;
    };
    bool is_closed()
    {
        return file_ == NULL;
    };

    /**
     * ファイル/ディレクトリが存在するかどうかを調べます．
     *
     * @param[in] pool プール
     * @param[in] path パス
     */
    static bool is_exist(apr_pool_t *pool, const char *path)
    {
        apr_finfo_t info;
        apr_status_t status;

        status = apr_stat(&info, path, APR_FINFO_MIN, pool);

        return !APR_STATUS_IS_ENOENT(status);
    }

    /**
     * ファイル/ディレクトリが書き込み可能かどうかを調べます．
     *
     * @param[in] pool プール
     * @param[in] path パス
     */
    static bool is_writable(apr_pool_t *pool, const char *path)
    {
        // 本当は info.valid を使って判定した方がいい
#ifdef WIN32
        return is_exist(pool, path);
#else
        apr_finfo_t info;
        apr_status_t status;

        status = apr_stat(&info, path, APR_FINFO_TYPE|APR_FINFO_UPROT, pool);

	if (APR_STATUS_IS_ENOSTAT(status)) {
	    return false;
	}

        if (info.filetype & APR_DIR) { // ディレクトリ
            return ((info.protection & (APR_UREAD|APR_UWRITE|APR_UEXECUTE)) ==
                    (APR_UREAD|APR_UWRITE|APR_UEXECUTE));
        } else {
            return ((info.protection & (APR_UREAD|APR_UWRITE)) ==
                    (APR_UREAD|APR_UWRITE));
        }
#endif
    }

protected:
    /** プール */
    apr_pool_t *pool_;
    /** バッファリングされているかどうか (true: バッファリング有り) */
    bool is_buffered_;
    /** ファイルのパス */
    const char *file_path_;
    /** ファイルハンドル */
    apr_file_t *file_;
    /** MMAP ハンドル */
    apr_mmap_t *mmap_;
    /** 参照カウント */
    int *ref_count_;
};

#undef REQUIRE_FILE_OPENDED

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
