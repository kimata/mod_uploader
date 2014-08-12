c###############################################################################
# Copyright (C) 2007 Tetsuya Kimata <kimata@acapulco.dyndns.org>
#
# All rights reserved.
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any
# damages arising from the use of this software.
#
# Permission is granted to anyone to use this software for any
# purpose, including commercial applications, and to alter it and
# redistribute it freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must
#    not claim that you wrote the original software. If you use this
#    software in a product, an acknowledgment in the product
#    documentation would be appreciated but is not required.
#
# 2. Altered source versions must be plainly marked as such, and must
#    not be misrepresented as being the original software.
#
# 3. This notice may not be removed or altered from any source
#    distribution.
#
# $Id: configure.in 1006 2006-03-14 14:01:54Z svn $
###############################################################################

###############################################################################
# �¹ԴĶ�������
AC_PREREQ(2.59)

###############################################################################
# �ѥå�����
AC_COPYRIGHT([Copyright (C) 2006-2009 Tetsuya Kimata. All Rights Reserved.])
AC_INIT([mod_uploader],[3.1.1],[kimata@acapulco.dyndns.org])
AC_REVISION($Revision$)

AC_CONFIG_AUX_DIR(build)

PACKAGE_CODE=`echo $PACKAGE_NAME-$PACKAGE_VERSION | md5sum | cut -b -12 | tr '[a-f]' '[A-F]'`
AC_SUBST(PACKAGE_CODE)

###############################################################################
# �����ƥ๽��
AC_CANONICAL_TARGET

###############################################################################
# �ǥ��쥯�ȥ깽��
AC_CONFIG_SRCDIR([src/mod_uploader.cpp])
AC_CONFIG_HEADER([include/mod_uploader_config.h])

###############################################################################
# ����ѥ���
AC_PROG_CPP
AC_PROG_CC
AC_PROG_CXX
AC_EXEEXT

AC_CHECK_LIBTOOL

AC_LANG(C++)

AC_C_INLINE
AC_C_CONST
AC_C_VOLATILE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T

AC_HEADER_STDC

AC_CHECK_GCC_ATOMIC_BUILTINS

###############################################################################
# CPU
AC_CHECK_MARCH

AC_CACHE_SAVE

###############################################################################
# �إå�
AC_CHECK_HEADERS([ \
    ctype.h fcntl.h iconv.h limits.h setjmp.h signal.h stdlib.h string.h \
    sys/mman.h sys/mman.h sys/types.h sys/wait.h unistd.h utime.h sched.h \
])
AC_CHECK_FUNCS([fork getpagesize memmem strndup wait madvise mprotect])

AC_CHECK_LIB([iconv], [main], [LIB_ICONV=-liconv], [LIB_ICONV=])
AC_SUBST(LIB_ICONV)
AC_CHECK_ICONV_2ND_ARG_TYPE

###############################################################################
# ɬ�ܴؿ�
AC_CHECK_FUNCS(isalpha,	, AC_MSG_ERROR([isalpha not found]))
AC_CHECK_FUNCS(isalnum, , AC_MSG_ERROR([isalnum not found]))
AC_CHECK_FUNCS(isdigit,	, AC_MSG_ERROR([isdigit not found]))
AC_CHECK_FUNCS(isxdigit,, AC_MSG_ERROR([isxdigit not found]))

AC_CHECK_FUNCS(malloc,  , AC_MSG_ERROR([malloc not found]))
AC_CHECK_FUNCS(realloc, , AC_MSG_ERROR([realloc not found]))

AC_CHECK_FUNCS(memchr,  , AC_MSG_ERROR([memchr not found]))
AC_CHECK_FUNCS(memcpy,  , AC_MSG_ERROR([memcpy not found]))
AC_CHECK_FUNCS(memmove, , AC_MSG_ERROR([memmove not found]))
AC_CHECK_FUNCS(memset,  , AC_MSG_ERROR([memset not found]))

AC_CHECK_FUNCS(strcpy,  , AC_MSG_ERROR([strcpy not found]))
AC_CHECK_FUNCS(strlen,	, AC_MSG_ERROR([strlen not found]))
AC_CHECK_FUNCS(strcmp,  , AC_MSG_ERROR([strcmp not found]))
AC_CHECK_FUNCS(strncmp, , AC_MSG_ERROR([strncmp not found]))
AC_CHECK_FUNCS(strstr,  , AC_MSG_ERROR([strstr not found]))

