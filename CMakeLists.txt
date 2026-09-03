# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Alessandro Henriques Teixeira — Studio Arn

cmake_minimum_required(VERSION 3.21)

project(ArnPlay VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Quick QuickControls2)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)

set(ARNPLAY_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/ArnPlay.icns")
if(APPLE)
    set(ARNPLAY_ICON "${CMAKE_CURRENT_BINARY_DIR}/ArnPlay.icns")
    execute_process(
        COMMAND /usr/bin/iconutil -c icns
                "${CMAKE_CURRENT_SOURCE_DIR}/assets/ArnPlay.iconset"
                -o "${ARNPLAY_ICON}"
        RESULT_VARIABLE ARNPLAY_ICON_RESULT
    )
    if(NOT ARNPLAY_ICON_RESULT EQUAL 0)
        message(FATAL_ERROR "Não foi possível gerar o ícone do ArnPlay")
    endif()
endif()

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(ArnPlay
    src/main.cpp
    src/MpvItem.cpp
    src/MpvItem.h
    ${ARNPLAY_ICON}
)

set_source_files_properties(${ARNPLAY_ICON} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)

qt_add_qml_module(ArnPlay
    URI StudioArn.ArnPlay
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/PlayerControls.qml
)

target_link_libraries(ArnPlay PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    PkgConfig::MPV
)

set_target_properties(ArnPlay PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE "ArnPlay.icns"
    MACOSX_BUNDLE_BUNDLE_NAME "ArnPlay"
    MACOSX_BUNDLE_GUI_IDENTIFIER "br.com.studioarn.arnplay"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "1"
)

install(TARGETS ArnPlay
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
