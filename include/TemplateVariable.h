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
 * $Id: TemplateVariable.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_VARIABLE_H
#define TEMPLATE_VARIABLE_H

#include "Environment.h"

#include <cstdlib>

#include "apr.h"

#include "Uncopyable.h"


/**
 * @brief テンプレートの変数を表すクラス
 */
class TemplateVariable: public Uncopyable
{
public:
    enum variable_type_t {
        END,            // 配列の終了
        SCALAR,         // スカラ
        ARRAY,          // 配列
        HASH,           // ハッシュ
    };

    enum scalar_type_t {
        STRING,         // 文字列
        INTEGER,        // 整数
    };

    typedef struct Scalar {
        scalar_type_t type;
        union {
            int i;
            struct {
                const char *s;
                apr_size_t l;
            };
        };
    } scalar_t;

    typedef struct Variable {
        variable_type_t type;
        union {
            scalar_t *s;        // スカラ
            struct Variable *v; // 配列 or ハッシュ
        };
    } variable_t;

#ifdef DEBUG
    static void print_dot(const variable_t *var);
#endif

private:
#ifdef DEBUG
    typedef struct {
        variable_type_t type;
        const char *str;
    } variable_type_str;
    typedef struct {
        scalar_type_t type;
        const char *str;
    } scalar_type_str;
    static const variable_type_str variable_type_list[];
    static const scalar_type_str scalar_type_list[];

    static apr_size_t print_dot_variable(const variable_t *var);
    static const char *get_variable_type(const variable_t *node);
    static const char *get_scalar_type(const scalar_t *node);
#endif
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
