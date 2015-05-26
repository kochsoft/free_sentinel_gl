# /**
# Sentinel Gl -- an OpenGL based remake of the Firebird classic the Sentinel.
# Copyright (C) May 25th, 2015 Markus-Hermann Koch, mhk@markuskoch.eu
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# */

# CMakeLists.txt
# ==============
# In order to successfully install Sentinel Gl on your computer you are going
# to need
# * cmake
# * a valid compiler like g++ under Linux or as offered by Visual Studio or
#   mingw under windows.
# * Qt Version 5.4
# * Under Windows make sure that ..\Qt\<version>\bin\ is included in your path
#   and that you build the program as Release version if you do so under
#   Visual Studio.
#
# If all dependencies are present it should suffice to enter the directory
# where you found this CMakeLists.txt and cast
#   cmake .
#   make
# This should generate the executable 'sentinel'.
#
# General Hints (that should not be needed if all runs well)
# -------------
# Nice tutorial for CMake: http://www.cmake.org/cmake/help/cmake_tutorial.html
# Read some time:
# http://stackoverflow.com/questions/7046956/populating-srcs-from-cmakelists-txt-in-subdirectories
#
# Generic valgrind command:
#     valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes ./sentinel 2&> valgrind.txt
#     grep "definitely.*lost" valgrind.txt  | less
#
# My favorite Guru, Alex Allain, writes on
#   http://www.cprogramming.com/debugging/segfaults.html
# that compiling after calling
#   ulimit -c unlimited
# or with limited output:
#   ulimit -f 200000
# is the gospel for generating core files in the event of a crash. Then
#   gdb ./sentinel core
# and there
#   backtrace
#
# gdb: Check out this beautiful reference card
#   http://www.yolinux.com/TUTORIALS/GDB-Commands.html
#
# In CMakeLists.txt the project(..) cmd really should come first. If not this
# may lead to the symptom of internal variables that are not defined.
#   http://www.mail-archive.com/cmake@cmake.org/msg13392.html
#
# About cmake and Qt5:
# http://doc.qt.io/qt-5/cmake-manual.html
# http://www.kdab.com/using-cmake-with-qt-5/
# All Modules are listed here:
# http://doc.qt.io/qt-5/qtmodules.html

# 2.8.11 instead of 2.8 required for automatic linking to the qtmain.lib
# library if this ever should expand to Windows.
# (http://doc.qt.io/qt-5/cmake-manual.html)
cmake_minimum_required(VERSION 2.8.11)

#> Finding my local installation of Qt 5.4. --------------------------
# Actually first calling
#   export CMAKE_PREFIX_PATH=[directory containing Qt5Widgets .cmake]
# on the console works as well.

# set(CMAKE_PREFIX_PATH "/home/kochmn/sw/Qt/Qt_5_4/5.4/gcc_64/lib/cmake/")
#< -------------------------------------------------------------------

project(sentinel)

#> Getting the Qt essentials. ----------------------------------------
set( CMAKE_AUTOMOC ON )
# CMake uses uic in order to generate header files from .ui forms from designer.
set ( CMAKE AUTOUIC ON )
# Auto-generating functions write their headers into the build directory.
# Hence the build directory should be included.
set( CMAKE_INCLUDE_CURRENT_DIR ON )
# Widgets finds its own dependencies (QtGui and QtCore).
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)

message ("Found Qt5Widgets Version ${Qt5Widgets_VERSION_STRING}")

