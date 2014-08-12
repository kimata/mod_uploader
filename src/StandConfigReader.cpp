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
 * $Id: StandConfigReader.cpp 2768 2007-12-16 13:03:37Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <memory>

#include "StandConfigReader.h"
#include "ConfigReader.h"


#define SET_PARAM(directive, var, value) do {                            \
        if (strcmp(i->first.c_str(), UploaderConfig::directive.param) == 0) { \
            config->var = value;                                        \
        }                                                               \
    } while (0)

#define SET_STR_PARAM(directive, var) \
    SET_PARAM(directive, var, apr_pstrdup(pool, i->second.c_str()))
#define SET_INT_PARAM(directive, var) \
    SET_PARAM(directive, var, atoi(i->second.c_str()))
#define SET_INT64_PARAM(directive, var) \
    SET_PARAM(directive, var, apr_atoi64(i->second.c_str()) * 1024)
#define SET_TMPL_PARAM(directive, type) do {                            \
        if (strcmp(i->first.c_str(), UploaderConfig::directive.param) == 0) { \
            config->set_template(UploaderConfig::type, i->second.c_str()); \
        }                                                               \
    } while (0)


/******************************************************************************
 * public メソッド
 *****************************************************************************/
UploaderConfig *StandConfigReader::read(apr_pool_t *pool,
                                        const char *conf_file_path)
{
    UploaderConfig *config;

    APR_PCALLOC(config, UploaderConfig *, pool, sizeof(UploaderConfig));
    new(config) UploaderConfig(pool);

    std::auto_ptr<ConfigReader::config_map>
        cmap(ConfigReader::read(pool, conf_file_path));
    for (ConfigReader::config_map::const_iterator i = cmap.get()->begin();
         i != cmap.get()->end(); i++) {
        SET_STR_PARAM(PATH,                 path);
        SET_STR_PARAM(BASE_URL,             base_url);
        SET_STR_PARAM(DATA_DIRECTORY,       data_dir_path);
        SET_STR_PARAM(FILE_DIRECTORY,       file_dir_path);
        SET_STR_PARAM(THUMB_DIRECTORY,      thumb_dir_path);
        SET_STR_PARAM(TEMP_DIRECTORY,       temp_dir_path);
        SET_INT64_PARAM(TOTAL_FILE_SIZE,    total_file_size_limit);
        SET_INT_PARAM(TOTAL_FILE_NUMBER,    total_file_number_limit);
        SET_INT64_PARAM(FILE_SIZE_LIMIT,    file_size_limit);
        SET_INT_PARAM(PER_PAGE_ITEM_NUMBER, per_page_item_number);

        SET_TMPL_PARAM(INDEX_VIEW_TEMPLATE,     INDEX_VIEW);
        SET_TMPL_PARAM(PROGRESS_VIEW_TEMPLATE,  PROGRESS_VIEW);
        SET_TMPL_PARAM(DOWNLOAD_VIEW_TEMPLATE,  DOWNLOAD_VIEW);
        SET_TMPL_PARAM(THUMBNAIL_VIEW_TEMPLATE, THUMBNAIL_VIEW);
        SET_TMPL_PARAM(ADMIN_VIEW_TEMPLATE,     ADMIN_VIEW);
        SET_TMPL_PARAM(ERROR_VIEW_TEMPLATE,     ERROR_VIEW);
    }

    config->init();

    return config;
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
