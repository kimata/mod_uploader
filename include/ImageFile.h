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
 * $Id: ImageFile.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef IMAGE_FILE_H
#define IMAGE_FILE_H

#include "Environment.h"

#include <cstdlib>

#include "File.h"


namespace Magick {
    class Image;
}

#ifdef MAKE_THUMBNAIL

/**
 * @brief 画像ファイルを表すクラス．
 */
class ImageFile: File
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] file_path ファイルのパス
     */
    ImageFile(apr_pool_t *pool, const char *file_path="");
    ~ImageFile();
    /**
     * サムネイル画像を作成します．
     *
     * @param[in] file_path サムネイル画像のパス
     * @param[in] width サムネイル画像の横サイズ
     * @param[in] height サムネイル画像の縦サイズ
     */
    void create_thumbnail(const char *file_path,
                          apr_size_t width, apr_size_t height) const;

    apr_uint16_t get_width() const;
    apr_uint16_t get_height() const;

private:
    Magick::Image *image_;
};

#endif

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
