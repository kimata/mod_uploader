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
 * $Id: ApacheLogger.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

// Apache の config.h を先に処理させるため，httpd.h のインクルードはこの位置
#include "httpd.h"

#include "Environment.h"

#include "http_log.h"

#include "ApacheLogger.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: ApacheLogger.cpp 2756 2007-12-11 10:57:59Z svn $");

ApacheLogger logger;

////////// Apache 1.x 用 //////////////////////////////////////////////
#if AP_SERVER_MAJORVERSION_NUMBER == 1
#define AP_LOG_ERROR(file, line, level, status, s, fmt, ...)    \
    ap_log_error(file, line, level, s, fmt, __VA_ARGS__)
#define AP_LOG_RERROR(file, line, level, status, r_wrapper, fmt, ...)   \
    ap_log_rerror(file, line, level, r_wrapper->r, fmt, __VA_ARGS__)
////////// Apache 2.x 用 //////////////////////////////////////////////
#else
#define AP_LOG_ERROR ap_log_error
#define AP_LOG_RERROR ap_log_rerror
#endif
///////////////////////////////////////////////////////////////////////


/******************************************************************************
 * public メソッド
 ****************************************************************************/
void ApacheLogger::info(const char *file, int line, server_rec *s,
                        const char *format, ...)
{
    TemporaryPool temp_pool;
    const char *message;
    va_list args;

    va_start(args, format);
    message = apr_pvsprintf(temp_pool.get(), format, args);
    va_end(args);

    AP_LOG_ERROR(file, line, APLOG_INFO, 0, s, "%s", message);
}

void ApacheLogger::warn(const char *file, int line, server_rec *s,
                        const char *format, ...)
{
    TemporaryPool temp_pool;
    const char *message;
    va_list args;

    va_start(args, format);
    message = apr_pvsprintf(temp_pool.get(), format, args);
    va_end(args);

    AP_LOG_ERROR(file, line, APLOG_WARNING, 0, s, "%s", message);
}

void ApacheLogger::error(const char *file, int line, server_rec *s,
                         const char *format, ...)
{
    TemporaryPool temp_pool;
    const char *message;
    va_list args;

    va_start(args, format);
    message = apr_pvsprintf(temp_pool.get(), format, args);
    va_end(args);

    AP_LOG_ERROR(file, line, APLOG_ERR, 0, s, "%s", message);
}

void ApacheLogger::info(const char *file, int line,
                        ApacheResponse::Handle *r, const char *format, ...)
{
    TemporaryPool temp_pool;
    const char *message;
    va_list args;

    va_start(args, format);
    message = apr_pvsprintf(temp_pool.get(), format, args);
    va_end(args);

    AP_LOG_RERROR(file, line, APLOG_INFO, 0, r, "%s", message);
}

void ApacheLogger::warn(const char *file, int line,
                        ApacheResponse::Handle *r, const char *format, ...)
{
    TemporaryPool temp_pool;
    const char *message;
    va_list args;

    va_start(args, format);
    message = apr_pvsprintf(temp_pool.get(), format, args);
    va_end(args);

    AP_LOG_RERROR(file, line, APLOG_WARNING, 0, r, "%s", message);
}

void ApacheLogger::error(const char *file, int line,
                         ApacheResponse::Handle *r, const char *format, ...)
{
    TemporaryPool temp_pool;
    const char *message;
    va_list args;

    va_start(args, format);
    message = apr_pvsprintf(temp_pool.get(), format, args);
    va_end(args);

    AP_LOG_RERROR(file, line, APLOG_ERR, 0, r, "%s", message);
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
