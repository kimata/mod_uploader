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
 * $Id: MessageDigest5.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef MESSAGE_DIGEST5_H
#define MESSAGE_DIGEST5_H

#include "Environment.h"

#include <cstdlib>

#include "apr.h"

#include "Uncopyable.h"


/**
 * @brief MD5 を計算するクラス．
 */
class MessageDigest5: public Uncopyable
{
public:
    MessageDigest5();
    void update(const apr_byte_t *input, apr_size_t size);
    void finish();
    const char *c_str();

private:
    /** 最後のパディング用データ（0x8000_0000_0000_0000） */
    static const apr_byte_t PADDING[64];

    apr_uint32_t calc_f(apr_uint32_t x, apr_uint32_t y, apr_uint32_t z);
    apr_uint32_t calc_g(apr_uint32_t x, apr_uint32_t y, apr_uint32_t z);
    apr_uint32_t calc_h(apr_uint32_t x, apr_uint32_t y, apr_uint32_t z);
    apr_uint32_t calc_i(apr_uint32_t x, apr_uint32_t y, apr_uint32_t z);
    apr_uint32_t rotl(apr_uint32_t value, apr_uint32_t shift);
    apr_uint32_t round1(apr_uint32_t a, apr_uint32_t b, apr_uint32_t c,
                        apr_uint32_t d, apr_uint32_t x, apr_uint32_t s,
                        apr_uint32_t t);
    apr_uint32_t round2(apr_uint32_t a, apr_uint32_t b, apr_uint32_t c,
                        apr_uint32_t d, apr_uint32_t x, apr_uint32_t s,
                        apr_uint32_t t);
    apr_uint32_t round3(apr_uint32_t a, apr_uint32_t b, apr_uint32_t c,
                        apr_uint32_t d, apr_uint32_t x, apr_uint32_t s,
                        apr_uint32_t t);
    apr_uint32_t round4(apr_uint32_t a, apr_uint32_t b, apr_uint32_t c,
                        apr_uint32_t d, apr_uint32_t x, apr_uint32_t s,
                        apr_uint32_t t);
    /**
     * リトルエンディアンのバイト列を apr_uint32_t の配列に変換します．
     *
     * バイト列の長さは，64 固定です．
     *
     * @param[in] bytes バイト列
     * @param[out] ints apr_uint32_t の配列
     */
    void byte2int(const apr_byte_t *bytes, apr_uint32_t *ints);
    /**
     * apr_uint32_t の配列をリトルエンディアンのバイト列に変換します．
     *
     * @param[in] ints apr_uint32_t の配列
     * @param[in] size apr_uint32_t の配列の長さ
     * @param[out] bytes バイト列
     */
    void int2byte(const apr_uint32_t *ints, apr_size_t size,
                  apr_byte_t *bytes);
    void process(const apr_byte_t block[64]);
    /** ハッシュ値を算出します． */
    void create_digest();

    apr_uint32_t state_[4];
    /** 入力の bit サイズ */
    apr_uint32_t size_[2];
    apr_byte_t buffer_[64];
    /** バッファに残っているサイズ */
    apr_size_t remain_size_;
    /** 既にハッシュ値を算出しているかどうか */
    bool is_finish_;
    /** ハッシュ値のバイト表現 */
    apr_byte_t digest_bytes_[16];
    /** ハッシュ値の文字列 */
    char digest_str_[33];
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
