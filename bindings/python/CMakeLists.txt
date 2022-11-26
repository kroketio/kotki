find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)

message("Using Python: " ${Python_EXECUTABLE})

# pybind11 method:
pybind11_add_module(kotki SHARED kotki-py.cpp)
target_link_libraries(kotki PUBLIC pybind11::module pybind11::headers kotki-lib)
target_include_directories(kotki PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src)