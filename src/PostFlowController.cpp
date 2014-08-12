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
 * $Id: PostFlowController.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include "apr_strings.h"

#include "PostFlowController.h"
#include "ReadLocker.h"
#include "WriteLocker.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: PostFlowController.cpp 2756 2007-12-11 10:57:59Z svn $");

/******************************************************************************
 * public メソッド
 *****************************************************************************/
PostFlowController::PostFlowController(apr_size_t min_post_interval_sec)
  : min_post_interval_time_(apr_time_from_sec(min_post_interval_sec)),
    poster_top_(0),
    poster_tail_(0)
{
    (void)apr_atomic_set(&lock_, ReadWriteLocker::NOT_LOCKED);
}

bool PostFlowController::can_post(apr_sockaddr_t *sockaddr)
{
    apr_time_t time_limit;
    ReadLocker lock(&lock_);

    // MEMO: 短時間の間に異なる ARRAY_SIZE_OF(poster_list_) 個の IP ア
    // ドレスから投稿があった場合，制限に漏れが生じる．

    time_limit = apr_time_now() - min_post_interval_time_;

    return poster_list_is_contain(sockaddr, time_limit);
}

void PostFlowController::regist_post(apr_sockaddr_t *sockaddr)
{
    WriteLocker lock(&lock_);

    poster_list_add(sockaddr);
}

PostFlowController *PostFlowController::get_instance(apr_shm_t *shm,
                                                     apr_size_t min_post_interval_sec)
{
    PostFlowController *flow_controller;

#ifdef DEBUG
    if (apr_shm_size_get(shm) != get_memory_size()) {
        THROW(MESSAGE_SHM_SIZE_INVALID);
    }
#endif

    flow_controller =
        reinterpret_cast<PostFlowController *>(apr_shm_baseaddr_get(shm));
    new(flow_controller) PostFlowController(min_post_interval_sec);

    return flow_controller;
}

apr_size_t PostFlowController::get_memory_size()
{
    return sizeof(PostFlowController);
}

const char *PostFlowController::dump_list(apr_pool_t *pool,
                                          PostFlowController *flow_controller)
{
    apr_size_t i;
    const char *dump;

    if (flow_controller->poster_top_ == flow_controller->poster_tail_) {
        return "";
    }

    i = flow_controller->poster_top_;
    dump = "";
    do {
        dump = apr_pstrcat
            (pool, dump, dump_poster(pool, flow_controller->poster_list_ + i),
             NULL);
    } while ((i = (i == 0)
              ? (ARRAY_SIZE_OF(flow_controller->poster_list_)-1)
              : (i-1)) != flow_controller->poster_tail_);

    return dump;
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void PostFlowController::poster_list_add(apr_sockaddr_t *sockaddr)
{
    poster_top_++;
    if (poster_top_ == ARRAY_SIZE_OF(poster_list_)) {
        poster_top_ = 0;
    }
    if (poster_top_ == poster_tail_) {
        poster_tail_++;
        if (poster_tail_ == ARRAY_SIZE_OF(poster_list_)) {
            poster_tail_ = 0;
        }
    }

    strncpy(poster_list_[poster_top_].paddress, get_paddress(sockaddr),
            MAX_PADDRESS_SIZE - 1);
    poster_list_[poster_top_].time = apr_time_now();
}

void PostFlowController::poster_list_clean()
{
    apr_time_t current_time;
    apr_size_t i;

    current_time = apr_time_now();
    for (i = poster_tail_; i != poster_top_;
         i = (i == (ARRAY_SIZE_OF(poster_list_)-1)) ? 0 : (i+1)) {
        if ((current_time - poster_list_[i].time) <= min_post_interval_time_) {
            break;
        }
    }

    poster_tail_ = i;
}

bool PostFlowController::poster_list_is_contain(apr_sockaddr_t *sockaddr,
                                                apr_time_t time_limit) const
{
    const char *paddress;
    apr_size_t i;

    if (poster_top_ == poster_tail_) {
        return true;
    }

    paddress = get_paddress(sockaddr);

    i = poster_top_;
    do {
        if (poster_list_[i].time < time_limit) {
            return true;
        }
        if (is_address_match(poster_list_[i].paddress, paddress)) {
            return false;
        }
    } while ((i = (i == 0)
              ? (ARRAY_SIZE_OF(poster_list_)-1)
              : (i-1)) != poster_tail_);

    return true;
}

const char *PostFlowController::dump_poster(apr_pool_t *pool, poster_t *poster)
{
    return apr_psprintf(pool, "%24s : %" APR_TIME_T_FMT "\n",
                        &(poster->paddress[1]), poster->time);
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_PostFlowController
#include "TestRunner.h"

const char DUMMY_ADDRESS[]              = "127.0.0.1";
const apr_time_t MIN_POST_INTERVAL_SEC  = 1;
const apr_size_t ATTACK_COUNT           = 100;

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << endl;
}

void run_can_post(PostFlowController *flow_controller,
                  apr_sockaddr_t *sockaddr)
{
    apr_size_t i;

    show_test_name("can_post");

    flow_controller->regist_post(sockaddr);

    for (i = 0; i < ATTACK_COUNT; i++) {
        if (flow_controller->can_post(sockaddr)) {
            break;
        }
        apr_sleep(100000);
    }

    if ((i == 0) || (i == ATTACK_COUNT)) {
        THROW(MESSAGE_BUG_FOUND);
    }
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    apr_sockaddr_t *sockaddr;

    if (argc != 1) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    PostFlowController flow_controller(MIN_POST_INTERVAL_SEC);
    show_line();

    sockaddr = get_sockaddr(pool, DUMMY_ADDRESS);

    run_can_post(&flow_controller, sockaddr);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
