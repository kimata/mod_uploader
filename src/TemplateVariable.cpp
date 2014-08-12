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
 * $Id: TemplateVariable.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <iostream>

#include "TemplateVariable.h"
#include "Macro.h"
#include "SourceInfo.h"

using namespace std;

SOURCE_INFO_ADD("$Id: TemplateVariable.cpp 2756 2007-12-11 10:57:59Z svn $");

#ifdef DEBUG
const TemplateVariable::variable_type_str TemplateVariable::variable_type_list[] = {
    { TemplateVariable::END,                "end"           },
    { TemplateVariable::SCALAR,             "scalar"        },
    { TemplateVariable::ARRAY,              "array"         },
    { TemplateVariable::HASH,               "hash"          },
};
const TemplateVariable::scalar_type_str TemplateVariable::scalar_type_list[] = {
    { TemplateVariable::STRING,             "string"        },
    { TemplateVariable::INTEGER,            "integer"       },
};
#endif

/******************************************************************************
 * public メソッド
 *****************************************************************************/
#ifdef DEBUG
void TemplateVariable::print_dot(const variable_t *var)
{
    cout << "digraph variable_structure {"      << endl;
    cout << "rankdir    = \"LR\""               << endl;
    cout << "ratio      = \"auto\";"            << endl;
    cout << "ranksep    = \"0.3\";"             << endl;
    cout << "node ["                            << endl;
    cout << "    fontname   = \"Helvetica\""    << endl;
    cout << "    fontsize   = \"7\""            << endl;
    cout << "    shape      = \"record\""       << endl;
    cout << "];"                                << endl;
    print_dot_variable(var);
    cout << "}"                                 << endl;
}
#endif


/******************************************************************************
 * private メソッド
 *****************************************************************************/
#ifdef DEBUG
apr_size_t TemplateVariable::print_dot_variable(const variable_t *var)
{
    static apr_size_t dot_id = 1;

    // TODO: 作る

    return dot_id;
}


const char *TemplateVariable::get_variable_type(const variable_t *var)
{
    for (apr_size_t i = 0; i < ARRAY_SIZE_OF(variable_type_list); i++) {
        if (var->type == variable_type_list[i].type) {
            return variable_type_list[i].str;
        }
    }
    return "(UNKNOWN)";
}

const char *TemplateVariable::get_scalar_type(const scalar_t *var)
{
    for (apr_size_t i = 0; i < ARRAY_SIZE_OF(scalar_type_list); i++) {
        if (var->type == scalar_type_list[i].type) {
            return scalar_type_list[i].str;
        }
    }
    return "(UNKNOWN)";
}
#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
