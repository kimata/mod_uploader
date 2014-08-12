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
 * $Id: PostFlowController.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef POST_FLOW_CONTROLLER_H
#define POST_FLOW_CONTROLLER_H

#include "Environment.h"

#include <cstdlib>

#include "apr_shm.h"
#include "apr_time.h"
#include "apr_network_io.h"
#include "AtomicWrapper.h"

#include "FlowController.h"
#include "Macro.h"
#include "Uncopyable.h"


/**
 * @brief 書き込みのフロー制御を行うクラス．
 */
class PostFlowController: public FlowController
{
public:
    PostFlowController(apr_size_t min_post_interval_sec);
    bool can_post(apr_sockaddr_t *sockaddr);
    void regist_post(apr_sockaddr_t *sockaddr);
    static PostFlowController *get_instance(apr_shm_t *shm,
                                            apr_size_t min_post_interval_sec);
    static apr_size_t get_memory_size();

    static const char *dump_list(apr_pool_t *pool,
                                 PostFlowController *flow_controller);

private:
    typedef struct Poster {
        char paddress[MAX_PADDRESS_SIZE]; // パスカル文字列
        apr_time_t time;
    } poster_t;

    void poster_list_add(apr_sockaddr_t *sockaddr);
    void poster_list_clean();
    bool poster_list_is_contain(apr_sockaddr_t *sockaddr,
                                apr_time_t time_limit) const;

    static const char *dump_poster(apr_pool_t *pool, poster_t *poster);

    apr_atomic_t lock_;
    apr_time_t min_post_interval_time_;
    apr_size_t poster_top_;
    apr_size_t poster_tail_;
    poster_t poster_list_[PST_FLOW_CONTROL_BUFFER_SIZE];
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
