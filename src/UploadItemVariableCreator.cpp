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
 * $Id: UploadItemVariableCreator.cpp 2875 2008-04-27 14:42:48Z svn $
 *****************************************************************************/

#include "Environment.h"

#include "UploadItemVariableCreator.h"
#include "UploadItemIterator.h"
#include "Auxiliary.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: UploadItemVariableCreator.cpp 2875 2008-04-27 14:42:48Z svn $");

/******************************************************************************
 * public メソッド
 *****************************************************************************/
UploadItemVariableCreator::UploadItemVariableCreator(const char **keys)
  : keys_(keys)
{
    init();
}

TemplateVariable::variable_t *
UploadItemVariableCreator::create(apr_pool_t *pool,
                                  UploadItemIterator *item_iter) const
{
    void *memory;
    variable_t *item_array_memory;
    scalar_t *item_memory;
    variable_t *var;

    // メモリを一気に確保
    APR_PALLOC(memory, void *, pool,
               PAD_DIV(sizeof(variable_t) +
                       get_item_array_memory_size(item_iter->size()),
                       sizeof(variable_t)) * sizeof(variable_t) +
               (get_item_memory_size() * item_iter->size()));

    item_array_memory = AS_VARIABLE(memory);
    item_memory
        = AS_SCALAR(item_array_memory +
                    PAD_DIV(sizeof(variable_t) +
                            get_item_array_memory_size(item_iter->size()),
                            sizeof(variable_t)));

    var = item_array_memory++;
    var->type = TemplateVariable::ARRAY;
    var->v = item_array_memory;

    for (apr_size_t i = 0; i < item_iter->size(); i++) {
        create_item(pool, item_iter->get(), item_array_memory++, item_memory);

        item_memory += item_index_max_ + 1; // +1 でインデックスから個数に変換
        item_iter->next();
    }

    item_array_memory->type = TemplateVariable::END;

    return var;
}

TemplateVariable::variable_t *
UploadItemVariableCreator::create(apr_pool_t *pool, UploadItem *uitem) const
{
    void *memory;
    scalar_t *item_memory;
    variable_t *var;

    // メモリを一気に確保
    APR_PALLOC(memory, void *, pool,
               sizeof(variable_t) + get_item_memory_size());

    var = AS_VARIABLE(memory);
    item_memory = AS_SCALAR(var + 1);

    create_item(pool, uitem, var, item_memory);

    return var;
}

