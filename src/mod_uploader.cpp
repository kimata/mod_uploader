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
 * $Id: mod_uploader.cpp 2907 2009-01-14 12:56:44Z svn $
 *****************************************************************************/

/**
 * @file
 * @brief Apache モジュールのエントリ関数群．
 */

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

// Apache の config.h を先に処理させるため，httpd.h のインクルードはこの位置
#include "httpd.h"

#ifndef UPLOADER_TYPE_APACHE
#define UPLOADER_TYPE_APACHE
#endif

#include "Environment.h"

#include "http_main.h"
#include "http_config.h"
#include "http_request.h"

#include "apr_pools.h"
#include "apr_tables.h"
#include "apr_strings.h"

#include "mod_uploader.h"
#include "uploader_handler.h"

#include "ApacheResponse.h"
#include "UploaderConfig.h"
#include "Auxiliary.h"
#include "Macro.h"
#include "SourceInfo.h"

#define TEMPLATE_INSTANTIATION
#include "uploader_handler.cpp"

using namespace std;

SOURCE_INFO_ADD("$Id: mod_uploader.cpp 2907 2009-01-14 12:56:44Z svn $");

static const char *HANDLER_NAME = "uploader";
static apr_array_header_t *uploader_config_array = NULL;

#define AS_UCONFIG(pointer)         reinterpret_cast<UploaderConfig *>(pointer)

////////// Apache 1.x 用 //////////////////////////////////////////////
#if AP_SERVER_MAJORVERSION_NUMBER == 1

#include "http_conf_globals.h"

#define DIRECTIVE_ENTRY(directive, member) {                \
        UploaderConfig::directive.param,                    \
        reinterpret_cast<const char*(*)()>(set_ ## member), \
        NULL, ACCESS_CONF, TAKE1,                           \
        UploaderConfig::directive.desc                      \
    }
#undef apr_pstrdup
#define apr_pstrdup ap_pstrdup

static void *uploader_create_dir_config(apr_pool_t *p, char *dirspec);
static void *uploader_create_server_config(apr_pool_t *p, server_rec *s);
static int uploader_post_config(apr_pool_t *pconf, apr_pool_t *plog,
                                apr_pool_t *ptemp, server_rec *s);
static void uploader_child_init(apr_pool_t *p, server_rec *s);

static apr_pool_t *uploader_get_global_pool(bool force_init=false)
{
    static bool is_init = false;
    static apr_pool_t *pool = NULL;

    if (force_init || !is_init) {
        apr_app_initialize(NULL, NULL, NULL);
        if (apr_pool_create(&pool, NULL) != APR_SUCCESS) {
            // 例外投げちゃいます
            THROW(MESSAGE_POOL_CREATION_FAILED);
        }
    }

    return pool;
}

static void *uploader_create_server_config_1x(pool *p, server_rec *s)
{
    return uploader_create_server_config(uploader_get_global_pool(), s);
}

static void *uploader_create_dir_config_1x(pool *p, char *dirspec)
{
    return uploader_create_dir_config(uploader_get_global_pool(), dirspec);
}

static void uploader_post_config_1x(server_rec *s, pool *p)
{
    uploader_post_config(uploader_get_global_pool(),
                         uploader_get_global_pool(),
                         uploader_get_global_pool(), s);
}

static void uploader_child_init_1x(server_rec *s, pool *p)
{
    return uploader_child_init(uploader_get_global_pool(true), s);
}

////////// Apache 2.x 用 //////////////////////////////////////////////
#else

#define DIRECTIVE_ENTRY(directive, member)                              \
    AP_INIT_TAKE1(UploaderConfig::directive.param,                      \
                  reinterpret_cast<const char*(*)()>(set_ ## member),   \
                  NULL, ACCESS_CONF, UploaderConfig::directive.desc)
#endif
///////////////////////////////////////////////////////////////////////

#define DIRECTIVE_HANDLER(member, value)                                \
    static const char *set_ ## member(cmd_parms *parms, void *mconfig,  \
                                      char *arg)                        \
    {                                                                   \
        UploaderConfig *config;                                         \
                                                                        \
        config = AS_UCONFIG(mconfig);                                   \
        config->member = value;                                         \
                                                                        \
        return NULL;                                                    \
    }
