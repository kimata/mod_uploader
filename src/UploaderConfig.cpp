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
 * $Id: UploaderConfig.cpp 2907 2009-01-14 12:56:44Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <cstring>

#include "apr_file_io.h"
#include "apr_strings.h"

#include "UploaderConfig.h"
#include "UploadItemList.h"
#include "UploadItemManager.h"
#include "UploadItemListReader.h"
#include "ThumbnailList.h"
#include "DownloadFlowController.h"
#include "PostFlowController.h"
#include "PostProgressList.h"
#include "UploaderTemplate.h"
#include "Macro.h"
#include "Auxiliary.h"
#include "Message.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: UploaderConfig.cpp 2907 2009-01-14 12:56:44Z svn $");

#define PARAM_PREFIX "Uploader"

const UploaderConfig::item_t UploaderConfig::PATH               = {
    PARAM_PREFIX "Path",
    "Path",
};
const UploaderConfig::item_t UploaderConfig::BASE_URL           = {
    PARAM_PREFIX "BaseUrl",
    "Base URL",
};
const UploaderConfig::item_t UploaderConfig::DATA_DIRECTORY     = {
    PARAM_PREFIX "DataDirectory",
    "Data Directory Path",
};
const UploaderConfig::item_t UploaderConfig::FILE_DIRECTORY     = {
    PARAM_PREFIX "FileDirectory",
    "File Directory Path",
};
const UploaderConfig::item_t UploaderConfig::THUMB_DIRECTORY    = {
    PARAM_PREFIX "ThumbDirectory",
    "Thumbnail Directory Path",
};
const UploaderConfig::item_t UploaderConfig::TEMP_DIRECTORY     = {
    PARAM_PREFIX "TempDirectory",
    "Temporary Directory Path",
};
const UploaderConfig::item_t UploaderConfig::TOTAL_FILE_SIZE    = {
    PARAM_PREFIX "TotalFileSizeLimit",
    "Total File Size Limit (KB)",
};
const UploaderConfig::item_t UploaderConfig::TOTAL_FILE_NUMBER  = {
    PARAM_PREFIX "TotalFileNumberLimit",
    "Total File Number Limit",
};
const UploaderConfig::item_t UploaderConfig::FILE_SIZE_LIMIT    = {
    PARAM_PREFIX "FileSizeLimit",
    "File Size Limit (KB)",
};
const UploaderConfig::item_t UploaderConfig::PER_PAGE_ITEM_NUMBER = {
    PARAM_PREFIX "PerPageItemNumber",
    "Per Page Item Number",
};
const UploaderConfig::item_t UploaderConfig::INDEX_VIEW_TEMPLATE = {
    PARAM_PREFIX "IndexViewTemplate",
    "Template of index view",
};
const UploaderConfig::item_t UploaderConfig::INFO_VIEW_TEMPLATE = {
    PARAM_PREFIX "InfoViewTemplate",
    "Template of info view",
};
const UploaderConfig::item_t UploaderConfig::PROGRESS_VIEW_TEMPLATE = {
    PARAM_PREFIX "ProgressViewTemplate",
    "Template of progress view",
};
const UploaderConfig::item_t UploaderConfig::DOWNLOAD_VIEW_TEMPLATE = {
    PARAM_PREFIX "DownloadViewTemplate",
    "Template of download view",
};
const UploaderConfig::item_t UploaderConfig::THUMBNAIL_VIEW_TEMPLATE = {
    PARAM_PREFIX "ThumbnailViewTemplate",
    "Template of thumbnail view",
};
const UploaderConfig::item_t UploaderConfig::ADMIN_VIEW_TEMPLATE = {
    PARAM_PREFIX "AdminViewTemplate",
    "Template of admin view",
};
const UploaderConfig::item_t UploaderConfig::ERROR_VIEW_TEMPLATE = {
    PARAM_PREFIX "ErrorViewTemplate",
    "Template of error view",
};


