cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(lasershow)
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# place binaries and libraries according to GNU standards

include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

find_package(cppzmq REQUIRED)
find_package(pigpio)

add_executable(
    bat_read
    main.cpp
)

target_link_libraries(
    bat_read
    PRIVATE cppzmq ${CMAKE_THREAD_LIBS_INIT}
    pigpio
)
