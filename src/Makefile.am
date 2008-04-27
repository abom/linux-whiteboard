SUBDIRS = pixmaps windows

whiteboarddatadir = ${datadir}/whiteboard
#whiteboardgtkwindowsdir = ${whiteboarddatadir}/windows/gtk
#whiteboardqtwindowsdir = ${whiteboarddatadir}/windows/qt
whiteboardwindowsdir = ${whiteboarddatadir}/windows/gtk
whiteboardpixmapsdir = ${datadir}/pixmaps

bin_PROGRAMS = whiteboard
whiteboard_CXXFLAGS = 	${whiteboard_CFLAGS} \
			-DWINDOWSDIR=\"$(whiteboardwindowsdir)\" \
			-DPIXMAPSDIR=\"$(whiteboardpixmapsdir)\" \
			-DWHITEBOARDLOCALEDIR=\"$(WHITEBOARDLOCALEDIR)\"
whiteboard_LDFLAGS = ${whiteboard_LIBS} $(LIBINTL)
whiteboard_SOURCES = 	main.cpp common.cpp common.h matrix.h \
			auxiliary.cpp auxiliary.h events.cpp events.h \
			ConfigFileParser.cpp ConfigFileParser.h \
			configurator.cpp configurator.h \
			irfilter.cpp irfilter.h \
			gui.cpp gui.h wiicontrol.cpp wiicontrol.h \
			wiicursor.cpp wiicursor.h gtk-gui.cpp gtk-gui.h \
			wiicursormanager.cpp wiicursormanager.h \
			gettext.h

whiteboarddesktopdir = ${datadir}/applications
whiteboarddesktop_in_files = whiteboard.desktop.in
whiteboarddesktop_DATA = $(whiteboarddesktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
