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
 * $Id: UploaderConfig.h 2907 2009-01-14 12:56:44Z svn $
 *****************************************************************************/

#ifndef UPLOADER_CONFIG_H
#define UPLOADER_CONFIG_H

#include "Environment.h"

#include <cstdlib>

#include "apr_shm.h"

#include "Macro.h"
#include "Uncopyable.h"

class DownloadFlowController;
class PostFlowController;
class PostProgressList;
class UploadItemList;
class ThumbnailList;
class UploadItemManager;
class UploaderTemplate;

#include "ThumbnailList.h"


/**
 * @brief アップローダの設定を保持するクラス．
 */
class UploaderConfig: public Uncopyable
{
public:
    typedef struct Item {
        const char *param;
        const char *desc;
    } item_t;
    enum template_type_t {
        INDEX_VIEW,
        INFO_VIEW,
        PROGRESS_VIEW,
        DOWNLOAD_VIEW,
        THUMBNAIL_VIEW,
        ADMIN_VIEW,
        ERROR_VIEW,
        __TMPL_COUNT__
    };

    static const item_t PATH;
    static const item_t BASE_URL;
    static const item_t DATA_DIRECTORY;
    static const item_t FILE_DIRECTORY;
    static const item_t THUMB_DIRECTORY;
    static const item_t TEMP_DIRECTORY;

    static const item_t TOTAL_FILE_SIZE;
    static const item_t TOTAL_FILE_NUMBER;
    static const item_t FILE_SIZE_LIMIT;
    static const item_t PER_PAGE_ITEM_NUMBER;

    static const item_t INDEX_VIEW_TEMPLATE;
    static const item_t INFO_VIEW_TEMPLATE;
    static const item_t PROGRESS_VIEW_TEMPLATE;
    static const item_t DOWNLOAD_VIEW_TEMPLATE;
    static const item_t THUMBNAIL_VIEW_TEMPLATE;
    static const item_t ADMIN_VIEW_TEMPLATE;
    static const item_t ERROR_VIEW_TEMPLATE;

    UploaderConfig(apr_pool_t *pool);

    void init();
    void child_init();
    void finalize();

    void set_template(template_type_t type, const char *tmpl_file_path);

    UploadItemList *get_item_list() const
    {
        return item_list_;
    };
    ThumbnailList *get_thumbnail_list() const
    {
        return thumbnail_list_;
    };

    UploadItemManager *get_item_manager() const
    {
        return item_manager_;
    };
    DownloadFlowController *get_download_flow_controller() const
    {
        return download_flow_controller_;
    };
    PostFlowController *get_post_flow_controller() const
    {
        return post_flow_controller_;
    };
    PostProgressList *get_post_progress_list() const
    {
        return post_progress_list_;
    };
    UploaderTemplate *get_template(template_type_t type) const
    {
        return tmpls_[type];
    };
    bool is_active() const
    {
        return is_active_;
    };
    void update_template();

    const char *to_string(apr_pool_t *pool);

    // 面倒なので public
    const char *path;
    const char *base_url;
    const char *data_dir_path;
    const char *file_dir_path;
    const char *thumb_dir_path;
    const char *temp_dir_path;
    apr_uint64_t total_file_size_limit;
    apr_size_t total_file_number_limit;
    apr_uint64_t file_size_limit;
    apr_size_t per_page_item_number;
    bool is_debug_mode;

private:
    void check();
    void check_dir(const char *dir_path);
    void check_template();
    void load_template();
    const char *config_to_string(apr_pool_t *pool);

    apr_pool_t *pool_;

    bool is_active_;

    apr_shm_t *item_list_shm_;
    apr_shm_t *thumbnail_list_shm_;
    apr_shm_t *item_manager_shm_;
    apr_shm_t *download_flow_controller_shm_;
    apr_shm_t *post_flow_controller_shm_;
    apr_shm_t *post_progress_list_shm_;

    UploadItemList *item_list_;
    ThumbnailList *thumbnail_list_;
    UploadItemManager *item_manager_;
    DownloadFlowController *download_flow_controller_;
    PostFlowController *post_flow_controller_;
    PostProgressList *post_progress_list_;

    UploaderTemplate *tmpls_[__TMPL_COUNT__];
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
