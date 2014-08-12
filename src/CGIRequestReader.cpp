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
 * $Id: CGIRequestReader.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <iostream>

#include "CGIRequestReader.h"
#include "Macro.h"
#include "SourceInfo.h"

using namespace std;

SOURCE_INFO_ADD("$Id: CGIRequestReader.cpp 2756 2007-12-11 10:57:59Z svn $");

/******************************************************************************
 * public メソッド
 *****************************************************************************/
CGIRequestReader::CGIRequestReader(post_progress_t *progress, void *r,
                                   istream *stream)
  : RequestReader(progress),
    stream_(stream)
{

}

void CGIRequestReader::read(char *buffer, apr_size_t size,
                            apr_size_t *read_size)
{
    if (UNLIKELY(stream_->eof())) {
        return;
    }

    stream_->read(buffer, static_cast<streamsize>(size));
    *read_size = stream_->gcount();

    update(*read_size);
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
