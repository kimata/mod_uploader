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
 * $Id: RequestReader.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include "RequestReader.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: RequestReader.cpp 2756 2007-12-11 10:57:59Z svn $");

const apr_size_t RequestReader::DEFAULT_BLOCK_SIZE      = REQ_READ_BLOCK_SIZE;


/******************************************************************************
 * public メソッド
 *****************************************************************************/
RequestReader::RequestReader(post_progress_t *progress)
    : block_size_(DEFAULT_BLOCK_SIZE),
      progress_(progress)
{

}

void RequestReader::set_block_size(apr_size_t block_size)
{
    block_size_ = block_size;
}

RequestReader::~RequestReader()
{

}


/******************************************************************************
 * protected メソッド
 *****************************************************************************/
void RequestReader::update(apr_size_t read_size)
{
#ifdef DEBUG
    if (progress_ == NULL) {
        THROW(MESSAGE_BUG_FOUND);
    }
#endif

    progress_->read_size += read_size;
}

apr_size_t RequestReader::get_block_size() const
{
    return block_size_;
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
