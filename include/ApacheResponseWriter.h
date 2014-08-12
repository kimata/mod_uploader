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
 * $Id: ApacheResponseWriter.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef APACHE_RESPONSE_WRITER_H
#define APACHE_RESPONSE_WRITER_H

#include "Environment.h"

#include <cstring>

#include "apr_buckets.h"
#include "apr_strings.h"
#define APR_WANT_IOVEC
#include "apr_want.h"

#include "ResponseWriter.h"
#include "ApacheResponse.h"
#include "Message.h"
#include "Macro.h"


typedef struct request_rec request_rec;
typedef struct conn_rec conn_rec;
typedef struct ap_filter_t ap_filter_t;

/**
 * @brief Apache でレスポンスを書き出すクラス．
 */
class ApacheResponseWriter: public ResponseWriter
{
public:
    ApacheResponseWriter(ApacheResponse::Handle *r);
    ApacheResponseWriter(ApacheResponseWriter& writer);
    ~ApacheResponseWriter();
    void write(const char *str, apr_size_t length)
    {
        if (UNLIKELY(length == 0)) {
            return;
        }

        iovec_[iovec_count_].iov_base   = const_cast<char *>(str);
        iovec_[iovec_count_].iov_len    = length;
        iovec_count_++;

        if (UNLIKELY(iovec_count_ == IO_VECTOR_SIZE)) {
            flush();
        }
    };
    void write(const char *str)
    {
        write(str, strlen(str));
    };
    void write(int i)
    {
        write(apr_itoa(pool_, i));
    };
    void finish();

    static bool is_sendfile_enabled(ApacheResponse::Handle *r);
    static int sendfile(ApacheResponse::Handle *r, apr_file_t *file,
                        apr_uint64_t length);

private:
    static const apr_size_t IO_VECTOR_SIZE = RES_IO_VECTOR_SIZE;

    void flush();
    static bool is_mmap_enabled(ApacheResponse::Handle *r);

    conn_rec *connection_;
    apr_bucket_brigade *brigade_;
    ap_filter_t *filter_;

    struct iovec iovec_[IO_VECTOR_SIZE];
    apr_size_t iovec_count_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
