cmake_minimum_required (VERSION 3.6)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project (${PROJECT_NAME})

file(GLOB SOURCES "src/*.cpp" "src/*.c")
file(GLOB SHADERS "shader/*.glsl")
source_group("Shaders" FILES ${SHADERS})


add_executable(${PROJECT_NAME} ${SOURCES} ${SHADERS})

target_link_libraries(${PROJECT_NAME} ${LIBRARIES_TO_LINK} GLUS)