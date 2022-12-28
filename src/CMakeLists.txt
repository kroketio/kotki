# Generate version file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/translator/project_version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/translator/project_version.h @ONLY)

file(GLOB_RECURSE SRCS translator/*.cpp)
file(GLOB_RECURSE HDRS translator/*.h)
add_library(kotki-lib SHARED ${SRCS} ${HDRS})

target_link_libraries(kotki-lib PUBLIC
        ${MARIAN-LITE_LIBRARY}
        yaml-cpp
        )
target_include_directories(kotki-lib PUBLIC
        ${RAPIDJSON_INCLUDE_DIRS}
        ${MARIAN-LITE_DIRECTORY}
        ${MARIAN-LITE_DIRECTORY}/marian-lite/
        ${MARIAN-LITE_DIRECTORY}/marian-lite/3rd_party/
        ${MARIAN-LITE_DIRECTORY}/marian-lite/3rd_party/proto/
        )

set_target_properties(kotki-lib PROPERTIES
        CXX_STANDARD 17
        VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}"
        SOVERSION "${CMAKE_PROJECT_VERSION_MAJOR}"
        )

install(TARGETS kotki-lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY "${CMAKE_SOURCE_DIR}/kotki"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kotki"
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


message(STATUS "=========================================== kotki-lib: ${CMAKE_PROJECT_VERSION}")
message(STATUS "SHARED: ${SHARED} | STATIC: ${STATIC}")
message(STATUS "marian-lite: ${MARIAN-LITE_LIBRARY}")
message(STATUS "===========================================")
