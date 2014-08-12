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
 * $Id: DownloadFlowController.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef DOWNLOAD_FLOW_CONTROLLER_H
#define DOWNLOAD_FLOW_CONTROLLER_H

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
 * @brief ダウンロードのフロー制御を行うクラス．
 */
class DownloadFlowController: public FlowController
{
public:
    DownloadFlowController();
    bool add_session(apr_sockaddr_t *sockaddr);
    void remove_session(apr_sockaddr_t *sockaddr);

    static DownloadFlowController *get_instance(apr_shm_t *shm);
    static apr_size_t get_memory_size();

    static const char *dump_list(apr_pool_t *pool,
                                 DownloadFlowController *flow_controller);

private:
    typedef struct Downloader {
        char paddress[MAX_PADDRESS_SIZE]; // パスカル文字列
        apr_size_t session_count;
    } downloader_t;

    bool downloader_list_add(apr_sockaddr_t *sockaddr);
    downloader_t *downloader_list_get(apr_sockaddr_t *sockaddr);

    static const char *dump_downloader(apr_pool_t *pool,
                                       downloader_t *downloader);

    apr_atomic_t lock_;
    apr_size_t downloader_count_;
    downloader_t downloader_list_[DLD_MAX_SESSION_COUNT];
};

/**
 * @brief ダウンロードのセッションを表すクラス．
 */
class DownloadSession: public Uncopyable
{
public:
    DownloadSession(DownloadFlowController *flow_controller,
                    apr_sockaddr_t *sockaddr);
    ~DownloadSession();
    bool is_valid()
    {
        return is_valid_;
    };

private:
    static const apr_size_t DOWNLOAD_BUSY_SLEEP_SEC;

    DownloadFlowController *flow_controller_;
    apr_sockaddr_t *sockaddr_;
    bool is_valid_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
