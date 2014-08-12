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
 * $Id: StandRequestReader.h 2764 2007-12-16 12:49:51Z svn $
 *****************************************************************************/

#ifndef STAND_REQUEST_READER_H
#define STAND_REQUEST_READER_H

#include "Environment.h"

#include <iostream>

#include "apr.h"

#include "RequestReader.h"


/**
 * @brief スタンドアローン版での POST リクエストを読み込むクラス．
 */
class StandRequestReader : public RequestReader
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] progress 進捗管理オブジェクト
     * @param[in] r リクエスト
     * @param[in] stream 入力ストリーム
     */
    StandRequestReader(post_progress_t *progress, void *r, std::istream *stream=&std::cin);
    /**
     * POST リクエストを読み込みます．
     *
     * @param[in] buffer 読み込んだデータを書き込むバッファ
     * @param[in] size 読み込むサイズ
     * @param[out] read_size 実際に読み込んだサイズ
     */
    void read(char *buffer, apr_size_t size, apr_size_t *read_size);
private:
    std::istream *stream_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
