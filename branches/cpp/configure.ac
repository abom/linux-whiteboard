AC_INIT([whiteboard], [0.1], [http://code.google.com/p/linux-whiteboard/issues/list])
AM_INIT_AUTOMAKE([-Wall -Werror])

AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_INSTALL
#AC_PROG_LIBTOOL

PKG_CHECK_MODULES([whiteboard], [gtkmm-2.4] [libglademm-2.4] [cairomm-1.0] [xtst] [sigc++-2.0])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    src/Makefile
])

AC_OUTPUT
