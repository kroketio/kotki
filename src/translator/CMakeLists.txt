# Generate version file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/project_version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/project_version.h @ONLY)

add_library(kotki-lib STATIC
    byte_array_util.cpp
    text_processor.cpp
    translation_model.cpp 
    request.cpp 
    batching_pool.cpp
    response_builder.cpp
    quality_estimator.cpp
    batch.cpp
    annotation.cpp
    parser.cpp
    response.cpp
    xh_scanner.cpp
    kotki.cpp
    nb_prefix.h
    cfgpath.h
)

if(ENABLE_CACHE_STATS)
    target_compile_definitions(kotki-lib PUBLIC ENABLE_CACHE_STATS)
endif(ENABLE_CACHE_STATS)

target_link_libraries(kotki-lib marian ssplit)

target_include_directories(kotki-lib
    PUBLIC ${PROJECT_SOURCE_DIR}
           ${PROJECT_SOURCE_DIR}/src
           ${RAPIDJSON_INCLUDE_DIRS})
