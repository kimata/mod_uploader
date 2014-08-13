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
 * $Id: uploader_handler.cpp 2907 2009-01-14 12:56:44Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_INSTANTIATION
#include "Environment.h"
#endif

#include <cstring>
#include <memory>

#include "apr_strings.h"

#include "uploader_handler.h"
#include "uploader_util.h"

#include "RequestResponseImpl.h"

#include "UploadItem.h"
#include "UploadItemIO.h"
#include "UploadItemIterator.h"
#include "UploadItemList.h"
#include "UploaderTemplate.h"
#include "UploadItemVariableCreator.h"
#include "UploadItemManager.h"
#include "ThumbnailIterator.h"
#include "ThumbnailVariableCreator.h"
#include "PostProgressList.h"
#include "DownloadFlowController.h"
#include "CharCodeConverter.h"
#include "Message.h"
#include "Macro.h"
#include "Auxiliary.h"
#include "SourceInfo.h"

#ifndef TEMPLATE_INSTANTIATION
SOURCE_INFO_ADD("$Id: uploader_handler.cpp 2907 2009-01-14 12:56:44Z svn $");
#endif

#define STR_CMP_LITERAL(arg, literal)                            \
    ((strncmp(arg, literal, LITERAL_STRLEN(literal)) == 0) &&    \
     (arg[LITERAL_STRLEN(literal)] == '\0'))
#define STR_CMP_LITERAL_START_WITH(arg, literal)            \
    (strncmp(arg, literal, LITERAL_STRLEN(literal)) == 0)

static const apr_size_t PER_PAGE_THUMBNAIL_NUMBER = 100;

static const char BASE_URL_VAR_NAME[]           = "BASE_URL";
static const char COMMAND_VAR_NAME[]            = "COMMAND";
static const char ITEM_VAR_NAME[]               = "ITEM";
static const char ITEM_LIST_VAR_NAME[]          = "ITEM_LIST";
static const char THUMBNAIL_LIST_VAR_NAME[]     = "THUMBNAIL_LIST";
static const char MAX_FILE_SIZE_VAR_NAME[]      = "MAX_FILE_SIZE";
static const char TOTAL_FILE_SIZE_VAR_NAME[]    = "TOTAL_FILE_SIZE";
static const char TOTAL_FILE_NUMBER_VAR_NAME[]  = "TOTAL_FILE_NUMBER";
static const char TOTAL_THUMBNAIL_NUMBER_VAR_NAME[] = "TOTAL_THUMBNAIL_NUMBER";
static const char PAGE_COUNT_VAR_NAME[]         = "PAGE_COUNT";
static const char CURRENT_PAGE_VAR_NAME[]       = "CURRENT_PAGE";
static const char ERROR_MESSAGE_VAR_NAME[]      = "ERROR_MESSAGE";

static const char INDEX_COMMAND[]               = "index";
static const char SORT_COMMAND[]                = "sort_";
static const char INFO_COMMAND[]                = "info";
static const char THUMBNAIL_COMMAND[]           = "thumbnail";
static const char THUMBNAIL_VIEW_COMMAND[]      = "view";
static const char MAIL_COMMAND[]                = "mail";
static const char DOWNLOAD_COMMAND[]            = "download";
static const char REMOVE_COMMAND[]              = "remove";
static const char RSS_COMMAND[]                 = "rss";
static const char UPLOAD_COMMAND[]              = "upload";
static const char PROGRESS_DATA_COMMAND[]       = "progress_data";
static const char ADMIN_COMMAND[]               = "admin";
static const char ADMIN_INFO_COMMAND[]          = "info";

static const char UPLOAD_ID_ARG[]               = "upload_id";

static const char PAGE_CONTENT_TYPE[]           = "text/html; charset=EUC-JP";

UploadItem::CompareFunc get_sort_func(const char *sort_type)
{
    if (STR_CMP_LITERAL(sort_type, "file_name")) {
        return UploadItem::cmp_file_name;
    } else if (STR_CMP_LITERAL(sort_type, "file_size")) {
        return UploadItem::cmp_file_size;
    } else if (STR_CMP_LITERAL(sort_type, "file_mime")) {
        return UploadItem::cmp_file_mime;
    } else if (STR_CMP_LITERAL(sort_type, "comment")) {
        return UploadItem::cmp_comment;
    } else if (STR_CMP_LITERAL(sort_type, "download_count")) {
        return UploadItem::cmp_download_count;
    } else {
        return NULL;
    }
}

template<class Response>
static int redirect(typename Response::Handle *r, typename Response::Writer& o,
                    const char *url)
{
    // POST した後にページを簡単にリロードできるようにするため，HTML で
    // リダイレクトさせる．

    Response::set_content_type(r, PAGE_CONTENT_TYPE);
    Response::header_end(r);

    o.write("<?xml version=\"1.0\" encoding=\"EUC-JP\"?>\n");
    o.write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" "
             "\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n");
    o.write("<html xmlns=\"http://www.w3.org/1999/xhtml\" "
             "xml:lang=\"ja\" >\n");
    o.write(" <head>\n");
    o.write("  <meta http-equiv=\"content-type\" "
             "content=\"application/xhtml+xml\" />\n");
    o.write("  <meta http-equiv=\"refresh\" content=\"0;url=");
    o.write(url);
    o.write("\" />\n");
    o.write("  <title>" PACKAGE_NAME "</title>\n");
    o.write(" </head>\n");
    o.write(" <body />\n");
    o.write("</html>\n");

    o.finish();

    return OK;
}

