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
 * $Id: UploadItemRss.cpp 2893 2008-05-24 13:18:29Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_INSTANTIATION
#include "Environment.h"
#endif

#include "UploadItemRss.h"
#include "UploadItem.h"
#include "UploadItemIterator.h"
#include "Auxiliary.h"
#include "SourceInfo.h"

#ifndef TEMPLATE_INSTANTIATION
SOURCE_INFO_ADD("$Id: UploadItemRss.cpp 2893 2008-05-24 13:18:29Z svn $");
#endif

template<class W> const char
UploadItemRss<W>::CONTENT_TYPE[]      = "application/xml; charset=" SYS_CHARACTER_CODE;
template<class W> const apr_size_t
UploadItemRss<W>::MAX_TIME_SIZE       = sizeof("0000-00-00T00:00:00+09:00");
template<class W> const char
UploadItemRss<W>::TIME_FORMAT[]       = "%Y-%m-%dT%H:%M:%S" SYS_TIMEZONE_OFFSET_SUFFIX;
template<class W> const apr_int32_t
UploadItemRss<W>::TIME_ZONE_OFFSET    = SYS_TIMEZONE_OFFSET;

/******************************************************************************
 * public メソッド
 *****************************************************************************/
template<class W>
UploadItemRss<W>::UploadItemRss(apr_pool_t *pool, W& writer)
  : pool_(pool),
    writer_(writer)
{

}

template<class W>
void UploadItemRss<W>::print(apr_pool_t *pool, const char *base_url,
                             UploadItemIterator *item_iter, apr_time_t mtime)
{
    UploadItem *uitem;;

    print("<?xml version=\"1.0\" encoding=\"" SYS_CHARACTER_CODE "\"?>\n");
    print("<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n");
    print("         xmlns=\"http://purl.org/rss/1.0/\"\n");
    print("         xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n");
    print(" <channel rdf:about=\""); print(base_url); print("\">\n");
    print("  <title>" PACKAGE_NAME "</title>\n");
    print("  <link>"); print(base_url); print("</link>\n");
    print("  <description>" PACKAGE_STRING "</description>\n");
    print("  <dc:language>ja-jp</dc:language>\n");
    print("  <dc:date>");
    print(time_str(pool_, mtime));
    print("</dc:date>\n");

    if (item_iter->size() != 0) {
        print(" <items>\n");
        print("  <rdf:Seq>\n");
        do {
            uitem = item_iter->get();

            print("   <rdf:li rdf:resource=\"");
            print(base_url);
            print("/download/");
            print(uitem->get_id());
            print("/");
            print("\" />\n");
        } while (item_iter->next());
        print("  </rdf:Seq>\n");
        print(" </items>\n");
        print(" </channel>\n");

        item_iter->reset();

        do {
            print(pool, base_url, item_iter->get());
        } while (item_iter->next());
    } else {
        print(" </channel>\n");
    }
    print("</rdf:RDF>\n");
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
template<class W>
void UploadItemRss<W>::print(apr_pool_t *pool, const char *base_url,
                             UploadItem *uitem)
{
    print("  <item rdf:about=\"");
    print(base_url);
    print("/download/");
    print(uitem->get_id());
    print("/");
    print("\">\n");

    print("   <title>");
    print(uitem->get_file_name());
    print("</title>\n");

    print("<link>");
    print(base_url);
    print("/download/");
    print(uitem->get_id());
    print("/");
    print("</link>\n");

    print("   <description><![CDATA[");
    print(uitem->get_comment());
    print("]]></description>\n");

    print("   <dc:date>");
    print(time_str(pool_, uitem->get_mtime()));
    print("</dc:date>\n");

    print("  </item>\n");
}

template<class W>
inline void UploadItemRss<W>::print(const char *str, apr_size_t length)
{
    writer_.write(str, length);
}

template<class W>
inline void UploadItemRss<W>::print(const char *str)
{
    writer_.write(str);
}

template<class W>
inline void UploadItemRss<W>::print(apr_size_t i)
{
    writer_.write(static_cast<int>(i));
}

template<class W>
const char *UploadItemRss<W>::time_str(apr_pool_t *pool,
                                       apr_time_t time)
{
    apr_time_exp_t time_exp;
    char *time_str;
    apr_size_t size;

    APR_PALLOC(time_str, char *, pool, sizeof(char)*MAX_TIME_SIZE);

    apr_time_exp_tz(&time_exp, time, TIME_ZONE_OFFSET);
    apr_strftime(time_str, &size, MAX_TIME_SIZE, TIME_FORMAT, &time_exp);

    return time_str;
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
