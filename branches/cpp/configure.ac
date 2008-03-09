AC_INIT([Wiimote Whiteboard], [0.2], [http://code.google.com/p/linux-whiteboard/issues/list], [whiteboard])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror])

# ?
#AC_PROG_INTLTOOL

#AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])
#AM_GLIB_GNU_GETTEXT

AC_DEFINE_UNQUOTED([LOCALEDIR], "$(localedir)", [The locale directory])

AC_PROG_CXX
AC_PROG_LIBTOOL

AC_CHECK_LIB([cwiid], [cwiid_connect])
PKG_CHECK_MODULES([whiteboard], [gtkmm-2.4] [libglademm-2.4] [cairomm-1.0] [xtst] [sigc++-2.0])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    po/Makefile.in
    src/Makefile
    src/windows/Makefile
])

AC_OUTPUT
