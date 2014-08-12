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
 * $Id: ConfigReader.cpp 2508 2007-07-06 22:47:18Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <cctype>

#include "apr_mmap.h"
#include "apr_strings.h"

#include "ConfigReader.h"
#include "File.h"

#define FORMAT_ERROR(message, pos) do {         \
        THROW(message);                         \
    } while (0)

const char ConfigReader::COMMENT_CHAR   = '#';
const char ConfigReader::QUOTE_CHAR     = '"';
const char ConfigReader::ASSIGN_CHAR    = '=';


/******************************************************************************
 * public メソッド
 *****************************************************************************/
ConfigReader::config_map *ConfigReader::read(apr_pool_t *pool,
                                             const char *file_path)
{
    const char *data;
    const char *name;
    const char *value;
    config_map *cmap;

    read_file(pool, file_path, &data);

    cmap = new config_map();
    while (*data != '\0') {
        read_line(pool, &data, &name, &value);
        cmap->insert(config_pair(name, value));
    }

    return cmap;
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void ConfigReader::read_file(apr_pool_t *pool, const char *file_path,
                             const char **data)
{
    apr_mmap_t *file_map;
    apr_size_t file_size;

    File conf_file(pool, file_path);
    conf_file.open(APR_READ|APR_BINARY);
    file_map = conf_file.mmap();

    file_size = static_cast<apr_size_t>(conf_file.get_size());

    APR_PALLOC(*data, char *, pool, file_size);

    memcpy(const_cast<char *>(*data), file_map->mm, file_size);
    const_cast<char *>(*data)[file_size] = '\0';
}

void ConfigReader::read_line(apr_pool_t *pool, const char **data,
                             const char **name, const char **value)
{
    skip_space(data);

    // コメント行または空行
    while ((**data == COMMENT_CHAR) || (**data == '\n')) {
        skip_line(data);
        skip_space(data);
    }

    if (**data == '\0') {
        return;
    }

    read_param(pool, data, name, value);

    skip_space(data);

    if (**data != '\n') {
        FORMAT_ERROR(MESSAGE_CONF_RETURN_NEEDED, *data);
    }

    skip_line(data);
}

void ConfigReader::read_param(apr_pool_t *pool, const char **data,
                              const char **name, const char **value)
{
    read_word(pool, data, name);

    skip_space(data);
    if (**data != ASSIGN_CHAR) {
        FORMAT_ERROR(MESSAGE_CONF_ASSIGN_NEEDED, *data);
    }
    (*data)++;
    skip_space(data);

    read_word(pool, data, value);
}

void ConfigReader::read_word(apr_pool_t *pool, const char **data,
                             const char **word)
{
    const char *pos;

    if (**data == QUOTE_CHAR) {
        pos = ++(*data);

        // QUOTE_CHAR のエスケープは未サポート
        while ((*pos != '\0') && (*pos != QUOTE_CHAR)) {
            pos++;
        }
    } else {
        pos = *data;

        while (isalnum(*pos & 0xff) || (*pos == '-')) {
            pos++;
        }

    }

    *word = apr_pstrndup(pool, *data, pos - *data);
    *data = pos + 1;
}

void ConfigReader::skip_space(const char **data)
{
    // 改行はスキップしない
    while ((**data != '\0') && (**data != '\n') && isspace(**data & 0xff)) {
        (*data)++;
    }
}

void ConfigReader::skip_line(const char **data)
{
    while ((**data != '\0') && (**data != '\n')) {
        (*data)++;
    }

    if (**data == '\n') {
        (*data)++;
    }
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_ConfigReader
#include "TestRunner.h"

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << " <CONF_FILE_PATH>" << endl;
}

void run_read(apr_pool_t *pool, const char *conf_file_path)
{
    show_test_name("read");

    auto_ptr<ConfigReader::config_map>
        cmap(ConfigReader::read(pool, conf_file_path));
    for (ConfigReader::config_map::const_iterator i = cmap.get()->begin();
         i != cmap.get()->end(); i++) {
        show_item(i->first, i->second);
    }
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    const char *conf_file_path;

    if (argc != 2) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    conf_file_path = argv[1];
    if (!File::is_exist(pool, conf_file_path)) {
        THROW(MESSAGE_CONF_FILE_NOT_FOUND);
    }

    show_item("conf_file_path", conf_file_path);
    show_line();

    run_read(pool, conf_file_path);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