AC_CACHE_SAVE

###############################################################################
# Apache
AC_CHECK_APXS 
AC_CHECK_APACHECTL
AC_CHECK_APRCONF

AC_CHECK_APACHE_VERSION
AC_CHECK_APACHE_MOD_DSO

AC_CACHE_SAVE

###############################################################################
# ���ޥ��

# ���ץ���ʥ�ʤΤǡ��ä˥��顼��å������ϽФ��ʤ���
AC_PATH_PROG(GREP,          grep,           grep)
AC_PATH_PROG(SED,           sed,            sed)
AC_PATH_PROG(RUBY,          ruby,           ruby)
AC_PATH_PROG(RST2HTML,      rst2html.py,    rst2html.py)
AC_PATH_PROG(DOXYGEN,       doxygen,        doxygen)
AC_PATH_PROG(DOT, 		    dot,            dot)
AC_PATH_PROG(PS2PDF,        ps2pdf,         ps2pdf)
AC_PATH_PROG(SVN,           svn,            svn)
AC_PATH_PROG(FIND,          find,           find)
AC_PATH_PROG(CAT,           cat,            cat)
AC_PATH_PROG(TAR,           tar,            tar)
AC_PATH_PROG(TEE,           tee,            tee)
AC_PATH_PROG(NKF,           nkf,            nkf)
AC_PATH_PROG(GREP,          grep,           grep)
AC_PATH_PROG(WC,            wc,             wc)
AC_PATH_PROG(IPCS,          ipcs,           ipcs)
AC_PATH_PROG(IPCRM,         ipcrm,          ipcrm)
AC_PATH_PROG(AWK,           awk,            awk)
AC_PATH_PROG(DIFF,          diff,           diff)
AC_PATH_PROG(ENV,           env,            env)
AC_PATH_PROG(DOT, 		    dot,            dot)
AC_PATH_PROG(XARGS, 		xargs,          xargs)
AC_PATH_PROG(GDB, 		    gdb,            gdb)
AC_PATH_PROG(VALGRIND,      valgrind,       valgrind)
AC_PATH_PROG(LSB_RELEASE,   lsb_release,    lsb_release)

###############################################################################
# �Ķ�
UNAME=`uname`
HOSTNAME=`hostname`
USER=`whoami`
PWD=`pwd`
LIBEXECDIR=`$APXS -q LIBEXECDIR`

AC_SUBST(PWD)
AC_SUBST(USER)
AC_SUBST(HOSTNAME)
AC_SUBST(LIBEXECDIR)

###############################################################################
# �������ޥ����ѿ�
AC_DEFINE(TPL_DEFAULT_TOKEN_POOL_COUNT, 200,    [�ȡ�����Υǥե���ȿ���])

AC_DEFINE(ITM_MAX_IDENTIFIER_SIZE,      16,		[���åץ����ɥ����ƥ�μ��̻Ҥκ���Ĺ])
AC_DEFINE(ITM_MAX_VERSION_SIZE,         8,      [���åץ����ɥ����ƥ�ΥС������κ���Ĺ])
AC_DEFINE(ITM_MAX_DATE_SIZE,            32,		[���åץ����ɥ����ƥ����������κ���Ĺ])

