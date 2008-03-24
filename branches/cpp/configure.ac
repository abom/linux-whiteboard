AC_INIT([Wiimote Whiteboard], [0.3], [http://code.google.com/p/linux-whiteboard/issues/list], [whiteboard])
# No AUX_DIR because of intltoolize's bug
#AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror])

AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL

AC_PROG_INTLTOOL
GETTEXT_PACKAGE=whiteboard
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [The domain to use with gettext])
AM_GLIB_GNU_GETTEXT

WHITEBOARDLOCALEDIR=[${datadir}/locale]
AC_SUBST(WHITEBOARDLOCALEDIR)

AC_CHECK_LIB([m], [sin], [], AC_MSG_ERROR([Dependency unmet: math library not found. Your system must be broken]))
AC_CHECK_LIB([cwiid], [cwiid_command], [], AC_MSG_ERROR([Dependency unmet: Wiimote library (libcwiid) not found]))
AC_CHECK_LIB([confuse], [cfg_parse], [], AC_MSG_ERROR([Dependency unmet: Configuration parsing library (libconfuse) not found]))
PKG_CHECK_MODULES([whiteboard], [gtkmm-2.4] [libglademm-2.4] [cairomm-1.0] [xtst] [sigc++-2.0])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    po/Makefile.in
    src/Makefile
    src/pixmaps/Makefile
    src/windows/Makefile
])

AC_OUTPUT