# All those darling variables are explained here:
# http://doc.qt.io/qt-5/cmake-manual.html
message("Core    FOUND: ${Qt5Core_FOUND}")
message("Gui     FOUND: ${Qt5Gui_FOUND}")
message("Widgets FOUND: ${Qt5Widgets_FOUND}")
message("Multimedia FOUND: ${Qt5Multimedia_FOUND}")
message("Core    VERSION: ${Qt5Core_VERSION_STRING}")
message("Gui     VERSION: ${Qt5Gui_VERSION_STRING}")
message("Widgets VERSION: ${Qt5Widgets_VERSION_STRING}")
message("Multimedia VERSION: ${Qt5Multimedia_VERSION_STRING}")
message("Core    INCLUDE: ${Qt5Core_INCLUDE_DIRS}")
message("Gui     INCLUDE: ${Qt5Gui_INCLUDE_DIRS}")
message("Widgets INCLUDE: ${Qt5Widgets_INCLUDE_DIRS}")
message("Multimedia INCLUDE: ${Qt5Multimedia_INCLUDE_DIRS}")
message("Core    LIBRARIES: ${Qt5Core_LIBRARIES}")
message("Gui     LIBRARIES: ${Qt5Gui_LIBRARIES}")
message("Widgets LIBRARIES: ${Qt5Widgets_LIBRARIES}")
message("Multimedia LIBRARIES: ${Qt5Multimedia_LIBRARIES}")
message("Core    DEFINITIONS: ${Qt5Core_DEFINITIONS}")
message("Gui     DEFINITIONS: ${Qt5Gui_DEFINITIONS}")
message("Widgets DEFINITIONS: ${Qt5Widgets_DEFINITIONS}")
message("Multimedia DEFINITIONS: ${Qt5Multimedia_DEFINITIONS}")
message("Core    COMPILE_DEFINITIONS: ${Qt5Core_COMPILE_DEFINITIONS}")
message("Gui     COMPILE_DEFINITIONS: ${Qt5Gui_COMPILE_DEFINITIONS}")
message("Widgets COMPILE_DEFINITIONS: ${Qt5Widgets_COMPILE_DEFINITIONS}")
message("Multimedia COMPILE_DEFINITIONS: ${Qt5Multimedia_COMPILE_DEFINITIONS}")
message("Core    EXECUTABLE_COMPILE_FLAGS: ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")
message("Gui     EXECUTABLE_COMPILE_FLAGS: ${Qt5Gui_EXECUTABLE_COMPILE_FLAGS}")
message("Widgets EXECUTABLE_COMPILE_FLAGS: ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
message("Multimedia EXECUTABLE_COMPILE_FLAGS: ${Qt5Multimedia_EXECUTABLE_COMPILE_FLAGS}")

include_directories(
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Multimedia_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS})

add_definitions(${Qt5Widgets_DEFINITIONS})
#add_definitions(${Qt5Core_DEFINITIONS}) # Unnecessary. In Widgets.
#add_definitions(${Qt5Gui_DEFINITIONS})  # Unnecessary. In Widgets.
add_definitions(${Qt5Multimedia_DEFINITIONS})
#< -------------------------------------------------------------------

#> As for LAPACKe and BLAS. ------------------------------------------
# sudo apt-get install libblas-dev checkinstall
# sudo apt-get install libblas-doc checkinstall
# sudo apt-get install liblapacke-dev checkinstall
# sudo apt-get install liblapack-doc checkinstall
#
# The libraries went in /usr/lib and the includes in /usr/include.
#
# g++ svd_demo.cpp -I"/usr/include" -L"/usr/lib" -llapacke -lblas
#< -------------------------------------------------------------------

set (DEBUG 1)
set (SENTINEL_NAME "Free Sentinel GL")
set (SENTINEL_VERSION_MAJOR "1")
set (SENTINEL_VERSION_MINOR "0")
set (SENTINEL_VERSION "${SENTINEL_VERSION_MAJOR}.${SENTINEL_VERSION_MINOR}")

## Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} ${Qt5Multimedia_EXECUTABLE_COMPILE_FLAGS}")
#  ${Qt5Core_EXECUTABLE_COMPILE_FLAGS} ${Qt5Gui_EXECUTABLE_COMPILE_FLAGS} <- redundant.

if(CMAKE_COMPILER_IS_GNUCXX)
  message("Using GnuCXX compiler.")
  add_definitions("-O0 -std=c++0x")
  #add_definitions("-O0 -std=c++0x -llapacke -lblas")
endif()

if (DEBUG MATCHES 1)
  message("\nBuilding DEBUG build.")
  add_definitions(-Wall)
  set(CMAKE_BUILD_TYPE Debug)
endif()

message("\n${SENTINEL_NAME} -- C++ Project V ${SENTINEL_VERSION}.\n")
message("(c) Markus-Hermann Koch, mhk@markuskoch.eu, 2015/04/28-?\n")
message("Primary directory is ${PROJECT_SOURCE_DIR}")
message("System is ${CMAKE_SYSTEM_NAME}")

set(DIR_BASE "${PROJECT_SOURCE_DIR}/..")
set(DIR_SRC "${PROJECT_SOURCE_DIR}/../src")
set(DIR_RES "${PROJECT_SOURCE_DIR}/../resources")
set(DIR_BUILD "${PROJECT_SOURCE_DIR}/../build")

