SUBDIRS = windows

whiteboarddir = ${prefix}/share/whiteboard
windowsdir = ${whiteboarddir}/windows

bin_PROGRAMS = whiteboard
whiteboard_DATA = whiteboard.svg
whiteboard_CXXFLAGS = ${whiteboard_CFLAGS} -DDATADIR=\"$(whiteboarddir)\" -DWINDOWSDIR=\"$(windowsdir)\"
whiteboard_LDFLAGS = ${whiteboard_LIBS} -lcwiid $(LIBINTL)
whiteboard_SOURCES = 	main.cpp common.h matrix.h \
			auxiliary.cpp auxiliary.h events.cpp events.h \
			gui.cpp gui.h wii.cpp wii.h \
			wiicursor.cpp wiicursor.h gtk-gui.cpp gtk-gui.h \
			gettext.h

#desktopdir = ${whiteboarddir}

#desktop_in_files = whiteboard.desktop.in
#desktop_DATA = $(whiteboard_in_files:.desktop.in=.desktop)
#@INTLTOOL_DESKTOP_RULE@
