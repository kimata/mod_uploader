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
 * $Id: FlowController.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef FLOW_CONTROLLER_H
#define FLOW_CONTROLLER_H

#include "Environment.h"

#include <cstdlib>

#include "apr_network_io.h"

#include "Macro.h"
#include "Uncopyable.h"

/**
 * @brief フロー制御を行うクラスの親クラス．
 */
class FlowController: public Uncopyable
{
protected:
    static const apr_size_t MAX_ADDRESS_SIZE          = 40;
    static const apr_size_t MAX_PADDRESS_SIZE         = MAX_ADDRESS_SIZE + 1;

    static const char *get_address(apr_sockaddr_t *sockaddr)
    {
        char *address;

        if (apr_sockaddr_ip_get(&address, sockaddr) != APR_SUCCESS) {
            THROW(MESSAGE_POST_IP_ADDRESS_GET_FAILED);
        }

        return address;
    };
    static const char *get_paddress(apr_sockaddr_t *sockaddr)
    {
        const char *address;
        char *paddress;
        char address_length;

        address = get_address(sockaddr);
        address_length = static_cast<char>(strlen(address));

        if (address_length == 0) {
            THROW(MESSAGE_BUG_FOUND);
        }

        APR_PALLOC(paddress, char *, sockaddr->pool, address_length + 1 + 1);

        paddress[0] = address_length;
        memcpy(paddress + 1, address, address_length);
        paddress[1 + address_length] = '\0';

        return paddress;
    };
    static bool is_address_match(const char *paddress_a,
                                 const char *paddress_b)
    {
        if (paddress_a[0] != paddress_b[0]) {
            return false;
        }

        return (strncmp(paddress_a + 1, paddress_b + 1, paddress_a[0]) == 0);
    };
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
