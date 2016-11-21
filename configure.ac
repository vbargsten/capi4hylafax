dnl Process this file with autoconf to produce a configure script.

AC_INIT([capi4hylafax],[01.03.01vb])
AC_CONFIG_SRCDIR([src/defaults.h.in])
AC_CONFIG_AUX_DIR(src/scripts)
AM_MAINTAINER_MODE
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE

CXXFLAGS="-Wall -W $CXXFLAGS -D_GNU_SOURCE -DDONT_USE_PROTECT_FEATURE"

AC_LANG([C++])

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_INSTALL
LT_INIT

dnl Check for va_copy in stdarg.h
AC_CACHE_CHECK([for va_copy],ac_func_cv_va_copy,[
AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <stdarg.h>
void x(va_list args)
{
    va_list copy;
    va_copy(copy,args);
}
], []) ], [ac_func_cv_va_copy=yes], [ac_func_cv_va_copy=no])
])

if test $ac_func_cv_va_copy = no; then
    AC_CACHE_CHECK([for __va_copy],ac_func_cv___va_copy,[
        AC_LINK_IFELSE([AC_LANG_PROGRAM([
        #include <stdarg.h>
        void x(va_list args)
        {
           va_list copy;
           __va_copy(copy,args);
        }
    ], [])], [ac_func_cv___va_copy=yes], [ac_func_cv___va_copy=no])
    ])
    if test $ac_func_cv___va_copy = no; then
       AC_DEFINE(va_copy(d,s),(d)=(s))
    else
       AC_DEFINE(va_copy(d,s),__va_copy(d,s))
    fi
fi

AC_SUBST(LIBTOOL_DEPS)

dnl Optional hylafax spool directory
AC_ARG_WITH(hylafax-spooldir,
    AS_HELP_STRING([--with-hylafax-spooldir=DIR],[Set dir where hylafax spool is installed. (/var/spool/hylafax)]),
    hylafaxspooldir="$withval",
    hylafaxspooldir="/var/spool/hylafax"
)

dnl Optional debug
AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug],[Turn debugging on (default=no)]),
    [case "$enableval" in
        yes | true) debugflags="-g -O0";;
        no | false) debugflags="-DNDEBUG -g -O2";;
        *)          AC_MSG_ERROR(bad value $enableval for --enable-debug) ;;
     esac],
    [debugflags=-DNDEBUG]
)
CXXFLAGS="$CXXFLAGS $debugflags"
dnl AM_CONDITIONAL(NDEBUG, test x$debug = xfalse)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(string.h memory.h)
AC_CHECK_HEADERS(capi20.h)

AC_CHECK_LIB(capi20, capi20_register, , AC_MSG_ERROR(libcapi20 not found))
AC_CHECK_LIB(tiff, TIFFGetVersion, , AC_MSG_ERROR(libtiff not found))
AC_CHECK_LIB(pthread, pthread_create, , AC_MSG_ERROR(libpthread not found))

AC_CHECK_SIZEOF(void *)

dnl Read the libtool configuration
dnl rm -f conftest
dnl ./libtool --config > conftest
dnl . ./conftest
dnl rm -f conftest

CFLAGS=$CXXFLAGS

AC_SUBST(INSTALL)
AC_SUBST(hylafaxbindir)
AC_SUBST(hylafaxspooldir)
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_CONFIG_FILES([Makefile src/Makefile src/defaults.h
          src/standard/Makefile src/capi/Makefile
          src/convert/Makefile src/linuxdep/Makefile
          src/faxrecv/Makefile src/faxsend/Makefile])
AC_OUTPUT