/******************************************************************************
 * public メソッド
 *****************************************************************************/
UploaderConfig::UploaderConfig(apr_pool_t *pool)
  : path(""),
    base_url(""),
    data_dir_path(""),
    file_dir_path(""),
    temp_dir_path(""),
    total_file_size_limit(DEFAULT_TOTAL_FILE_SIZE_LIMIT),
    total_file_number_limit(DEFAULT_TOTAL_FILE_NUMBER_LIMIT),
    file_size_limit(DEFAULT_FILE_SIZE_LIMIT),
    per_page_item_number(DEFAULT_PER_PAGE_ITEM_NUMBER),
    is_debug_mode(false),
    pool_(pool),
    is_active_(false),
    item_list_shm_(NULL),
    thumbnail_list_shm_(NULL),
    item_manager_shm_(NULL),
    download_flow_controller_shm_(NULL),
    post_flow_controller_shm_(NULL),
    post_progress_list_shm_(NULL),
    item_list_(NULL),
    thumbnail_list_(NULL),
    item_manager_(NULL)

{
    apr_pool_create(&pool_, NULL);

    apr_temp_dir_get(&temp_dir_path, pool_);
    memset(tmpls_, 0, sizeof(tmpls_));
}

void UploaderConfig::init()
{
    apr_time_t mtime;

    if (is_active_) {
        return;
    }

    if (strlen(base_url) == 0) {
        return;
    }

    check();

    item_list_shm_ = create_shm
        (pool_, UploadItemList::get_memory_size(total_file_number_limit));
    thumbnail_list_shm_ = create_shm
        (pool_, ThumbnailList::get_memory_size(total_file_number_limit));
    item_manager_shm_ = create_shm(pool_, UploadItemManager::get_memory_size());
    download_flow_controller_shm_ = create_shm
        (pool_, DownloadFlowController::get_memory_size());
    post_flow_controller_shm_ = create_shm
        (pool_, PostFlowController::get_memory_size());
    post_progress_list_shm_ = create_shm
        (pool_, PostProgressList::get_memory_size());

    thumbnail_list_ = ThumbnailList::get_instance(thumbnail_list_shm_,
                                                  total_file_number_limit);
    item_list_ = UploadItemListReader::read
        (pool_, data_dir_path, thumb_dir_path,
         total_file_size_limit, total_file_number_limit,
         item_list_shm_, thumbnail_list_, &mtime);
    item_manager_ = UploadItemManager::get_instance
        (item_manager_shm_, pool_,
         data_dir_path, file_dir_path, thumb_dir_path,
         item_list_, thumbnail_list_, mtime);

    download_flow_controller_ = DownloadFlowController::get_instance
        (download_flow_controller_shm_);
    post_flow_controller_ = PostFlowController::get_instance
        (post_flow_controller_shm_, PST_INTERVAL_SEC);
    post_progress_list_ = PostProgressList::get_instance
        (post_progress_list_shm_);

    load_template();

    is_active_ = true;
}

void UploaderConfig::child_init()
{
    if (!is_active_) {
        return;
    }

    attach_shm(pool_, item_list_shm_);
    attach_shm(pool_, thumbnail_list_shm_);
    attach_shm(pool_, item_manager_shm_);
    attach_shm(pool_, download_flow_controller_shm_);
    attach_shm(pool_, post_flow_controller_shm_);
    attach_shm(pool_, post_progress_list_shm_);

    item_list_ = UploadItemList::child_init(item_list_shm_);
    thumbnail_list_ = ThumbnailList::child_init(thumbnail_list_shm_);
    item_manager_->child_init(item_manager_shm_, item_list_, thumbnail_list_);
}

