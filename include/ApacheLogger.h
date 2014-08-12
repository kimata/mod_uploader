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
 * $Id: ApacheLogger.h 2880 2008-05-01 12:19:40Z svn $
 *****************************************************************************/

#ifndef APACHE_LOGGER_H
#define APACHE_LOGGER_H

#include "Environment.h"

#include <cstdarg>

#include "apr_strings.h"

#include "Logger.h"
#include "ApacheResponse.h"
#include "TemporaryPool.h"


typedef struct server_rec resever_rec;

/**
 * @brief コンソールへログの出力を行うクラス．
 */
class ApacheLogger: public Logger
{
public:
    void info(const char *file, int line, server_rec *s,
              const char *format, ...)
#ifdef __GNUC__
        __attribute__((format(printf,5,6)))
#endif
        ;
    void info(const char *file, int line, ApacheResponse::Handle *r,
              const char *format, ...)
#ifdef __GNUC__
        __attribute__((format(printf,5,6)))
#endif
        ;
    void warn(const char *file, int line, server_rec *s,
              const char *format, ...)
#ifdef __GNUC__
        __attribute__((format(printf,5,6)))
#endif
        ;
    void warn(const char *file, int line, ApacheResponse::Handle *r,
              const char *format, ...)
#ifdef __GNUC__
        __attribute__((format(printf,5,6)))
#endif
        ;
    void error(const char *file, int line, server_rec *s,
               const char *format, ...)
#ifdef __GNUC__
        __attribute__((format(printf,5,6)))
#endif
        ;
    void error(const char *file, int line, ApacheResponse::Handle *r,
               const char *format, ...)
#ifdef __GNUC__
        __attribute__((format(printf,5,6)))
#endif
        ;
};

#endif

extern ApacheLogger logger;

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