template<class Response>
static int error_without_template(typename Response::Handle *r,
                                  typename Response::Writer& o,
                                  UploaderConfig *config, const char *message)
{
    o.write("<div class=\"warning\">ERROR: ");
    o.write(message);
    o.write("</div>\n");
    o.finish();

    return OK;
}

template<class Response>
static int error(typename Response::Handle *r,
                 typename Response::Writer& o,
                 UploaderConfig *config, const char *message)
{
    Response::set_content_type(r, PAGE_CONTENT_TYPE);
    Response::header_end(r);

    try {
        UploaderTemplate *tmpl;

        tmpl = config->get_template(UploaderConfig::ERROR_VIEW);

        TemplateVariableCreator var_creator(r->pool, tmpl->get_ids());

        var_creator.create(BASE_URL_VAR_NAME, config->base_url);
        var_creator.create(ERROR_MESSAGE_VAR_NAME, message);

        TemplateExecutor<typename Response::Writer> executor(r->pool, o);

        executor.exec(tmpl->get_node_tree(), var_creator.get_variables(),
                      tmpl->get_key_count());
    } catch(const char *message) {
        return error_without_template<Response>(r, o, config, message);
    }
    o.finish();

    return OK;
}

template<class Response>
static int error(typename Response::Handle *r,
                 UploaderConfig *config, const char *message)
{
    typename Response::Writer o(r);

    return error<Response>(r, o, config, message);
}

template<class Response>
static int error_with_log(typename Response::Handle *r,
                          typename Response::Writer& o,
                          UploaderConfig *config, const char *message)
{
    logger.info(__FILE__, __LINE__, r, "ERROR: %s (%s)",
                message, get_last_error_message(r->pool));

    return error<Response>(r, o, config, message);
}

template<class Response>
static int error_with_log(typename Response::Handle *r,
                          UploaderConfig *config, const char *message)
{
    typename Response::Writer o(r);

    logger.info(__FILE__, __LINE__, r, "ERROR: %s (%s)",
                message, get_last_error_message(r->pool));

    return error<Response>(r, o, config, message);
}

template<class Response>
static int index(typename Response::Handle *r, UploaderConfig *config,
                 const char *arg)
{
    apr_size_t page_no;
    apr_size_t page_count;

    Response::set_content_type(r, PAGE_CONTENT_TYPE);
#ifndef DEBUG
    Response::set_modified_time(r, config->get_item_manager()->get_mtime());
    Response::set_last_modified(r, config->get_item_manager()->get_mtime());

    int status;
    if ((status = Response::is_meets_condition(r)) != OK) {
        return status;
    }
#endif
    Response::header_end(r);

    if (Response::is_head_method(r)) {
        return OK;
    }

    get_page_count(config->get_item_list()->size(),
                   config->per_page_item_number, &page_count);
    get_page(r->pool, arg, page_count, &page_no);

    typename Response::Writer o(r);
    try {
        UploaderTemplate *tmpl;
        apr_size_t start_index;
        apr_size_t end_index;

        end_index = page_no * config->per_page_item_number;
        start_index = end_index - config->per_page_item_number;

        UploadItemIterator item_iter(r->pool, config->get_item_manager(),
                                     start_index, end_index);

        tmpl = config->get_template(UploaderConfig::INDEX_VIEW);

        TemplateVariableCreator var_creator(r->pool, tmpl->get_ids());

        var_creator.create(BASE_URL_VAR_NAME, config->base_url);
        var_creator.create(COMMAND_VAR_NAME, INDEX_COMMAND);

        var_creator.create
            (ITEM_LIST_VAR_NAME,
             tmpl->get_item_var_creator()->create(r->pool, &item_iter));

        var_creator.create(MAX_FILE_SIZE_VAR_NAME,
                           size_str(r->pool, config->file_size_limit));
        var_creator.create
            (TOTAL_FILE_SIZE_VAR_NAME,
             size_str(r->pool,
                      config->get_item_list()->get_total_file_size()));
        var_creator.create(TOTAL_FILE_NUMBER_VAR_NAME,
                           config->get_item_list()->size());
        var_creator.create(PAGE_COUNT_VAR_NAME, page_count);

        var_creator.create(CURRENT_PAGE_VAR_NAME, page_no);

        TemplateExecutor<typename Response::Writer> executor(r->pool, o);

        executor.exec(tmpl->get_node_tree(), var_creator.get_variables(),
                      tmpl->get_key_count());
    } catch(const char *message) {
        return error_with_log<Response>(r, o, config, message);
    }
    o.finish();

    return OK;
}

