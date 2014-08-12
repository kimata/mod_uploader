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
 *    documentation would be appreciated but is not bcktuired.
 *
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 * $Id: MultipartMessageParserBuffer.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef MULTIPART_MESSAGE_PARSER_BUFFER_H
#define MULTIPART_MESSAGE_PARSER_BUFFER_H

#include "Environment.h"

#include <cstdlib>

#include "apr.h"

#include "Macro.h"
#include "Message.h"
#include "Uncopyable.h"


/**
 * @brief MultipartMessageParser 用のバッファを管理するクラス．
 */
class MultipartMessageParserBuffer: public Uncopyable
{
public:
    MultipartMessageParserBuffer(apr_size_t size=0);
    ~MultipartMessageParserBuffer();
    void reserve(apr_size_t size);
    char *get_data() const
    {
#ifdef DEBUG
        if (buffer_ == NULL) {
            THROW(MESSAGE_BUG_FOUND);
        }
#endif
        return buffer_;
    };
    char *get_data_end() const
    {
        return get_data() + get_size();
    };
    void add_size(apr_size_t size)
    {
#ifdef DEBUG
        if ((get_size() + size) > buffer_size_) {
            THROW(MESSAGE_BUG_FOUND);
        }
#endif
        set_size(get_size() + size);
    };
    void set_size(apr_size_t size)
    {
#ifdef DEBUG
        if (size > buffer_size_) {
            THROW(MESSAGE_BUG_FOUND);
        }
#endif
        size_ = size;
        *(buffer_+size_) = '\0';
    };
    apr_size_t get_size() const
    {
        return size_;
    };
    void erase(apr_size_t size)
    {
#ifdef DEBUG
        if (size > size_) {
            THROW(MESSAGE_BUG_FOUND);
        } else if (buffer_ == NULL) {
            THROW(MESSAGE_BUG_FOUND);
        }
#endif
        if (size == 0) {
            return;
        }

        size_ -= size;
        memmove(buffer_, buffer_+size, size_);
    }

private:
    static const apr_size_t DEFAULT_SIZE;

    char *buffer_;
    apr_size_t buffer_size_;
    apr_size_t size_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
