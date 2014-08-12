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
 * $Id: PostDataChecker.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef POST_DATA_CHECKER_H
#define POST_DATA_CHECKER_H

#include "Environment.h"

#include "apr_time.h"

#include <cstdlib>

#include "Uncopyable.h"


/**
 * @brief 書き込みデータのチェックを行うクラス．
 */
class PostDataChecker: public Uncopyable
{
public:
    static void validate_uitem(apr_pool_t *pool, apr_uint64_t file_size,
                               apr_time_t  mtime, const char **file_name,
                               const char **file_mime, const char *file_digest,
                               const char *remove_pass,
                               const char *download_pass,
                               const char **comment, const char *code_pat);
private:
    static const char HTML_EXT[];

    static void validate_file_name(apr_pool_t *pool, const char **file_name,
                                   const char *code_pat);
    static void validate_file_mime(apr_pool_t *pool, const char *file_mime);
    static void validate_comment(apr_pool_t *pool, const char **comment,
                                 const char *code_pat);
    static void validate_file_ext(const char *file_ext);
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