void UploaderConfig::finalize()
{
    if (!is_active_) {
        return;
    }

    item_manager_->sync_all_item(pool_);

    apr_shm_destroy(download_flow_controller_shm_);
    apr_shm_destroy(post_flow_controller_shm_);
    apr_shm_destroy(post_progress_list_shm_);
    apr_shm_destroy(item_manager_shm_);
    apr_shm_destroy(thumbnail_list_shm_);
    apr_shm_destroy(item_list_shm_);
}

void UploaderConfig::set_template(template_type_t type, const char *tmpl_file_path)
{
    if (tmpls_[type] != NULL) {
        delete tmpls_[type];
    }
    tmpls_[type] = new UploaderTemplate(pool_, tmpl_file_path);
}

void UploaderConfig::update_template()
{
    for (apr_size_t i = 0; i < ARRAY_SIZE_OF(tmpls_); i++) {
        if (tmpls_[i]->update()) {
            get_item_manager()->update_mtime(tmpls_[i]->get_mtime());
        }
    }
}

const char *UploaderConfig::to_string(apr_pool_t *pool)
{
    return apr_pstrcat
        (pool,
         "* UploaderConfig:\n",
         config_to_string(pool),
         "* PostFlowController:\n",
         PostFlowController::dump_list(pool, post_flow_controller_),
         "* DownloadFlowController:\n",
         DownloadFlowController::dump_list(pool, download_flow_controller_),
         NULL);
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void UploaderConfig::check()
{
    if (base_url == NULL) {
        THROW(MESSAGE_CONF_PARAM_COMPULSORY_NOT_SPECIFIED);
    }

    check_dir(data_dir_path);
    check_dir(file_dir_path);
    check_dir(thumb_dir_path);
    check_dir(temp_dir_path);

    check_template();
}

void UploaderConfig::check_dir(const char *dir_path)
{
    if (dir_path == NULL) {
        THROW(MESSAGE_CONF_PARAM_COMPULSORY_NOT_SPECIFIED);
    }
    if (!File::is_writable(pool_, dir_path)) {
        throw apr_pstrcat(pool_,
                          MESSAGE_DIR_NOT_WRITABLE,
                          " [", dir_path, "]",
                          " (" __FILE__ ":" APR_STRINGIFY(__LINE__) ")",
                          NULL);
    }
}

void UploaderConfig::check_template()
{
    for (apr_size_t i = 0; i < ARRAY_SIZE_OF(tmpls_); i++) {
        if (tmpls_[i] == NULL) {
            THROW(MESSAGE_CONF_TEMPLATE_INITIALIZE_FAILED);
        }
    }
}

void UploaderConfig::load_template()
{
    for (apr_size_t i = 0; i < ARRAY_SIZE_OF(tmpls_); i++) {
        tmpls_[i]->load();
        get_item_manager()->update_mtime(tmpls_[i]->get_mtime());
    }
}

const char *UploaderConfig::config_to_string(apr_pool_t *pool)
{
    return apr_pstrcat
        (pool,
         apr_psprintf(pool, "%-24s : %s\n",
                      "base_url", base_url),
         apr_psprintf(pool, "%-24s : %s\n",
                      "data_dir_path", data_dir_path),
         apr_psprintf(pool, "%-24s : %s\n",
                      "file_dir_path", file_dir_path),
         apr_psprintf(pool, "%-24s : %s\n",
                      "thumb_dir_path", thumb_dir_path),
         apr_psprintf(pool, "%-24s : %s\n",
                      "temp_dir_path", temp_dir_path),
         apr_psprintf(pool, "%-24s : %s\n",
                      "total_file_size_limit",
                      size_str(pool, total_file_size_limit)),
         apr_psprintf(pool, "%-24s : %" APR_SIZE_T_FMT "\n",
                      "total_file_number_limit", total_file_number_limit),
         apr_psprintf(pool, "%-24s : %s\n",
                      "file_size_limit", size_str(pool, file_size_limit)),
         apr_psprintf(pool, "%-24s : %" APR_SIZE_T_FMT "\n",
                      "per_page_item_number", per_page_item_number),
         NULL);
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
