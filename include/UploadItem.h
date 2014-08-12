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
 * $Id: UploadItem.h 2813 2008-03-07 14:35:04Z svn $
 *****************************************************************************/

#ifndef UPLOAD_ITEM_H
#define UPLOAD_ITEM_H

#include "Environment.h"

#include <cstdlib>

#include "AtomicWrapper.h"
#include "apr_time.h"

#include "TemplateVariable.h"
#include "Message.h"
#include "Uncopyable.h"
#include "Macro.h"

#define AS_UITEM(pointer)           reinterpret_cast<UploadItem *>(pointer)
#define AS_UITEM_H(pointer)         reinterpret_cast<UploadItem::header_t *>(pointer)
#define AS_UITEM_H_VER2x(pointer)   reinterpret_cast<UploadItem::header_ver2x_t *>(pointer)

/**
 * @brief アップロードアイテムを表すクラス．
 */
class UploadItem: public Uncopyable
{
public:
    /**
     * @brief アップロードアイテムの情報を表す構造体．
     */
    typedef struct header {
        char            identifier[ITM_MAX_IDENTIFIER_SIZE];
        char            version[ITM_MAX_VERSION_SIZE];
        apr_size_t      id;
        apr_size_t      index;
        apr_atomic_t    download_count;
        union {
            struct {
                bool    is_gzipped      : 1;
                bool    is_need_sync    : 1;
            };
            bool        flags;
        };
        apr_uint64_t    file_size;
        apr_time_t      mtime;
        apr_time_t      atime;
        char            date[ITM_MAX_DATE_SIZE];
        char            ip_address[ITM_MAX_IP_ADDRESS_SIZE];
        char            file_name[ITM_MAX_FILE_NAME_SIZE];
        char            file_mime[ITM_MAX_FILE_MIME_SIZE];
        char            file_ext[ITM_MAX_FILE_EXT_SIZE];
        char            file_digest[ITM_MAX_FILE_DIGEST_SIZE];
        char            remove_pass[ITM_MAX_REMOVE_PASS_SIZE];
        char            download_pass[ITM_MAX_DOWNLOAD_PASS_SIZE];
        char            comment[ITM_MAX_COMMENT_SIZE];

        header() {
            memset(identifier, 0, sizeof(struct header));

            strncpy(identifier, PACKAGE_NAME, ITM_MAX_IDENTIFIER_SIZE-1);
            strncpy(version, PACKAGE_VERSION, ITM_MAX_VERSION_SIZE-1);
        }
    } header_t;

    typedef struct header_ver2x {
        char            identifier[ITM_MAX_IDENTIFIER_SIZE];
        char            version[ITM_MAX_VERSION_SIZE];
        apr_size_t      id;
        apr_size_t      index;
        apr_atomic_t    download_count;
        union {
            struct {
                bool    is_gzipped      : 1;
                bool    is_need_sync    : 1;
            };
            bool        flags;
        };
        apr_uint64_t    file_size;
        apr_time_t      mtime;
        char            date[ITM_MAX_DATE_SIZE];
        char            ip_address[ITM_MAX_IP_ADDRESS_SIZE];
        char            file_name[ITM_MAX_FILE_NAME_SIZE];
        char            file_mime[ITM_MAX_FILE_MIME_SIZE];
        char            file_ext[ITM_MAX_FILE_EXT_SIZE];
        char            file_digest[ITM_MAX_FILE_DIGEST_SIZE];
        char            remove_pass[ITM_MAX_REMOVE_PASS_SIZE];
        char            download_pass[ITM_MAX_DOWNLOAD_PASS_SIZE];
        char            comment[ITM_MAX_COMMENT_SIZE];

        header_ver2x() {
            memset(identifier, 0, sizeof(struct header));

            strncpy(identifier, PACKAGE_NAME, ITM_MAX_IDENTIFIER_SIZE-1);
            strncpy(version, PACKAGE_VERSION, ITM_MAX_VERSION_SIZE-1);
        }
    } header_ver2x_t;

    typedef int(*CompareFunc)(const UploadItem *a, const UploadItem *b);

    UploadItem();

