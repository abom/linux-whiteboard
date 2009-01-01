add_executable(whiteboard main.cpp common.cpp auxiliary.cpp events.cpp ConfigFileParser.cpp configurator.cpp irfilter.cpp calibration.cpp wiicontrol.cpp wiicursor.cpp gtk-gui.cpp wiicursormanager.cpp)
add_test(whiteboard ${whiteboard_BINARY_DIR}/whiteboard)

target_link_libraries(whiteboard gtkmm-2.4 glademm-2.4 cairomm-1.0 Xtst sigc-2.0 gthread-2.0 cwiid)

#set_target_properties(whiteboard PROPERTIES COMPILE_FLAGS ${CMAKE_C_FLAGS_DEBUG})

set(datadir share)
set(whiteboarddatadir ${datadir}/whiteboard)
#whiteboardgtkwindowsdir = ${whiteboarddatadir}/windows/gtk
#whiteboardqtwindowsdir = ${whiteboarddatadir}/windows/qt
set(whiteboardwindowsdir windows/gtk)
set(whiteboardpixmapsdir pixmaps)

add_definitions(-DGETTEXT_PACKAGE="${CMAKE_PROJECT_NAME}" -DWINDOWSDIR="${CMAKE_INSTALL_PREFIX}/${whiteboarddatadir}/${whiteboardwindowsdir}" -DPIXMAPSDIR="${CMAKE_INSTALL_PREFIX}/${datadir}/${whiteboardpixmapsdir}" -DWHITEBOARDLOCALEDIR="${CMAKE_INSTALL_PREFIX}/${datadir}/locale")

install(FILES ${whiteboardwindowsdir}/main-window.glade ${whiteboardwindowsdir}/calibration-window.glade DESTINATION ${CMAKE_INSTALL_PREFIX}/${whiteboarddatadir}/${whiteboardwindowsdir})
install(TARGETS whiteboard RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
#install(DIRECTORY ${whiteboardwindowsdir} DESTINATION ${CMAKE_INSTALL_PREFIX}/${whiteboarddatadir}/${whiteboardwindowsdir} PATTERN "*.glade")
install(FILES ${whiteboardpixmapsdir}/arrangement.svg ${whiteboardpixmapsdir}/calibration-camera.svg ${whiteboardpixmapsdir}/calibration-percentages.svg ${whiteboardpixmapsdir}/calibration-points.svg ${whiteboardpixmapsdir}/calibration-strays.svg ${whiteboardpixmapsdir}/whiteboard.png ${whiteboardpixmapsdir}/whiteboard.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/${datadir}/${whiteboardpixmapsdir})
install(FILES whiteboard.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/${datadir}/applications)
