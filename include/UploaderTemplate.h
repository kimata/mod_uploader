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
 * $Id: UploaderTemplate.h 2907 2009-01-14 12:56:44Z svn $
 *****************************************************************************/

#ifndef UPLOADER_TEMPLATE_H
#define UPLOADER_TEMPLATE_H

#include "Environment.h"

#include "apr_shm.h"
#include "apr_time.h"

#include "TemplateParser.h"
#include "TemporaryPool.h"
#include "Uncopyable.h"


class TemplateLexer;
class TemplateVariableCreator;
class UploadItemVariableCreator;
class ThumbnailVariableCreator;

/**
 * @brief アップローダのテンプレートを表すクラス
 */
class UploaderTemplate: public Uncopyable
{
public:
    UploaderTemplate(apr_pool_t *pool, const char *file_path);

    void load();
    void load(const char *file_path);
    bool update();

    const TemplateParser::node_t *get_node_tree() const
    {
        return node_tree_;
    };
    const char *get_file_path() const
    {
        return file_path_;
    };
    const char **get_ids()
    {
        return ids_;
    };
    const char **get_keys() const
    {
        return keys_;
    };
    apr_size_t get_key_count() const
    {
        return key_count_;
    };
    apr_time_t get_mtime() const
    {
        return mtime_;
    };
    UploadItemVariableCreator *get_item_var_creator() const
    {
        return item_var_creator_;
    };
    ThumbnailVariableCreator *get_thumbnail_var_creator() const
    {
        return thumbnail_var_creator_;
    };

private:
    typedef TemplateLexer               Lexer;
    typedef TemplateParser              Parser;
    typedef TemplateVariableCreator     VariableCreator;
    typedef Parser::node_t              node_t;

    void load_impl();

    apr_pool_t *pool_;
    TemporaryPool *tmpl_pool_;
    const char *file_path_;
    apr_time_t mtime_;

    apr_shm_t *node_shm_;
    apr_shm_t *ids_and_keys_shm_;
    apr_shm_t *item_var_creator_shm_;
    apr_shm_t *thumbnail_var_creator_shm_;

    const node_t *node_tree_;
    const char **ids_;
    const char **keys_;
    apr_size_t key_count_;

    UploadItemVariableCreator *item_var_creator_;
    ThumbnailVariableCreator *thumbnail_var_creator_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
