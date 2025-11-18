cmake_minimum_required(VERSION 3.16)

project(MapAddress VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/addressdialog.cpp
    src/addressdialog.h
    src/addressdialog.ui
    src/settingsdialog.cpp
    src/settingsdialog.h
    src/settingsdialog.ui
    src/address.cpp
    src/address.h
    src/addresslist.cpp
    src/addresslist.h
    src/database.cpp
    src/database.h
    src/mapprovider.h
    src/googlemapsprovider.cpp
    src/googlemapsprovider.h
    src/openstreetmapprovider.cpp
    src/openstreetmapprovider.h
    src/geocodingservice.cpp
    src/geocodingservice.h
    src/mapwidget.cpp
    src/mapwidget.h
    src/logger.cpp
    src/logger.h
    src/routingservice.cpp
    src/routingservice.h
    resources/resources.qrc
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebEngineWidgets
    Qt6::WebChannel
    Qt6::Location
    Qt6::Positioning
    Qt6::Sql
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Tests
enable_testing()

# Test Address
add_executable(test_address
    tests/test_address.cpp
    src/address.cpp
    src/address.h
)
target_link_libraries(test_address Qt6::Core Qt6::Test)
target_include_directories(test_address PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME test_address COMMAND test_address)

# Test AddressList
add_executable(test_addresslist
    tests/test_addresslist.cpp
    src/addresslist.cpp
    src/addresslist.h
    src/address.cpp
    src/address.h
)
target_link_libraries(test_addresslist Qt6::Core Qt6::Test)
target_include_directories(test_addresslist PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME test_addresslist COMMAND test_addresslist)

# Test Database
add_executable(test_database
    tests/test_database.cpp
    src/database.cpp
    src/database.h
    src/address.cpp
    src/address.h
    src/addresslist.cpp
    src/addresslist.h
    src/logger.cpp
    src/logger.h
)
target_link_libraries(test_database Qt6::Core Qt6::Test Qt6::Sql)
target_include_directories(test_database PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME test_database COMMAND test_database)

# CPack Configuration for installers
set(CPACK_PACKAGE_NAME "MapAddress")
set(CPACK_PACKAGE_VENDOR "DataInquiry")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Map Address - Manage and visualize addresses on interactive maps")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MapAddress")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Windows NSIS installer
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "MapAddress")
    set(CPACK_NSIS_PACKAGE_NAME "MapAddress")
    set(CPACK_NSIS_HELP_LINK "https://github.com/datainquiry/mapaddress")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/datainquiry/mapaddress")
    set(CPACK_NSIS_CONTACT "support@datainquiry.com")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app-icon.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app-icon.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MapAddress.exe")
    set(CPACK_NSIS_MENU_LINKS "bin/MapAddress.exe" "MapAddress")
endif()

# Linux DEB/RPM packages
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "DataInquiry")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6, libqt6gui6, libqt6widgets6, libqt6network6, libqt6webenginecore6, libqt6webenginewidgets6")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Productivity")
endif()

# macOS DMG
if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
endif()

include(CPack)

# Android-specific configuration
if(ANDROID)
    message(STATUS "Configuring for Android build")
    
    set_target_properties(${PROJECT_NAME} PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/installer/android"
        QT_ANDROID_MIN_SDK_VERSION 28
        QT_ANDROID_TARGET_SDK_VERSION 33
        QT_ANDROID_VERSION_CODE 1
        QT_ANDROID_VERSION_NAME "1.0.0"
    )
    
    # Set Android package details
    set(ANDROID_PACKAGE_NAME "com.datainquiry.mapaddress")
    set(ANDROID_APP_NAME "MapAddress")
    
    # Strip debug symbols to reduce APK size
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
    
    message(STATUS "Android package: ${ANDROID_PACKAGE_NAME}")
    message(STATUS "Android min SDK: 28, target SDK: 33")
endif()
