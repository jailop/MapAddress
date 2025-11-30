cmake_minimum_required(VERSION 3.16)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Test Core Widgets WebEngineWidgets Sql Network WebChannel Location Positioning)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(test_helpers STATIC
    test_database.cpp
    ${CMAKE_SOURCE_DIR}/src/database.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
)

target_link_libraries(test_helpers PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Test
)

add_executable(test_address test_address.cpp
    ${CMAKE_SOURCE_DIR}/src/address.cpp
)
target_link_libraries(test_address PRIVATE
    Qt6::Test
    Qt6::Core
)
add_test(NAME test_address COMMAND test_address)

add_executable(test_addresslist test_addresslist.cpp
    ${CMAKE_SOURCE_DIR}/src/addresslist.cpp
    ${CMAKE_SOURCE_DIR}/src/address.cpp
    ${CMAKE_SOURCE_DIR}/src/database.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
)
target_link_libraries(test_addresslist PRIVATE
    test_helpers
    Qt6::Test
    Qt6::Core
    Qt6::Sql
)
add_test(NAME test_addresslist COMMAND test_addresslist)