#define TEMPLATE_DIRECTIVE_HANDLER(type, tmpl_file_path)                \
    static const char *set_ ## type(cmd_parms *parms, void *mconfig,    \
                                    char *arg)                          \
    {                                                                   \
        UploaderConfig *config;                                         \
                                                                        \
        config = AS_UCONFIG(mconfig);                                   \
        config->set_template(UploaderConfig::type, tmpl_file_path);     \
                                                                        \
        return NULL;                                                    \
    }

static UploaderConfig *uploader_get_dir_config(request_rec *r)
{
    return AS_UCONFIG(ap_get_module_config(r->per_dir_config,
                                           &uploader_module));
}

static apr_status_t uploader_finalize_server_config(void *data)
{
    uploader_config_array = NULL;

    return APR_SUCCESS;
}

static apr_status_t uploader_finalize_dir_config(void *data)
{
    UploaderConfig *config;

    config = AS_UCONFIG(data);
    config->finalize();

    return APR_SUCCESS;
}

static void *uploader_create_server_config(apr_pool_t *p, server_rec *s)
{
    if (uploader_config_array == NULL) {
        uploader_config_array = apr_array_make(p, 4, sizeof(UploaderConfig *));

        apr_pool_cleanup_register(p, NULL, uploader_finalize_server_config,
                                  apr_pool_cleanup_null);
    }

    return NULL;
}

static void *uploader_create_dir_config(apr_pool_t *p, char *dirspec)
{
    UploaderConfig *config;

    APR_PCALLOC(config, UploaderConfig *, p, sizeof(UploaderConfig));

    new(config) UploaderConfig(p);


    memcpy(apr_array_push(uploader_config_array), &config,
           sizeof(UploaderConfig *));

    return config;
}

static bool get_apache_debug_mode()
{
    const char **defs;

    defs = AS_CONST_CHAR_P(AVOID_ALIGN_WARN ap_server_config_defines->elts);

    for (int i = 0; i < ap_server_config_defines->nelts; i++) {
        // -X オプション付きで起動されたときは DEBUG が定義されている
        if ((strcmp(defs[i], "DEBUG") == 0) ||
            (strcmp(defs[i], "UPLOADER_DEBUG") == 0)) {
            return true;
        }
    }

    return false;
}

static int uploader_post_config_server(apr_pool_t *p, server_rec *s)
{
    UploaderConfig **configs;
    UploaderConfig *config;

    configs = (UploaderConfig **)AVOID_ALIGN_WARN uploader_config_array->elts;

    for (int i = 0; i < uploader_config_array->nelts; i++) {
        config = configs[i];

        try {
            config->init();
            config->is_debug_mode = get_apache_debug_mode();

            apr_pool_cleanup_register(p, config, uploader_finalize_dir_config,
                                      apr_pool_cleanup_null);
        } catch(const char *message) {
            logger.error(__FILE__, __LINE__, s,
                         "(%s) %s: %s", "UploaderConfig::init", message,
                         get_last_error_message(p));

            return HTTP_INTERNAL_SERVER_ERROR;
        }
    }

    return OK;
}

static int uploader_post_config(apr_pool_t *pconf, apr_pool_t *plog,
                                apr_pool_t *ptemp, server_rec *s)
{
    apr_status_t status;

#if AP_SERVER_MAJORVERSION_NUMBER != 1
    void *user_data;

    apr_pool_userdata_get(&user_data, PACKAGE_NAME, s->process->pool);

    // 2 回呼ばれるので，1 回目は何もせずに return する
    if (user_data == NULL) {
        apr_pool_userdata_set(reinterpret_cast<const void *>(1), PACKAGE_NAME,
                              apr_pool_cleanup_null, s->process->pool);

        return OK;
    }
#endif

    status = uploader_post_config_server(pconf, s);
    if (status != OK) {
        return status;
    }

#ifndef WIN32
    // Windows でこれをするとサービス名が変わってしまうらしい
#if AP_SERVER_MAJORVERSION_NUMBER == 1
    ap_add_version_component(PACKAGE_NAME "/" PACKAGE_VERSION);
#else
    ap_add_version_component(pconf, PACKAGE_NAME "/" PACKAGE_VERSION);
#endif
#endif

    logger.info(__FILE__, __LINE__, s, "SUCCESS: %s", "post config");

    return OK;
}

