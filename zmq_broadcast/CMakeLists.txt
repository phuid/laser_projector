cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(lasershow)
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# place binaries and libraries according to GNU standards

include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})


find_package(cppzmq)

add_executable(
    s
    s.cpp
)

target_link_libraries(
    s
    PRIVATE cppzmq ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(
    c
    c.cpp
)

target_link_libraries(
    c
    PRIVATE cppzmq ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(
    noblock_c
    noblock_c.cpp
)

target_link_libraries(
    noblock_c
    PRIVATE cppzmq ${CMAKE_THREAD_LIBS_INIT}
)
