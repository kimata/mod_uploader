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
 * $Id: ThumbnailVariableCreator.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef THUMBNAIL_VARIABLE_CREATOR_H
#define THUMBNAIL_VARIABLE_CREATOR_H

#include "Environment.h"

#include <cstdlib>

#include "TemplateVariableCreator.h"
#include "TemplateVariable.h"
#include "Uncopyable.h"


class ThumbnailIterator;

/**
 * @brief テンプレートの変数を生成するクラス
 */
class ThumbnailVariableCreator: public Uncopyable
{
public:
    ThumbnailVariableCreator(const char **keys);

    TemplateVariable::variable_t *create(apr_pool_t *pool,
                                         ThumbnailIterator *thumbnail_iter) const;

    static ThumbnailVariableCreator *get_instance(void *memory,
                                                  const char **keys);

private:
    typedef TemplateVariable::variable_t            variable_t;
    typedef TemplateVariable::scalar_t              scalar_t;
    typedef TemplateVariableCreator::key_index_t    key_index_t;

    typedef struct ThumbnailKeyIndex {
        key_index_t id;

        ThumbnailKeyIndex()
          : id("id")
        {

        }
    } thumbnail_key_index_t;

    void init();

    TemplateVariable::variable_t *create_thumbnail(apr_pool_t *pool,
                                                   apr_size_t item_id,
                                                   void *var_memory,
                                                   void *sca_memory) const;

    apr_size_t get_thumbnail_array_memory_size(apr_size_t list_size) const;
    apr_size_t get_thumbnail_memory_size() const;

    const char **keys_;

    thumbnail_key_index_t thumbnail_index_;
    apr_size_t thumbnail_index_max_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
