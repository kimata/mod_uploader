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
 * $Id: MessageDigest5.cpp 2868 2008-04-24 15:02:00Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <cstdio>
#include <cstring>

#include "MessageDigest5.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: MessageDigest5.cpp 2868 2008-04-24 15:02:00Z svn $");

#ifdef DEBUG
#define REQUIRE_UPDATE_STATE(state)                             \
    do {                                                        \
        if (!(state)) {                                         \
            THROW(MESSAGE_DIGEST_STATE_INVALID);                \
        }                                                       \
    } while (0)
#else
#define REQUIRE_UPDATE_STATE(state)
#endif

const apr_byte_t MessageDigest5::PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0
};

/******************************************************************************
 * public メソッド
 *****************************************************************************/
MessageDigest5::MessageDigest5()
  : remain_size_(0),
    is_finish_(false)
{
    size_[0]        = 0;
    size_[1]        = 0;

    state_[0]       = 0x67452301;
    state_[1]       = 0xefcdab89;
    state_[2]       = 0x98badcfe;
    state_[3]       = 0x10325476;

    memset(digest_str_, 0, sizeof(digest_str_));
}

void MessageDigest5::update(const apr_byte_t *input, apr_size_t size)
{
    apr_size_t gap;
    apr_size_t read_size;

    REQUIRE_UPDATE_STATE(!is_finish_);

    size_[0] += static_cast<apr_uint32_t>(size << 3);
    if (UNLIKELY(size_[0] < (size << 3))) {
        size_[1]++;
    }
    size_[1] += static_cast<apr_uint32_t>(size >> (32-3));

    gap = 64 - remain_size_;

    if (LIKELY(size >= gap)) {
        read_size = 0;

        memcpy(buffer_+remain_size_, input, gap);
        process(buffer_);

        for (read_size = gap; read_size + 64 <= size; read_size += 64) {
            process(input+read_size);
        }

        memcpy(buffer_, input+read_size, size-read_size);
    } else {
        memcpy(buffer_+remain_size_, input, size);
    }

    remain_size_ = (size_[0] >> 3) & 0x3f;
}

void MessageDigest5::finish()
{
    apr_byte_t input_size[8];
    apr_size_t padding_size;

    REQUIRE_UPDATE_STATE(!is_finish_);

    int2byte(size_, 2, input_size);

    padding_size = (remain_size_ < ((512-64) >> 3))
        ? (((512   - 64) >> 3) - remain_size_)
        : (((512*2 - 64) >> 3) - remain_size_);
    update(PADDING, padding_size);

    update(input_size, 8);

    create_digest();

    is_finish_ = true;
}

