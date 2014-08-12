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
 * $Id: DownloadFlowController.cpp 2813 2008-03-07 14:35:04Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <iomanip>

#include "apr_strings.h"

#include "DownloadFlowController.h"
#include "WriteLocker.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: DownloadFlowController.cpp 2813 2008-03-07 14:35:04Z svn $");

const apr_size_t DownloadSession::DOWNLOAD_BUSY_SLEEP_SEC   = 5;


/******************************************************************************
 * public メソッド
 *****************************************************************************/
DownloadFlowController::DownloadFlowController()
  : downloader_count_(0)
{
    memset(downloader_list_, 0, sizeof(downloader_list_));
    (void)apr_atomic_set(&lock_, ReadWriteLocker::NOT_LOCKED);
}

bool DownloadFlowController::add_session(apr_sockaddr_t *sockaddr)
{
    downloader_t *downloader;
    WriteLocker lock(&lock_);

    if (downloader_count_ == DLD_MAX_SESSION_COUNT) {
        return false;
    }

    downloader = downloader_list_get(sockaddr);
    if (downloader == NULL) {
        return downloader_list_add(sockaddr);
    } else {
        if (downloader->session_count == DLD_MAX_SESSION_COUNT_PER_IP) {
            return false;
        } else {
            downloader_count_++;
            downloader->session_count++;
            return true;
        }
    }
}

void DownloadFlowController::remove_session(apr_sockaddr_t *sockaddr)
{
    downloader_t *downloader;
    WriteLocker lock(&lock_);

    downloader = downloader_list_get(sockaddr);

    if (downloader == NULL) {
        THROW(MESSAGE_BUG_FOUND);
    }

    if (downloader->session_count == 0) {
        THROW(MESSAGE_BUG_FOUND);
    }
    downloader->session_count--;

    if (downloader_count_ == 0) {
        THROW(MESSAGE_BUG_FOUND);
    }
    downloader_count_--;
}

DownloadFlowController *DownloadFlowController::get_instance(apr_shm_t *shm)
{
    DownloadFlowController *flow_controller;

#ifdef DEBUG
    if (apr_shm_size_get(shm) != get_memory_size()) {
        THROW(MESSAGE_SHM_SIZE_INVALID);
    }
#endif

    flow_controller =
        reinterpret_cast<DownloadFlowController *>(apr_shm_baseaddr_get(shm));
    new(flow_controller) DownloadFlowController;

    return flow_controller;
}

apr_size_t DownloadFlowController::get_memory_size()
{
    return sizeof(DownloadFlowController);
}

const char *DownloadFlowController::dump_list(apr_pool_t *pool,
                                              DownloadFlowController *flow_controller)
{
    const char *dump;

    dump = apr_psprintf(pool, "%-24s : %" APR_SIZE_T_FMT "\n",
                        "total count", flow_controller->downloader_count_);
    for (apr_size_t i = 0; i < DLD_MAX_SESSION_COUNT; i++) {
        if (flow_controller->downloader_list_[i].session_count == 0) {
            continue;
        }
        dump = apr_pstrcat
            (pool, dump,
             dump_downloader(pool,
                             flow_controller->downloader_list_ + i),
             NULL);
    }

    return dump;
}

DownloadSession::DownloadSession(DownloadFlowController *flow_controller,
                                 apr_sockaddr_t *sockaddr)
  : flow_controller_(flow_controller),
    sockaddr_(sockaddr)
{
    is_valid_ = flow_controller_->add_session(sockaddr_);

    if (!is_valid_) {
        apr_sleep(apr_time_from_sec(DOWNLOAD_BUSY_SLEEP_SEC));
    }
}

DownloadSession::~DownloadSession()
{
    if (is_valid_) {
        flow_controller_->remove_session(sockaddr_);
    }
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
bool DownloadFlowController::downloader_list_add(apr_sockaddr_t *sockaddr)
{
    apr_size_t i;

    if (downloader_count_ == DLD_MAX_SESSION_COUNT) {
        return false;
    }

    for (i = 0; i < ARRAY_SIZE_OF(downloader_list_); i++) {
        if (downloader_list_[i].session_count == 0) {
            break;
        }
    }

    if (i == ARRAY_SIZE_OF(downloader_list_)) {
        THROW(MESSAGE_BUG_FOUND);
    }

    strncpy(downloader_list_[i].paddress, get_paddress(sockaddr),
            MAX_PADDRESS_SIZE - 1);
    downloader_list_[i].session_count = 1;
    downloader_count_++;

    return true;
}

DownloadFlowController::downloader_t *
DownloadFlowController::downloader_list_get(apr_sockaddr_t *sockaddr)
{
    const char *paddress;

    paddress = get_paddress(sockaddr);

    for (apr_size_t i = 0; i < ARRAY_SIZE_OF(downloader_list_); i++) {
        if (is_address_match(downloader_list_[i].paddress, paddress)) {
            if (downloader_list_[i].session_count == 0) {
                continue;
            }
            return (downloader_list_ + i);
        }
    }

    return NULL;
}

const char *DownloadFlowController::dump_downloader(apr_pool_t *pool,
                                                    downloader_t *downloader)
{
    return apr_psprintf(pool, "%24s : %" APR_SIZE_T_FMT "\n",
                        &(downloader->paddress[1]), downloader->session_count);
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_DownloadFlowController
#include "TestRunner.h"

const char DUMMY_ADDRESS[]              = "127.0.0.1";
const apr_size_t ATTACK_COUNT           = 100;

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << endl;
}

void run_can_download(DownloadFlowController *flow_controller,
                      apr_sockaddr_t *sockaddr)
{
    show_test_name("can_download");

    for (apr_size_t i = 0; i < DLD_MAX_SESSION_COUNT_PER_IP; i++) {
        flow_controller->add_session(sockaddr);
    }
    if (flow_controller->add_session(sockaddr)) {
        THROW(MESSAGE_BUG_FOUND);
    }
    flow_controller->remove_session(sockaddr);
    if (!flow_controller->add_session(sockaddr)) {
        THROW(MESSAGE_BUG_FOUND);
    }
    if (flow_controller->add_session(sockaddr)) {
        THROW(MESSAGE_BUG_FOUND);
    }
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    apr_sockaddr_t *sockaddr;

    if (argc != 1) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    DownloadFlowController flow_controller;
    show_line();

    sockaddr = get_sockaddr(pool, DUMMY_ADDRESS);

    run_can_download(&flow_controller, sockaddr);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