AC_DEFINE(ITM_DATE_FORMAT,              "%y/%m/%d(%a) %H:%M:%S", [���åץ����ɥ����ƥ����������Υե����ޥå�])
AC_DEFINE(ITM_MAX_IP_ADDRESS_SIZE,      40,		[���åץ����ɥ����ƥ����ƼԤ� IP ���ɥ쥹�κ���Ĺ])
AC_DEFINE(ITM_MAX_FILE_NAME_SIZE,       64,		[���åץ����ɥե������̾���κ���Ĺ])
AC_DEFINE(ITM_MAX_FILE_MIME_SIZE,       64,		[���åץ����ɥե������ MIME �����פκ���Ĺ])
AC_DEFINE(ITM_MAX_FILE_EXT_SIZE,        8,		[���åץ����ɥե�����γ�ĥ�Ҥκ���Ĺ])
AC_DEFINE(ITM_MAX_FILE_DIGEST_SIZE,     48,		[���åץ����ɥե������ MD5 �ͤκ���Ĺ])
AC_DEFINE(ITM_MAX_REMOVE_PASS_SIZE,     16,		[���åץ����ɥ����ƥ�κ���ѥ���ɤκ���Ĺ])
AC_DEFINE(ITM_MAX_DOWNLOAD_PASS_SIZE,   16,		[���åץ����ɥ����ƥ�Υ���������ɥѥ���ɤκ���Ĺ])
AC_DEFINE(ITM_MAX_COMMENT_SIZE,         256,	[���åץ����ɥ����ƥ�Υ����Ȥκ���Ĺ])

AC_DEFINE(PST_MAX_ITEM_COUNT,           10,     [��Ƥ�����ܤκ����])
AC_DEFINE(PST_FLOW_CONTROL_BUFFER_SIZE, 128,    [Ϣ³��Ƥ����椹��Τ˻Ȥ��Хåե��Υ�����])
AC_DEFINE(PST_INTERVAL_SEC,             60,     [����Ʋ�ǽ�ˤʤ�ޤǤλ���(��)])
AC_DEFINE(PST_PROGRESS_HOLD_SEC,        300,    [�ꥯ�����Ȥο�Ľ������ݻ��������(��)])

AC_DEFINE(LCK_TIMEOUT_SEC,              300,    [���å��򥿥��ॢ���Ȥˤ���ޤǤλ���(��)])

AC_DEFINE(VIW_EXPIRES,                  60*60*24*7, [�ե����륭��å���Τ�ͭ������])

AC_DEFINE(VIW_THUMBNAIL_EXT,            "gif",  [����ͥ�������η���])
AC_DEFINE(VIW_THUMBNAIL_SUFFIX,         "_thumb", [����ͥ��������̾����������])
AC_DEFINE(VIW_THUMBNAIL_WIDTH_LIMIT,    250,    [����ͥ�������κ��粣������])
AC_DEFINE(VIW_THUMBNAIL_HEIGHT_LIMIT,   250,    [����ͥ�������κ���ĥ�����])
AC_DEFINE(VIW_THUMBNAIL_SIZE_LIMIT,     10*1024*1024, [����ͥ�������κ������ߤ����ե����륵���� (Byte)])

AC_DEFINE(ZIP_BUFFER_SIZE,              1024*1024, [ZIP ���̤ΥХåե������� (Byte)])

AC_DEFINE(SYS_CHARACTER_CODE,           "euc-jp", [�ǡ�����ʸ��������])
AC_DEFINE(SYS_TIMEZONE_OFFSET_SUFFIX,   "+09:00", [����ɸ�������Υ��ե��å�ɽ��])
AC_DEFINE(SYS_TIMEZONE_OFFSET,          9*60*60, [����ɸ�������Υ��ե��å�ɽ��])
AC_DEFINE(SYS_MMAP_BLOCK_SIZE,          32*1024*1024, [MMAP ����ñ�� (Byte)])
AC_DEFINE(SYS_PAGE_SIZE,                4096,   [�ڡ��������� (KB)])

AC_DEFINE(REQ_READ_TIMEOUT_SEC,         3600,   [��ƥǡ������ɤ߽Ф������ॢ���Ȼ���(��)])
AC_DEFINE(REQ_READ_BLOCK_SIZE,          4*1024, [��ƥǡ������ɤ߽Ф��˻Ȥ��֥��å������� (Byte)])

AC_DEFINE(RES_IO_VECTOR_SIZE,           128,    [������ iovec �Υ�����])

AC_DEFINE(HDL_MAX_PATH_INFO_SIZE,       256,    [PATH_INFO �κ���Ĺ��])

AC_DEFINE(BYTE_UNIT,                    "Byte", [��������ñ�̤�ɽ��(Byte)��])
AC_DEFINE(KBYTE_UNIT,                   "KB",   [��������ñ�̤�ɽ��(KByte)��])
AC_DEFINE(MBYTE_UNIT,                   "MB",   [��������ñ�̤�ɽ��(MByte)��])
AC_DEFINE(GBYTE_UNIT,                   "GB",   [��������ñ�̤�ɽ��(GByte)��])