template<class Response>
static int sort(typename Response::Handle *r, UploaderConfig *config,
                const char *arg, const char *sort_type)
{
    apr_size_t page_no;
    apr_size_t page_count;
    UploadItem::CompareFunc cmp_func;

    cmp_func = get_sort_func(sort_type);
    if (cmp_func == NULL) {
        return index<Response>(r, config, arg);
    }

    Response::set_content_type(r, PAGE_CONTENT_TYPE);
#ifndef DEBUG
    Response::set_modified_time(r, config->get_item_manager()->get_mtime());
    Response::set_last_modified(r, config->get_item_manager()->get_mtime());

    int status;
    if ((status = Response::is_meets_condition(r)) != OK) {
        return status;
    }
#endif
    Response::header_end(r);

    if (Response::is_head_method(r)) {
        return OK;
    }

    get_page_count(config->get_item_list()->size(),
                   config->per_page_item_number, &page_count);
    get_page(r->pool, arg, page_count, &page_no);

    typename Response::Writer o(r);
    try {
        UploaderTemplate *tmpl;
        apr_size_t start_index;
        apr_size_t end_index;

        end_index = page_no * config->per_page_item_number;
        start_index = end_index - config->per_page_item_number;

        UploadItemIterator item_iter(r->pool, config->get_item_manager(),
                                     start_index, end_index, cmp_func);

        tmpl = config->get_template(UploaderConfig::INDEX_VIEW);

        TemplateVariableCreator var_creator(r->pool, tmpl->get_ids());

        var_creator.create(BASE_URL_VAR_NAME, config->base_url);
        var_creator.create
            (COMMAND_VAR_NAME,
             apr_pstrcat(r->pool, SORT_COMMAND, sort_type, NULL));

        var_creator.create
            (ITEM_LIST_VAR_NAME,
             tmpl->get_item_var_creator()->create(r->pool, &item_iter));

        var_creator.create(MAX_FILE_SIZE_VAR_NAME,
                           size_str(r->pool, config->file_size_limit));
        var_creator.create
            (TOTAL_FILE_SIZE_VAR_NAME,
             size_str(r->pool,
                      config->get_item_list()->get_total_file_size()));
        var_creator.create(TOTAL_FILE_NUMBER_VAR_NAME,
                           config->get_item_list()->size());
        var_creator.create(PAGE_COUNT_VAR_NAME, page_count);

        var_creator.create(CURRENT_PAGE_VAR_NAME, page_no);

        TemplateExecutor<typename Response::Writer> executor(r->pool, o);

        executor.exec(tmpl->get_node_tree(), var_creator.get_variables(),
                      tmpl->get_key_count());
    } catch(const char *message) {
        return error_with_log<Response>(r, o, config, message);
    }
    o.finish();

    return OK;
}

template<class Response>
static int info(typename Response::Handle *r, UploaderConfig *config,
                apr_size_t item_id)
{
    Response::set_content_type(r, PAGE_CONTENT_TYPE);
    Response::header_end(r);

    if (Response::is_head_method(r)) {
        return OK;
    }

    typename Response::Writer o(r);
    try {
        UploaderTemplate *tmpl;
        UploadItem *uitem;

        uitem = config->get_item_manager()->get_item(r->pool, item_id);

        tmpl = config->get_template(UploaderConfig::INFO_VIEW);

        TemplateVariableCreator var_creator(r->pool, tmpl->get_ids());

        var_creator.create(BASE_URL_VAR_NAME, config->base_url);

        var_creator.create
            (ITEM_VAR_NAME,
             tmpl->get_item_var_creator()->create(r->pool, uitem));

        TemplateExecutor<typename Response::Writer> executor(r->pool, o);

        executor.exec(tmpl->get_node_tree(), var_creator.get_variables(),
                      tmpl->get_key_count());
    } catch(const char *message) {
        return error_with_log<Response>(r, o, config, message);
    }
    o.finish();

    return OK;
}

template<class Response>
static int info(typename Response::Handle *r, UploaderConfig *config,
                const char *arg)
{
    apr_size_t item_id;
    const char *first_arg;

    first_arg = get_word(r->pool, &arg, ARG_SEPARATE_STR[0]);

    if (strlen(first_arg) == 0) {
        return HTTP_BAD_REQUEST;
    }

    if (strncmp(first_arg, UPLOAD_ID_ARG, LITERAL_STRLEN(UPLOAD_ID_ARG)) == 0) {
        apr_uint32_t upload_id;
        RequestReader::post_progress_t *progress;

        upload_id = static_cast<apr_uint32_t>
            (apr_atoi64(get_word(r->pool, &arg, ARG_SEPARATE_STR[0])));
        progress = config->get_post_progress_list()->get(upload_id);

        if (UNLIKELY(progress->id != upload_id)) {
            typename Response::Writer o(r);

            return redirect<Response>
                (r, o,
                 apr_pstrcat(r->pool, config->base_url, ARG_SEPARATE_STR,
                             NULL));
        }

        item_id = progress->item_id;
    } else {
        const char *item_id_str;

        item_id_str = apr_pstrdup(r->pool, first_arg);
        item_id = static_cast<apr_size_t>
            (apr_atoi64(get_word(r->pool, &item_id_str, FILE_EXT_SEPARATOR[0])));
    }

    return info<Response>(r, config, item_id);
}

template<class Response>
static int thumbnail_view(typename Response::Handle *r, UploaderConfig *config,
                          const char *arg)
{
    apr_size_t item_id;
    const char *file_name;
    const char *item_id_str;

    file_name = get_word(r->pool, &arg, ARG_SEPARATE_STR[0]);
    if (strlen(file_name) == 0) {
        return HTTP_BAD_REQUEST;
    }

    item_id_str = apr_pstrdup(r->pool, file_name);
    item_id = static_cast<apr_size_t>
        (apr_atoi64(get_word(r->pool, &item_id_str, FILE_EXT_SEPARATOR[0])));

    try {
        const char  *thumb_file_path;

        Response::set_content_type(r, get_image_mime(file_name));
#ifndef DEBUG
        Response::set_modified_time(r,
                                    config->get_item_manager()->get_mtime());
        Response::set_last_modified(r,
                                    config->get_item_manager()->get_mtime());


        int status;
        if ((status = Response::is_meets_condition(r)) != OK) {
            Response::header_end(r);
            return status;
        }
#endif
        Response::header_end(r);

        if (Response::is_head_method(r)) {
            return OK;
        }

        thumb_file_path = UploadItemIO::get_thumb_path(r->pool,
                                                       config->thumb_dir_path,
                                                       item_id);

        File thumb_file(r->pool, thumb_file_path);
        thumb_file.open(APR_READ);

        Response::Writer::sendfile(r, thumb_file.get_handle(),
                                   thumb_file.get_size());
    } catch(const char *message) {
        return error<Response>(r, config, message);
    }

    return OK;
}

