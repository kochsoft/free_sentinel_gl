#!/bin/bash

echo "Attempting to clean house!";

if [ -e CMakeFiles ]; then rm -rf CMakeFiles; fi;
if [ -e __ ]; then rm -rf __; fi;
if [ -e CMakeCache.txt ]; then rm CMakeCache.txt; fi;
if [ -e cmake_install.cmake ]; then rm cmake_install.cmake; fi;
if [ -e liboptimization.a ]; then rm liboptimization.a; fi;
if [ -e Makefile ]; then rm Makefile; fi;
if [ -e optimization_automoc.cpp ]; then rm optimization_automoc.cpp; fi;
if [ -e sentinel ]; then rm sentinel; fi;
if [ -e sentinel_automoc.cpp ]; then rm sentinel_automoc.cpp; fi;
if [ -e mhk_cmake_config.h ]; then rm mhk_cmake_config.h; fi;
if [ -e moc_ui_main.cpp_parameters ]; then rm moc_ui_main.cpp_parameters; fi;
if [ -e moc_ui_main.cpp ]; then rm moc_ui_main.cpp; fi;
if [ -e qrc_application.cpp ]; then rm qrc_application.cpp; fi;
if [ -e ui_main.h ]; then rm ui_main.h; fi;
if [ -e libqt.a ]; then rm libqt.a; fi;
if [ -e qt_automoc.cpp ]; then rm qt_automoc.cpp; fi;
if [ -e moc_ui_dialog_setup_game.cpp ]; then rm moc_ui_dialog_setup_game.cpp; fi;
if [ -e moc_ui_dialog_setup_game.cpp_parameters ]; then rm moc_ui_dialog_setup_game.cpp_parameters; fi;
if [ -e ui_dialog_setup_game.h ]; then rm ui_dialog_setup_game.h; fi;
if [ -e core ]; then rm core; fi;
if [ -e gl_automoc.cpp ]; then rm gl_automoc.cpp; fi;
if [ -e io_automoc.cpp ]; then rm io_automoc.cpp; fi;
if [ -e moc_ui_dialog_about.cpp ]; then rm moc_ui_dialog_about.cpp; fi;
if [ -e moc_ui_dialog_about.cpp_parameters ]; then rm moc_ui_dialog_about.cpp_parameters; fi;
if [ -e ui_dialog_about.h ]; then rm ui_dialog_about.h; fi;