#> Generate mhk_cmake_config.h ---------------------------------------
message("Generating configuration header: \"${DIR_BUILD}/mhk_cmake_config.h\"")
file (WRITE "${DIR_BUILD}/mhk_cmake_config.h"
  "// CMake generated config file. Editing it by hand is ultimately futile!\n"
  "// If you must seek out ${PROJECT_SOURCE_DIR}/CMakeLists.txt\n"
  "// \n"
  "// Markus-Hermann Koch, mhk@markuskoch.eu, April 30th, 2015\n\n"
  "#ifndef MHK_CMAKE_CONFIG_H\n"
  "#define MHK_CMAKE_CONFIG_H\n\n"
  "#define MHK_PROJECT_NAME \"${SENTINEL_NAME}\"\n"
  "#define MHK_VERSION_MAJOR ${SENTINEL_VERSION_MAJOR}\n"
  "#define MHK_VERSION_MINOR ${SENTINEL_VERSION_MINOR}\n"
  "#define MHK_ORGANIZATION \"Koch-Soft\"\n"
  "#define MHK_PATH_ROOT = \"${DIR_BASE}\"\n\n"
  "#define MHK_GL_MIN_HEIGHT 480\n"
  "#define MHK_GL_MIN_WIDTH 640\n"
  "#define MHK_GL_DEFAULT_HEIGHT 768\n"
  "#define MHK_GL_DEFAULT_WIDTH 1024\n"
  "\n#endif\n"
)
#< -------------------------------------------------------------------

# ui_ headers are Generated using uic FormMain.ui > ui_FormMain.h
# In addition all Q_OBJECTS I defined myself need to be here.
# Else the vtable error will occur at compile time.
set(qt_H
  "${DIR_BUILD}/ui_main.h"
  "${DIR_BUILD}/ui_dialog_setup_game.h"
  "${DIR_BUILD}/ui_dialog_about.h"
  "${DIR_SRC}/include/form_main.h"
  "${DIR_SRC}/include/form_game_setup.h"
  "${DIR_SRC}/include/form_about.h"
  "${DIR_SRC}/include/data_structures.h"
  "${DIR_SRC}/include/game.h"
  "${DIR_SRC}/include/widget_openGl.h"
  "${DIR_SRC}/include/io_qt.h"
)
# Generated using the trusty QtDesigner.
set(qt_UI
  "${DIR_SRC}/ui/main.ui"
  "${DIR_SRC}/ui/dialog_setup_game.ui"
  "${DIR_SRC}/ui/dialog_about.ui"
)
# My own hand-written XML describing the internal resources.
set(qt_QRC "${DIR_RES}/application.qrc")

# generate rules for building source files that moc generates
QT5_WRAP_CPP(qt_H_MOC ${qt_H})
# generate rules for building header files from the ui files
QT5_WRAP_UI(qt_UI_H ${qt_UI})
# Resource Handling. QRC: "Qt Resource Collection"
QT5_ADD_RESOURCES(qt_RCCS ${qt_QRC})
# btw.: rcc generates a C program from ./resources/application.qrc
# However, this is not needed. cmake sees to that. :-)
#< -------------------------------------------------------------------

include_directories("${DIR_SRC}/include" "${PROJECT_SOURCE_DIR}")

add_executable(sentinel "${DIR_SRC}/sentinel.cpp" ${qt_H_MOC} ${qt_UI_H} ${qt_RCCS})

# Available modules are listed here: http://doc.qt.io/qt-5/qtmodules.html
#   find /usr/lib/x86_64-linux-gnu/cmake -iname "*.cmake*" | less
# Note: http://stackoverflow.com/questions/20266235/cmake-error-qglwidget-no-such-file-or-directory
qt5_use_modules(sentinel Widgets Gui Core)

# Adding ${qt_UI_H} to this add_library-command is necessary for cmake
# to ensure that UIC (generating ui_main.h) is called before the
# qt library is buildt. A warm glow of gratitude for this info
# goes to Antwane on the Stackoverflow forum
# http://stackoverflow.com/questions/29968264/linking-and-uic-order-in-a-cmake-qt-project
add_library(qt
  "${DIR_SRC}/qt/form_main.cpp"
  "${DIR_SRC}/qt/form_game_setup.cpp"
  "${DIR_SRC}/qt/form_about.cpp"
  "${DIR_SRC}/qt/widget_openGl.cpp"
  "${DIR_SRC}/game/game.cpp"
  "${DIR_SRC}/game/landscape.cpp"
  "${DIR_SRC}/game/scanner.cpp"
  "${DIR_SRC}/qt/data_structures.cpp"
  "${DIR_SRC}/io/io.cpp"
  "${DIR_SRC}/io/io_qt.cpp"
  ${qt_H_MOC} ${qt_UI_H})

target_link_libraries(sentinel qt
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Multimedia_LIBRARIES}
)

