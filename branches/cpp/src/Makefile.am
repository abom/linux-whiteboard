whiteboarddir = ${prefix}/share/whiteboard

bin_PROGRAMS = whiteboard
whiteboard_DATA = icon.svg main-window.glade calibration-window.glade
whiteboard_CXXFLAGS = ${whiteboard_CFLAGS} -DDATADIR=\"$(whiteboarddir)\"
whiteboard_LDFLAGS = ${whiteboard_LIBS} -lcwiid
whiteboard_SOURCES = auxiliary.cpp events.cpp gui.cpp main.cpp wii.cpp wiicursor.cpp gtk-gui.cpp
