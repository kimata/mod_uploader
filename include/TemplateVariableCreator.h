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
 * $Id: TemplateVariableCreator.h 2873 2008-04-27 14:19:47Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_VARIABLE_CREATOR_H
#define TEMPLATE_VARIABLE_CREATOR_H

#include "Environment.h"

#include "apr_tables.h"

#include "TemplateVariable.h"


#define AS_VARIABLE(pointer)        reinterpret_cast<variable_t *>(pointer)
#define AS_SCALAR(pointer)          reinterpret_cast<scalar_t *>(pointer)
#define AS_KEY_INDEX(pointer)                                           \
    reinterpret_cast<TemplateVariableCreator::key_index_t *>(pointer)

/**
 * @brief テンプレートの変数を生成するクラス
 */
class TemplateVariableCreator
{
public:
    typedef struct KeyIndex {
        static const apr_size_t UNDEFINED   = 0xFFFF;

        const char *name;
        apr_size_t index;

        KeyIndex(const char *name_arg)
          : name(name_arg),
            index(UNDEFINED)
        {

        }
    } key_index_t;

    TemplateVariableCreator(apr_pool_t *pool, const char **ids);

    void create(apr_size_t id, TemplateVariable::variable_t *variable)
    {
        variables_[id] = variable;
    };
    void create(const char *name, TemplateVariable::variable_t *variable)
    {
        return create(get_id(name), variable);

    };
    void create(const char *name, const char *value);
    void create(const char *name, int value);
    void create(const char *name, apr_size_t value);

    apr_size_t get_id(const char *name) const;

    TemplateVariable::variable_t **get_variables()
    {
        return variables_;
    };

    static apr_size_t get_entry_count(const char **entries);
    static apr_size_t get_array_memory_size(apr_array_header_t *array);
    static const char **convert_array(apr_array_header_t *array, void *memory);
    static const char **convert_array(apr_pool_t *pool,
                                      apr_array_header_t *array);
    static apr_size_t calc_index(const char **keys, key_index_t *index_list,
                                 apr_size_t index_list_size);

private:
    typedef TemplateVariable::variable_t    variable_t;
    typedef TemplateVariable::scalar_t      scalar_t;

    static const apr_size_t VAR_SCR_POOL_SIZE;

    variable_t *get_var_scr_memory();

    apr_pool_t *pool_;
    const char **ids_;
    variable_t **variables_;

    long *var_scr_pool_;
    long *var_scr_pool_end_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
