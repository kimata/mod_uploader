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
 * $Id: UploaderTemplate.cpp 2907 2009-01-14 12:56:44Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <memory>

#include "apr_shm.h"

#include "UploaderTemplate.h"
#include "File.h"
#include "TemplateLexer.h"
#include "TemplateVariableCreator.h"
#include "UploadItemVariableCreator.h"
#include "ThumbnailVariableCreator.h"
#include "Auxiliary.h"
#include "SourceInfo.h"

using namespace std;

SOURCE_INFO_ADD("$Id: UploaderTemplate.cpp 2907 2009-01-14 12:56:44Z svn $");

/******************************************************************************
 * public メソッド
 *****************************************************************************/
UploaderTemplate::UploaderTemplate(apr_pool_t *pool, const char *file_path)
  : pool_(pool),
    tmpl_pool_(NULL),
    file_path_(file_path),
    node_shm_(NULL),
    ids_and_keys_shm_(NULL),
    item_var_creator_shm_(NULL),
    thumbnail_var_creator_shm_(NULL),
    node_tree_(NULL),
    ids_(NULL),
    keys_(NULL),
    key_count_(0),
    item_var_creator_(NULL),
    thumbnail_var_creator_(NULL)
{

}

void UploaderTemplate::load()
{
    // TODO: この shm の attatch をちゃんとする
    apr_shm_t *node_shm;
    apr_shm_t *ids_and_keys_shm;
    apr_shm_t *item_var_creator_shm;
    apr_shm_t *thumbnail_var_creator_shm;
    const node_t *node_tree;
    const char **ids;
    const char **keys;
    apr_size_t key_count;
    UploadItemVariableCreator *item_var_creator;
    ThumbnailVariableCreator *thumbnail_var_creator;
    Lexer *lexer;
    TemporaryPool *tmpl_pool;

    tmpl_pool = new TemporaryPool(pool_);

    lexer = Lexer::get_instance(tmpl_pool->get(), file_path_);
    auto_ptr<Lexer> lexer_ap(lexer);

    node_shm = create_shm
        (tmpl_pool->get(),
         Parser::calc_node_memory_size(lexer->get_token_array()));

    Parser parser(tmpl_pool->get(), apr_shm_baseaddr_get(node_shm),
                          lexer);
    parser.parse();
    node_tree = parser.get_node_tree();

    ids_and_keys_shm = create_shm
        (tmpl_pool->get(),
         VariableCreator::get_array_memory_size(lexer->get_id_array()) +
         VariableCreator::get_array_memory_size(parser.get_key_array()));

    ids = VariableCreator::convert_array
        (lexer->get_id_array(),
         apr_shm_baseaddr_get(ids_and_keys_shm));
    keys = VariableCreator::convert_array
        (parser.get_key_array(),
         AS_CHAR(apr_shm_baseaddr_get(ids_and_keys_shm)) +
         VariableCreator::get_array_memory_size(lexer->get_id_array()));
    key_count = VariableCreator::get_entry_count(keys);

    item_var_creator_shm = create_shm
        (tmpl_pool->get(), sizeof(UploadItemVariableCreator));
    thumbnail_var_creator_shm = create_shm
        (tmpl_pool->get(), sizeof(ThumbnailVariableCreator));

    item_var_creator = UploadItemVariableCreator::get_instance
        (apr_shm_baseaddr_get(item_var_creator_shm), keys);
    thumbnail_var_creator = ThumbnailVariableCreator::get_instance
        (apr_shm_baseaddr_get(thumbnail_var_creator_shm), keys);

    if (tmpl_pool_ != NULL) {
        apr_shm_destroy(node_shm_);
        apr_shm_destroy(ids_and_keys_shm_);
        apr_shm_destroy(item_var_creator_shm_);
        apr_shm_destroy(thumbnail_var_creator_shm_);

        delete tmpl_pool_;
    }

    tmpl_pool_ = tmpl_pool;

    node_shm_          = node_shm;
    ids_and_keys_shm_  = ids_and_keys_shm;
    item_var_creator_shm_       = item_var_creator_shm;
    thumbnail_var_creator_shm_  = thumbnail_var_creator_shm;

    node_tree_  = node_tree;
    ids_        = ids;
    keys_       = keys;
    key_count_  = key_count;

    item_var_creator_ = item_var_creator;
    thumbnail_var_creator_ = thumbnail_var_creator;

    File tmpl_file(tmpl_pool_->get(), file_path_);
    mtime_ = tmpl_file.get_mtime();
}

void UploaderTemplate::load(const char *file_path)
{
    file_path_ = file_path;
    load();
}

bool UploaderTemplate::update()
{
    apr_time_t curr_mtime;
    TemporaryPool tmpl_pool;
    File tmpl_file(tmpl_pool.get(), file_path_);

    curr_mtime = tmpl_file.get_mtime();
    if (curr_mtime <= mtime_) {
        return false;
    }

    load();

    return true;
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
