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
 * $Id: UploadItemVariableCreator.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef UPLOAD_ITEM_VARIABLE_CREATOR_H
#define UPLOAD_ITEM_VARIABLE_CREATOR_H

#include "Environment.h"

#include "UploadItem.h"
#include "TemplateVariableCreator.h"
#include "TemplateVariable.h"
#include "Uncopyable.h"


class UploadItemIterator;

/**
 * @brief テンプレートの変数を生成するクラス
 */
class UploadItemVariableCreator: public Uncopyable
{
public:
    UploadItemVariableCreator(const char **keys);

    TemplateVariable::variable_t *create(apr_pool_t *pool,
                                         UploadItemIterator *item_iter) const;
    TemplateVariable::variable_t *create(apr_pool_t *pool,
                                         UploadItem *uitem) const;

    static UploadItemVariableCreator *get_instance(void *memory,
                                                   const char **keys);

private:
    typedef TemplateVariable::variable_t            variable_t;
    typedef TemplateVariable::scalar_t              scalar_t;
    typedef TemplateVariableCreator::key_index_t    key_index_t;

    typedef struct ItemKeyIndex {
        key_index_t id;
        key_index_t index;
        key_index_t download_count;
        key_index_t file_size;
        key_index_t date;
        key_index_t ip_address;
        key_index_t file_name;
        key_index_t file_mime;
        key_index_t file_ext;
        key_index_t file_digest;
        key_index_t comment;

        ItemKeyIndex()
          : id("id"),
            index("index"),
            download_count("download_count"),
            file_size("file_size"),
            date("date"),
            ip_address("ip_address"),
            file_name("file_name"),
            file_mime("file_mime"),
            file_ext("file_ext"),
            file_digest("file_digest"),
            comment("comment")
        {

        }
    } item_key_index_t;

    void init();

    TemplateVariable::variable_t *create_item(apr_pool_t *pool,
                                              UploadItem *uitem,
                                              void *var_memory,
                                              void *sca_memory) const;

    apr_size_t get_item_array_memory_size(apr_size_t list_size) const;
    apr_size_t get_item_memory_size() const;

    const char **keys_;

    item_key_index_t item_index_;
    apr_size_t item_index_max_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
