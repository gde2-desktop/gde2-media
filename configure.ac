AC_PREREQ(2.60)

AC_INIT([gde2-media],
        [1.16.0],
        [http://www.gde2-desktop.org/])

AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.9 no-dist-gzip dist-xz tar-pax foreign check-news])
AM_MAINTAINER_MODE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

IT_PROG_INTLTOOL([0.35.0])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h math.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

GLIB_REQUIRED_VERSION=2.36.0
GIO_REQUIRED_VERSION=2.36.0
CANBERRA_REQUIRED_VERSION=0.13
MATE_MIXER_REQUIRED_VERSION=1.9.1
GDE2_DESKTOP_REQUIRED_VERSION=1.1.0

dnl=======================================================================
dnl Support multiple GTK versions
dnl=======================================================================

AC_MSG_CHECKING([which gtk+ version to compile against])
AC_ARG_WITH([gtk],
  [AS_HELP_STRING([--with-gtk=2.0|3.0],[which gtk+ version to compile against (default: 2.0)])],
  [case "$with_gtk" in
     2.0|3.0) ;;
     *) AC_MSG_ERROR([invalid gtk version specified]) ;;
   esac],
  [with_gtk=2.0])
AC_MSG_RESULT([$with_gtk])

case "$with_gtk" in
  2.0) GTK_API_VERSION=2.0
       GTK_REQUIRED_VERSION=2.24.0
       CANBERRA_API_VERSION=
       UNIQUE_API_VERSION=1.0
       ;;
  3.0) GTK_API_VERSION=3.0
       GTK_REQUIRED_VERSION=3.0.0
       CANBERRA_API_VERSION=3
       UNIQUE_API_VERSION=3.0
       ;;
esac

AC_SUBST(GTK_API_VERSION)

dnl=======================================================================
dnl Check for the volume control modules
dnl=======================================================================

PKG_CHECK_MODULES(VOLUME_CONTROL,
                  gobject-2.0 >= $GLIB_REQUIRED_VERSION
                  gtk+-$GTK_API_VERSION >= $GTK_REQUIRED_VERSION
                  gio-2.0 >= $GIO_REQUIRED_VERSION
                  libcanberra-gtk$CANBERRA_API_VERSION >= $CANBERRA_REQUIRED_VERSION
                  unique-$UNIQUE_API_VERSION
                  libxml-2.0
                  gde2-desktop-2.0 >= $GDE2_DESKTOP_REQUIRED_VERSION
                  libmatemixer >= $MATE_MIXER_REQUIRED_VERSION
)
AC_SUBST(VOLUME_CONTROL_CFLAGS)
AC_SUBST(VOLUME_CONTROL_LIBS)

dnl=======================================================================
dnl GLib
dnl=======================================================================

GLIB_GSETTINGS

dnl=======================================================================
dnl i18n
dnl=======================================================================

GETTEXT_PACKAGE=gde2-media
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Define to the Gettext package name])
AM_GLIB_GNU_GETTEXT

dnl ---------------------------------------------------------------------------
dnl - Finish
dnl ---------------------------------------------------------------------------

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

GDE2_MAINTAINER_MODE_DEFINES
GDE2_COMPILE_WARNINGS([maximum])
GDE2_CXX_WARNINGS

AC_ARG_ENABLE(deprecated,
        [AS_HELP_STRING([--enable-deprecated],
                [warn about deprecated usages [default=no]])],,
         [enable_deprecated=no])

if test "x$enable_deprecated" = "xyes"; then
    DISABLE_DEPRECATED=""
    AC_SUBST(DISABLE_DEPRECATED)
fi

# Turn on the additional warnings last, so warnings don't affect other tests.
AC_ARG_ENABLE(more-warnings,
        [AC_HELP_STRING([--enable-more-warnings],
        [Maximum compiler warnings])],
        set_more_warnings="$enableval",[
                if test -d $srcdir/.git; then
                        set_more_warnings=yes
                else
                        set_more_warnings=no
                fi
        ])

AC_MSG_CHECKING(for more warnings)
if test "$GCC" = "yes" -a "$set_more_warnings" != "no"; then
        AC_MSG_RESULT(yes)
        CFLAGS="\
        -Wall \
        -Wchar-subscripts -Wmissing-declarations -Wmissing-prototypes \
        -Wnested-externs -Wpointer-arith \
        -Wcast-align -Wsign-compare \
        $CFLAGS"

        for option in -Wno-unused-parameter -Wno-strict-aliasing -Wno-sign-compare; do
                SAVE_CFLAGS="$CFLAGS"
                CFLAGS="$CFLAGS $option"
                AC_MSG_CHECKING([whether gcc understands $option])
                AC_TRY_COMPILE([], [],
                        has_option=yes,
                        has_option=no,)
                if test $has_option = no; then
                        CFLAGS="$SAVE_CFLAGS"
                fi
                AC_MSG_RESULT($has_option)
                unset has_option
                unset SAVE_CFLAGS
        done
        unset option
else
        AC_MSG_RESULT(no)
fi
AC_SUBST(CFLAGS)

AC_CONFIG_FILES([
Makefile
data/Makefile
data/gde2-volume-control.desktop.in
data/icons/Makefile
data/icons/16x16/Makefile
data/icons/16x16/status/Makefile
data/icons/22x22/Makefile
data/icons/22x22/status/Makefile
data/icons/24x24/Makefile
data/icons/24x24/status/Makefile
data/icons/32x32/Makefile
data/icons/32x32/status/Makefile
data/icons/scalable/Makefile
data/icons/scalable/devices/Makefile
data/icons/scalable/status/Makefile
data/sounds/Makefile
man/Makefile
po/Makefile.in
gde2-volume-control/Makefile
])

AC_OUTPUT

echo "

                    ${PACKAGE} ${VERSION}
                    ==========


        Prefix:                      ${prefix}
        Source code location:        ${srcdir}
        Compiler:                    ${CC}
        CFLAGS:                      ${CFLAGS}
        GTK+ API version:            ${GTK_API_VERSION}
"
