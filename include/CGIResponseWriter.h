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
 * $Id: CGIResponseWriter.h 2895 2008-07-15 16:39:11Z svn $
 *****************************************************************************/

#ifndef CGI_RESPONSE_WRITER_H
#define CGI_RESPONSE_WRITER_H

#include "Environment.h"

#include "apr_file_io.h"
#if APR_HAVE_SYS_UIO_H
#define APR_WANT_IOVEC
#include "apr_want.h"
#else
#include <iostream>
#endif

#include "ResponseWriter.h"
#include "CGIResponse.h"


/**
 * @brief コンソールにでレスポンスを書き出すクラス．
 */
class CGIResponseWriter: public ResponseWriter
{
public:
    CGIResponseWriter(CGIResponse::Handle *r)
        : iovec_count_(0)
    {
        pool_ = r->pool;
    };
    CGIResponseWriter(apr_pool_t *pool)
        : iovec_count_(0)
    {
        pool_ = pool;
    };

    ~CGIResponseWriter()
    {
        flush();
    };
    void write(const char *str, apr_size_t length)
    {
#if APR_HAVE_SYS_UIO_H
        iovec_[iovec_count_].iov_base   = const_cast<char *>(str);
        iovec_[iovec_count_].iov_len    = length;
        iovec_count_++;

        if (UNLIKELY(iovec_count_ == IO_VECTOR_SIZE)) {
            flush();
        }
#else
        std::cout.write(str, static_cast<std::streamsize>(length));
#endif
    };
    void write(const char *str)
    {
        write(str, strlen(str));
    };
    void write(int i)
    {
        write(apr_itoa(pool_, i));
    };
    void finish() const
    {
        std::cout.flush();
    };
    void flush()
    {
        if (iovec_count_ == 0) {
            return;
        }

#if APR_HAVE_SYS_UIO_H
        writev(STDOUT_FILENO, iovec_, iovec_count_);
#endif
        iovec_count_ = 0;
    };
    static bool is_sendfile_enabled(CGIResponse::Handle *r){
        return false;
    };
    static int sendfile(CGIResponse::Handle *r,
                        apr_file_t *file, apr_uint64_t length)
    {
        char buffer[SYS_PAGE_SIZE];

        for (apr_size_t read_size = sizeof(buffer);
             apr_file_read(file, buffer, &read_size) == APR_SUCCESS;) {
            std::cout.write(buffer, static_cast<std::streamsize>(read_size));
        }

        return OK;
    };

private:
    static const apr_size_t IO_VECTOR_SIZE = RES_IO_VECTOR_SIZE;

    struct iovec iovec_[IO_VECTOR_SIZE];
    apr_size_t iovec_count_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
