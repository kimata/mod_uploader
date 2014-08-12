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
 * $Id: SourceInfo.cpp 2889 2008-05-10 16:36:45Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <functional>

#ifdef DEBUG
#include <iostream>
#endif

#include "SourceInfo.h"
#include "TemporaryPool.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

using namespace std;

SOURCE_INFO_ADD("$Id: SourceInfo.cpp 2889 2008-05-10 16:36:45Z svn $");

#if _MSC_VER >= 1400
#undef strdup
#define strdup _strdup
#endif

/******************************************************************************
 * public メソッド
 *****************************************************************************/
void SourceInfo::add(const char *id)
{
    char *temp_id;
    char *formated_id;
    apr_size_t formated_id_size;
    const char *name;
    const char *revision;
    const char *date;

#ifdef DEBUG
    if (strlen(id) < 7) {
        cerr << __FILE__ << ": " << MESSAGE_ARGUMENT_INVALID;
        cerr << "(" << id << ")" << endl;
        abort();
    }
#endif

    temp_id = strdup(id);
    formated_id_size = strlen(id) + 256;
    MALLOC(formated_id, char *, sizeof(char), formated_id_size);

    parse_id(temp_id, &name, &revision, &date);
    snprintf(formated_id, formated_id_size,
             "%-32s %5s  %s", name, revision, date);

    info_list_.push_back(formated_id);

    is_sorted_ = false;

    free(formated_id);
    free(temp_id);
}

const char *SourceInfo::get(apr_size_t i)
{
    if (UNLIKELY(!is_sorted_)) {
        sort(info_list_.begin(), info_list_.end(), less<string>());
        is_sorted_ = true;
    }

    return info_list_.at(i).c_str();
}

apr_size_t SourceInfo::count() const
{
    return info_list_.size();
}

SourceInfo *SourceInfo::instance()
{
    static SourceInfo *instance = NULL;

    if (UNLIKELY(instance == NULL)) {
        instance = new SourceInfo();
    }

    return instance;
}

/******************************************************************************
 * private メソッド
 *****************************************************************************/
void SourceInfo::parse_id(char *id, const char **name, const char **revision,
                          const char **date)
{
    apr_size_t i;

#ifdef DEBUG
    {
        apr_size_t space;

        space = 0;
        for (i = 0; id[i] != '\0'; i++) {
            if (id[i] == ' ') {
                space++;
            }
        }
        if (space < 5) {
            cerr << __FILE__ << ": " << MESSAGE_ARGUMENT_INVALID;
            cerr << "(" << id << ")" << endl;
            abort();
        }
    }
#endif

    // Id は svn のものを想定
    // 例: "Id: SourceInfo.cpp 1369 2006-08-30 13:35:04Z svn"

    i = 0;
    while (id[i++] != ' ') ;

    *name = id  + i;
    while (id[i++] != ' ') ;
    id[i-1] = '\0';

    *revision = id + i;
    while (id[i++] != ' ') ;
    id[i-1] = '\0';

    *date = id + i;
    while (id[i++] != ' ') ;
    while (id[i++] != ' ') ;
    id[i-1] = '\0';
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
