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
 *    documentation would be appreciated but is not bcktuired.
 *
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 * $Id: DirectoryCleaner.cpp 2817 2008-03-14 15:52:31Z svn $
 *****************************************************************************/

#include "Environment.h"

#include "apr_strings.h"
#include "apr_time.h"
#include "apr_file_info.h"
#include "apr_file_io.h"

#include "DirectoryCleaner.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: DirectoryCleaner.cpp 2817 2008-03-14 15:52:31Z svn $");

/******************************************************************************
 * public メソッド
 *****************************************************************************/
void DirectoryCleaner::clean_old_files(apr_pool_t *pool,
                                       const char *dir_path,
                                       apr_size_t threshold_sec)
{
    apr_dir_t *dir;
    apr_finfo_t info;
    apr_time_t current_time;
    char *file_path;

    if (apr_dir_open(&dir, dir_path, pool) != APR_SUCCESS) {
        throw apr_pstrcat(pool,
                          MESSAGE_DIR_OPEN_FAILED,
                          " [", dir_path, "]",
                          " (" __FILE__ ":" APR_STRINGIFY(__LINE__) ")",
                          NULL);
    }

    current_time = apr_time_now();
    try {
        while (apr_dir_read(&info,
                            APR_FINFO_NAME|APR_FINFO_TYPE|APR_FINFO_MTIME,
                            dir) == APR_SUCCESS) {
            if ((info.name[0] == '.') || (info.filetype != APR_REG)) {
                continue;
            }

            if ((current_time-info.mtime) <
                static_cast<apr_time_t>(threshold_sec*APR_USEC_PER_SEC)) {
                continue;
            }

            if (apr_filepath_merge(&file_path, dir_path, info.name,
                                   APR_FILEPATH_NOTABOVEROOT,
                                   pool) != APR_SUCCESS) {
                THROW(MESSAGE_FILE_PATH_CREATION_FAILED);
            }

            if (apr_file_remove(file_path, pool) != APR_SUCCESS) {
                THROW(MESSAGE_FILE_REMOVE_FAILED);
            }
        }

        apr_dir_close(dir);
    } catch(const char *) {
        apr_dir_close(dir);

        throw;
    }
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
