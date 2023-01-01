# Generate version file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kotki/project_version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/kotki/project_version.h @ONLY)

file(GLOB_RECURSE SRCS kotki/*.cpp)
file(GLOB_RECURSE HDRS kotki/*.h)
add_library(kotki-lib SHARED ${SRCS} ${HDRS})

if(VENDORED_LIBS)
    target_link_libraries(kotki-lib PUBLIC marian-lite-SHARED)
    target_include_directories(kotki-lib PUBLIC
            ${marian-lite_BINARY_DIR}/src/include
            ${marian-lite_BINARY_DIR}/src/include/marian-lite
            )
else()
    target_link_libraries(kotki-lib PUBLIC marian-lite::marian-lite)
endif()


target_link_libraries(kotki-lib PUBLIC
        yaml-cpp
        )

target_include_directories(kotki-lib PUBLIC
        ${RAPIDJSON_INCLUDE_DIRS}
        )

set_target_properties(kotki-lib PROPERTIES
        OUTPUT_NAME kotki
        CXX_STANDARD 17
        VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}"
        SOVERSION "${CMAKE_PROJECT_VERSION_MAJOR}"
        )

install(TARGETS kotki-lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/kotki"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.inl"
        )

# some demo apps
if(BUILD_DEMO)
    add_executable(kotki-cli demo/kotki.cpp)
    target_link_libraries(kotki-cli PRIVATE kotki-lib)
    target_include_directories(kotki-cli PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}
            )

    add_executable(kotki-srt demo/kotki-srt.cpp)
    target_link_libraries(kotki-srt PRIVATE kotki-lib)
    target_include_directories(kotki-srt PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}
            )
endif()

configure_file("${PROJECT_SOURCE_DIR}/kotki.pc.in" "kotki.pc" @ONLY)
if (NOT MSVC)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kotki.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

message(STATUS "=========================================== kotki-lib")
message(STATUS "SHARED: ${SHARED} | STATIC: ${STATIC} | VENDORED: ${VENDORED_LIBS}")
message(STATUS "Build demo application(s): ${BUILD_DEMO}")
if(NOT VENDORED_LIBS)
message(STATUS "marian-lite: ${MARIAN-LITE_LIBRARIES}")
endif()

