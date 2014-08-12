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
 * $Id: RFC2822Parser.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef RFC2822_PARSER_H
#define RFC2822_PARSER_H

#include "Environment.h"

#include <cstdlib>

#include "apr_pools.h"
#include "apr_tables.h"

#include "Base64FileWriter.h"
#include "MessageDigest5.h"


template<class W> class Base64FileWriter;
template<class R, class W> class MultipartMessageParser;

/**
 * @brief RFC2822 形式の入力をパースするクラス．
 */
template<class R, class W=Base64FileWriter<> >
class RFC2822Parser: public MultipartMessageParser<R, W>
{
public:
    typedef typename MultipartMessageParser<R, W>::content_t        content_t;
    typedef typename MultipartMessageParser<R, W>::file_content_t   file_content_t;

    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] reader POST リクエストを読み込むオブジェクト
     * @param[in] boundary 区切り文字
     * @param[in] file_dir_path 一時ファイルを作成するディレクトリ
     * @param[in] max_text_size テキストの最大サイズ
     * @param[in] max_file_size ファイルの最大サイズ
     * @param[in] max_item_num マルチパートのの最大個数
     * @param[in] file_offset ファイルを書き込む際のオフセット
     */
    RFC2822Parser(apr_pool_t *pool, R& reader,
                  const char *file_dir_path, apr_size_t max_text_size,
                  apr_uint64_t max_file_size, apr_size_t max_item_num,
                  apr_size_t file_offset=0);
    /**
     * マルチパートを全て読み込みます．
     */
    apr_array_header_t *parse();

    static content_t *get_content(apr_array_header_t *content_array,
                                  const char *name)
    {
        return MultipartMessageParser<R, W>::get_content(content_array, name);
    }
    static content_t *get_text_content(apr_array_header_t *content_array,
                                       apr_size_t index=0)
    {
        return MultipartMessageParser<R, W>::get_text_content(content_array,
                                                              index);
    }
    static content_t *get_file_content(apr_array_header_t *content_array,
                                       apr_size_t index=0)
    {
        return MultipartMessageParser<R, W>::get_file_content(content_array,
                                                              index);
    }

private:
    typedef R PostReaderClass;
    typedef W FileWriterClass;

    typedef MultipartMessageParser<R, W>    M;

    static const char MULTIPART_MIXED[];

    /**
     * 次のマルチパートを読み込みます．
     *
     * @param[out] content マルチパートの内容
     */
    void get_content(content_t *content);
    /**
     * ヘッダを読み飛ばします．
     */
    void skip_header();
    /**
     * 先頭にある 1 つのヘッダを返します．
     *
     * @return ヘッダ
     */
    const char *get_one_header();
    /**
     * ヘッダをパースしてマルチパートの区切り文字列を取得します．
     */
    void parse_header();
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
