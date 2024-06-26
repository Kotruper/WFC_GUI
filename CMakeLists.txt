cmake_minimum_required(VERSION 3.5)

project(WFC_GUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

#find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)


set(TS_FILES WFC_GUI_en_GB.ts)

set(PROJECT_SOURCES
        images.qrc
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WFC_GUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        tile.cpp tile.h main.cpp view.cpp view.h wfc_gui.cpp wfc_gui.h wfc_gui.ui
        tilepatterncreator.h tilepatterncreator.cpp
        wavefunctioncollapser.h wavefunctioncollapser.cpp
        patternlibrary.h patternlibrary.cpp
    )

#    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
#        qt_add_executable(WFC_GUI
#            MANUAL_FINALIZATION
#            ${PROJECT_SOURCES}
#            tile.cpp tile.h fileprint.png main.cpp qt4logo.png rotateleft.png rotateright.png view.cpp view.h wfc_gui.cpp wfc_gui.h wfc_gui.ui WFC_GUI_en_GB.ts zoomin.png zoomout.png
#            tilepatterncreator.h tilepatterncreator.cpp
#            wfc.h wfc.cpp    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET WFC_GUI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_add_lupdate(WFC_GUI ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(WFC_GUI SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(WFC_GUI
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(WFC_GUI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
#target_link_libraries(WFC_GUI PRIVATE Qt6::OpenGLWidgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.WFC_GUI)
endif()
set_target_properties(WFC_GUI PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS WFC_GUI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WFC_GUI)
endif()
