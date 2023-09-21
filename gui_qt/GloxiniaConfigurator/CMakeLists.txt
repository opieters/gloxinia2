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
# load serial port and charts headers
find_package(Qt6 REQUIRED COMPONENTS SerialPort Charts)

set(TS_FILES GloxiniaConfigurator_en_GB.ts)

set(PROJECT_SOURCES
        main.cpp
        gmessage.h gmessage.cpp
        gloxiniaconfigurator.cpp gloxiniaconfigurator.h gloxiniaconfigurator.ui
        settingsdialog.h settingsdialog.cpp settingsdialog.ui
        measurementsettingsdialog.h measurementsettingsdialog.cpp measurementsettingsdialog.ui
        sensordialog.h sensordialog.cpp sensordialog.ui
        sensormeasurementdialog.h sensormeasurementdialog.cpp sensormeasurementdialog.ui
        sensorapds9306_065dialog.h sensorapds9306_065dialog.cpp sensorapds9306_065dialog.ui
        sensorsht35dialog.h sensorsht35dialog.cpp sensorsht35dialog.ui
        sensoradc12dialog.h sensoradc12dialog.cpp sensoradc12dialog.ui
        sensoradc16dialog.h sensoradc16dialog.cpp sensoradc16dialog.ui
        sensorliadialog.h sensorliadialog.cpp sensorliadialog.ui
        liaenginedialog.h liaenginedialog.cpp liaenginedialog.ui
        gliaengine.h gliaengine.cpp
        globalmeasurementpolicydialog.h globalmeasurementpolicydialog.cpp globalmeasurementpolicydialog.ui
        gcreadoutdialog.h gcreadoutdialog.cpp gcreadoutdialog.ui
        serialsettingsdialog.h serialsettingsdialog.cpp serialsettingsdialog.ui
        newprojectdialog.h newprojectdialog.cpp newprojectdialog.ui
        updatedialog.h updatedialog.cpp updatedialog.ui
        gdevicecommunication.h gdevicecommunication.cpp
        gcmessagehandling.cpp
        gcfileoperations.cpp
        gcserialcommunication.cpp
        nodediciodialog.h nodediciodialog.cpp nodediciodialog.ui
        #gcsystem.h gcsystem.cpp
        gcnode.h gcnode.cpp
        gcsensor.h gcsensor.cpp
        gcsamplepoint.h gcsamplepoint.cpp
        treeitem.h treeitem.cpp
        treemodel.h treemodel.cpp
        resources.qrc
        exampleConfig.txt
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
target_link_libraries(GloxiniaConfigurator PRIVATE Qt${QT_VERSION_MAJOR}::Charts)

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
