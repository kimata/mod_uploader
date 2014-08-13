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
 * $Id: ApacheResponse.h 2888 2008-05-07 12:23:50Z svn $
 *****************************************************************************/

#ifndef APACHE_RESPONSE_H
#define APACHE_RESPONSE_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include "http_protocol.h"
#pragma GCC diagnostic pop
#include "http_request.h"

#include "Environment.h"

#include "apr_strings.h"
#include "apr_network_io.h"

#include "Uncopyable.h"
#include "Macro.h"


class ApacheResponseWriter;
class ApacheRequestReader;

#if AP_SERVER_MAJORVERSION_NUMBER == 1
extern "C" {
int ap_find_types(request_rec *);
}
#endif

/**
 * @brief Apache のレスポンスを表すクラス．
 */
class ApacheResponse: public Uncopyable {
public:
#if AP_SERVER_MAJORVERSION_NUMBER == 1
    typedef struct Handle {
        apr_pool_t *pool;
        request_rec *r;
        char *uri;
        char *args;
        conn_rec *connection;
        int method_number;
        int header_only;
        table *headers_in;
        table *headers_out;
        table *subprocess_env;
        char *filename;

        Handle(apr_pool_t *pool_arg, request_rec *r_arg)
          : pool(pool_arg),
            r(r_arg),
            uri(r->uri),
            args(r->args),
            connection(r->connection),
            method_number(r->method_number),
            header_only(r->header_only),
            headers_in(r->headers_in),
            headers_out(r->headers_out),
            subprocess_env(r->subprocess_env),
            filename(r->filename)
        {

        };
    } Handle;

#undef apr_table_get
#undef apr_table_set
#undef apr_table_setn
#define apr_table_get ap_table_get
#define apr_table_set ap_table_set
#define apr_table_setn ap_table_setn
#else
    typedef request_rec Handle;
#endif

    typedef ApacheRequestReader  Reader;
    typedef ApacheResponseWriter Writer;

    static const char *get_request_uri(Handle *r)
    {
        return r->uri;
    };
    static const char *get_content_type(Handle *r)
    {
        const char *content_type;

        content_type = apr_table_get(r->headers_in, "Content-Type");

        if (content_type == NULL) {
            return "";
        }

        return content_type;
    };
    static apr_uint64_t get_content_size(Handle *r)
    {
        const char *content_size;

        content_size = apr_table_get(r->headers_in, "Content-Length");

        if (content_size == NULL) {
            return 0;
        }

        return apr_atoi64(content_size);
    };
    static const char *get_query(Handle *r)
    {
        const char *query;

        query = r->args;

        if (query == NULL) {
            return "";
        }

        return query;
    };
    static const char *get_remote_ip(Handle *r)
    {
#ifdef AP_SERVER_VERSION_2_4_OR_HIGHER
        return r->connection->client_ip;
#else
        return r->connection->remote_ip;
#endif
    };
    static apr_sockaddr_t *get_remote_addr(Handle *r)
    {
#if AP_SERVER_MAJORVERSION_NUMBER == 1
        apr_sockaddr_t *remote_addr;

        if (apr_sockaddr_info_get(&remote_addr, r->connection->remote_ip,
                                  APR_INET,
                                  r->connection->remote_addr.sin_port,
                                  0, r->pool) != APR_SUCCESS) {
            // 失敗することは想定していないので
            THROW(MESSAGE_BUG_FOUND);
        }

        return remote_addr;
#else
#ifdef AP_SERVER_VERSION_2_4_OR_HIGHER
        return r->connection->client_addr;
#else
        return r->connection->remote_addr;
#endif
#endif
    };
    static const char *get_user_agent(Handle *r)
    {
        const char *agent;

        agent = apr_table_get(r->headers_in, "User-Agent");

        if (agent == NULL) {
            return "";
        }

        return agent;
    }
    static bool is_head_method(Handle *r)
    {
        return (r->header_only != 0);
    };
    static bool is_post_method(Handle *r)
    {
        return (r->method_number == M_POST);
    };
    static int is_meets_condition(Handle *r)
    {
#if AP_SERVER_MAJORVERSION_NUMBER == 1
        return ap_meets_conditions(r->r);
#else
        return ap_meets_conditions(r);
#endif
    };
    static int prepare_post_read(Handle *r)
    {
        int status;

#if AP_SERVER_MAJORVERSION_NUMBER == 1
        if ((status = ap_setup_client_block(r->r, REQUEST_CHUNKED_ERROR)) != OK) {
            return status;
        }
        if (ap_should_client_block(r->r) == 0) {
            return HTTP_NO_CONTENT;
        }
#else
        if ((status = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR)) != OK) {
            return status;
        }
        if (ap_should_client_block(r) == 0) {
            return HTTP_NO_CONTENT;
        }
#endif
        return OK;
    };
    static void set_modified_time(Handle *r, apr_time_t time)
    {
#if AP_SERVER_MAJORVERSION_NUMBER == 1
        ap_update_mtime(r->r, time);
#else
        ap_update_mtime(r, time);
#endif
    };
    static void set_last_modified(Handle *r, apr_time_t time)
    {
#if AP_SERVER_MAJORVERSION_NUMBER == 1
        ap_set_last_modified(r->r);
#else
        ap_set_last_modified(r);
#endif
    };
    static void set_expires(Handle *r, apr_time_t time)
    {
        char *expire_str;

        APR_PALLOC(expire_str, char *, r->pool, APR_RFC822_DATE_LEN);
        apr_rfc822_date(expire_str, time);

        apr_table_set(r->headers_out, "Expires", expire_str);
    };
    static void set_etag(Handle *r)
    {
#if AP_SERVER_MAJORVERSION_NUMBER == 1
        ap_set_etag(r->r);
#else
        ap_set_etag(r);
#endif
    };
    static void set_content_type(Handle *r, const char *file_mime,
                                 bool prefer_magick)
    {
        if (prefer_magick) {
#if AP_SERVER_MAJORVERSION_NUMBER == 1
            r->r->content_type = NULL;

            // MEMO: run mod_mime{_magick} again
            ap_find_types(r->r);

            if (r->r->content_type == NULL) {
                set_content_type(r, file_mime);
            }
#else
            r->content_type = NULL;

            // MEMO: run mod_mime{_magick} again
            ap_run_type_checker(r);

            if (r->content_type == NULL) {
                set_content_type(r, file_mime);
            }
#endif
        } else {
            set_content_type(r, file_mime);
        }
    };
    static void set_content_type(Handle *r, const char *content_type)
    {
#if AP_SERVER_MAJORVERSION_NUMBER == 1
        r->r->content_type = content_type;
#else
        ap_set_content_type(r, content_type);
#endif
    };
    static void set_accept_ranges(Handle *r)
    {
        apr_table_setn(r->headers_out, "Accept-Ranges", "bytes");
    }
    static void set_location(Handle *r, const char *uri)
    {
        apr_table_set(r->headers_out, "Location", uri);
    };
    static void set_content_disposition(Handle *r, const char *dispos)
    {
        apr_table_set(r->headers_out, "Content-Disposition", dispos);
    };
    static void set_header(Handle *r, const char *name, const char *value)
    {
        apr_table_setn(r->headers_out, name, value);
    }
    static void header_end(Handle *r)
    {

    };
    static void set_env(Handle *r, const char *name, const char *value)
    {
        apr_table_setn(r->subprocess_env, name, value);
    };
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
