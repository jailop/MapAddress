cmake_minimum_required(VERSION 3.16)

project(MapAddress VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

if(WIN32)
    set(CMAKE_WIN32_EXECUTABLE ON)
endif()
if(APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME "MapAddress")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "dev.datainquiry.mapaddress")
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING
    ${PROJECT_VERSION})
    # set(MACOSX_BUNDLE_ICON_FILE "mapaddress.icns")
endif()
if (UNIX AND NOT APPLE)
    add_compile_options(-Wall -g)
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    WebEngineWidgets
    WebChannel
    Location
    Positioning
    Sql
    Test
)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/addressdialog.cpp
    src/settingsdialog.cpp
    src/address.cpp
    src/addresslist.cpp
    src/database.cpp
    src/googlemapsprovider.cpp
    src/openstreetmapprovider.cpp
    src/geocodingservice.cpp
    src/mapwidget.cpp
    src/logger.cpp
    src/routingservice.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(HEADERS
    include/mainwindow.h
    include/addressdialog.h
    include/settingsdialog.h
    include/address.h
    include/addresslist.h
    include/database.h
    include/mapprovider.h
    include/googlemapsprovider.h
    include/openstreetmapprovider.h
    include/geocodingservice.h
    include/mapwidget.h
    include/logger.h
    include/routingservice.h
)

set(UIS
    ui/mainwindow.ui
    ui/addressdialog.ui
    ui/settingsdialog.ui
)

set(RESOURCES
    resources/resources.qrc
)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32
        ${SOURCES}
        ${HEADERS}
        ${UIS}
        ${RESOURCES}
    )
elseif(APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE
        ${SOURCES}
        ${HEADERS}
        ${UIS}
        ${RESOURCES}
    )
else()
    add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS}
        ${UIS}
        ${RESOURCES}
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebEngineWidgets
    Qt6::WebChannel
    Qt6::Location
    Qt6::Positioning
    Qt6::Sql
)

# target_include_directories(${PROJECT_NAME} PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/src
# )

if(WIN32)
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION
    )
    # deploy QT dependencies
    if (Qt6_DIR)
        get_target_property(QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
        if (WINDEPLOYQT_EXECUTABLE)
            install(CODE "execute_process(COMMAND \"${WINDEPLOYQT_EXECUTABLE}\" \"\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.exe\")")
        endif()
    endif()
elseif(APPLE)
    install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
    )
    # deploy QT dependencies
    if (Qt6_DIR)
        get_target_property(QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
        find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")
        if (MACDEPLOYQT_EXECUTABLE)
            install(CODE "execute_process(COMMAND \"${MACDEPLOYQT_EXECUTABLE}\" \"\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app\")")
        endif()
    endif()
else()
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
    )
endif()

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# CPack Configuration for installers
set(CPACK_PACKAGE_NAME "MapAddress")
set(CPACK_PACKAGE_VENDOR "MapAddress")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Markdown editor with wiki-style linking")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MapAddress")

# Set license file if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
else()
    message(WARNING "LICENSE file not found - installer will not include license")
endif()

# Windows NSIS installer
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "MapAddress")
    set(CPACK_NSIS_PACKAGE_NAME "MapAddress")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/mapaddress.ico")
        set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/mapaddress.ico")
        set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/resources/mapaddress.ico")
    endif()
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\mapaddress.exe")
    set(CPACK_NSIS_MENU_LINKS
        "bin/mapaddress.exe" "MapAddress"
    )
endif()

# Linux DEB package
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MapAddress Developers")
    set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6, libqt6gui6, libqt6widgets6, libqt6webenginewidgets6, libmd4c-html0")
    
    # Install desktop file if it exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/mapaddress.desktop")
        install(FILES resources/mapaddress.desktop
            DESTINATION share/applications)
    endif()
    
    # Install icon if it exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/mapaddress.png")
        install(FILES resources/mapaddress.png
            DESTINATION share/icons/hicolor/256x256/apps)
    endif()
endif()

include(CPack)

