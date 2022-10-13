 cmake_minimum_required(VERSION 3.5)

project(GloxiniaConfigurator VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
# load serial port headers
find_package(Qt6 REQUIRED COMPONENTS SerialPort)

set(TS_FILES GloxiniaConfigurator_en_GB.ts)

set(PROJECT_SOURCES
        main.cpp
        serialcom.h serialcom.cpp
        gmessage.h gmessage.cpp
        gloxiniaconfigurator.cpp gloxiniaconfigurator.h gloxiniaconfigurator.ui
        settingsdialog.h settingsdialog.cpp settingsdialog.ui
        nodedialog.ui
        sensordialog.h sensordialog.cpp sensordialog.ui
        configuresht35dialog.h configuresht35dialog.cpp configuresht35dialog.ui
        gcsystem.h gcsystem.cpp
        gcnode.h gcnode.cpp
        gcsensor.h gcsensor.cpp
        treeitem.h treeitem.cpp
        treemodel.h treemodel.cpp
        moduledialog.h moduledialog.cpp moduledialog.ui
        resources.qrc
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(GloxiniaConfigurator
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET GloxiniaConfigurator APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(GloxiniaConfigurator SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(GloxiniaConfigurator
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(GloxiniaConfigurator PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(GloxiniaConfigurator PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)

# resources

set_target_properties(GloxiniaConfigurator PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS GloxiniaConfigurator
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(GloxiniaConfigurator)
endif()