template<class Response>
static int thumbnail(typename Response::Handle *r, UploaderConfig *config,
                     const char *arg)
{
    const char *thumbnail_arg;
    const char *thumbnail_command;
    apr_size_t page_no;
    apr_size_t page_count;

    Response::set_content_type(r, PAGE_CONTENT_TYPE);
#ifndef DEBUG
    Response::set_modified_time(r, config->get_item_manager()->get_mtime());
    Response::set_last_modified(r, config->get_item_manager()->get_mtime());

    int status;
    if ((status = Response::is_meets_condition(r)) != OK) {
        Response::header_end(r);
        return status;
    }
#endif
    if (Response::is_head_method(r)) {
        Response::header_end(r);
        return OK;
    }

    // コマンド
    thumbnail_arg = arg;
    thumbnail_command = get_word(r->pool, &thumbnail_arg, ARG_SEPARATE_STR[0]);

    if (STR_CMP_LITERAL(thumbnail_command, THUMBNAIL_VIEW_COMMAND)) {
        return thumbnail_view<Response>(r, config, thumbnail_arg);
    }
    Response::header_end(r);

    get_page_count(config->get_thumbnail_list()->size(),
                   PER_PAGE_THUMBNAIL_NUMBER, &page_count);
    get_page(r->pool, arg, page_count, &page_no);

    typename Response::Writer o(r);
    try {
        UploaderTemplate *tmpl;
        apr_size_t start_index;
        apr_size_t end_index;

        end_index = page_no * PER_PAGE_THUMBNAIL_NUMBER;
        start_index = end_index - PER_PAGE_THUMBNAIL_NUMBER;

        ThumbnailIterator thumbnail_iter(r->pool, config->get_item_manager(),
                                         start_index, end_index);

        tmpl = config->get_template(UploaderConfig::THUMBNAIL_VIEW);

        TemplateVariableCreator var_creator(r->pool, tmpl->get_ids());

        var_creator.create(BASE_URL_VAR_NAME, config->base_url);

        var_creator.create
            (THUMBNAIL_LIST_VAR_NAME,
             tmpl->get_thumbnail_var_creator()->create(r->pool,
                                                       &thumbnail_iter));

        var_creator.create(TOTAL_THUMBNAIL_NUMBER_VAR_NAME,
                           config->get_thumbnail_list()->size());
        var_creator.create(PAGE_COUNT_VAR_NAME, page_count);

        var_creator.create(CURRENT_PAGE_VAR_NAME, page_no);

        TemplateExecutor<typename Response::Writer> executor(r->pool, o);

        executor.exec(tmpl->get_node_tree(), var_creator.get_variables(),
                      tmpl->get_key_count());
    } catch(const char *message) {
        return error_with_log<Response>(r, o, config, message);
    }
    o.finish();

    return OK;
}

template<class Response>
static int upload_impl(typename Response::Handle *r, UploaderConfig *config,
                       const char *file_name, const char *file_mime,
                       apr_uint64_t file_size, const char *file_digest,
                       const char *file_temp_path, const char *remove_pass,
                       const char *download_pass, const char *comment,
                       const char *code_pat, apr_size_t *item_id)
{
    *item_id = config->get_item_manager()->add
        (r->pool, file_name, file_mime, file_size, file_digest,
         file_temp_path, remove_pass, download_pass, comment,
         code_pat, Response::get_remote_ip(r));

    regist_post(config, Response::get_remote_addr(r));

    Response::set_env(r, "uploader_upload", "");
    Response::set_env(r, "uploader_item_id",
                      apr_psprintf(r->pool, "%" APR_SIZE_T_FMT, *item_id));

    typename Response::Writer o(r);

    return redirect<Response>
        (r, o,
         apr_pstrcat(r->pool, config->base_url, ARG_SEPARATE_STR,
                     INFO_COMMAND, ARG_SEPARATE_STR,
                     apr_psprintf(r->pool, "%" APR_SIZE_T_FMT, *item_id),
                     NULL));
}

template<class Response>
static int upload(typename Response::Handle *r, UploaderConfig *config,
                  const char *arg)
{
    apr_uint32_t upload_id;
    RequestReader::post_progress_t *progress;
    apr_array_header_t *content_array;

    const char *file_name;
    const char *file_mime;
    apr_uint64_t file_size;
    const char *file_digest;
    const char *file_temp_path;
    const char *remove_pass;
    const char *download_pass;
    const char *comment;
    const char *code_pat;

    int status;

    if (!Response::is_post_method(r)) {
        return HTTP_BAD_REQUEST;
    }

    if ((status = Response::prepare_post_read(r)) != OK) {
        return status;
    }

    if (!can_post(config, Response::get_remote_addr(r))) {
        return error<Response>(r, config, MESSAGE_POST_INTERVAL_TOO_SHORT);
    }

    upload_id = static_cast<apr_uint32_t>
        (apr_atoi64(get_word(r->pool, &arg, ARG_SEPARATE_STR[0])));

    progress = config->get_post_progress_list()->add(upload_id);

    typename Response::Reader i(progress, r);
    RFC1867Parser<typename Response::Reader> parser
        (r->pool, i, config->temp_dir_path,
         ITM_MAX_COMMENT_SIZE, config->file_size_limit,
         PST_MAX_ITEM_COUNT);

    progress->total_size = Response::get_content_size(r);
    try {
        content_array = parser.parse(Response::get_content_type(r),
                                     Response::get_content_size(r));
        progress->end_time = apr_time_now();
    } catch(const char *message) {
        progress->end_time = apr_time_now();
        return error_with_log<Response>(r, config, message);
    }

    file_temp_path = NULL;
    try {
        apr_size_t item_id;

        get_upload_param<typename Response::Reader>
            (r->pool, content_array,
             &file_name, &file_mime, &file_size, &file_digest, &file_temp_path,
             &remove_pass, &download_pass, &comment, &code_pat);

        status = upload_impl<Response>
            (r, config,
             file_name, file_mime, file_size, file_digest, file_temp_path,
             remove_pass, download_pass, comment, code_pat, &item_id);

        progress->item_id = item_id;
        progress->is_succeeded  = true;

        return status;
    } catch(const char *message) {
        if (file_temp_path != NULL) {
            apr_file_remove(file_temp_path, r->pool);
        }
        return error_with_log<Response>(r, config, message);
    }
}

