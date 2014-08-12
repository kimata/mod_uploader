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
 * $Id: RequestReader.h 2880 2008-05-01 12:19:40Z svn $
 *****************************************************************************/

#ifndef REQUEST_READER_H
#define REQUEST_READER_H

#include "Environment.h"

#include <cstdlib>

#include "apr_time.h"


/**
 * @brief POST リクエストを読み込むクラス．
 */
class RequestReader
{
public:
    /**
     * POST リクエストの進捗情報
     */
    typedef struct PostProgress {
        /** アップロード ID */
        apr_uint32_t id;
        /** POST リクエストのトータルサイズ */
        apr_uint64_t total_size;
        /** 読み込み済みののトータルサイズ */
        apr_uint64_t read_size;
        /** POST リクエストの完了時刻 */
        apr_time_t end_time;
        /** その他のデータ */
        bool is_succeeded;
        /** アイテム ID */
        apr_size_t item_id;
    } post_progress_t;

    /**
     * コンストラクタです．
     *
     * @param[in] progress 進捗管理オブジェクト
     */
    RequestReader(post_progress_t *progress);
    /**
     * リクエストを読み込む単位をセットします．
     *
     * @param[in] block_size リクエストを読み込む単位
     */
    void set_block_size(apr_size_t block_size);
    /**
     * POST リクエストを読み込みます．
     *
     * @param[in] buffer 読み込んだデータを書き込むバッファ
     * @param[in] size 読み込むサイズ
     * @param[out] read_size 実際に読み込んだサイズ
     */
    virtual void read(char *buffer, apr_size_t size, apr_size_t *read_size) = 0;
    virtual ~RequestReader();

protected:
    void update(apr_size_t read_size);
    apr_size_t get_block_size() const;

private:
    /** リクエストを読み込む単位のデフォルト値 */
    static const apr_size_t DEFAULT_BLOCK_SIZE;

    /**
     * リクエストを読み込む単位
     *
     * この値は目安です．毎回必ずここで指定したサイズを read する必要は
     * ありません．
     */
    apr_size_t block_size_;
    /** 進捗管理オブジェクト */
    post_progress_t *progress_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
