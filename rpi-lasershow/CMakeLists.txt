cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(lasershow)
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../FindWiringPi.cmake ${CMAKE_MODULE_PATH})
include_guard(GLOBAL)

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
    lasershow
    main.cpp
    lasershow.cpp
    ABE_ADCDACPi.cpp
    IldaReader.cpp
    my_zmq_helper.cpp
)
target_link_libraries(
    lasershow
    PRIVATE cppzmq ${CMAKE_THREAD_LIBS_INIT}
    wiringPi ${WIRINGPI_LIBRARIES}
)