template<class Response>
static int progress_data(typename Response::Handle *r, UploaderConfig *config,
                         const char *arg)
{
    apr_uint32_t upload_id;
    RequestReader::post_progress_t *progress;

    upload_id = static_cast<apr_uint32_t>
        (apr_atoi64(get_word(r->pool, &arg, ARG_SEPARATE_STR[0])));

    Response::set_content_type(r, "text/plain");
    Response::set_header(r, "Access-Control-Allow-Origin", "*");
    Response::header_end(r);

    typename Response::Writer o(r);

    // upload_id が 0 の時はエラー．
    if (UNLIKELY(upload_id == 0)) {
        o.write("e");
        o.finish();

        return OK;
    }

    progress = config->get_post_progress_list()->get(upload_id);

    // ダミーかどうかチェック
    if (UNLIKELY(progress->id != upload_id)) {
        o.write("u");
        o.finish();

        return OK;
    }

    o.write(apr_psprintf(r->pool, "%c %" APR_UINT64_T_FMT " %" APR_UINT64_T_FMT,
                         ((progress->end_time != PostProgressList::ENTRY_USED) &&
                          progress->is_succeeded)
                         ? 'S' : 's',
                         progress->total_size, progress->read_size));
    o.finish();

    return OK;
}

template<class Response>
static int mail_upload(typename Response::Handle *r, UploaderConfig *config,
                       const char *arg)
{
    int status;

    const char *file_name;
    const char *file_mime;
    apr_uint64_t file_size;
    const char *file_digest;
    const char *file_temp_path;
    const char *remove_pass;
    const char *download_pass;
    const char *comment;

    RequestReader::post_progress_t progress;
    apr_array_header_t *content_array;

    if (!Response::is_post_method(r)) {
        return HTTP_BAD_REQUEST;
    }

    if ((status = Response::prepare_post_read(r)) != OK) {
        return status;
    }

    typename Response::Reader i(&progress, r);

    RFC2822Parser<typename Response::Reader> parser
        (r->pool, i, config->temp_dir_path,
         ITM_MAX_COMMENT_SIZE, config->file_size_limit,
         PST_MAX_ITEM_COUNT);

    content_array = parser.parse();

    file_temp_path = NULL;
    try {
        apr_size_t item_id;

        get_mail_upload_param<typename Response::Reader>
            (r->pool, content_array,
             &file_name, &file_mime, &file_size, &file_digest, &file_temp_path,
             &remove_pass, &download_pass, &comment);

        return upload_impl<Response>
            (r, config,
             file_name, file_mime, file_size, file_digest, file_temp_path,
             remove_pass, download_pass, comment, "", &item_id);
    } catch(const char *message) {
        if (file_temp_path != NULL) {
            apr_file_remove(file_temp_path, r->pool);
        }
        return error_with_log<Response>(r, config, message);
    }

    return OK;
}

template<class Response>
static int input_pass(typename Response::Handle *r, UploaderConfig *config,
                      UploadItem *uitem)
{
    Response::set_content_type(r, PAGE_CONTENT_TYPE);
    Response::header_end(r);

    if (Response::is_head_method(r)) {
        return OK;
    }

    typename Response::Writer o(r);
    try {
        UploaderTemplate *tmpl;

        tmpl = config->get_template(UploaderConfig::DOWNLOAD_VIEW);

        TemplateVariableCreator var_creator(r->pool, tmpl->get_ids());

        var_creator.create(BASE_URL_VAR_NAME, config->base_url);

        var_creator.create
            (ITEM_VAR_NAME,
             tmpl->get_item_var_creator()->create(r->pool, uitem));

        TemplateExecutor<typename Response::Writer> executor(r->pool, o);

        executor.exec(tmpl->get_node_tree(), var_creator.get_variables(),
                      tmpl->get_key_count());
    } catch(const char *message) {
        return error_with_log<Response>(r, o, config, message);
    }
    o.finish();

    return OK;
}

