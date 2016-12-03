cmake_minimum_required (VERSION 3.6)
project (Example01)

file(GLOB SOURCES "src/*.cpp" "src/*.c")
add_executable(Example01 ${SOURCES})