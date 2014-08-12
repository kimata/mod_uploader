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
 * $Id: ApacheRequestReader.cpp 2883 2008-05-02 12:59:25Z svn $
 *****************************************************************************/

#include "ApacheRequestReader.h"

#include "Environment.h"

#include <cstring>

#include "apr_time.h"

#include "RequestReader.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: ApacheRequestReader.cpp 2883 2008-05-02 12:59:25Z svn $");

const apr_size_t ApacheRequestReader::READ_SLEEP_FACTOR = 16;
const apr_size_t ApacheRequestReader::READ_SLEEP_USEC   = 100000;

////////// Apache 1.x 用 //////////////////////////////////////////////
#if AP_SERVER_MAJORVERSION_NUMBER == 1

/******************************************************************************
 * public メソッド
 *****************************************************************************/
ApacheRequestReader::ApacheRequestReader(post_progress_t *progress,
                                         ApacheResponse::Handle *request)
    : RequestReader(progress),
      request_(request),
      brigade_(NULL),
      bucket_(NULL),
      is_eos_(false)
{

}

void ApacheRequestReader::read(char *buffer, apr_size_t size, apr_size_t *read_size)
{
    if (is_eos_) {
        *read_size = 0;
        return;
    }

    if ((*read_size = ap_get_client_block(request_->r, buffer, static_cast<int>(size))) == 0) {
        is_eos_ = true;
    }
}

ApacheRequestReader::~ApacheRequestReader()
{

}

////////// Apache 2.x 用 //////////////////////////////////////////////
#else

/******************************************************************************
 * public メソッド
 *****************************************************************************/
ApacheRequestReader::ApacheRequestReader(post_progress_t *progress,
                                         ApacheResponse::Handle *request)
    : RequestReader(progress),
      request_(request),
      brigade_(apr_brigade_create(request->pool,
                                  request->connection->bucket_alloc)),
      bucket_(NULL),
      is_eos_(false)
{

}

void ApacheRequestReader::read(char *buffer, apr_size_t size, apr_size_t *read_size)
{
    // apr_brigade_flatten 使うように書き直した方が良いかも

    const char *read_data;

    *read_size = 0;

    if (UNLIKELY(is_eos_)) {
        return;
    }

    if (UNLIKELY(bucket_ == NULL)) {
        get_brigade();
    } else if (UNLIKELY(bucket_ == APR_BRIGADE_SENTINEL(brigade_))) {
        apr_brigade_cleanup(brigade_);
        get_brigade();
    }

    if (UNLIKELY(APR_BUCKET_IS_EOS(bucket_))) {
        is_eos_ = true;
        return;
    }
    if (APR_BUCKET_IS_METADATA(bucket_)) {
        bucket_ = APR_BUCKET_NEXT(bucket_);
        return read(buffer, size, read_size);
    }

    apr_bucket_read(bucket_, &read_data, read_size, APR_BLOCK_READ);
    memcpy(buffer, read_data, *read_size);

    update(*read_size);

    bucket_ = APR_BUCKET_NEXT(bucket_);

    // リクエストしたサイズよりも大幅に小さいサイズしか read できなかっ
    // た場合は少し待つ
    if (*read_size < (size/READ_SLEEP_FACTOR)) {
        apr_sleep(READ_SLEEP_USEC);
    }

    return;
}

ApacheRequestReader::~ApacheRequestReader()
{
    apr_brigade_destroy(brigade_);
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void ApacheRequestReader::get_brigade()
{
    if (UNLIKELY(ap_get_brigade(request_->input_filters, brigade_, AP_MODE_READBYTES,
                                APR_BLOCK_READ, get_block_size()) != APR_SUCCESS)) {
        THROW(MESSAGE_POST_READ_FAILED);
    }

    bucket_ = APR_BRIGADE_FIRST(brigade_);
}

#endif
///////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
