cmake_minimum_required(VERSION 3.16)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
project(kotki
        VERSION 0.6.1
        DESCRIPTION "Library for language translation without using the cloud."
        HOMEPAGE_URL https://github.com/kroketio/kotki
        LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)

include(GNUInstallDirs)  # initializes ${CMAKE_INSTALL_*DIR} to sane defaults
include(CMakePackageConfigHelpers)

option(STATIC "Produce static binary" OFF)
option(SHARED "Produce shared binary" ON)
option(BUILD_DEMO "Build example demo application(s)" OFF)
option(COMPILE_PYTHON "Compile Python bindings" OFF)
option(VENDORED_LIBS "Download dependencies during CMake configure time. Not recommended, off by default. Used during 'pip install kotki -v'" OFF)

if(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm*")
    # using ARM
    #find_package(ARM)
    #add_compile_options("${ARM_COMPILE_OPTIONS}")
    #add_compile_options(-mfpu=crypto-neon-fp-armv8)
    include(cmake/armFlags.cmake)

    add_compile_definitions(ARM FMA SSE) #added for ARM
    add_compile_definitions(USE_RUY_SGEMM=1 RUY_PLATFORM_NEON=1)
endif()

if(STATIC)
    message(STATUS "${PROJECT_NAME} - STATIC BUILD ON")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

include(FindCcache)
find_package(PkgConfig REQUIRED)
find_package(rapidjson REQUIRED)

if(VENDORED_LIBS)
    include(cmake/DownloadAllTheThings.cmake)
else()
    find_package(marian-lite REQUIRED)
endif()

add_subdirectory(src)

if(COMPILE_PYTHON)
    add_subdirectory(python/kotki)
endif(COMPILE_PYTHON)
