cmake_minimum_required(VERSION 3.18)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
project(kotki VERSION 0.0.3 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)

option(STATIC "Produce static binary" OFF)
option(SHARED "Produce shared binary" ON)
option(BUILD_DEMO "Build example demo application(s)" OFF)
option(COMPILE_PYTHON "Compile Python bindings" OFF)

if(STATIC)
    message(STATUS "${PROJECT_NAME} - STATIC BUILD ON")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

include(FindCcache)
find_package(PkgConfig REQUIRED)
find_package(rapidjson REQUIRED)

pkg_check_modules(MARIAN-LITE REQUIRED marian-lite)

add_subdirectory(src)

if(COMPILE_PYTHON)
    add_subdirectory(pybind11)
endif(COMPILE_PYTHON)