    const char *get_identifier() const
    {
        return header_.identifier;
    };
    const char *get_version() const
    {
        return header_.version;
    };
    apr_size_t get_id() const
    {
        return header_.id;
    };
    apr_size_t get_index() const
    {
        return header_.index;
    };
    apr_size_t get_download_count() const
    {
        return  apr_atomic_read
            (const_cast<apr_atomic_t *>(&(header_.download_count)));
    };
    apr_uint64_t get_file_size() const
    {
        return header_.file_size;
    };
    apr_time_t get_mtime() const
    {
        return header_.mtime;
    };
    apr_time_t get_atime() const
    {
        return header_.atime;
    };
    const char *get_date() const
    {
        return header_.date;
    };
    const char *get_ip_address() const
    {
        return header_.ip_address;
    };
    const char *get_file_name() const
    {
        return header_.file_name;
    };
    const char *get_file_mime() const
    {
        return header_.file_mime;
    };
    const char *get_file_ext() const
    {
        return header_.file_ext;
    };
    const char *get_file_digest() const
    {
        return header_.file_digest;
    };
    const char *get_remove_pass() const
    {
        return header_.remove_pass;
    };
    const char *get_download_pass() const
    {
        return header_.download_pass;
    };
    const char *get_comment() const
    {
        return header_.comment;
    };
    bool is_gzipped() const
    {
        return header_.is_gzipped;
    };
    bool is_need_sync() const
    {
        return header_.is_need_sync;
    };
    void increment_download_count()
    {
        (void)apr_atomic_inc(&(header_.download_count));
        header_.is_need_sync = true;
    };
    void update_atime()
    {
        header_.atime = apr_time_now();
    };
    /**
     * メモリを使ってインスタンスを生成します．
     *
     * @paramp[out] memory メモリ
     * @paramp[in] item_id アイテム ID
     * @paramp[in] item_index アイテムのインデックス
     * @paramp[in] file_size ファイルサイズ
     * @paramp[in] mtime アイテムの投稿日時
     * @paramp[in] ip_address 投稿者の IP アドレス
     * @paramp[in] file_name ファイルの名前
     * @paramp[in] file_mime ファイルの MIME タイプ
     * @paramp[in] file_digest ファイルの MD5 値
     * @paramp[in] remove_pass 削除パスワード
     * @paramp[in] download_pass ダウンロードパスワード
     * @paramp[in] comment コメント
     * @paramp[in] code_pat 「京」の 1 バイト目の値
     * @paramp[in] is_need_validate 内容をチェックするかどうか
     * @return インスタンス
     */
    static UploadItem *get_instance(void *memory, apr_size_t item_id,
                                    apr_size_t item_index,
                                    apr_uint64_t file_size, apr_time_t mtime,
                                    const char *ip_address,
                                    const char *file_name,
                                    const char *file_mime,
                                    const char *file_digest,
                                    const char *remove_pass,
                                    const char *download_pass,
                                    const char *comment,
                                    const char *code_pat,
                                    bool is_need_validate=true);

    /**
     * プールを使ってインスタンスを生成します．
     *
     * @paramp[in] pool プール
     * @paramp[in] item_id アイテム ID
     * @paramp[in] item_index アイテムのインデックス
     * @paramp[in] file_size ファイルサイズ
     * @paramp[in] mtime アイテムの投稿日時
     * @paramp[in] ip_address 投稿者の IP アドレス
     * @paramp[in] file_name ファイルの名前
     * @paramp[in] file_mime ファイルの MIME タイプ
     * @paramp[in] file_digest ファイルの MD5 値
     * @paramp[in] remove_pass 削除パスワード
     * @paramp[in] download_pass ダウンロードパスワード
     * @paramp[in] comment コメント
     * @paramp[in] is_need_validate 内容をチェックするかどうか
     * @return インスタンス
     */
    static UploadItem *get_instance(apr_pool_t *pool, apr_size_t item_id=0,
                                    apr_size_t item_index=0,
                                    apr_uint64_t file_size=0,
                                    apr_time_t mtime=0,
                                    const char *ip_address="",
                                    const char *file_name="",
                                    const char *file_mime="",
                                    const char *file_digest="",
                                    const char *remove_pass="",
                                    const char *download_pass="",
                                    const char *comment="",
                                    const char *code_pat="",
                                    bool is_need_validate=true);
    static int cmp_file_name(const UploadItem *a, const UploadItem *b);
    static int cmp_file_size(const UploadItem *a, const UploadItem *b);
    static int cmp_file_mime(const UploadItem *a, const UploadItem *b);
    static int cmp_file_atime(const UploadItem *a, const UploadItem *b);
    static int cmp_comment(const UploadItem *a, const UploadItem *b);
    static int cmp_download_count(const UploadItem *a, const UploadItem *b);

private:
    /**
     * 日付をフォーマットします．
     *
     * @param[in] pool プール
     * @param[in] time 日付
     * @return フォーマット済みの日付
     */
    static const char *get_formatted_date(apr_pool_t *pool, apr_time_t time);

    friend class UploadItemManager;
    friend class UploadItemList;

    /** アイテムデータ */
    header_t header_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
