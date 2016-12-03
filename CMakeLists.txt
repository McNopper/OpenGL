cmake_minimum_required(VERSION 3.6)
project (OpenGLExamples)
 
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
 
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
 
add_subdirectory(GLUS)
add_subdirectory(Example01)
