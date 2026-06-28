cmake_minimum_required(VERSION 3.14)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

include(FetchContent)

# cgltf v1.14 - single-header glTF 2.0 parser
FetchContent_Declare(
    cgltf
    GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
    GIT_TAG        v1.14
)
FetchContent_MakeAvailable(cgltf)

file(GLOB SOURCES "src/*.c")
file(GLOB SHADERS "shader/*.glsl")
source_group("Shaders" FILES ${SHADERS})

add_executable(${PROJECT_NAME} ${SOURCES} ${SHADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${cgltf_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} ${LIBRARIES_TO_LINK} GLUS)
