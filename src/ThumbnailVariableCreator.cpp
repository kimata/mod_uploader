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
 * $Id: ThumbnailVariableCreator.cpp 2874 2008-04-27 14:38:46Z svn $
 *****************************************************************************/

#include "Environment.h"

#include "ThumbnailVariableCreator.h"
#include "ThumbnailIterator.h"
#include "Auxiliary.h"
#include "Macro.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: ThumbnailVariableCreator.cpp 2874 2008-04-27 14:38:46Z svn $");

/******************************************************************************
 * public メソッド
 *****************************************************************************/
ThumbnailVariableCreator::ThumbnailVariableCreator(const char **keys)
  : keys_(keys)
{
    init();
}

TemplateVariable::variable_t *
ThumbnailVariableCreator::create(apr_pool_t *pool,
                                 ThumbnailIterator *thumbnail_iter) const
{
    void *memory;
    variable_t *thumbnail_array_memory;
    scalar_t *thumbnail_memory;
    variable_t *var;

    // メモリを一気に確保
    APR_PALLOC(memory, void *, pool,
               PAD_DIV(sizeof(variable_t) +
                       get_thumbnail_array_memory_size(thumbnail_iter->size()),
                       sizeof(variable_t)) * sizeof(variable_t) +
               (get_thumbnail_memory_size() * thumbnail_iter->size()));

    thumbnail_array_memory = AS_VARIABLE(memory);
    thumbnail_memory
        = AS_SCALAR(thumbnail_array_memory +
                    PAD_DIV(sizeof(variable_t) +
                            get_thumbnail_array_memory_size(thumbnail_iter->size()),
                            sizeof(variable_t)));

    var = thumbnail_array_memory++;
    var->type = TemplateVariable::ARRAY;
    var->v = thumbnail_array_memory;

    for (apr_size_t i = 0; i < thumbnail_iter->size(); i++) {
        create_thumbnail(pool, thumbnail_iter->get(), thumbnail_array_memory++, thumbnail_memory);

        thumbnail_memory += thumbnail_index_max_ + 1; // +1 でインデックスから個数に変換
        thumbnail_iter->next();
    }

    thumbnail_array_memory->type = TemplateVariable::END;

    return var;
}

ThumbnailVariableCreator *ThumbnailVariableCreator::get_instance(void *memory,
                                                                 const char **keys)
{
    new(memory) ThumbnailVariableCreator(keys);

    return reinterpret_cast<ThumbnailVariableCreator *>(memory);
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void ThumbnailVariableCreator::init()
{
    thumbnail_index_max_ = TemplateVariableCreator::calc_index
        (keys_, AS_KEY_INDEX(&thumbnail_index_),
         sizeof(thumbnail_key_index_t)/sizeof(key_index_t));
}

inline TemplateVariable::variable_t *
ThumbnailVariableCreator::create_thumbnail(apr_pool_t *pool,
                                           apr_size_t item_id,
                                           void *var_memory,
                                           void *sca_memory) const
{
    variable_t *thumbnail_var;
    scalar_t *thumbnail_sca;
    scalar_t *sca;

    thumbnail_var = AS_VARIABLE(var_memory);
    thumbnail_sca = AS_SCALAR(sca_memory);

    thumbnail_var->type = TemplateVariable::HASH;
    thumbnail_var->s = thumbnail_sca;

    sca         = thumbnail_sca + thumbnail_index_.id.index;
    sca->type   = TemplateVariable::INTEGER;
    sca->i      = static_cast<int>(item_id);

    return thumbnail_var;
}

apr_size_t ThumbnailVariableCreator::get_thumbnail_array_memory_size(apr_size_t list_size) const
{
    return sizeof(variable_t) * (list_size + 1);
}

apr_size_t ThumbnailVariableCreator::get_thumbnail_memory_size() const
{
    return sizeof(scalar_t) * (thumbnail_index_max_ + 1);
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_ThumbnailVariableCreator
#include "TemplateParser.h"
#include "UploadItemList.h"
#include "UploadItemListReader.h"
#include "ThumbnailList.h"
#include "ThumbnailIterator.h"
#include "Message.h"

#include "TestRunner.h"

static const apr_size_t THUMBNAIL_LIST_SIZE  = 10;
static const apr_size_t MAX_LIST_SIZE   = 1000;

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << " <DATA_DIR_PATH> <TMPL_FILE_PATH>" << endl;
}

void run_create(apr_pool_t *pool, ThumbnailList *thumbnail_list,
                const char *data_dir_path, const char *tmpl_file_path)
{
    const char **ids;
    const char **keys;

    show_test_name("create variable");

    TemplateParser::parse(pool, tmpl_file_path, &ids, &keys);

    ThumbnailVariableCreator var_creator(keys);
    ThumbnailIterator thumbnail_iter(pool, thumbnail_list, 0, THUMBNAIL_LIST_SIZE);

    var_creator.create(pool, &thumbnail_iter);

    show_spacer();
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    const char *data_dir_path;
    const char *tmpl_file_path;
    apr_shm_t *item_list_shm;
    apr_shm_t *thumbnail_list_shm;
    UploadItemList *item_list;
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

    item_list_shm = create_shm(pool,
                               UploadItemList::get_memory_size(MAX_LIST_SIZE));
    thumbnail_list_shm = create_shm(pool,
                                    ThumbnailList::get_memory_size(MAX_LIST_SIZE));

    thumbnail_list = ThumbnailList::get_instance(thumbnail_list_shm,
                                                 MAX_LIST_SIZE);
    item_list = UploadItemListReader::read(pool, data_dir_path, item_list_shm,
                                           thumbnail_list, MAX_LIST_SIZE);

    show_item("data dir", data_dir_path);
    show_item("template file", tmpl_file_path);
    show_line();

    run_create(pool, thumbnail_list, data_dir_path, tmpl_file_path);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