template<class Response>
static int download_set_header(typename Response::Handle *r, UploadItem *uitem,
                               apr_file_t *item_file)
{
    Response::set_content_type(r, uitem->get_file_mime(), true);

#ifdef UPLOADER_TYPE_APACHE
#if AP_SERVER_MAJORVERSION_NUMBER != 1
    if (item_file != NULL) {
        const char *item_path;

        apr_file_name_get(&item_path, item_file);
        r->filename = const_cast<char *>(item_path);
        r->finfo.filetype = APR_REG;
        r->finfo.size = uitem->get_file_size();
#if 0
        r->finfo.inode = uitem->get_id(); // ETag 生成用

        Response::set_etag(r);
#endif
    }
#endif
#endif

#ifndef DEBUG
    Response::set_modified_time(r, uitem->get_mtime());
    Response::set_last_modified(r, uitem->get_mtime());

    int status;
    if ((status = Response::is_meets_condition(r)) != OK) {
        return status;
    }
#endif

    Response::set_accept_ranges(r);

    return OK;
}

// TODO: ApacheResponse.h に移動させる
template<class Response>
static bool is_gzip_accepted(typename Response::Handle *r)
{
#ifdef UPLOADER_TYPE_APACHE
    const char *accepted;

    accepted = apr_table_get(r->headers_in, "Accept-Encoding");
    if (accepted == NULL) {
        return false;
    }
    return strstr(accepted, "gzip") != NULL;
#else
    return true;
#endif
}

template<class Response>
static int download_impl(typename Response::Handle *r, UploaderConfig *config,
                         UploadItem *uitem, apr_file_t *item_file,
                         const char *mode)
{
    int status;

    status = download_set_header<Response>(r, uitem, item_file);
    if (status != OK) {
        Response::header_end(r);
        return status;
    }

    if (Response::is_head_method(r)) {
        Response::header_end(r);
        return OK;
    }

    Response::set_expires(r, apr_time_now() +  apr_time_from_sec(VIW_EXPIRES));
    Response::set_content_disposition(r, mode);

#if defined(USE_GZIP) && defined(UPLOADER_TYPE_APACHE)
    if (uitem->is_gzipped()) {
        if (!is_gzip_accepted<Response>(r)) {
            typename Response::Writer o(r);

            return error<Response>(r, o, config, MESSAGE_DOWNLOAD_GZIP_NEEDED);
        }

        // mod_deflate が動かないようにする
        apr_table_setn(r->subprocess_env, "no-gzip", "on");

        apr_table_setn(r->headers_out, "Content-Encoding", "gzip");
    }
#endif

    Response::header_end(r);

    Response::set_env(r, "uploader_download", "");
    Response::set_env(r, "uploader_item_id",
                      apr_psprintf(r->pool, "%" APR_SIZE_T_FMT, uitem->get_id()));

    // better to use ap_internal_fast_redirect for Apache
    Response::Writer::sendfile(r, item_file, uitem->get_file_size());

    return OK;
}

template<class Response>
static int download_prep(typename Response::Handle *r, UploaderConfig *config,
                         UploadItem *uitem, bool is_admin_mode=false)
{
    const char *file_name;
    const char *command;
    int status;

    if (Response::is_head_method(r)) {
        status = download_set_header<Response>(r, uitem, NULL);

        Response::header_end(r);
        if (status != OK) {
            return status;
        } else {
            return OK;
        }
    }

#ifdef NUMNAME
    file_name = apr_psprintf(r->pool, "%s%5" APR_SIZE_T_FMT ".%s",
                             NUMNAME, uitem->get_index(),
                             uitem->get_file_ext());
#else
    file_name = CharCodeConverter::convert
        (r->pool, uitem->get_file_name(),
         CharCodeConverter::DEFAULT_CODE, "utf-8");
#endif

    if (is_admin_mode) {
        command = apr_pstrcat(r->pool, ADMIN_COMMAND, ARG_SEPARATE_STR,
                              DOWNLOAD_COMMAND, NULL);
    } else {
        command = DOWNLOAD_COMMAND;
    }

    typename Response::Writer o(r);

    return redirect<Response>
        (r, o,
         apr_pstrcat
         (r->pool, config->base_url, ARG_SEPARATE_STR,
          command, ARG_SEPARATE_STR,
          apr_psprintf(r->pool, "%" APR_SIZE_T_FMT, uitem->get_id()),
          ARG_SEPARATE_STR, "attach", ARG_SEPARATE_STR,
          rfc2396_encode(r->pool, file_name), NULL));
}

