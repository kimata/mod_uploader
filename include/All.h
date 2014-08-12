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
 * $Id: All.h 2794 2008-02-16 06:40:59Z svn $
 *****************************************************************************/

#ifndef ALL_H
#define ALL_H

#include "AtomicWrapper.h"
#include "Auxiliary.h"
#include "BasicFileWriter.h"
#include "CharCodeConverter.h"
#include "CleanPointer.h"
#include "ConfigReader.h"
#include "DirectoryCleaner.h"
#include "File.h"
#include "FileWriter.h"
#include "ImageFile.h"
#include "Locker.h"
#include "Logger.h"
#include "Macro.h"
#include "Message.h"
#include "MessageDigest5.h"
#include "MmapFileWriter.h"
#include "MultipartMessageParser.h"
#include "MultipartMessageParserBuffer.h"
#include "PostDataChecker.h"
#include "PostFlowController.h"
#include "RFC1867Parser.h"
#include "ReadLocker.h"
#include "ReadWriteLocker.h"
#include "SourceInfo.h"
#include "TemplateLexer.h"
#include "TemplateParser.h"
#include "TemplateVariable.h"
#include "TemplateVariableCreator.h"
#include "TemporaryFile.h"
#include "TemporaryPool.h"
#include "Uncopyable.h"
#include "UploadItem.h"
#include "UploadItemIO.h"
#include "UploadItemIterator.h"
#include "UploadItemList.h"
#include "UploadItemListReader.h"
#include "UploadItemManager.h"
#include "UploadItemReader.h"
#include "UploadItemRss.h"
#include "UploadItemVariableCreator.h"
#include "UploadItemWriter.h"
#include "UploaderConfig.h"
#include "UploaderTemplate.h"
#include "WriteLocker.h"
#include "ZipFileWriter.h"

#include "apr.h"
#include "apr_file_info.h"
#include "apr_file_io.h"
#include "apr_general.h"
#include "apr_global_mutex.h"
#include "apr_mmap.h"
#include "apr_pools.h"
#include "apr_shm.h"
#include "apr_strings.h"
#include "apr_tables.h"
#include "apr_thread_cond.h"
#include "apr_thread_mutex.h"
#include "apr_thread_proc.h"
#include "apr_time.h"

#ifdef WIN32
#include "httpd.h"
#endif

#include <algorithm>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <limits.h>

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
