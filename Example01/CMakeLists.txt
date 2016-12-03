cmake_minimum_required (VERSION 3.6)
project (Example01)

set (PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set (PROJECT_SOURCE_DIR ${PROJECT_INCLUDE_DIR})

file(GLOB SOURCES "src/*.cpp" "src/*.c")

add_executable(Example01 ${SOURCES})

target_link_libraries (Example01 LINK_PUBLIC GLUS)