template<class Response>
static int download(typename Response::Handle *r, UploaderConfig *config,
                    const char *arg, bool is_admin_mode=false)
{
    apr_size_t item_id;
    const char *file_path;
    const char *item_id_str;

    file_path = get_word(r->pool, &arg, ARG_SEPARATE_STR[0]);
    if (strlen(file_path) == 0) {
        return HTTP_BAD_REQUEST;
    }

    item_id_str = apr_pstrdup(r->pool, file_path);
    item_id = static_cast<apr_size_t>
        (apr_atoi64(get_word(r->pool, &item_id_str, FILE_EXT_SEPARATOR[0])));

    try {
        UploadItem *uitem;
        const char *download_pass;
        apr_file_t *item_file;
        bool is_view_mode;

        if (Response::is_post_method(r)) {
            RequestReader::post_progress_t progress;
            apr_array_header_t *content_array;
            int status;

            if ((status = Response::prepare_post_read(r)) != OK) {
                return status;
            }

            typename Response::Reader i(&progress, r);
            RFC1867Parser<typename Response::Reader> parser
                (r->pool, i, config->temp_dir_path,
                 ITM_MAX_COMMENT_SIZE, 0, PST_MAX_ITEM_COUNT);

            content_array = parser.parse(Response::get_content_type(r),
                                         Response::get_content_size(r));

            download_pass = get_download_password<typename Response::Reader>
                (r->pool, content_array);
        } else {
            download_pass = "";
        }

        uitem = config->get_item_manager()->get_item(r->pool, item_id);
        is_view_mode = (strlen(get_word(r->pool, &arg,
                                        ARG_SEPARATE_STR[0])) == 0);

        // DL pass のチェック
        if (!is_admin_mode) {
            if ((strlen(uitem->get_download_pass()) != 0) &&
                (strncmp(uitem->get_download_pass(), download_pass,
                         strlen(uitem->get_download_pass())) != 0)) {
                if (strlen(download_pass) != 0) {
                    THROW(MESSAGE_DOWNLOAD_PASS_MISMATCH);
                }
                if (!is_view_mode &&
                    (strlen(get_word(r->pool, &arg, ARG_SEPARATE_STR[0])) == 0)) {
                    return download_prep<Response>(r, config, uitem);
                } else {
                    return input_pass<Response>(r, config, uitem);
                }
            }
        }

        DownloadSession download_session
            (config->get_download_flow_controller(),
             Response::get_remote_addr(r));
        if (!is_admin_mode && !download_session.is_valid()) {
            THROW(MESSAGE_DOWNLOAD_SESSION_LIMIT_EXCEEDED);
        }

        item_file = config->get_item_manager()
            ->get_item_file(r->pool, item_id,
                            Response::Writer::is_sendfile_enabled(r));

        config->get_item_manager()->update_atime(item_id);

        if (is_view_mode) {
            // 表示
            return download_impl<Response>(r, config, uitem, item_file,
                                           "inline");
        } else {
            if (strlen(get_word(r->pool, &arg, ARG_SEPARATE_STR[0])) == 0) {
                // ファイル名を追加した URL に飛ばす
                return download_prep<Response>(r, config, uitem, is_admin_mode);
            } else {
                // ダウンロード
                return download_impl<Response>(r, config, uitem, item_file,
                                               "attachment");
            }
        }
    } catch(const char *message) {
        return error<Response>(r, config, message);
    }

    return OK;
}

template<class Response>
static int remove(typename Response::Handle *r, UploaderConfig *config,
                  const char *arg, bool is_admin_mode=false)
{
    int status;

    if (!Response::is_post_method(r)) {
        return HTTP_BAD_REQUEST;
    }

    if ((status = Response::prepare_post_read(r)) != OK) {
        return status;
    }

    typename Response::Writer o(r);
    try {
        UploadItem *uitem;
        apr_size_t item_id;
        const char *remove_pass;
        RequestReader::post_progress_t progress;
        apr_array_header_t *content_array;

        typename Response::Reader i(&progress, r);
        RFC1867Parser<typename Response::Reader> parser
            (r->pool, i, config->temp_dir_path,
             ITM_MAX_COMMENT_SIZE, 0, PST_MAX_ITEM_COUNT);

        content_array = parser.parse(Response::get_content_type(r),
                                     Response::get_content_size(r));

        get_remove_param<typename Response::Reader>
            (r->pool, content_array, &item_id, &remove_pass);

        uitem = config->get_item_manager()->get_item(r->pool, item_id);

        if (!is_admin_mode) {
            if (
#ifndef EMPTY_PASSWORD
                (strlen(uitem->get_remove_pass()) == 0) ||
#endif
                (strncmp(remove_pass, uitem->get_remove_pass(),
                         strlen(uitem->get_remove_pass())) != 0)) {
                THROW(MESSAGE_REMOVE_PASS_MISMATCH);
            }
        }

        config->get_item_manager()->remove(r->pool, uitem);

        if (is_admin_mode) {
            return redirect<Response>
                (r, o,
                 apr_pstrcat(r->pool, config->base_url, ARG_SEPARATE_STR,
                             ADMIN_COMMAND, ARG_SEPARATE_STR, arg, NULL));
        } else {
            Response::set_env(r, "uploader_remove", "");
            Response::set_env(r, "uploader_item_id",
                              apr_psprintf(r->pool, "%" APR_SIZE_T_FMT, item_id));

            return redirect<Response>
                (r, o,
                 apr_pstrcat(r->pool, config->base_url, ARG_SEPARATE_STR,
                             NULL));
        }
    } catch(const char *message) {
        return error<Response>(r, config, message);
    }

    return OK;
}

template<class Response>
static int rss(typename Response::Handle *r, UploaderConfig *config,
               const char *arg)
{
    Response::set_content_type
        (r, UploadItemRss<typename Response::Writer>::CONTENT_TYPE);
#ifndef DEBUG
    Response::set_modified_time(r, config->get_item_manager()->get_mtime());
    Response::set_last_modified(r, config->get_item_manager()->get_mtime());
    Response::header_end(r);

    int status;
    if ((status = Response::is_meets_condition(r)) != OK) {
        return status;
    }
#endif
    if (Response::is_head_method(r)) {
        return OK;
    }

    typename Response::Writer o(r);
    try {
        UploadItemIterator item_iter(r->pool, config->get_item_manager(), 0,
                                     config->per_page_item_number);

        UploadItemRss<typename Response::Writer> item_rss(r->pool, o);
        item_rss.print(r->pool, config->base_url, &item_iter,
                       config->get_item_manager()->get_mtime());
    } catch(const char *message) {
        return error_with_log<Response>(r, o, config, message);
    }
    o.finish();

    return OK;
}

template<class Response>
static int admin_info(typename Response::Handle *r, UploaderConfig *config)
{
    Response::set_content_type(r, "text/plain");
    Response::header_end(r);

    if (Response::is_head_method(r)) {
        return OK;
    }

    typename Response::Writer o(r);

    o.write(config->to_string(r->pool));
    o.finish();

    return OK;
}

