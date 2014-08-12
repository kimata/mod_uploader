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
 * $Id: ApacheResponseWriter.cpp 2883 2008-05-02 12:59:25Z svn $
 *****************************************************************************/

// Apache の config.h を先に処理させるため，util_filter.h の
// インクルードはこの位置
#include "httpd.h"
#include "http_protocol.h"

#include "Environment.h"

#if AP_SERVER_MAJORVERSION_NUMBER != 1
#include "util_filter.h"
#endif

#include "http_config.h"
#define CORE_PRIVATE
#include "http_core.h"

#include "ApacheResponseWriter.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: ApacheResponseWriter.cpp 2883 2008-05-02 12:59:25Z svn $");

////////// Apache 1.x 用 //////////////////////////////////////////////
#if AP_SERVER_MAJORVERSION_NUMBER == 1

/******************************************************************************
 * public メソッド
 ****************************************************************************/
ApacheResponseWriter::ApacheResponseWriter(ApacheResponse::Handle *r)
  : connection_(r->r->connection),
    brigade_(reinterpret_cast<apr_bucket_brigade *>(r)), // :-)
    filter_(NULL),
    iovec_count_(0)
{
    pool_ = r->pool;
}

ApacheResponseWriter::ApacheResponseWriter(ApacheResponseWriter& writer)
  : connection_(writer.connection_),
    brigade_(writer.brigade_),
    filter_(writer.filter_),
    iovec_count_(writer.iovec_count_)
{
    pool_ = writer.pool_;
}

ApacheResponseWriter::~ApacheResponseWriter()
{
    flush();
}

void ApacheResponseWriter::finish()
{
    flush();
}

bool ApacheResponseWriter::is_sendfile_enabled(ApacheResponse::Handle *r)
{
    return false;
}

int ApacheResponseWriter::sendfile(ApacheResponse::Handle *r, apr_file_t *file,
                                   apr_uint64_t length)
{
    apr_mmap_t *mmap;
    TemporaryPool temp_pool;

    if (apr_mmap_create(&mmap, file, 0,
                        static_cast<apr_size_t>(length), APR_MMAP_READ,
                        temp_pool.get()) != APR_SUCCESS) {
        THROW(MESSAGE_FILE_MMAP_FAILED);
    }

    ap_send_mmap(mmap->mm, r->r, 0, length);

    return OK;
}

/******************************************************************************
 * private メソッド
 ****************************************************************************/
void ApacheResponseWriter::flush()
{
    if (UNLIKELY((iovec_count_ == 0) || connection_->aborted)) {
        iovec_count_ = 0;
        return;
    }

    for (apr_size_t i = 0; i < iovec_count_; i++) {
        ap_rwrite(iovec_[i].iov_base,
                  static_cast<int>(iovec_[i].iov_len),
                  reinterpret_cast<ApacheResponse::Handle *>(brigade_)->r); // :-)
    }

    iovec_count_ = 0;
}

////////// Apache 2.x 用 //////////////////////////////////////////////
#else

/******************************************************************************
 * public メソッド
 ****************************************************************************/
ApacheResponseWriter::ApacheResponseWriter(ApacheResponse::Handle *r)
  : connection_(r->connection),
    brigade_(apr_brigade_create(r->pool, r->connection->bucket_alloc)),
    filter_(r->output_filters),
    iovec_count_(0)
{
    pool_ = r->pool;
}

ApacheResponseWriter::ApacheResponseWriter(ApacheResponseWriter& writer)
  : connection_(writer.connection_),
    brigade_(writer.brigade_),
    filter_(writer.filter_),
    iovec_count_(writer.iovec_count_)
{
    pool_ = writer.pool_;
}

ApacheResponseWriter::~ApacheResponseWriter()
{
    flush();
}

void ApacheResponseWriter::finish()
{
    apr_bucket *b;

    flush();

    b = apr_bucket_eos_create(connection_->bucket_alloc);
    APR_BRIGADE_INSERT_TAIL(brigade_, b);
    ap_filter_flush(brigade_, filter_);
}

bool ApacheResponseWriter::is_sendfile_enabled(ApacheResponse::Handle *r)
{
#if APR_HAS_SENDFILE
    core_dir_config *core_config;

    core_config = reinterpret_cast<core_dir_config *>
        (ap_get_module_config(r->per_dir_config, &core_module));

    return (core_config->enable_sendfile != ENABLE_SENDFILE_OFF);
#else
    return false;
#endif
}

int ApacheResponseWriter::sendfile(ApacheResponse::Handle *r, apr_file_t *file,
                                   apr_uint64_t length)
{
    conn_rec *connection;
    apr_bucket_brigade *brigade;
    apr_bucket *file_bucket;
    apr_bucket *bucket;
    apr_bucket_alloc_t *bucket_alloc;

    connection = r->connection;

    bucket_alloc = connection->bucket_alloc;
    brigade = apr_brigade_create(r->pool, bucket_alloc);

    if ((sizeof(apr_off_t) > sizeof(apr_size_t)) &&
        (length > AP_MAX_SENDFILE)) {
        file_bucket = apr_bucket_file_create(file, 0, AP_MAX_SENDFILE, r->pool,
                                             bucket_alloc);

        while (length > AP_MAX_SENDFILE) {
            apr_bucket_copy(file_bucket, &bucket);
            APR_BRIGADE_INSERT_TAIL(brigade, bucket);

            file_bucket->start += AP_MAX_SENDFILE;
            length -= AP_MAX_SENDFILE;
        }
        file_bucket->length = static_cast<apr_size_t>(length);
    } else {
        file_bucket = apr_bucket_file_create(file, 0,
                                             static_cast<apr_size_t>(length),
                                             r->pool, bucket_alloc);
    }

#if APR_HAS_MMAP
    if (!is_mmap_enabled(r)) {
        apr_bucket_file_enable_mmap(file_bucket, 0);
    }
#endif
    APR_BRIGADE_INSERT_TAIL(brigade, file_bucket);

    bucket = apr_bucket_eos_create(bucket_alloc);
    APR_BRIGADE_INSERT_TAIL(brigade, bucket);

    if (ap_pass_brigade(r->output_filters, brigade) != APR_SUCCESS) {
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    return OK;
}


/******************************************************************************
 * private メソッド
 ****************************************************************************/
void ApacheResponseWriter::flush()
{
    if (UNLIKELY((iovec_count_ == 0) || connection_->aborted)) {
        iovec_count_ = 0;
        return;
    }

    apr_brigade_writev(brigade_, ap_filter_flush, filter_,
                       iovec_, iovec_count_);

    iovec_count_ = 0;
}

bool ApacheResponseWriter::is_mmap_enabled(ApacheResponse::Handle *r)
{
#if APR_HAS_MMAP
    core_dir_config *core_config;

    core_config = reinterpret_cast<core_dir_config *>
        (ap_get_module_config(r->per_dir_config, &core_module));

    return (core_config->enable_mmap != ENABLE_MMAP_OFF);
#else
    return false;
#endif
}

#endif
///////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