UploadItemVariableCreator *UploadItemVariableCreator::get_instance(void *memory,
                                                                   const char **keys)
{
    new(memory) UploadItemVariableCreator(keys);

    return reinterpret_cast<UploadItemVariableCreator *>(memory);
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void UploadItemVariableCreator::init()
{
    item_index_max_ = TemplateVariableCreator::calc_index
        (keys_, AS_KEY_INDEX(&item_index_),
         sizeof(item_key_index_t)/sizeof(key_index_t));
}

inline TemplateVariable::variable_t *
UploadItemVariableCreator::create_item(apr_pool_t *pool, UploadItem *uitem,
                                       void *var_memory,
                                       void *sca_memory) const
{
    variable_t *item_var;
    scalar_t *item_sca;
    scalar_t *sca;

    item_var = AS_VARIABLE(var_memory);
    item_sca = AS_SCALAR(sca_memory);

    item_var->type = TemplateVariable::HASH;
    item_var->s = item_sca;

    sca         = item_sca + item_index_.id.index;
    sca->type   = TemplateVariable::INTEGER;
    sca->i      = static_cast<int>(uitem->get_id());

    sca         = item_sca + item_index_.index.index;
    sca->type   = TemplateVariable::INTEGER;
    sca->i      = static_cast<int>(uitem->get_index());

    sca         = item_sca + item_index_.download_count.index;
    sca->type   = TemplateVariable::INTEGER;
    sca->i      = static_cast<int>(uitem->get_download_count());

    sca         = item_sca + item_index_.file_size.index;
    sca->type   = TemplateVariable::STRING;
    sca->s      = size_str(pool, uitem->get_file_size());
    sca->l      = 0;

    sca         = item_sca + item_index_.date.index;
    sca->type   = TemplateVariable::STRING;
    sca->s      = uitem->get_date();
    sca->l      = 0;

    sca         = item_sca + item_index_.ip_address.index;
    sca->type   = TemplateVariable::STRING;
    sca->s      = uitem->get_ip_address();
    sca->l      = 0;

    sca         = item_sca + item_index_.file_name.index;
    sca->type   = TemplateVariable::STRING;
    sca->s      = uitem->get_file_name();
    sca->l      = 0;

    sca         = item_sca + item_index_.file_mime.index;
    sca->type   = TemplateVariable::STRING;
    sca->s      = uitem->get_file_mime();
    sca->l      = 0;

    sca         = item_sca + item_index_.file_ext.index;
    sca->type   = TemplateVariable::STRING;
    sca->s      = uitem->get_file_ext();
    sca->l      = 0;

    sca         = item_sca + item_index_.file_digest.index;
    sca->type   = TemplateVariable::STRING;
    sca->s      = uitem->get_file_digest();
    sca->l      = 0;

    sca         = item_sca + item_index_.comment.index;
    sca->type   = TemplateVariable::STRING;
    sca->s      = uitem->get_comment();
    sca->l      = 0;

    return item_var;
}

apr_size_t UploadItemVariableCreator::get_item_array_memory_size(apr_size_t list_size) const
{
    return sizeof(variable_t) * (list_size + 1);
}

apr_size_t UploadItemVariableCreator::get_item_memory_size() const
{
    return sizeof(scalar_t) * (item_index_max_ + 1);
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_UploadItemVariableCreator
#include "TemplateParser.h"
#include "UploadItemList.h"
#include "UploadItemListReader.h"
#include "UploadItemIterator.h"
#include "ThumbnailList.h"
#include "Message.h"
#include "File.h"

#include "TestRunner.h"

static const apr_size_t ITEM_LIST_SIZE  = 10;
static const apr_uint64_t MAX_FILE_SIZE = 1000000;
static const apr_size_t MAX_LIST_SIZE   = 1000;

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << " <DATA_DIR_PATH> <TMPL_FILE_PATH>" << endl;
}

void run_create(apr_pool_t *pool, UploadItemList *item_list,
                const char *data_dir_path, const char *tmpl_file_path)
{
    const char **ids;
    const char **keys;

    show_test_name("create variable");

    TemplateParser::parse(pool, tmpl_file_path, &ids, &keys);

    UploadItemVariableCreator var_creator(keys);
    UploadItemIterator item_iter(pool, item_list, 0, ITEM_LIST_SIZE);

    var_creator.create(pool, &item_iter);

    show_spacer();
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    const char *data_dir_path;
    const char *tmpl_file_path;
    apr_shm_t *item_list_shm;
    apr_shm_t *thumbnail_list_shm;
    UploadItemList *item_list;
    apr_time_t mtime;
    ThumbnailList *thumbnail_list;

    if (argc != 3) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    data_dir_path = argv[1];
    tmpl_file_path = argv[2];

    if (!File::is_exist(pool, data_dir_path)) {
        THROW(MESSAGE_DATA_DIR_NOT_FOUND);
    }
    if (!File::is_exist(pool, tmpl_file_path)) {
        THROW(MESSAGE_TMPL_FILE_NOT_FOUND);
    }

    item_list_shm = create_shm
        (pool, UploadItemList::get_memory_size(MAX_LIST_SIZE));
    thumbnail_list_shm = create_shm
        (pool, ThumbnailList::get_memory_size(MAX_LIST_SIZE));

    thumbnail_list = ThumbnailList::get_instance
        (thumbnail_list_shm, MAX_LIST_SIZE);
    item_list = UploadItemListReader::read
        (pool, data_dir_path, data_dir_path, MAX_FILE_SIZE, MAX_LIST_SIZE,
         item_list_shm, thumbnail_list, &mtime);

    show_item("data dir", data_dir_path);
    show_item("template file", tmpl_file_path);
    show_line();

    run_create(pool, item_list, data_dir_path, tmpl_file_path);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
