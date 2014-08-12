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
 * $Id: TemporaryPool.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef TEMPORARY_POOL_H
#define TEMPORARY_POOL_H

#include "Environment.h"

#include "Message.h"
#include "Macro.h"
#include "Uncopyable.h"

/**
 * @brief 一時的なプールを表すクラス．
 */
class TemporaryPool: public Uncopyable
{
public:
    /**
     * コンストラクタです．
     *
     * 一時的なプールを作成します．
     *
     * @param[in] pool プール
     */
    explicit TemporaryPool(apr_pool_t *pool=NULL)
    {
        if (apr_pool_create(&pool_, pool) != APR_SUCCESS) {
            // 例外投げちゃいます
            THROW(MESSAGE_POOL_CREATION_FAILED);
        }
    };

    /**
     * デストラクタです．
     *
     * 一時的なプールを開放します．
     */
    ~TemporaryPool()
    {
        apr_pool_destroy(pool_);
    };

    /**
     * コメントを書き込みます．
     */
    apr_pool_t *get() const
    {
        return pool_;
    };

private:
    /** プール */
    apr_pool_t *pool_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