static void uploader_child_init_server(apr_pool_t *pool, server_rec *s)
{
    UploaderConfig **configs;
    UploaderConfig *config;

    configs = (UploaderConfig **)AVOID_ALIGN_WARN uploader_config_array->elts;

    for (int i = 0; i < uploader_config_array->nelts; i++) {
        config = configs[i];

        try {
            config->child_init();
        } catch(const char *message) {
            logger.error(__FILE__, __LINE__, s,
                         "(%s) %s: %s", "UploaderConfig::child_init", message,
                         get_last_error_message(pool));
        }
    }
}

static void uploader_child_init(apr_pool_t *p, server_rec *s)
{
    uploader_child_init_server(p, s);

    logger.info(__FILE__, __LINE__, s, "SUCCESS: %s", "child init");
}

////////// Apache 1.x 用 //////////////////////////////////////////////
#if AP_SERVER_MAJORVERSION_NUMBER == 1
static int uploader_handler(request_rec *r)
{
    UploaderConfig *config;

    if (UNLIKELY(!ap_is_initial_req(r))) {
        return DECLINED;
    }

    config = uploader_get_dir_config(r);

    if (UNLIKELY(!config->is_active())) {
        return DECLINED;
    }

    TemporaryPool temp_pool;
    ApacheResponse::Handle r_wrapper(temp_pool.get(), r);
    try {
        return uploader_command_handler<ApacheResponse>(&r_wrapper, config,
                                                        r->path_info);
    } catch(const char *message) {
        logger.error(__FILE__, __LINE__, &r_wrapper, "Exception: %s", message);

        return HTTP_INTERNAL_SERVER_ERROR;
    }
}

static const handler_rec uploader_handlers[] = {
    { HANDLER_NAME, uploader_handler },
    { NULL, NULL }
};

////////// Apache 2.x 用 //////////////////////////////////////////////
#else
static int uploader_handler(request_rec *r)
{
    UploaderConfig *config;

    if (strcmp(r->handler, HANDLER_NAME)) {
        return DECLINED;
    }

    if (UNLIKELY(!ap_is_initial_req(r))) {
        return DECLINED;
    }

    config = uploader_get_dir_config(r);

    if (UNLIKELY(!config->is_active())) {
        return DECLINED;
    }

    try {
        if (UNLIKELY(config->is_debug_mode)) {
            config->update_template();
        }

        return uploader_command_handler<ApacheResponse>(r, config,
                                                        r->path_info);
    } catch(const char *message) {
        logger.error(__FILE__, __LINE__, r, "Exception: %s", message);

        return HTTP_INTERNAL_SERVER_ERROR;
    }
}