const char *MessageDigest5::c_str()
{
    REQUIRE_UPDATE_STATE(is_finish_);

    return digest_str_;
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
inline apr_uint32_t MessageDigest5::calc_f(apr_uint32_t x, apr_uint32_t y,
                                           apr_uint32_t z)
{
    return z ^ (x & (y ^ z));
}

inline apr_uint32_t MessageDigest5::calc_g(apr_uint32_t x, apr_uint32_t y,
                                           apr_uint32_t z)
{
    return (x & z) | (y & ~z);
}

inline apr_uint32_t MessageDigest5::calc_h(apr_uint32_t x, apr_uint32_t y,
                                           apr_uint32_t z)
{
    return x ^ y ^ z;
}

inline apr_uint32_t MessageDigest5::calc_i(apr_uint32_t x, apr_uint32_t y,
                                           apr_uint32_t z)
{
    return y ^ (x | ~z);
}

inline apr_uint32_t MessageDigest5::rotl(apr_uint32_t value,
                                         apr_uint32_t shift)
{
    // GCC は次の式を単一の roll 命令に変換してくれるようです
    return (value << shift) | (value >> (32-shift));
}

inline apr_uint32_t MessageDigest5::round1(apr_uint32_t a, apr_uint32_t b,
                                           apr_uint32_t c, apr_uint32_t d,
                                           apr_uint32_t x, apr_uint32_t s,
                                           apr_uint32_t t)
{
    return b + rotl(a + calc_f(b, c, d) + x + t, s);
}

inline apr_uint32_t MessageDigest5::round2(apr_uint32_t a, apr_uint32_t b,
                                           apr_uint32_t c, apr_uint32_t d,
                                           apr_uint32_t x, apr_uint32_t s,
                                           apr_uint32_t t)
{
    return b + rotl(a + calc_g(b, c, d) + x + t, s);
}

inline apr_uint32_t MessageDigest5::round3(apr_uint32_t a, apr_uint32_t b,
                                           apr_uint32_t c, apr_uint32_t d,
                                           apr_uint32_t x, apr_uint32_t s,
                                           apr_uint32_t t)
{
    return b + rotl(a + calc_h(b, c, d) + x + t, s);
}

inline apr_uint32_t MessageDigest5::round4(apr_uint32_t a, apr_uint32_t b,
                                           apr_uint32_t c, apr_uint32_t d,
                                           apr_uint32_t x, apr_uint32_t s,
                                           apr_uint32_t t)
{
    return b + rotl(a + calc_i(b, c, d) + x + t, s);
}

void MessageDigest5::byte2int(const apr_byte_t *bytes,
                              apr_uint32_t *ints)
{
    for (apr_size_t i = 0; i < 16; i++) {
        ints[i] = ((apr_uint32_t)bytes[i*4    ]) |
                  ((apr_uint32_t)bytes[i*4 + 1] <<  8) |
                  ((apr_uint32_t)bytes[i*4 + 2] << 16) |
                  ((apr_uint32_t)bytes[i*4 + 3] << 24);
    }
}

void MessageDigest5::int2byte(const apr_uint32_t *ints, apr_size_t size,
                              apr_byte_t *bytes)
{
    for (apr_size_t i = 0; i < size; i++) {
        bytes[i*4    ] = (apr_byte_t)((ints[i]      ) & 0xff);
        bytes[i*4 + 1] = (apr_byte_t)((ints[i] >>  8) & 0xff);
        bytes[i*4 + 2] = (apr_byte_t)((ints[i] >> 16) & 0xff);
        bytes[i*4 + 3] = (apr_byte_t)((ints[i] >> 24) & 0xff);
    }
}

void MessageDigest5::process(const apr_byte_t block[64])
{
    apr_uint32_t a, b, c, d;
    apr_uint32_t x[16];

    a = state_[0];
    b = state_[1];
    c = state_[2];
    d = state_[3];

    byte2int(block, x);

    a = round1(a, b, c, d, x[ 0],  7, 0xd76aa478U);
    d = round1(d, a, b, c, x[ 1], 12, 0xe8c7b756U);
    c = round1(c, d, a, b, x[ 2], 17, 0x242070dbU);
    b = round1(b, c, d, a, x[ 3], 22, 0xc1bdceeeU);
    a = round1(a, b, c, d, x[ 4],  7, 0xf57c0fafU);
    d = round1(d, a, b, c, x[ 5], 12, 0x4787c62aU);
    c = round1(c, d, a, b, x[ 6], 17, 0xa8304613U);
    b = round1(b, c, d, a, x[ 7], 22, 0xfd469501U);
    a = round1(a, b, c, d, x[ 8],  7, 0x698098d8U);
    d = round1(d, a, b, c, x[ 9], 12, 0x8b44f7afU);
    c = round1(c, d, a, b, x[10], 17, 0xffff5bb1U);
    b = round1(b, c, d, a, x[11], 22, 0x895cd7beU);
    a = round1(a, b, c, d, x[12],  7, 0x6b901122U);
    d = round1(d, a, b, c, x[13], 12, 0xfd987193U);
    c = round1(c, d, a, b, x[14], 17, 0xa679438eU);
    b = round1(b, c, d, a, x[15], 22, 0x49b40821U);
    a = round2(a, b, c, d, x[ 1],  5, 0xf61e2562U);
    d = round2(d, a, b, c, x[ 6],  9, 0xc040b340U);
    c = round2(c, d, a, b, x[11], 14, 0x265e5a51U);
    b = round2(b, c, d, a, x[ 0], 20, 0xe9b6c7aaU);
    a = round2(a, b, c, d, x[ 5],  5, 0xd62f105dU);
    d = round2(d, a, b, c, x[10],  9, 0x02441453U);
    c = round2(c, d, a, b, x[15], 14, 0xd8a1e681U);
    b = round2(b, c, d, a, x[ 4], 20, 0xe7d3fbc8U);
    a = round2(a, b, c, d, x[ 9],  5, 0x21e1cde6U);
    d = round2(d, a, b, c, x[14],  9, 0xc33707d6U);
    c = round2(c, d, a, b, x[ 3], 14, 0xf4d50d87U);
    b = round2(b, c, d, a, x[ 8], 20, 0x455a14edU);
    a = round2(a, b, c, d, x[13],  5, 0xa9e3e905U);
    d = round2(d, a, b, c, x[ 2],  9, 0xfcefa3f8U);
    c = round2(c, d, a, b, x[ 7], 14, 0x676f02d9U);
    b = round2(b, c, d, a, x[12], 20, 0x8d2a4c8aU);
    a = round3(a, b, c, d, x[ 5],  4, 0xfffa3942U);
    d = round3(d, a, b, c, x[ 8], 11, 0x8771f681U);
    c = round3(c, d, a, b, x[11], 16, 0x6d9d6122U);
    b = round3(b, c, d, a, x[14], 23, 0xfde5380cU);
    a = round3(a, b, c, d, x[ 1],  4, 0xa4beea44U);
    d = round3(d, a, b, c, x[ 4], 11, 0x4bdecfa9U);
    c = round3(c, d, a, b, x[ 7], 16, 0xf6bb4b60U);
    b = round3(b, c, d, a, x[10], 23, 0xbebfbc70U);
    a = round3(a, b, c, d, x[13],  4, 0x289b7ec6U);
    d = round3(d, a, b, c, x[ 0], 11, 0xeaa127faU);
    c = round3(c, d, a, b, x[ 3], 16, 0xd4ef3085U);
    b = round3(b, c, d, a, x[ 6], 23, 0x04881d05U);
    a = round3(a, b, c, d, x[ 9],  4, 0xd9d4d039U);
    d = round3(d, a, b, c, x[12], 11, 0xe6db99e5U);
    c = round3(c, d, a, b, x[15], 16, 0x1fa27cf8U);
    b = round3(b, c, d, a, x[ 2], 23, 0xc4ac5665U);
    a = round4(a, b, c, d, x[ 0],  6, 0xf4292244U);
    d = round4(d, a, b, c, x[ 7], 10, 0x432aff97U);
    c = round4(c, d, a, b, x[14], 15, 0xab9423a7U);
    b = round4(b, c, d, a, x[ 5], 21, 0xfc93a039U);
    a = round4(a, b, c, d, x[12],  6, 0x655b59c3U);
    d = round4(d, a, b, c, x[ 3], 10, 0x8f0ccc92U);
    c = round4(c, d, a, b, x[10], 15, 0xffeff47dU);
    b = round4(b, c, d, a, x[ 1], 21, 0x85845dd1U);
    a = round4(a, b, c, d, x[ 8],  6, 0x6fa87e4fU);
    d = round4(d, a, b, c, x[15], 10, 0xfe2ce6e0U);
    c = round4(c, d, a, b, x[ 6], 15, 0xa3014314U);
    b = round4(b, c, d, a, x[13], 21, 0x4e0811a1U);
    a = round4(a, b, c, d, x[ 4],  6, 0xf7537e82U);
    d = round4(d, a, b, c, x[11], 10, 0xbd3af235U);
    c = round4(c, d, a, b, x[ 2], 15, 0x2ad7d2bbU);
    b = round4(b, c, d, a, x[ 9], 21, 0xeb86d391U);

    state_[0] += a;
    state_[1] += b;
    state_[2] += c;
    state_[3] += d;
}

void MessageDigest5::create_digest()
{
    int2byte(state_, 4, digest_bytes_);

    snprintf(digest_str_, sizeof(digest_str_),
             "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
             digest_bytes_[ 0], digest_bytes_[ 1], digest_bytes_[ 2],
             digest_bytes_[ 3], digest_bytes_[ 4], digest_bytes_[ 5],
             digest_bytes_[ 6], digest_bytes_[ 7], digest_bytes_[ 8],
             digest_bytes_[ 9], digest_bytes_[10], digest_bytes_[11],
             digest_bytes_[12], digest_bytes_[13], digest_bytes_[14],
             digest_bytes_[15]);
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_MessageDigest5
#include "TestRunner.h"

static const char TEST_DATA[]   = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
static const char TEST_DIGEST[] = "57edf4a22be3c955ac49da2e2107b67a";

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << endl;
}

void run_digest()
{
    show_test_name("digest");

    MessageDigest5 digest;
    digest.update(reinterpret_cast<const apr_byte_t *>(TEST_DATA),
                  strlen(TEST_DATA));
    digest.finish();

    show_item("input", TEST_DATA);
    show_item("digest", digest.c_str());
    show_item("expected", TEST_DIGEST);

    if (strcmp(digest.c_str(), TEST_DIGEST) != 0) {
        THROW(MESSAGE_BUG_FOUND);
    }

    show_spacer();
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    if (argc != 1) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    show_line();
    run_digest();
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
