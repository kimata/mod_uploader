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
 * $Id: CharCodeConverter.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef CHAR_CODE_CONVERTER_H
#define CHAR_CODE_CONVERTER_H

#include "Environment.h"

#include <cstdlib>

#include "apr_pools.h"

#include "Uncopyable.h"


/**
 * @brief 文字コードの変換を行うクラス．
 */
class CharCodeConverter: public Uncopyable
{
public:
    /** デフォルトの文字コード */
    static const char DEFAULT_CODE[];

    /**
     * 文字列の文字コードを，デフォルトのものに変換します．
     *
     * 入力文字列の文字コードは，pat の値に基づき自動的に判別されます．
     *
     * @param[in] pool プール
     * @param[in] str 入力文字列
     * @param[in] pat 「京」の 1 バイト目の値
     * @return 変換後の文字列
     * @exception const char * 文字コードを判別できなかった場合および，文字コードの変換に失敗した場合．
     */
    static const char *convert(apr_pool_t *pool, const char *str,
                               const unsigned char *pat);
    /**
     * 入力の文字コードを，to で指令されたもの変換します．
     *
     * @param[in] pool プール
     * @param[in] str 入力文字列
     * @param[in] from 入力文字列の文字コード
     * @param[in] to 出力文字列の文字コード
     * @return 変換後の文字列
     * @exception const char * 文字コードの変換に失敗した場合．
     */
    static const char *convert(apr_pool_t *pool, const char *str,
                               const char *from, const char *to);

private:
    /** 「京」を EUC-JP で表現した場合の 1 バイト目の値 */
    static const unsigned char EUCJP_PAT_FIRST;
    /** 「京」を Shift-JIS で表現した場合の 1 バイト目の値 */
    static const unsigned char SJIS_PAT_FIRST;
    /** 「京」を JIS で表現した場合の 1 バイト目の値 */
    static const unsigned char JIS_PAT_FIRST;
    /** 「京」を UTF-8 で表現した場合の 1 バイト目の値 */
    static const unsigned char UTF8_PAT_FIRST;

    /**
     * 「京」の 1 バイト目の値に基づき，文字コードを判別します．
     *
     * 文字コードが，EUC-JP，Shift-JIS，JIS，UTF-8 のいずれでもない場合，
     * 例外を投げます．
     *
     * @param[in] pat 「京」の 1 バイト目の値
     * @exception const char * 文字コードを判別できなかった場合．
     */
    static const char *identify_code(const unsigned char *pat);
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
