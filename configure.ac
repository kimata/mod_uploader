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
# 実行環境の設定
AC_PREREQ(2.59)

###############################################################################
# パッケージ
AC_COPYRIGHT([Copyright (C) 2006-2009 Tetsuya Kimata. All Rights Reserved.])
AC_INIT([mod_uploader],[3.1.1],[kimata@acapulco.dyndns.org])
AC_REVISION($Revision$)

AC_CONFIG_AUX_DIR(build)

PACKAGE_CODE=`echo $PACKAGE_NAME-$PACKAGE_VERSION | md5sum | cut -b -12 | tr '[a-f]' '[A-F]'`
AC_SUBST(PACKAGE_CODE)

###############################################################################
# システム構成
AC_CANONICAL_TARGET

###############################################################################
# ディレクトリ構成
AC_CONFIG_SRCDIR([src/mod_uploader.cpp])
AC_CONFIG_HEADER([include/mod_uploader_config.h])

###############################################################################
# コンパイラ
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
# ヘッダ
AC_CHECK_HEADERS([ \
    ctype.h fcntl.h iconv.h limits.h setjmp.h signal.h stdlib.h string.h \
    sys/mman.h sys/mman.h sys/types.h sys/wait.h unistd.h utime.h sched.h \
])
AC_CHECK_FUNCS([fork getpagesize memmem strndup wait madvise mprotect])

AC_CHECK_LIB([iconv], [main], [LIB_ICONV=-liconv], [LIB_ICONV=])
AC_SUBST(LIB_ICONV)
AC_CHECK_ICONV_2ND_ARG_TYPE

###############################################################################
# 必須関数
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
# コマンド

# オプショナルなので，特にエラーメッセージは出さない．
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
# 環境
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
# カスタマイズ変数
AC_DEFINE(TPL_DEFAULT_TOKEN_POOL_COUNT, 200,    [トークンのデフォルト数．])

AC_DEFINE(ITM_MAX_IDENTIFIER_SIZE,      16,		[アップロードアイテムの識別子の最大長])
AC_DEFINE(ITM_MAX_VERSION_SIZE,         8,      [アップロードアイテムのバージョンの最大長])
AC_DEFINE(ITM_MAX_DATE_SIZE,            32,		[アップロードアイテムの投稿日時の最大長])

AC_DEFINE(ITM_DATE_FORMAT,              "%y/%m/%d(%a) %H:%M:%S", [アップロードアイテムの投稿日時のフォーマット])
AC_DEFINE(ITM_MAX_IP_ADDRESS_SIZE,      40,		[アップロードアイテムの投稿者の IP アドレスの最大長])
AC_DEFINE(ITM_MAX_FILE_NAME_SIZE,       64,		[アップロードファイルの名前の最大長])
AC_DEFINE(ITM_MAX_FILE_MIME_SIZE,       64,		[アップロードファイルの MIME タイプの最大長])
AC_DEFINE(ITM_MAX_FILE_EXT_SIZE,        8,		[アップロードファイルの拡張子の最大長])
AC_DEFINE(ITM_MAX_FILE_DIGEST_SIZE,     48,		[アップロードファイルの MD5 値の最大長])
AC_DEFINE(ITM_MAX_REMOVE_PASS_SIZE,     16,		[アップロードアイテムの削除パスワードの最大長])
AC_DEFINE(ITM_MAX_DOWNLOAD_PASS_SIZE,   16,		[アップロードアイテムのダウンロードパスワードの最大長])
AC_DEFINE(ITM_MAX_COMMENT_SIZE,         256,	[アップロードアイテムのコメントの最大長])

AC_DEFINE(PST_MAX_ITEM_COUNT,           10,     [投稿する項目の最大数])
AC_DEFINE(PST_FLOW_CONTROL_BUFFER_SIZE, 128,    [連続投稿を制御するのに使うバッファのサイズ])
AC_DEFINE(PST_INTERVAL_SEC,             60,     [再投稿可能になるまでの時間(秒)])
AC_DEFINE(PST_PROGRESS_HOLD_SEC,        300,    [リクエストの進捗情報を保持する時間(秒)])

AC_DEFINE(LCK_TIMEOUT_SEC,              300,    [ロックをタイムアウトにするまでの時間(秒)])

AC_DEFINE(VIW_EXPIRES,                  60*60*24*7, [ファイルキャッシュのの有効時間])

AC_DEFINE(VIW_THUMBNAIL_EXT,            "gif",  [サムネイル画像の形式])
AC_DEFINE(VIW_THUMBNAIL_SUFFIX,         "_thumb", [サムネイル画像の名前の接尾辞])
AC_DEFINE(VIW_THUMBNAIL_WIDTH_LIMIT,    250,    [サムネイル画像の最大横サイズ])
AC_DEFINE(VIW_THUMBNAIL_HEIGHT_LIMIT,   250,    [サムネイル画像の最大縦サイズ])
AC_DEFINE(VIW_THUMBNAIL_SIZE_LIMIT,     10*1024*1024, [サムネイル画像の作成を試みる最大ファイルサイズ (Byte)])

