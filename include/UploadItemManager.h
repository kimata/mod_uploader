/******************************************************************************
 * Copyright (C) 2007 Tetsuya Kimata <kimata@acapulco.dyndns.org>
 *
 * All rights reserved.
 *
 * This software is provided 'as-is', without any express or nolockied
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
 * $Id: UploadItemManager.h 2907 2009-01-14 12:56:44Z svn $
 *****************************************************************************/

#ifndef UPLOAD_ITEM_MANAGER_H
#define UPLOAD_ITEM_MANAGER_H

#include "Environment.h"

#include "apr_shm.h"
#include "AtomicWrapper.h"

#include "UploadItemReader.h"
#include "UploadItemWriter.h"

#include "Uncopyable.h"

class UploadItemList;
class UploadItemIterator;
class ThumbnailList;
class ThumbnailIterator;

/**
 * @brief アップロードアイテムを管理するクラス．
 * 排他処理を行います．
 *
 * リストの変更(追加/削除)および最終更新日時の取得はこのクラスを利用し
 * て行います．
 */
class UploadItemManager: public Uncopyable
{
public:
    apr_size_t add(apr_pool_t *pool,
                   const char *file_name, const char *file_mime,
                   apr_uint64_t file_size, const char *file_digest,
                   const char *file_temp_path, const char *remove_pass,
                   const char *download_pass, const char *comment,
                   const char *code_pat, const char *ip_address);
    void remove(apr_pool_t *pool, UploadItem *uitem);
    UploadItem *get_item(apr_pool_t *pool, apr_size_t item_id);
    apr_file_t *get_item_file(apr_pool_t *pool, apr_size_t item_id,
                              bool is_sendfile_enabled=false);
    void update_atime(apr_size_t item_id);
    apr_time_t get_mtime() const
    {
        return *mtime_;
    };
    void set_mtime(apr_time_t mtime)
    {
        *mtime_ = mtime;
    };
    void update_mtime(apr_time_t mtime)
    {
        if (mtime > *mtime_) {
            set_mtime(mtime);
        }
    };
    /**
     * 共有メモリを使ってインスタンスを生成します．
     *
     * @param[in] shm 共有メモリ
     * @param[in] pool プール
     * @param[in] data_dir_path アイテムが保存されているディレクトリ
     * @param[in] file_dir_path ファイルが保存されているディレクトリ
     * @param[in] thumb_dir_path サムネイルが保存されているディレクトリ
     * @param[in] item_list アイテムのリスト
     * @param[in] thumbnail_list サムネイルのリスト
     * @param[in] mtime 最終更新日時
     * @return UploadItemManager のインスタンス
     */
    static UploadItemManager *get_instance(apr_shm_t *shm, apr_pool_t *pool,
                                           const char *data_dir_path,
                                           const char *file_dir_path,
                                           const char *thumb_dir_path,
                                           UploadItemList *item_list,
                                           ThumbnailList *thumbnail_list,
                                           apr_time_t mtime);
    /**
     * メモリ上にあるアイテムデータをディスクに書き出します．
     *
     * @param[in] pool プール
     * @param[in] sync_limit 同期するアイテムの最大数
     */
    void sync_all_item(apr_pool_t *pool, apr_size_t sync_limit=0);
    /**
     * 共有メモリを使って子プロセス用の初期化を行います．
     *
     * @param[in] shm 共有メモリ
     * @param[in] item_list アイテムのリスト
     * @param[in] thumbnail_list サムネイルのリスト
     * @return UploadItemManager のインスタンス
     */
    void child_init(apr_shm_t *shm, UploadItemList *item_list,
                    ThumbnailList *thumbnail_list);
    /**
     * マネージャのデータの最大サイズを計算します．
     *
     * @return マネージャのデータの最大サイズ
     */
    static apr_size_t get_memory_size();
    /**
     * 終了状態のチェックを行います．
     * マネージャがロックされている場合は例外を投げます．
     *
     * @param[in] item_manager マネージャ
     */
    static void check_finalize_state(UploadItemManager *item_manager);

private:
    /** 書き込み時，1/SYNC_FACTOR の確率でディスクとの同期を行います． */
    static const apr_size_t SYNC_FACTOR;
    /** 書き込み時，一度に同期する数． */
    static const apr_size_t SYNC_LIMIT;

    UploadItemManager(apr_shm_t *shm, apr_pool_t *pool,
                      const char *data_dir_path, const char *file_dir_path,
                      const char *thumb_dir_path, UploadItemList *item_list,
                      ThumbnailList *thumbnail_list, apr_time_t mtime);
    apr_size_t add_impl(apr_pool_t *pool,
                        const char *file_name, const char *file_mime,
                        apr_uint64_t file_size, const char *file_digest,
                        const char *file_temp_path, const char *remove_pass,
                        const char *download_pass, const char *comment,
                        const char *code_pat, const char *ip_address);
    void remove_impl(apr_pool_t *pool, UploadItem *uitem);
    void remove_files(apr_pool_t *pool, apr_uint64_t file_size);
    void remove_old_files(apr_pool_t *pool, apr_uint64_t file_size);
    void remove_unpopular_files(apr_pool_t *pool, apr_uint64_t file_size);

    /**
     * ユニークなアイテム ID とインデックスを生成します．
     */
    void create_item_id_index(apr_size_t *item_id,
                              apr_size_t *item_index) const;
    /**
     * 適度に，メモリ上にあるアイテムデータをディスクに書き出します．
     */
    void sync_all_item_by_catches(apr_pool_t *pool);

    friend class UploadItemIterator;
    friend class ThumbnailIterator;

    apr_pool_t *pool_;

    const char *data_dir_path_;
    const char *file_dir_path_;
    const char *thumb_dir_path_;

    UploadItemList *item_list_;
    ThumbnailList *thumbnail_list_;

    UploadItemWriter item_writer_;

    /** マネージャのロック */
    apr_atomic_t *lock_;
    /** 最終更新時刻 */
    apr_time_t *mtime_;
    /** 変更カウンタ */
    apr_size_t *age_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
