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
 * $Id: TemplateVariableCreator.cpp 2872 2008-04-27 14:18:02Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <cstring>
#include <algorithm>

#include "TemplateVariableCreator.h"
#include "Macro.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: TemplateVariableCreator.cpp 2872 2008-04-27 14:18:02Z svn $");

const apr_size_t TemplateVariableCreator::VAR_SCR_POOL_SIZE = 10;

/******************************************************************************
 * public メソッド
 *****************************************************************************/
TemplateVariableCreator::TemplateVariableCreator(apr_pool_t *pool,
                                                 const char **ids)
  : pool_(pool),
    ids_(ids),
    var_scr_pool_(NULL),
    var_scr_pool_end_(NULL)
{
    // ダミー用に 1 つ分多く確保
    APR_PALLOC(variables_, variable_t **, pool_,
               sizeof(variable_t *) * (get_entry_count(ids) + 1));

    memset(variables_, 0, sizeof(variable_t *) * (get_entry_count(ids) + 1));
}

void TemplateVariableCreator::create(const char *name, const char *value)
{
    variable_t *var;
    scalar_t *scr;

    var = get_var_scr_memory();
    scr = reinterpret_cast<scalar_t *>(var + 1);

    scr->type = TemplateVariable::STRING;
    scr->s    = value;
    scr->l    = 0;

    var->type = TemplateVariable::SCALAR;
    var->s    = scr;

    return create(get_id(name), var);
}

void TemplateVariableCreator::create(const char *name, int value)
{
    variable_t *var;
    scalar_t *scr;

    var = get_var_scr_memory();
    scr = reinterpret_cast<scalar_t *>(var + 1);

    scr->type = TemplateVariable::INTEGER;
    scr->i    = value;

    var->type = TemplateVariable::SCALAR;
    var->s    = scr;

    return create(get_id(name), var);
}

void TemplateVariableCreator::create(const char *name, apr_size_t value)
{
    create(name, static_cast<int>(value));
}

apr_size_t TemplateVariableCreator::get_id(const char *name) const
{
    apr_size_t i;

    for (i = 0; ids_[i] != NULL; i++) {
        if (strcmp(ids_[i], name) == 0) {
            return i;
        }
    }

    return i;
}

apr_size_t TemplateVariableCreator::get_entry_count(const char **entries)
{
    apr_size_t i = 0;

    while (entries[i] != NULL) {
        i++;
    }

    return i;
}

apr_size_t TemplateVariableCreator::get_array_memory_size(apr_array_header_t *array)
{
    apr_size_t size;

    size = sizeof(char *) * (array->nelts + 1);
    for (int i = 0; i < array->nelts; i++) {
        size += strlen(AS_CONST_CHAR_P(AVOID_ALIGN_WARN array->elts)[i]) + 1;
    }

    // 処理の都合上 4 の倍数に丸める
    if (size & 3) {
        size = (size & ~3) + 4;
    }

    return size;
}

const char **TemplateVariableCreator::convert_array(apr_array_header_t *array,
                                                    void *memory)
{
    char **table;
    char *value;
    int i;

    table = AS_CHAR_P(memory);
    value = AS_CHAR(table + array->nelts + 1);

    for (i = 0; i < array->nelts; i++) {
        table[i] = value;
        strncpy(value, AS_CONST_CHAR_P(AVOID_ALIGN_WARN array->elts)[i],
                strlen(AS_CONST_CHAR_P(AVOID_ALIGN_WARN array->elts)[i])+1);
        value += strlen(AS_CONST_CHAR_P(AVOID_ALIGN_WARN array->elts)[i]) + 1;
    }
    table[i] = NULL;

    return AS_CONST_CHAR_P(table);
}

const char **TemplateVariableCreator::convert_array(apr_pool_t *pool,
                                                    apr_array_header_t *array)
{
    void *memory;

    APR_PALLOC(memory, void **, pool, get_array_memory_size(array));

    return convert_array(array, memory);
}

apr_size_t TemplateVariableCreator::calc_index(const char **keys,
                                               key_index_t *index_list,
                                               apr_size_t index_list_size)
{
    bool is_defined_all;
    apr_size_t max_index;

    for (apr_size_t i = 0; i < index_list_size; i++) {
        for (apr_size_t j = 0; keys[j] != NULL; j++) {
            if (strcmp(keys[j], index_list[i].name) == 0) {
                index_list[i].index = j;
            }
        }
    }

    is_defined_all = true;
    max_index = 0;

    for (apr_size_t i = 0; i < index_list_size; i++) {
        if (index_list[i].index == KeyIndex::UNDEFINED) {
            is_defined_all = false;
            continue;
        }
        if (index_list[i].index > max_index) {
            max_index = index_list[i].index;
        }
    }

    if (is_defined_all) {
        return max_index;
    }

    max_index++;

    for (apr_size_t i = 0; i < index_list_size; i++) {
        if (index_list[i].index == KeyIndex::UNDEFINED) {
            index_list[i].index = max_index;
        }
    }

    return max_index;
}

/******************************************************************************
 * public メソッド
 *****************************************************************************/
TemplateVariableCreator::variable_t *TemplateVariableCreator::get_var_scr_memory()
{
    variable_t *memory;

    if (var_scr_pool_ == var_scr_pool_end_) {
        APR_PALLOC(var_scr_pool_, long *, pool_,
                   (sizeof(variable_t)+sizeof(scalar_t)) * VAR_SCR_POOL_SIZE);

        var_scr_pool_end_ = var_scr_pool_ +
            (PAD_DIV(sizeof(variable_t)+sizeof(scalar_t), sizeof(long)) *
             VAR_SCR_POOL_SIZE);
    }

    memory = reinterpret_cast<variable_t *>(var_scr_pool_);
    var_scr_pool_ += PAD_DIV(sizeof(variable_t)+sizeof(scalar_t), sizeof(long));

    return memory;
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
