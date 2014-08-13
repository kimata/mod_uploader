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
 * $Id: ApacheRequestReader.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef APACHE_REQUEST_READER_H
#define APACHE_REQUEST_READER_H

#include "httpd.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include "http_protocol.h"
#pragma GCC diagnostic pop
#include "http_request.h"

#include "Environment.h"

#include <cstdlib>

#include "apr_buckets.h"

#include "RequestReader.h"
#include "ApacheResponse.h"


/**
 * @brief Apache で POST リクエストを読み込むクラス．
 */
class ApacheRequestReader: public RequestReader
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] progress 進捗管理オブジェクト
     * @param[in] request リクエスト
     */
    ApacheRequestReader(post_progress_t *progress, ApacheResponse::Handle *request);
    /**
     * POST リクエストを読み込みます．
     *
     * @param[in] buffer 読み込んだデータを書き込むバッファ
     * @param[in] size 読み込むサイズ
     * @param[out] read_size 実際に読み込んだサイズ
     */
    void read(char *buffer, apr_size_t size, apr_size_t *read_size);
    /**
     * デストラクタです．
     *
     * brigade_ を破棄します．
     */
    ~ApacheRequestReader();

private:
    /**
     * リクエストしたサイズの 1/READ_SLEEP_FACTOR しか読み込めなかった
     * 場合に sleep
     */
    static const apr_size_t READ_SLEEP_FACTOR;
    /** sleep する時間(usec) */
    static const apr_size_t READ_SLEEP_USEC;

    /**
     * 新しい brigade を取得します．
     *
     * @exception const char * brigade の取得に失敗した場合
     */
    void get_brigade();

    /** リクエスト */
    ApacheResponse::Handle *request_;
    apr_bucket_brigade *brigade_;
    apr_bucket *bucket_;
    /** 最後まで読み込んだかどうか */
    bool is_eos_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
