cmake_minimum_required(VERSION 2.6)
project(whiteboard)

include(CTest)
enable_testing()
include(FindPkgConfig)
include(FindGettext)
include(CPack)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build/lib)
set(whiteboard_SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/src)
set(whiteboard_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-Wall -Werror -O0 -g -ggdb -pipe")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Werror -O2 -march=core2 -pipe -msse3")

set(CPACK_PACKAGE_CONTACT VAnhTu1987)
set(CPACK_CMAKE_GENERATOR "Unix Makefiles")
set(CPACK_GENERATOR "DEB")

pkg_check_modules(GTKMM REQUIRED gtkmm-2.4)
pkg_check_modules(GLADEMM REQUIRED libglademm-2.4)
include_directories(${whiteboard_SOURCE_DIR} ${GTKMM_INCLUDE_DIRS} ${GLADEMM_INCLUDE_DIRS})
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(PO_DIR po)                                                                                                                                                 
GETTEXT_CREATE_TRANSLATIONS(${PO_DIR}/whiteboard.pot ALL ${PO_DIR}/ca.po ${PO_DIR}/de.po ${PO_DIR}/es.po ${PO_DIR}/fr.po ${PO_DIR}/vi.po ${PO_DIR}/sv.po)

add_subdirectory(${whiteboard_SOURCE_DIR})
