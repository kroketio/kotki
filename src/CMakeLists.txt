# Generate version file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kotki/project_version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/kotki/project_version.h @ONLY)

file(GLOB_RECURSE SRCS kotki/*.cpp)
file(GLOB_RECURSE HDRS kotki/*.h)

foreach(_BUILD_TYPE STATIC SHARED)
    if(NOT "${${_BUILD_TYPE}}")
        continue()
    endif()

    set(_TARGET "kotki-lib-${_BUILD_TYPE}")

    # establish marian-lite target name (based on SHARED/STATIC)
    if(STATIC)
        set(_MARIAN_TARGET "marian-lite-STATIC")
        add_library(${_TARGET} STATIC ${SRCS} ${HDRS})
    else()
        set(_MARIAN_TARGET "marian-lite-SHARED")
        add_library(${_TARGET} SHARED ${SRCS} ${HDRS})
    endif()

    message(STATUS "${_TARGET} linking against ${_MARIAN_TARGET}")

    if(VENDORED_LIBS)
        target_link_libraries(${_TARGET} PUBLIC ${_MARIAN_TARGET})
        target_include_directories(${_TARGET} PRIVATE
                ${marian-lite_BINARY_DIR}/src/include
                ${marian-lite_BINARY_DIR}/src/include/marian-lite)
    else()
        set(_MARIAN_TARGET "marian-lite::${_MARIAN_TARGET}")
        target_link_libraries(${_TARGET} PUBLIC
                "-Wl,--whole-archive"
                ${_MARIAN_TARGET}
                "-Wl,--no-whole-archive"
                )
    endif()

    target_link_libraries(${_TARGET} PUBLIC
            ZLIB::ZLIB
            ${YAMLCPP_LIBRARY}
            )

    if(ARM_FOUND)
        target_link_libraries(${_TARGET} PUBLIC
                ruy::ruy
                ruy::ruy_platform)
    endif()

    target_include_directories(${_TARGET} PUBLIC
            ${RAPIDJSON_INCLUDE_DIRS}
            ${YAMLCPP_INCLUDE_DIR}
            )

    set_target_properties(${_TARGET} PROPERTIES
            OUTPUT_NAME kotki
            CXX_STANDARD 17
            VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}"
            SOVERSION "${CMAKE_PROJECT_VERSION_MAJOR}"
            )

    target_include_directories(${_TARGET}
            PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            )

    list(APPEND _TARGETS "${_TARGET}")
endforeach()


install(TARGETS ${_TARGETS}
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

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/kotki/modules/"
        FILES_MATCHING
        PATTERN "*.cmake"
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
    target_link_libraries(kotki-cli PRIVATE kotki-lib-STATIC)
    target_include_directories(kotki-cli PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}
            )

    add_executable(kotki-srt demo/kotki-srt.cpp)
    target_link_libraries(kotki-srt PRIVATE kotki-lib-STATIC)
    target_include_directories(kotki-srt PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}
            )
endif()

message(STATUS "=========================================== ${_TARGET}")
message(STATUS "SHARED: ${SHARED} | STATIC: ${STATIC} | VENDORED: ${VENDORED_LIBS}")
message(STATUS "yaml-cpp: ${YAMLCPP_LIBRARY}")
message(STATUS "Build demo application(s): ${BUILD_DEMO}")
if(NOT VENDORED_LIBS)
message(STATUS "marian-lite: ${MARIAN-LITE_LIBRARIES}")
endif()

