cmake_minimum_required(VERSION 3.16.0)
set(PROJECT_NAME scwx-qt6ct)

find_package(QT NAMES Qt6
             COMPONENTS Gui Widgets
             REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}
             COMPONENTS Gui Widgets
             REQUIRED)

#extract version from qt6ct.h
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/qt6ct/src/qt6ct-common/qt6ct.h"
     QT6CT_VERSION_DATA REGEX "^#define[ \t]+QT6CT_VERSION_[A-Z]+[ \t]+[0-9]+.*$")

if(QT6CT_VERSION_DATA)
  foreach(item IN ITEMS MAJOR MINOR)
    string(REGEX REPLACE ".*#define[ \t]+QT6CT_VERSION_${item}[ \t]+([0-9]+).*"
       "\\1" QT6CT_VERSION_${item} ${QT6CT_VERSION_DATA})
  endforeach()
  set(QT6CT_VERSION "${QT6CT_VERSION_MAJOR}.${QT6CT_VERSION_MINOR}")
  set(QT6CT_SOVERSION "${QT6CT_VERSION_MAJOR}")
  message(STATUS "qt6ct version: ${QT6CT_VERSION}")
else()
  message(FATAL_ERROR "invalid header")
endif()

set(qt6ct-common-source
  qt6ct/src/qt6ct-common/qt6ct.cpp
)

set(qt6ct-widgets-source
  qt6ct/src/qt6ct/paletteeditdialog.cpp
  qt6ct/src/qt6ct/paletteeditdialog.ui
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(qt6ct/src/qt6ct-common)

add_library(qt6ct-common STATIC ${qt6ct-common-source})
set_target_properties(qt6ct-common PROPERTIES VERSION ${QT6CT_VERSION})
target_link_libraries(qt6ct-common PRIVATE Qt6::Gui)
target_compile_definitions(qt6ct-common PRIVATE QT6CT_LIBRARY)

add_library(qt6ct-widgets STATIC ${qt6ct-widgets-source})
set_target_properties(qt6ct-widgets PROPERTIES VERSION ${QT6CT_VERSION})
target_link_libraries(qt6ct-widgets PRIVATE Qt6::Widgets Qt6::WidgetsPrivate qt6ct-common)
target_compile_definitions(qt6ct-widgets PRIVATE QT6CT_LIBRARY)

if (MSVC)
    # Produce PDB file for debug
    target_compile_options(qt6ct-common PRIVATE "$<$<CONFIG:Release>:/Zi>")
    target_compile_options(qt6ct-widgets PRIVATE "$<$<CONFIG:Release>:/Zi>")
else()
    target_compile_options(qt6ct-common PRIVATE "$<$<CONFIG:Release>:-g>")
    target_compile_options(qt6ct-widgets PRIVATE "$<$<CONFIG:Release>:-g>")
endif()

target_include_directories( qt6ct-common INTERFACE qt6ct/src )
target_include_directories( qt6ct-widgets INTERFACE qt6ct/src )
