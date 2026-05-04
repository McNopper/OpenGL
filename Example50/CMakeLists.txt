cmake_minimum_required(VERSION 3.14)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

include(FetchContent)

# McNopper/EGL requires Vulkan on Windows for HDR signaling.  If the SDK
# is not installed we silently skip Example50 instead of failing the whole
# OpenGL examples build.
find_package(Vulkan QUIET)
if(NOT Vulkan_FOUND)
    message(STATUS "Example50: Vulkan SDK not found - skipping (McNopper/EGL needs it for HDR).")
    return()
endif()

# cgltf v1.14 - single-header glTF 2.0 parser
FetchContent_Declare(
    cgltf
    GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
    GIT_TAG        v1.14
)
FetchContent_MakeAvailable(cgltf)

# stb - for stb_image.h
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

# McNopper/EGL - portable HDR-capable EGL 1.5 implementation.
# We only need the EGL static library; their CMakeLists also adds an
# examples/ subdirectory which depends on test asset paths and would build
# 11+ extra executables.  Patch it out before MakeAvailable.
FetchContent_Declare(
    mcnopper_egl
    GIT_REPOSITORY https://github.com/McNopper/EGL.git
    GIT_TAG        master
)

FetchContent_GetProperties(mcnopper_egl)
if(NOT mcnopper_egl_POPULATED)
    FetchContent_Populate(mcnopper_egl)
    file(READ "${mcnopper_egl_SOURCE_DIR}/CMakeLists.txt" MCEGL_CMAKE)
    string(REPLACE "add_subdirectory(examples)" "# add_subdirectory(examples)  # disabled by Example50"
        MCEGL_CMAKE "${MCEGL_CMAKE}")
    file(WRITE "${mcnopper_egl_SOURCE_DIR}/CMakeLists.txt" "${MCEGL_CMAKE}")

    # The platform sources reference internal headers via ../../EGL/include/...
    # which only resolves when the repo directory is literally named "EGL".
    # Patch them to use a path relative to the source file instead.
    foreach(MCEGL_SRC egl_windows.cpp egl_windows_vk.cpp egl_windows_angle.cpp)
        set(MCEGL_PATH "${mcnopper_egl_SOURCE_DIR}/src/${MCEGL_SRC}")
        if(EXISTS "${MCEGL_PATH}")
            file(READ "${MCEGL_PATH}" MCEGL_CONTENT)
            string(REPLACE "../../EGL/include/EGL/" "../include/EGL/"
                MCEGL_CONTENT "${MCEGL_CONTENT}")
            file(WRITE "${MCEGL_PATH}" "${MCEGL_CONTENT}")
        endif()
    endforeach()

    add_subdirectory(${mcnopper_egl_SOURCE_DIR} ${mcnopper_egl_BINARY_DIR})
endif()

file(GLOB SOURCES "src/*.cpp" "src/*.c")
file(GLOB SHADERS "shader/*.glsl")
source_group("Shaders" FILES ${SHADERS})

add_executable(${PROJECT_NAME} ${SOURCES} ${SHADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${cgltf_SOURCE_DIR}
    ${stb_SOURCE_DIR}
)

# EGL must come before opengl32 so its eglXxx symbols are picked up; the
# library publicly links Vulkan on Windows already.
target_link_libraries(${PROJECT_NAME} ${LIBRARIES_TO_LINK} GLUS EGL)
