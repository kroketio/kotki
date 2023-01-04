# Generate version file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kotki/project_version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/kotki/project_version.h @ONLY)

file(GLOB_RECURSE SRCS kotki/*.cpp)
file(GLOB_RECURSE HDRS kotki/*.h)
add_library(kotki-lib SHARED ${SRCS} ${HDRS})

# establish marian-lite target name (based on SHARED/STATIC)
if(STATIC)
    set(_MARIAN_TARGET "marian-lite-STATIC")
else()
    set(_MARIAN_TARGET "marian-lite-SHARED")
endif()
if(NOT VENDORED_LIBS)
    set(_MARIAN_TARGET "marian-lite::${_MARIAN_TARGET}")
endif()

if(VENDORED_LIBS)
    message(STATUS "linking against ${_MARIAN_TARGET}")
    target_link_libraries(kotki-lib PUBLIC ${_MARIAN_TARGET})
    target_include_directories(kotki-lib PRIVATE
            ${marian-lite_BINARY_DIR}/src/include
            ${marian-lite_BINARY_DIR}/src/include/marian-lite
            )
else()
    message(STATUS "linking against ${_MARIAN_TARGET}")
    target_link_libraries(kotki-lib PUBLIC ${_MARIAN_TARGET})
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

target_include_directories(kotki-lib
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

install(TARGETS kotki-lib
        EXPORT kotki-libTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/kotki/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/kotki"
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.inl"
        )

install(EXPORT kotki-libTargets
        FILE kotki-libTargets.cmake
        NAMESPACE kotki::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kotki
        )

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/../Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/kotkiConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kotki
        )

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/kotkiConfigVersion.cmake"
        VERSION "${CMAKE_PROJECT_VERSION}"
        COMPATIBILITY AnyNewerVersion
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/kotkiConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/kotkiConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kotki
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

message(STATUS "=========================================== kotki-lib")
message(STATUS "SHARED: ${SHARED} | STATIC: ${STATIC} | VENDORED: ${VENDORED_LIBS}")
message(STATUS "Build demo application(s): ${BUILD_DEMO}")
if(NOT VENDORED_LIBS)
message(STATUS "marian-lite: ${MARIAN-LITE_LIBRARIES}")
endif()

