cmake_minimum_required(VERSION 3.18)

project(urg3d_python LANGUAGES CXX)

find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_native MODULE src/urg3d/_native.cpp)

target_compile_features(_native PRIVATE cxx_std_11)
target_compile_definitions(_native PRIVATE URG3D_PYTHON_BINDING)
target_include_directories(_native PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_sources(_native PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Urg3dConnection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Urg3dRingBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Urg3dSensor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Urg3dTcpclient.cpp
)

if(WIN32)
    target_link_libraries(_native PRIVATE ws2_32)
endif() # WIN32

install(TARGETS _native DESTINATION urg3d)
