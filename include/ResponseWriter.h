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
 * $Id: ResponseWriter.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef RESPONSE_WRITER_H
#define RESPONSE_WRITER_H

#include "Environment.h"

#include <cstdlib>

#include "apr.h"


/**
 * @brief レスポンスを書き出すクラス．
 */
class ResponseWriter
{
public:
    // 効率が落ちるので virtual にはしたくない
    void write(const char *str, apr_size_t length);
    void write(const char *str);
    void write(int i);

protected:
    apr_pool_t *pool_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
