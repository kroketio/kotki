message(STATUS "Generating Python bindings")
find_package(Python COMPONENTS Interpreter Development REQUIRED)
message(STATUS "Using Python: " ${Python_EXECUTABLE})

find_package(pybind11 CONFIG)

# pybind11 method:
pybind11_add_module(kotki SHARED kotki-py.cpp)
target_link_libraries(kotki PUBLIC pybind11::module kotki-lib)
target_include_directories(kotki PRIVATE ${pybind11_INCLUDE_DIR})
target_include_directories(kotki PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src
        ${CMAKE_BINARY_DIR}/src)