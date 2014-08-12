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
 * $Id: UploadItemRss.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef UPLOAD_ITEM_RSS_H
#define UPLOAD_ITEM_RSS_H

#include "Environment.h"

#include <cstdlib>

#include "apr_time.h"

#include "Uncopyable.h"


class UploadItem;
class UploadItemIterator;

/**
 * @brief RSS を出力するクラス，
 */
template<class W>
class UploadItemRss: public Uncopyable
{
public:
    /** ページの Content-type */
    static const char CONTENT_TYPE[];

    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] writer レスポンスを書き出すオブジェクト．
     */
    UploadItemRss(apr_pool_t *pool, W& writer);

    /**
     * ページを出力します．
     *
     * @param[in] pool プール
     * @param[in] base_url トップページの URL
     * @param[in] item_iter アイテムデータ
     * @param[in] mtime 最終更新時刻
     */
    void print(apr_pool_t *pool, const char *base_url,
               UploadItemIterator *item_iter, apr_time_t mtime);
private:
    // エラーメッセージが見にくくなるので，テンプレート引数は W にする
    typedef W                           ResponseWriterClass;

    /** 時刻表記の最大長 */
    static const apr_size_t MAX_TIME_SIZE;
    /** 時刻表記のフォーマット */
    static const char TIME_FORMAT[];
    /** タイムゾーンのオフセット値（±hh:mm） */
    static const apr_int32_t TIME_ZONE_OFFSET;

    /**
     * コメントを出力します．
     *
     * @param[in] pool プール
     * @param[in] base_url トップページの URL
     * @param[in] uitem アイテム
     */
    void print(apr_pool_t *pool, const char *base_url, UploadItem *uitem);
    /**
     * 文字列を出力します．
     *
     * @param[in] str 文字列
     * @param[in] length 文字列の長さ
     */
    void print(const char *str, apr_size_t length);
    /**
     * 文字列を出力します．
     *
     * @param[in] str 文字列
     */
    void print(const char *str);
    /**
     * 数値を出力します．
     *
     * @param[in] i 数値
     */
    void print(apr_size_t i);
    /**
     * 文字列を出力します．
     *
     * @param[in] pool プール
     * @param[in] time 時刻
     * @return フォーマットした時刻
     */
    static const char *time_str(apr_pool_t *pool, apr_time_t time);

    /** プール */
    apr_pool_t *pool_;
    /** レスポンスを書き出すオブジェクト */
    ResponseWriterClass writer_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
