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
 * $Id: SourceInfo.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef SOURCE_INFO_H
#define SOURCE_INFO_H

#include "Environment.h"

#include <vector>
#include <string>

#include "apr.h"

#include "Uncopyable.h"


#define SOURCE_INFO_ADD(id)                         \
    namespace {                                     \
        SourceInfo::adder_t source_info_adder(id);  \
    }

/**
 * @brief ソースファイルの情報をを表すクラス．
 */
class SourceInfo: public Uncopyable
{
public:
    typedef struct adder {
        adder(const char *id) {
            SourceInfo::instance()->add(id);
        };
    } adder_t;

    void add(const char *id);
    const char *get(apr_size_t i);
    apr_size_t count() const;

    static SourceInfo *instance();

private:
    SourceInfo()
      : is_sorted_(false)
    {

    };

    static void parse_id(char *id, const char **name, const char **revision,
                         const char **date);

    std::vector<std::string> info_list_;
    bool is_sorted_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