AC_DEFINE(DLD_MAX_SESSION_COUNT,        128,    [����������ɥ��å����ι�פκ������])
AC_DEFINE(DLD_MAX_SESSION_COUNT_PER_IP, 3,      [1 IP ����Υ���������ɥ��å����κ������])

AC_DEFINE(DEFAULT_PER_PAGE_ITEM_NUMBER, 	30,   							[��])
AC_DEFINE(DEFAULT_TOTAL_FILE_SIZE_LIMIT, 	1024LL*1024LL*1024LL*1024LL, 	[��])
AC_DEFINE(DEFAULT_TOTAL_FILE_NUMBER_LIMIT,	10000, 							[��])
AC_DEFINE(DEFAULT_FILE_SIZE_LIMIT,      	10*1024LL*1024LL*1024LL,		[��])

###############################################################################
# ����
AC_ARG_ENABLE(empty-comment,
        [  --enable-empty-comment  Accept empty comment],
        [EMPTY_COMMENT=on], [EMPTY_COMMENT=off])
AC_MSG_CHECKING([for whether to accept empty comment])
if test "$EMPTY_COMMENT" = on; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(EMPTY_COMMENT, "on", [���Υ����Ȥ�����դ��뤫�ɤ�����])
else
    AC_MSG_RESULT(no)
fi
AC_SUBST(EMPTY_COMMENT)

AC_ARG_ENABLE(empty-password,
        [  --enable-empty-password Accept empty password],
        [EMPTY_PASSWORD=on], [EMPTY_PASSWORD=off])
AC_MSG_CHECKING([for whether to accept empty password])
if test "$EMPTY_PASSWORD" = on; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(EMPTY_PASSWORD, "on", [���Υѥ���ɤˤ����������դ��뤫�ɤ�����])
else
    AC_MSG_RESULT(no)
fi
AC_SUBST(EMPTY_PASSWORD)

AC_ARG_ENABLE(remove-unpopular,
        [  --enable-remove-unpopular remove files based on popularity],
        [REMOVE_UNPOPULAR=on], [REMOVE_UNPOPULAR=off])
AC_MSG_CHECKING([for whether to remove files based on popularity])
if test "$REMOVE_UNPOPULAR" = on; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(REMOVE_UNPOPULAR, "on", [����������̵���ե����뤫����˺�����뤫�ɤ�����])
else
    AC_MSG_RESULT(no)
fi

AC_ARG_ENABLE(thumbnail,
        [  --enable-thumbnail      trun making thumbnail on],
        [THUMBNAIL=on], [THUMBNAIL=off])
AC_MSG_CHECKING([for whether to enable making thumbnail])
if test "$THUMBNAIL" = on; then
    AC_MSG_RESULT(yes)
    AC_CHECK_MAGICKCONF
    AC_DEFINE(MAKE_THUMBNAIL, "on", [����ͥ����������뤫�ɤ�����])
else
    AC_MSG_RESULT(no)
    MCONF="echo > /dev/null"
fi
AC_SUBST(MAKE_THUMBNAIL)

AC_ARG_ENABLE(numname,
        [  --enable-numname        use numbered file name],
        [NUMNAME=$enableval], [NUMNAME=off])
AC_MSG_CHECKING([for whether to use numbered file name])
if test "$NUMNAME" != off; then
    if test "$NUMNAME" = yes; then
        NUMNAME=$PACKAGE_NAME
    fi
    AC_DEFINE_UNQUOTED(NUMNAME, "$NUMNAME", [����������ɻ��Υե�����̾����Ƭ����])
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi

AC_ARG_ENABLE(fastest,
        [  --enable-fastest        omit dynamic template checking],
        [FASTEST=$enableval], [FASTEST=off])
AC_MSG_CHECKING([for whether to omit dynamic template checking])
if test "$FASTEST" != off; then
    if test "$FASTEST" = yes; then
        AC_DEFINE_UNQUOTED(FASTEST, "on", [ư����Υƥ�ץ졼�ȥ����å���Ԥ����ɤ�����])    
    fi
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi

AC_CACHE_SAVE

###############################################################################
# �������饹
AC_ARG_WITH(writer,
        [  --with-writer=TYPE      write method (mmap or basic)],
        [WRITER_TYPE=${withval}], [WRITER_TYPE=mmap])
AC_MSG_CHECKING([for write method])
if test "$WRITER_TYPE" = mmap; then
    WRITER_NAME="MmapFileWriter"
else
    WRITER_NAME="BasicFileWriter"
fi
WRITER_CLASS=$WRITER_NAME
WRITER_DECL="class $WRITER_NAME"
ORIG_WRITER_NAME=$WRITER_NAME
ORIG_WRITER_CLASS=$WRITER_CLASS

AC_MSG_RESULT($WRITER_NAME)

AC_SUBST(WRITER_NAME)
AC_SUBST(WRITER_CLASS)
AC_SUBST(WRITER_DECL)
AC_SUBST(ORIG_WRITER_NAME)
AC_SUBST(ORIG_WRITER_CLASS)

AC_ARG_ENABLE(gzip,
        [  --enable-gzip           use Content-Encoding: gzip],
        [USE_GZIP=on], [USE_GZIP=off])
AC_MSG_CHECKING([for whether to use use Content-Encoding: gzip])
if test "$USE_GZIP" = on; then
    AC_CHECK_LIB([z], [deflate], , AC_MSG_ERROR([zlib required]))
    AC_MSG_RESULT(yes)
    AC_DEFINE(USE_GZIP, "on", [�ե�����򰵽̤��뤫�ɤ�����])
    WRITER_NAME="ZipFileWriter"
    WRITER_CLASS="ZipFileWriter<>"
    WRITER_DECL="template<class W> class ZipFileWriter"
else
    AC_MSG_RESULT(no)
fi

###############################################################################
# �ǥХå�
AC_ARG_ENABLE(debug,
        [  --enable-debug          trun on debugging [default no]],
        [DEBUG=on], [DEBUG=off])
AC_MSG_CHECKING(for debugging)
AC_MSG_RESULT($DEBUG)
AC_SUBST(DEBUG)

###############################################################################
# ����
AC_CONFIG_FILES([
    conf/apache.conf conf/en/apache.conf conf/cgi.apache.conf conf/cgi.conf \
    conf/stand.conf test/conf/sample.conf \
    GNUmakefile GNUmakefile.apache GNUmakefile.dist \
    src/GNUmakefile.dep src/GNUmakefile.arg src/GNUmakefile.tst \
    src/GNUmakefile.cln src/GNUmakefile src/GNUmakefile.win32 \
    include/MultipartMessageParser.h include/RFC1867Parser.h \
    include/Base64FileWriter.h \
    include/ZipFileWriter.h src/ZipFileWriter.cpp \
    doc/GNUmakefile doc/doxygen.conf
    tmpl/index.htm tmpl/index_swf.htm tmpl/info.htm tmpl/progress.htm \
    tmpl/download.htm tmpl/thumbnail.htm tmpl/admin.htm tmpl/error.htm \
    tmpl/en/index.htm tmpl/en/info.htm tmpl/en/progress.htm tmpl/en/download.htm \
    tmpl/en/thumbnail.htm tmpl/en/admin.htm tmpl/en/error.htm \
    util/mod_uploader.rc \
    util/UploaderConfigSetup/UploaderConfigSetup.vdproj \
    util/UploaderConfig/Properties/AssemblyInfo.cs \
    util/mod_uploader.spec debian/files \
])

AC_OUTPUT

AC_MSG_RESULT([
Configuration:
    * Enviorment            :   ${UNAME}
        - apxs              :   ${APXS}
        - apachectl         :   ${APACHECTL}
        - apr-config        :   ${APRCONF}

    * Compiler              :   ${CXX} 
        - march             :   ${MARCH_OPT}
        - mcpu              :   ${MCPU}

    * Libtool               :   ${LIBTOOL}

    * Thumbnail             :   ${THUMBNAIL}

    * Debug                 :   ${DEBUG}
])

# Local Variables:
# mode: autoconf
# coding: euc-japan-unix
# End: