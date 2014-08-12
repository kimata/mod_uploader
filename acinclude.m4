###############################################################################
# Copyright (C) 2006 Tetsuya Kimata <kimata@acapulco.dyndns.org>
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
# $Id: acinclude.m4 1012 2006-04-11 15:32:28Z svn $
###############################################################################

AC_DEFUN([AC_CHECK_MCPU], [
AC_ARG_WITH(mcpu,
        [  --with-mcpu=CPU         Set optimization target (ie: pentium4, athlon-xp, ...)],
        [MCPU=${withval}], [MCPU='no'])
AC_MSG_CHECKING([for optimization target])
if test "$MCPU" = no; then
    AC_MSG_RESULT(no)
else
    CFLAGS_OLD=$CFLAGS
    CFLAGS="-mcpu=$MCPU"
    AC_LANG(C)
    AC_COMPILE_IFELSE([int i = 0;], , [MCPU='no'])
    if test "$MCPU" = no; then
        AC_MSG_RESULT([ignored])
    else
        AC_MSG_RESULT($MCPU)
        MCPU_OPT="-mcpu=$MCPU"
    fi
    CFLAGS=$CFLAGS_OLD
fi
AC_SUBST(MCPU_OPT)
])

AC_DEFUN([AC_CHECK_MARCH], [
AC_ARG_WITH(march,
        [  --with-march=CPU        Set CPU type (ie: pentium4, athlon-xp, ...)],
        [MARCH=${withval}], [MARCH='no'])
AC_MSG_CHECKING([for CPU type])
if test "$MARCH" = no; then
    AC_MSG_RESULT(no)
else
    CFLAGS_OLD=$CFLAGS
    CFLAGS="$CFLAGS -march=$MARCH"
    AC_LANG(C)
    AC_COMPILE_IFELSE([int i = 0;], , [MARCH='no'])
    if test "$MARCH" = no; then
        AC_MSG_RESULT([ignored])
    else
        AC_MSG_RESULT($MARCH)
        MARCH_OPT="-march=$MARCH"
    fi
    CFLAGS=$CFLAGS_OLD
fi
AC_SUBST(MARCH_OPT)
])

AC_DEFUN([AC_CHECK_LIBTOOL], [
AC_ARG_WITH(libtool,
        [  --with-libtool=LIBTOOL  Set path to libtool program],
        [LIBTOOL=${withval}], [LIBTOOL='no'])
if test "$LIBTOOL" = no; then
    AC_PATH_PROG(GLIBTOOL, glibtool, no, /sw/bin:/usr/bin:/usr/local/bin)
    LIBTOOL=$GLIBTOOL

    if test "$LIBTOOL" = no; then
        AC_PATH_PROG(LIBTOOL, libtool, no, /sw/bin:/usr/bin:/usr/local/bin)
    fi 
fi
if [ ! test -x $LIBTOOL ]; then
    AC_MSG_ERROR([libtool is required. Try --with-libtool option.])
fi 
])

AC_DEFUN([AC_CHECK_APXS], [
AC_ARG_WITH(apxs,
        [  --with-apxs=APXS       Set path to apxs program],
        [APXS=${withval}], [APXS='no'])
if test "$APXS" = no; then
    AC_PATH_PROG(APXS2, apxs2, no,
                 /usr/sbin:/usr/local/sbin:/usr/bin:/usr/local/bin:/usr/local/apache2/bin)
    APXS=$APXS2
    if test "$APXS" = no; then
        AC_PATH_PROG(APXS1, apxs, no,
                     /usr/pkg/sbin:/sw/sbin:/usr/sbin:/usr/local/sbin:/usr/bin:/usr/local/bin:/usr/local/apache2/bin)
        APXS=$APXS1
    fi
fi
if test ! -x $APXS; then
    AC_MSG_ERROR([apxs is required. Try --with-apxs option.])
fi
AP_BUILD_DIR=`env GREP_OPTIONS= grep installbuilddir $APXS | head -n 1 | awk -F '"' '{print $(2)}'`
AC_SUBST(AP_BUILD_DIR)
])

AC_DEFUN([AC_CHECK_APACHE_VERSION], [
AC_ARG_ENABLE(apache13,
        [  --enable-apache13       Use Apache 1.3.x],
        [APACHE_REQ_VERSION=1], [APACHE_REQ_VERSION=2])
AC_MSG_CHECKING([target Apache version])
if test "$APACHE_REQ_VERSION" = 1; then
    AC_MSG_RESULT([1.3.x])
else
    AC_MSG_RESULT([2.x])
fi

APACHE_BIN=`$APXS -q SBINDIR`/`$APXS -q TARGET`
APACHE_VERSION=`$APACHE_BIN -v | head -n 1 | awk -F '/' '{print $(2)}'`
APACHE_MAJOR_VERSION=`echo $APACHE_VERSION | awk -F '.' '{print $(1)}'`
APACHE_MINOR_VERSION=`echo $APACHE_VERSION | awk -F '.' '{print $(2)}'`
AC_MSG_CHECKING([for Apache version])
if test "$APACHE_MAJOR_VERSION" = $APACHE_REQ_VERSION; then
    AC_MSG_RESULT([ok])
    if test $APACHE_REQ_VERSION = 1; then
        ACCESS_MODULE_NAME=access
    else
        if test "$APACHE_MINOR_VERSION" = 0; then
            ACCESS_MODULE_NAME=access
        else
            ACCESS_MODULE_NAME=authz_host
        fi
    fi
else
    AC_MSG_ERROR([Apache HTTP Server $APACHE_REQ_VERSION.x is required.])
fi
AC_SUBST(APXS)
AC_SUBST(APACHE_BIN)
AC_SUBST(APACHE_VERSION)
AC_SUBST(ACCESS_MODULE_NAME)
])

AC_DEFUN([AC_CHECK_APACHE_MOD_DSO], [
APACHE_BIN=`$APXS -q SBINDIR`/`$APXS -q TARGET`
AC_MSG_CHECKING([for DSO enabled])
if test `$APACHE_BIN -l | env GREP_OPTIONS= grep mod_so.c`; then
    AC_MSG_RESULT([ok])
else
    AC_MSG_ERROR([DSO is disabled.])
fi
])

AC_DEFUN([AC_CHECK_APACHECTL], [
AC_ARG_WITH(apctl,
        [  --with-apctl=APCTL      Set path to apachectl program],
        [APACHECTL=${withval}], [APACHECTL='no'])
if test "$APACHECTL" = no; then
    AC_PATH_PROG(APACHE2CTL, apache2ctl, no,
                 /usr/sbin:/usr/local/sbin:/usr/bin:/usr/local/bin:/usr/local/apache2/bin)
    APACHECTL=$APACHE2CTL
    if test "$APACHECTL" = no; then
        AC_PATH_PROG(APACHE1CTL, apachectl, no,
                     /usr/pkg/sbin:/sw/sbin:/usr/sbin:/usr/local/sbin:/usr/bin:/usr/local/bin:/usr/local/apache2/bin)
        APACHECTL=$APACHE1CTL
    fi
fi
if test ! -x $APACHECTL; then
    AC_MSG_ERROR([apachectl is required. Try --with-apctl option.])
fi
])

AC_DEFUN([AC_CHECK_APRCONF], [
AC_ARG_WITH(aprconf,
        [  --with-aprconf=APRCONF  Set path to apr-config program],
        [APRCONF=${withval}], [APRCONF='no'])
if test "$APRCONF" = no; then
    AC_PATH_PROG(APRCONF, apr-1-config, no,
                 /usr/pkg/bin:/sw/bin:/usr/sbin:/usr/local/sbin:/usr/bin:/usr/local/bin:/usr/local/apache2/bin:/usr/local/lib/apache2)
fi
if test "$APRCONF" = no; then
    AC_PATH_PROG(APRCONF, apr-config, no,
                 /usr/pkg/bin:/sw/bin:/usr/sbin:/usr/local/sbin:/usr/bin:/usr/local/bin:/usr/local/apache2/bin:/usr/local/lib/apache2)
fi
if test ! -x $APRCONF; then
    AC_MSG_ERROR([apr-config is required. Try --with-aprconf option.])
fi 
])

AC_DEFUN([AC_CHECK_LIGHTTPD], [
AC_ARG_WITH(lighttpd,
        [  --with-lighttpd=LIGHTTPD Set path to lighttpd program],
        [LIGHTTPD=${withval}], [LIGHTTPD='no'])
if test "$LIGHTTPD" = no; then
    AC_PATH_PROG(LIGHTTPD, lighttpd, no, /sw/sbin:/usr/sbin:/usr/local/sbin)
fi
])

AC_DEFUN([AC_CHECK_MAGICKCONF], [
AC_ARG_WITH(mconf,
        [  --with-mconf=MCONF      Set path to Magick++-config program],
        [MCONF=${withval}], [MCONF='no'])
if test "$MCONF" = no; then
    AC_PATH_PROG(MCONF, Magick++-config, no, /sw/bin:/usr/sbin:/usr/local/sbin:/usr/bin:/usr/local/bin)
fi
if test ! -x $MCONF; then
    AC_MSG_ERROR([Magick++-config is required. Try --with-mconf option.])
fi 
])

AC_DEFUN([AC_CHECK_ICONV_2ND_ARG_TYPE], [
AC_MSG_CHECKING([for type of iconv 2nd argument])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[
#include <stdio.h>
#include <string.h>
#include <iconv.h>
]],
[[
char buffer[256];
iconv_t cd;
char    *inbuf;
size_t  inbytesleft;
char    *outbuf;
size_t  outbytesleft;

inbuf        = "あいうえお";
inbytesleft  = strlen(inbuf);
outbuf       = buffer;
outbytesleft = sizeof(buffer);

if ((cd = iconv_open("EUC-JP", "EUC-JP")) == (iconv_t)-1) {
    return -1;
}
iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
iconv_close(cd);

printf("%s -> %s\n", inbuf, outbuf);
]])], [ICONV_2ND_ARG_TYPE='char *'], [ICONV_2ND_ARG_TYPE='const char *'])
AC_MSG_RESULT($ICONV_2ND_ARG_TYPE)
AC_SUBST(ICONV_2ND_ARG_TYPE)
AC_DEFINE_UNQUOTED(ICONV_2ND_ARG_TYPE, $ICONV_2ND_ARG_TYPE, [iconv の第二引数の型])
])

AC_DEFUN([AC_CHECK_GCC_ATOMIC_BUILTINS], [
AC_ARG_ENABLE(atomic-builtins,
        [  --enable-atomic-builtins Use GCC Atomic Builtins],
        [ATOMIC_BUILTINS=on], [ATOMIC_BUILTINS=off])
AC_MSG_CHECKING([for whether to use GCC Atomic Builtins])
if test "$ATOMIC_BUILTINS" = on; then
    AC_MSG_RESULT(yes)
    AC_MSG_CHECKING([for GCC Atomic Builtins available])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[
    #include <stdint.h>
    ]],
    [[
    int *memory;
    int with;
    int compare;
    
    __sync_val_compare_and_swap(memory, compare, with);
    ]])], [GCC_ATOMIC_BUILTINS=on], [GCC_ATOMIC_BUILTINS=off])
    if test "$GCC_ATOMIC_BUILTINS" = on; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(GCC_ATOMIC_BUILTINS, "on", [GCC Atomic Builts が使えるかどうか．])
    else
        AC_MSG_RESULT(no)
    fi
else
    AC_MSG_RESULT(no)
fi
])

# Local Variables:
# mode: autoconf
# coding: euc-japan-unix
# End:
