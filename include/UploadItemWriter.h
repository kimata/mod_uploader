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
 * $Id: UploadItemWriter.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef UPLOAD_ITEM_WRITER_H
#define UPLOAD_ITEM_WRITER_H

#include "Environment.h"

#include "UploadItem.h"
#include "UploadItemIO.h"

class ImageFile;

/**
 * @brief アップロードアイテムをファイルに書き込むクラス．
 */
class UploadItemWriter: public UploadItemIO
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] data_dir_path アイテムを保存するディレクトリ
     * @param[in] file_dir_path ファイルを保存するディレクトリ
     * @param[in] thumb_dir_path サムネイルを保存するディレクトリ
     */
    UploadItemWriter(apr_pool_t *pool, const char *data_dir_path,
                     const char *file_dir_path, const char *thumb_dir_path);
    /**
     * アイテムを書き出します．
     *
     * @param[in] uitem アイテムデータ
     * @param[in] temp_file_path 一時ファイルへのパス
     * @retval true サムネイルを作成した
     * @retval false サムネイルを作成していない
     */
    bool write(UploadItem *uitem, const char *temp_file_path);
    /**
     * アイテムヘッダを書き出します．
     *
     * @param[in] header アイテムヘッダ
     * @param[in] temp_file_path 一時ファイルへのパス
     */
    void write_data(apr_pool_t *pool, UploadItem::header_t *header);
    /**
     * アイテムを削除します．
     *
     * @param[in] pool プール
     * @param[in] uitem アイテムデータ
     */
    void remove(apr_pool_t *pool, UploadItem *uitem);

private:
    void write_data(apr_pool_t *pool, UploadItem *uitem);
    void write_file(apr_pool_t *pool, UploadItem *uitem,
                    const char *temp_file_path);
#ifdef MAKE_THUMBNAIL
    bool write_thumb(apr_pool_t *pool, UploadItem *uitem);
    bool write_thumb_impl(apr_pool_t *pool, UploadItem *uitem);
    void get_thumbnail_size(ImageFile *image_file,
                            apr_size_t *width, apr_size_t *height);
#endif
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
