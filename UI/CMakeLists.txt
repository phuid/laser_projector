cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(lasershow)
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# place binaries and libraries according to GNU standards

include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})


find_package(wiringPi)
find_package(cppzmq)

# Include headers
include_directories(${WIRINGPI_INCLUDE_DIRS})


add_executable(
    UI
    main.cpp
    encoder.cpp
    my_zmq_helper.cpp
    soft_i2c.c
    soft_lcd.c
)

target_compile_features(
    UI
    PRIVATE cxx_std_17
)

target_link_libraries(
    UI
    PRIVATE cppzmq ${CMAKE_THREAD_LIBS_INIT}
    wiringPi ${WIRINGPI_LIBRARIES}
)
