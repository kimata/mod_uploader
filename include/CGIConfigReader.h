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
 * $Id: CGIConfigReader.h 2766 2007-12-16 12:54:38Z svn $
 *****************************************************************************/

#ifndef CGI_CONFIG_READER_H
#define CGI_CONFIG_READER_H

#include "Environment.h"

#include "apr_pools.h"

#include "Uncopyable.h"

class UploaderConfig;

/**
 * @brief アップローダの設定をファイルから読み出すクラス．
 */
class CGIConfigReader: public Uncopyable
{
public:
    static UploaderConfig *read(apr_pool_t *pool, const char *conf_file_path);
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
