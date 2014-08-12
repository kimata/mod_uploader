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
 * $Id: uploader_util.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include "apr_strings.h"
#include "apr_network_io.h"

#include "uploader_util.h"

#include "UploaderConfig.h"
#include "UploadItemIO.h"
#include "PostFlowController.h"
#include "DownloadFlowController.h"
#include "CharCodeConverter.h"
#include "Auxiliary.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: uploader_util.cpp 2756 2007-12-11 10:57:59Z svn $");

const char *get_word(apr_pool_t *pool, const char **input,
                     const char delimiter)
{
    const char *start;
    const char *end;

    start = end = *input;

    while ((*end != '\0') && (*end != delimiter)) {
        end++;
    }

    if (*end == '\0') {
        *input = end;
    } else {
        *input = end + 1;
    }

    if (end != start) {
        return apr_pstrmemdup(pool, start, end - start);
    } else {
        return "";
    }
}

void get_page_count(apr_size_t item_count, apr_size_t per_page_item_number,
                    apr_size_t *page_count)
{
    *page_count = (item_count == 0)
        ? 1
        : (item_count-1)/per_page_item_number + 1;
}

void get_page(apr_pool_t *pool, const char *arg, apr_size_t page_count,
              apr_size_t *page_no)
{
    *page_no = atosize(get_word(pool, &arg, ARG_SEPARATE_STR[0]));

    if (*page_no == 0) {
        *page_no = 1;
    } else if (*page_no > page_count) {
        *page_no = page_count;
    }
}

bool can_post(UploaderConfig *config, apr_sockaddr_t *ip_address)
{
    if (config->is_debug_mode) {
        return true;
    }

    return config->get_post_flow_controller()->can_post(ip_address);
}

void regist_post(UploaderConfig *config, apr_sockaddr_t *ip_address)
{
    if (config->is_debug_mode) {
        return;
    }

    config->get_post_flow_controller()->regist_post(ip_address);
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
