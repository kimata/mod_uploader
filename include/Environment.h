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
 * $Id: Environment.h 2898 2008-07-17 14:34:38Z svn $
 *****************************************************************************/

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#undef strtoul

#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "mod_uploader_config.h"

#ifdef APACHE_HTTPD_H
#ifdef AP_SERVER_MAJORVERSION
#ifndef AP_SERVER_MAJORVERSION_NUMBER
// Apache 2.0 系の若いバージョンもの
#define AP_SERVER_MAJORVERSION_NUMBER 2
#endif
#if AP_SERVER_MINORVERSION_NUMBER >= 4
#define AP_SERVER_VERSION_2_4_OR_HIGHER
#endif
#endif
#ifndef AP_SERVER_MAJORVERSION_NUMBER
// Apache 1.3 系
#define AP_SERVER_MAJORVERSION_NUMBER 1
#endif
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#ifdef __INTEL_COMPILER
// 警告対策 (invalid multibyte character sequence)
#pragma warning(disable:913)
#endif

#ifdef __GNUC__
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

#ifdef WIN32
#ifdef HAVE_UNISTD_H
#undef HAVE_UNISTD_H
#endif
#ifdef HAVE_SCHED_H
#undef HAVE_SCHED_H
#endif
#endif

#ifndef __INTEL_COMPILER
// プリコンパイルヘッダ用
#include "All.h"
#endif

#include "apr_pools.h"
#include "apr_file_io.h"
#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