template<class Response>
static int admin(typename Response::Handle *r, UploaderConfig *config,
                 const char *arg)
{
    const char *admin_arg;
    const char *admin_command;
    apr_size_t page_no;
    apr_size_t page_count;

    Response::set_content_type(r, PAGE_CONTENT_TYPE);
    Response::header_end(r);

    if (Response::is_head_method(r)) {
        return OK;
    }

    // コマンド
    admin_arg = arg;
    admin_command = get_word(r->pool, &admin_arg, ARG_SEPARATE_STR[0]);

    if (STR_CMP_LITERAL(admin_command, DOWNLOAD_COMMAND)) {
        return download<Response>(r, config, admin_arg, true);
    } else if (strcmp(admin_command, REMOVE_COMMAND) == 0) {
        return remove<Response>(r, config, admin_arg, true);
    } else if (STR_CMP_LITERAL(admin_command, ADMIN_INFO_COMMAND)) {
        return admin_info<Response>(r, config);
    }

    page_count = (config->get_item_list()->size() == 0)
        ? 1
        : (config->get_item_list()->size()-1)/config->per_page_item_number + 1;
    get_page(r->pool, admin_arg, page_count, &page_no);

    typename Response::Writer o(r);
    try {
        UploaderTemplate *tmpl;
        apr_size_t start_index;
        apr_size_t end_index;

        end_index = page_no * config->per_page_item_number;
        start_index = end_index - config->per_page_item_number;

        UploadItemIterator item_iter(r->pool, config->get_item_manager(),
                                     start_index, end_index);

        tmpl = config->get_template(UploaderConfig::ADMIN_VIEW);

        TemplateVariableCreator var_creator(r->pool, tmpl->get_ids());

        var_creator.create(BASE_URL_VAR_NAME, config->base_url);

        var_creator.create
            (ITEM_LIST_VAR_NAME,
             tmpl->get_item_var_creator()->create(r->pool, &item_iter));

        var_creator.create(MAX_FILE_SIZE_VAR_NAME,
                           size_str(r->pool, config->file_size_limit));
        var_creator.create
            (TOTAL_FILE_SIZE_VAR_NAME,
             size_str(r->pool,
                      config->get_item_list()->get_total_file_size()));
        var_creator.create(TOTAL_FILE_NUMBER_VAR_NAME, config->get_item_list()->size());
        var_creator.create(PAGE_COUNT_VAR_NAME, page_count);

        var_creator.create(CURRENT_PAGE_VAR_NAME, page_no);

        TemplateExecutor<typename Response::Writer> executor(r->pool, o);

        executor.exec(tmpl->get_node_tree(), var_creator.get_variables(),
                      tmpl->get_key_count());
    } catch(const char *message) {
        return error_with_log<Response>(r, o, config, message);
    }
    o.finish();

    return OK;
}

template<class Response>
int uploader_command_handler(typename Response::Handle *r, UploaderConfig *config,
                             const char *arg)
{
    const char *command;
    const char *request_uri;

    if (UNLIKELY(strlen(arg) > HDL_MAX_PATH_INFO_SIZE)) {
        THROW(MESSAGE_HANDLER_PATH_INFO_TOO_LONG);
    }

    if (UNLIKELY(*arg == '\0')) {
        request_uri = Response::get_request_uri(r);

        if (UNLIKELY(*(request_uri + strlen(request_uri) - 1) ==
                     ARG_SEPARATE_STR[0])) {
            return error<Response>(r, config,
                                   MESSAGE_ENVIRONMENT_LOCATION_DIR_EXIST);
        }

        Response::set_location(r,
                               apr_pstrcat(r->pool, request_uri, "/", NULL));

        return HTTP_TEMPORARY_REDIRECT;
    } else {
        arg++;
    }

    command = get_word(r->pool, &arg, ARG_SEPARATE_STR[0]);

    if ((*command == '\0') || STR_CMP_LITERAL(command, INDEX_COMMAND)) {
        return index<Response>(r, config, arg);
    } else if (STR_CMP_LITERAL_START_WITH(command, SORT_COMMAND)) {
        return sort<Response>(r, config, arg,
                              command + LITERAL_STRLEN(SORT_COMMAND));
    } else if (STR_CMP_LITERAL(command, INFO_COMMAND)) {
        return info<Response>(r, config, arg);
    } else if (STR_CMP_LITERAL(command, THUMBNAIL_COMMAND)) {
        return thumbnail<Response>(r, config, arg);
    } else if (STR_CMP_LITERAL(command, UPLOAD_COMMAND)) {
        return upload<Response>(r, config, arg);
    } else if (STR_CMP_LITERAL(command, PROGRESS_DATA_COMMAND)) {
        return progress_data<Response>(r, config, arg);
    } else if (STR_CMP_LITERAL(command, DOWNLOAD_COMMAND)) {
        return download<Response>(r, config, arg);
    } else if (strcmp(command, REMOVE_COMMAND) == 0) {
        return remove<Response>(r, config, arg);
    } else if (STR_CMP_LITERAL(command, RSS_COMMAND)) {
        return rss<Response>(r, config, arg);
    } else if (STR_CMP_LITERAL(command, MAIL_COMMAND)) {
        return mail_upload<Response>(r, config, arg);
    } else if (STR_CMP_LITERAL(command, ADMIN_COMMAND)) {
        return admin<Response>(r, config, arg);
    } else {
        return error<Response>(r, config, MESSAGE_HANDLER_COMMAND_INVALID);
    }

    return OK;
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
