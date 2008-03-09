SUBDIRS = pixmaps windows

whiteboarddatadir = ${datadir}/whiteboard
whiteboardwindowsdir = ${whiteboarddatadir}/windows
whiteboardpixmapsdir = ${datadir}/pixmaps

bin_PROGRAMS = whiteboard
whiteboard_CXXFLAGS = 	${whiteboard_CFLAGS} \
			-DWINDOWSDIR=\"$(whiteboardwindowsdir)\" \
			-DPIXMAPSDIR=\"$(whiteboardpixmapsdir)\" \
			-DWHITEBOARDLOCALEDIR=\"$(WHITEBOARDLOCALEDIR)\"
whiteboard_LDFLAGS = ${whiteboard_LIBS} -lcwiid $(LIBINTL)
whiteboard_SOURCES = 	main.cpp common.h matrix.h \
			auxiliary.cpp auxiliary.h events.cpp events.h \
			gui.cpp gui.h wii.cpp wii.h \
			wiicursor.cpp wiicursor.h gtk-gui.cpp gtk-gui.h \
			gettext.h

whiteboarddesktopdir = ${datadir}/applications
whiteboarddesktop_in_files = whiteboard.desktop.in
whiteboarddesktop_DATA = $(whiteboarddesktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
