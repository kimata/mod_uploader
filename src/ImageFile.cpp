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
 * $Id: ImageFile.cpp 2882 2008-05-02 12:28:05Z svn $
 *****************************************************************************/

#include "Environment.h"

#ifdef MAKE_THUMBNAIL

#include "ImageFile.h"
#include "SourceInfo.h"

// 上の二つより前だとまずいみたい．詳細は追い切れてないです．
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#include <Magick++.h>
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#ifdef _MSC_VER
#pragma warning(pop)
#endif

using namespace std;

SOURCE_INFO_ADD("$Id: ImageFile.cpp 2882 2008-05-02 12:28:05Z svn $");

/******************************************************************************
 * public メソッド
 *****************************************************************************/
ImageFile::ImageFile(apr_pool_t *pool, const char *file_path)
  : File(pool, file_path),
    image_(NULL)
{
    apr_mmap_t *file_map;

    open(APR_READ);
    file_map = mmap();

    try {
        image_ = new Magick::Image(Magick::Blob(file_map->mm, file_map->size));
    } catch(exception &e) {
        // File の資源は APR のプールが管理しているので問題ない
        throw e.what();
    }
}

ImageFile::~ImageFile()
{
    if (image_ != NULL) {
        delete image_;
    }
}

void ImageFile::create_thumbnail(const char *file_path,
                                 apr_size_t width, apr_size_t height) const
{
    try {
        Magick::Geometry size(static_cast<unsigned int>(width),
                              static_cast<unsigned int>(height));
        Magick::Image thumbnail(*image_);

        thumbnail.scale(size);

        thumbnail.write(file_path);
    } catch(exception &e) {
        throw e.what();
    }
}

apr_uint16_t ImageFile::get_width() const
{
    return static_cast<apr_uint16_t>(image_->size().width());
}

apr_uint16_t ImageFile::get_height() const
{
    return static_cast<apr_uint16_t>(image_->size().height());
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_ImageFile
#include "TestRunner.h"

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name;
    cerr << " <WIDTH> <HEIGHT> <THUMBNAIL_DIR> <IMAGE> [<IMAGE> ...]" << endl;
}

const char *create_thumbnail_path(apr_pool_t *pool,
                                  const char *thumbnail_dir_path,
                                  const char *image_path)
{
    char *thumbnail_path;

    if (apr_filepath_merge(&thumbnail_path, thumbnail_dir_path,
                           basename_ex(image_path),
                           APR_FILEPATH_NOTABOVEROOT, pool) != APR_SUCCESS) {
        THROW(MESSAGE_FILE_PATH_CREATION_FAILED);
    }

    return thumbnail_path;
}

void run_size(apr_pool_t *pool,
              apr_size_t image_width, apr_size_t image_height,
              const char * const *image_path_list, apr_size_t image_path_count)
{
    show_test_name("size");

    for (apr_size_t i = 0; i < image_path_count; i++) {
        ImageFile image(pool, image_path_list[i]);

        if ((image.get_width() != image_width) ||
            (image.get_height() != image_height)) {
            THROW(MESSAGE_BUG_FOUND);
        }
    }

    show_spacer();
}

void run_thumbnail(apr_pool_t *pool, const char *thumbnail_dir_path,
                   apr_size_t thumbnail_width, apr_size_t thumbnail_height,
                   const char * const *image_path_list,
                   apr_size_t image_path_count)
{
    const char *thumbnail_path;

    show_test_name("thumbnail");

    for (apr_size_t i = 0; i < image_path_count; i++) {
        ImageFile image(pool, image_path_list[i]);

        thumbnail_path = create_thumbnail_path(pool, thumbnail_dir_path,
                                               image_path_list[i]);

        apr_file_remove(thumbnail_path, pool);

        image.create_thumbnail(thumbnail_path,
                               thumbnail_width, thumbnail_height);

        if (!File::is_exist(pool, thumbnail_path)) {
            THROW(MESSAGE_BUG_FOUND);
        }
    }
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    apr_size_t image_width;
    apr_size_t image_height;
    const char *thumbnail_dir_path;
    const char * const *image_path_list;
    apr_size_t image_path_count;

    if (argc < 5) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    image_width = atoi(argv[1]);
    image_height = atoi(argv[2]);
    thumbnail_dir_path = argv[3];
    image_path_list = argv + 4;
    image_path_count = argc - 4;

    for (apr_size_t i = 0; i < image_path_count; i++) {
        if (!File::is_exist(pool, image_path_list[i])) {
            THROW(MESSAGE_DATA_DIR_NOT_FOUND);
        }
    }

    for (apr_size_t i = 0; i < image_path_count; i++) {
        show_item("image_path", image_path_list[i]);
    }

    show_line();

    run_size(pool, image_width, image_height,
             image_path_list, image_path_count);
    run_thumbnail(pool, thumbnail_dir_path, image_width/3, image_height/3,
                  image_path_list, image_path_count);
}

#endif

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
