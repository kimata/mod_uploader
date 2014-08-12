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
 * $Id: PostProgressList.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef POST_PROGRESS_LIST_H
#define POST_PROGRESS_LIST_H

#include "Environment.h"

#include <cstdlib>

#include "apr_shm.h"
#include "apr_time.h"
#include "AtomicWrapper.h"

#include "RequestReader.h"
#include "Uncopyable.h"


/**
 * @brief リクエストの進捗情報のリストを表すクラス．
 */
class PostProgressList: public Uncopyable
{
public:
    static const apr_time_t ENTRY_UNUSED;
    static const apr_time_t ENTRY_USED;

    PostProgressList();
    RequestReader::post_progress_t *get(apr_uint32_t upload_id);
    RequestReader::post_progress_t *add(apr_uint32_t upload_id);

    static PostProgressList *get_instance(apr_shm_t *shm);
    static apr_size_t get_memory_size();

    static void dump_list(PostProgressList *progress_list);

private:
    static const apr_uint32_t ID_DUMMY;

    static void dump_progress(RequestReader::post_progress_t *progress);

    apr_atomic_t lock_;
    RequestReader::post_progress_t progress_list_[DLD_MAX_SESSION_COUNT];
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
