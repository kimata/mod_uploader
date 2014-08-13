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
 * $Id: CGIResponse.h 2895 2008-07-15 16:39:11Z svn $
 *****************************************************************************/

#ifndef CGI_RESPONSE_H
#define CGI_RESPONSE_H

#include "Environment.h"

#include <iostream>

#include "apr_network_io.h"
#include "apr_strings.h"
#include "apr_tables.h"
#include "apr_time.h"
#include "apr_env.h"

#include "Uncopyable.h"

// NOTE: define じゃなくて static const int にすると，VisualStudio2005
// で何故かうまくいかない
#define OK                          0
#define HTTP_TEMPORARY_REDIRECT     307
#define HTTP_BAD_REQUEST            400

class CGIRequestReader;
class CGIResponseWriter;

/**
 * @brief CGI のレスポンスを表すクラス．
 */
class CGIResponse {
public:
    typedef struct Handle {
        apr_pool_t *pool;

        Handle(apr_pool_t *pool_arg)
          : pool(pool_arg)
        {

        };
    } Handle;
    typedef CGIRequestReader  Reader;
    typedef CGIResponseWriter Writer;

    static const char *get_request_uri(Handle *r)
    {
        return get_env(r->pool, "REQUEST_URI");
    };
    static const char *get_content_type(Handle *r)
    {
        return get_env(r->pool, "CONTENT_TYPE");
    };
    static apr_uint64_t get_content_size(Handle *r)
    {
        return apr_atoi64(get_env(r->pool, "CONTENT_LENGTH"));
    };
    static const char *get_query(Handle *r)
    {
        return get_env(r->pool, "QUERY_STRING");
    };
    static const char *get_remote_ip(Handle *r)
    {
        return get_env(r->pool, "REMOTE_ADDR");
    };
    static apr_sockaddr_t *get_remote_addr(Handle *r)
    {
        apr_sockaddr_t *sockaddr;

        APR_PALLOC(sockaddr, apr_sockaddr_t *, r->pool, sizeof(apr_sockaddr_t));
        if (apr_sockaddr_info_get(&sockaddr, "localhost", APR_UNSPEC, 80,
                                  APR_IPV4_ADDR_OK|APR_IPV6_ADDR_OK, r->pool)
            != APR_SUCCESS) {
            THROW(MESSAGE_BUG_FOUND);
        }

        return sockaddr;
    };
    static const char *get_user_agent(Handle *r)
    {
        return get_env(r->pool, "USER_AGENT");
    };
    static bool is_head_method(Handle *r)
    {
        return (strcmp(get_env(r->pool, "REQUEST_METHOD"), "HEAD") == 0);
    };
    static bool is_post_method(Handle *r)
    {
        return (strcmp(get_env(r->pool, "REQUEST_METHOD"), "POST") == 0);
    };
    static int is_meets_condition(Handle *r)
    {
        return OK;
    };
    static int prepare_post_read(Handle *r)
    {
        return OK;
    };
    static void set_modified_time(Handle *r, apr_time_t time)
    {

    };
    static void set_last_modified(Handle *r, apr_time_t time)
    {

    };
    static void set_expires(Handle *r, apr_time_t time)
    {

    };
    static void set_etag(Handle *r)
    {

    };
    static void set_content_type(Handle *r, const char *file_mime,
                                 bool prefer_magick)
    {
        set_content_type(r, file_mime);
    };
    static void set_content_type(Handle *r, const char *file_mime)
    {
        std::cout << "Content-type: " << file_mime << "\r\n";
    };
    static void set_accept_ranges(Handle *r)
    {
        std::cout << "Accept-Ranges: bytes\r\n";
    }
    static void set_location(Handle *r, const char *uri)
    {
        std::cout << "Location: " << uri << "\r\n\r\n";
    };
    static void set_content_disposition(Handle *r, const char *dispos)
    {
        std::cout << "Content-Disposition: " << dispos << "\r\n";
    };
    static void set_header(Handle *r, const char *name, const char *value)
    {
        std::cout << name << ": " << value << "\r\n";
    }
    static void header_end(Handle *r)
    {
        std::cout << "\r\n";
    };
    static void set_env(Handle *r, const char *name, const char *value)
    {
        // 何もしない
    };
    static const char *get_env(apr_pool_t *pool, const char *key)
    {
        char *value;

        if (apr_env_get(&value, key, pool) != APR_SUCCESS) {
            return "";
        }

        return value;
    };
};

#include "CGIResponseWriter.h"

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