static void uploader_register_hooks(apr_pool_t *pool)
{
    ap_hook_post_config(uploader_post_config, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_child_init(uploader_child_init, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_handler(uploader_handler, NULL, NULL, APR_HOOK_MIDDLE);
}
#endif
///////////////////////////////////////////////////////////////////////

DIRECTIVE_HANDLER(base_url, apr_pstrdup(parms->pool, arg));
DIRECTIVE_HANDLER(data_dir_path, apr_pstrdup(parms->pool, arg));
DIRECTIVE_HANDLER(file_dir_path, apr_pstrdup(parms->pool, arg));
DIRECTIVE_HANDLER(thumb_dir_path, apr_pstrdup(parms->pool, arg));
DIRECTIVE_HANDLER(temp_dir_path, apr_pstrdup(parms->pool, arg));

DIRECTIVE_HANDLER(total_file_size_limit, apr_atoi64(arg) * 1024);
DIRECTIVE_HANDLER(total_file_number_limit, atoi(arg));
DIRECTIVE_HANDLER(file_size_limit, apr_atoi64(arg) * 1024);
DIRECTIVE_HANDLER(per_page_item_number, atoi(arg));

TEMPLATE_DIRECTIVE_HANDLER(INDEX_VIEW, apr_pstrdup(parms->pool, arg));
TEMPLATE_DIRECTIVE_HANDLER(INFO_VIEW, apr_pstrdup(parms->pool, arg));
TEMPLATE_DIRECTIVE_HANDLER(PROGRESS_VIEW, apr_pstrdup(parms->pool, arg));
TEMPLATE_DIRECTIVE_HANDLER(DOWNLOAD_VIEW, apr_pstrdup(parms->pool, arg));
TEMPLATE_DIRECTIVE_HANDLER(THUMBNAIL_VIEW, apr_pstrdup(parms->pool, arg));
TEMPLATE_DIRECTIVE_HANDLER(ADMIN_VIEW, apr_pstrdup(parms->pool, arg));
TEMPLATE_DIRECTIVE_HANDLER(ERROR_VIEW, apr_pstrdup(parms->pool, arg));

static const command_rec uploader_cmds[] = {
    DIRECTIVE_ENTRY(BASE_URL, base_url),
    DIRECTIVE_ENTRY(DATA_DIRECTORY, data_dir_path),
    DIRECTIVE_ENTRY(FILE_DIRECTORY, file_dir_path),
    DIRECTIVE_ENTRY(THUMB_DIRECTORY, thumb_dir_path),
    DIRECTIVE_ENTRY(TEMP_DIRECTORY, temp_dir_path),

    DIRECTIVE_ENTRY(TOTAL_FILE_SIZE, total_file_size_limit),
    DIRECTIVE_ENTRY(TOTAL_FILE_NUMBER, total_file_number_limit),
    DIRECTIVE_ENTRY(FILE_SIZE_LIMIT, file_size_limit),
    DIRECTIVE_ENTRY(PER_PAGE_ITEM_NUMBER, per_page_item_number),

    DIRECTIVE_ENTRY(INDEX_VIEW_TEMPLATE, INDEX_VIEW),
    DIRECTIVE_ENTRY(INFO_VIEW_TEMPLATE, INFO_VIEW),
    DIRECTIVE_ENTRY(PROGRESS_VIEW_TEMPLATE, PROGRESS_VIEW),
    DIRECTIVE_ENTRY(DOWNLOAD_VIEW_TEMPLATE, DOWNLOAD_VIEW),
    DIRECTIVE_ENTRY(THUMBNAIL_VIEW_TEMPLATE, THUMBNAIL_VIEW),
    DIRECTIVE_ENTRY(ADMIN_VIEW_TEMPLATE, ADMIN_VIEW),
    DIRECTIVE_ENTRY(ERROR_VIEW_TEMPLATE, ERROR_VIEW),
    { NULL, NULL, NULL, 0, TAKE1, NULL }
};

extern "C" {
#if AP_SERVER_MAJORVERSION_NUMBER == 1
module MODULE_VAR_EXPORT uploader_module = {
    STANDARD_MODULE_STUFF,
    uploader_post_config_1x,          /* module initializer                  */
    uploader_create_dir_config_1x,    /* create per-dir    config structures */
    NULL,                             /* merge  per-dir    config structures */
    uploader_create_server_config_1x, /* create per-server config structures */
    NULL,                             /* merge  per-server config structures */
    uploader_cmds,                    /* table of config file commands       */
    uploader_handlers,                /* [#8] MIME-typed-dispatched handlers */
    NULL,                             /* [#1] URI to filename translation    */
    NULL,                             /* [#4] validate user id from request  */
    NULL,                             /* [#5] check if the user is ok _here_ */
    NULL,                             /* [#3] check access by host address   */
    NULL,                             /* [#6] determine MIME type            */
    NULL,                             /* [#7] pre-run fixups                 */
    NULL,                             /* [#9] log a transaction              */
    NULL,                             /* [#2] header parser                  */
    uploader_child_init_1x,           /* child_init                          */
    NULL,                             /* child_exit                          */
    NULL                              /* [#0] post read-request              */
};
#else
module EXPORT AP_MODULE_DECLARE_DATA uploader_module = {
    STANDARD20_MODULE_STUFF,
    uploader_create_dir_config,       /* create per-dir    config structures */
    NULL,                             /* merge  per-dir    config structures */
    uploader_create_server_config,    /* create per-server config structures */
    NULL,                             /* merge  per-server config structures */
    uploader_cmds,                    /* table of config file commands       */
    uploader_register_hooks           /* register hooks                      */
};
#endif
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