AC_DEFINE(ZIP_BUFFER_SIZE,              1024*1024, [ZIP 圧縮のバッファサイズ (Byte)])

AC_DEFINE(SYS_CHARACTER_CODE,           "euc-jp", [データの文字コード])
AC_DEFINE(SYS_TIMEZONE_OFFSET_SUFFIX,   "+09:00", [世界標準時からのオフセット表記])
AC_DEFINE(SYS_TIMEZONE_OFFSET,          9*60*60, [世界標準時からのオフセット表記])
AC_DEFINE(SYS_MMAP_BLOCK_SIZE,          32*1024*1024, [MMAP する単位 (Byte)])
AC_DEFINE(SYS_PAGE_SIZE,                4096,   [ページサイズ (KB)])

AC_DEFINE(REQ_READ_TIMEOUT_SEC,         3600,   [投稿データの読み出しタイムアウト時間(秒)])
AC_DEFINE(REQ_READ_BLOCK_SIZE,          4*1024, [投稿データの読み出しに使うブロックサイズ (Byte)])

AC_DEFINE(RES_IO_VECTOR_SIZE,           128,    [出力用 iovec のサイズ])

AC_DEFINE(HDL_MAX_PATH_INFO_SIZE,       256,    [PATH_INFO の最大長．])

AC_DEFINE(BYTE_UNIT,                    "Byte", [サイズの単位の表記(Byte)．])
AC_DEFINE(KBYTE_UNIT,                   "KB",   [サイズの単位の表記(KByte)．])
AC_DEFINE(MBYTE_UNIT,                   "MB",   [サイズの単位の表記(MByte)．])
AC_DEFINE(GBYTE_UNIT,                   "GB",   [サイズの単位の表記(GByte)．])

AC_DEFINE(DLD_MAX_SESSION_COUNT,        128,    [ダウンロードセッションの合計の最大数．])
AC_DEFINE(DLD_MAX_SESSION_COUNT_PER_IP, 3,      [1 IP 当りのダウンロードセッションの最大数．])

AC_DEFINE(DEFAULT_PER_PAGE_ITEM_NUMBER, 	30,   							[．])
AC_DEFINE(DEFAULT_TOTAL_FILE_SIZE_LIMIT, 	1024LL*1024LL*1024LL*1024LL, 	[．])
AC_DEFINE(DEFAULT_TOTAL_FILE_NUMBER_LIMIT,	10000, 							[．])
AC_DEFINE(DEFAULT_FILE_SIZE_LIMIT,      	10*1024LL*1024LL*1024LL,		[．])

###############################################################################
# 設定
AC_ARG_ENABLE(empty-comment,
        [  --enable-empty-comment  Accept empty comment],
        [EMPTY_COMMENT=on], [EMPTY_COMMENT=off])
AC_MSG_CHECKING([for whether to accept empty comment])
if test "$EMPTY_COMMENT" = on; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(EMPTY_COMMENT, "on", [空のコメントを受け付けるかどうか．])
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
    AC_DEFINE(EMPTY_PASSWORD, "on", [空のパスワードによる削除を受け付けるかどうか．])
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
    AC_DEFINE(REMOVE_UNPOPULAR, "on", [アクセスが無いファイルから先に削除するかどうか．])
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
    AC_DEFINE(MAKE_THUMBNAIL, "on", [サムネイルを作成するかどうか．])
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
    AC_DEFINE_UNQUOTED(NUMNAME, "$NUMNAME", [ダウンロード時のファイル名の接頭辞．])
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
        AC_DEFINE_UNQUOTED(FASTEST, "on", [動作時のテンプレートチェックを行うかどうか．])    
    fi
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi

AC_CACHE_SAVE

###############################################################################
# 実装クラス
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
    AC_DEFINE(USE_GZIP, "on", [ファイルを圧縮するかどうか．])
    WRITER_NAME="ZipFileWriter"
    WRITER_CLASS="ZipFileWriter<>"
    WRITER_DECL="template<class W> class ZipFileWriter"
else
    AC_MSG_RESULT(no)
fi

###############################################################################
# デバッグ
AC_ARG_ENABLE(debug,
        [  --enable-debug          trun on debugging [default no]],
        [DEBUG=on], [DEBUG=off])
AC_MSG_CHECKING(for debugging)
AC_MSG_RESULT($DEBUG)
AC_SUBST(DEBUG)

###############################################################################
# 出力
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
