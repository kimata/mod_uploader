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
 * $Id: ConfigReader.h 2573 2007-10-13 01:20:59Z svn $
 *****************************************************************************/

#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include "Environment.h"

#include <string>
#include <map>

#include "apr_pools.h"

#include "Uncopyable.h"


/**
 * @brief 設定ファイルを読み込むクラス．
 */
class ConfigReader: public Uncopyable
{
public:
    typedef std::map<std::string, std::string> config_map; // multimap ではない！
    typedef std::pair<std::string, std::string> config_pair;

    static config_map *read(apr_pool_t *pool, const char *file_path);

private:
    static const char COMMENT_CHAR;
    static const char QUOTE_CHAR;
    static const char ASSIGN_CHAR;

    static void read_file(apr_pool_t *pool, const char *file_path,
                          const char **data);
    static void read_line(apr_pool_t *pool, const char **data,
                          const char **name, const char **value);
    static void read_param(apr_pool_t *pool, const char **data,
                           const char **name, const char **value);
    static void read_word(apr_pool_t *pool, const char **data,
                          const char **word);
    static void skip_space(const char **data);
    static void skip_line(const char **data);
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
