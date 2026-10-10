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

# McNopper/EGL - portable HDR-capable EGL 1.5 implementation.  We only need
# the EGL static library; its CMakeLists also adds an examples/ subdirectory
# which depends on test asset paths and would build 11+ extra executables, so
# those are switched off through its EGL_BUILD_EXAMPLES option.
#
# Prefer the sibling local checkout - the same multi-repo development layout
# the top-level CMakeLists uses for GLUS - and fall back to the published
# repository.  Using the sibling also means fixes land in this build
# immediately instead of after they have been published.
set(EGL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if(EXISTS "${CMAKE_SOURCE_DIR}/../EGL/CMakeLists.txt")
    message(STATUS "Example50: using sibling EGL checkout at ${CMAKE_SOURCE_DIR}/../EGL")
    FetchContent_Declare(mcnopper_egl SOURCE_DIR ${CMAKE_SOURCE_DIR}/../EGL)
else()
    # Published repository.  Two patches are needed for older revisions:
    #   * examples/ switched off (pre-EGL_BUILD_EXAMPLES revisions)
    #   * the platform sources include internal headers via
    #     ../../EGL/include/..., which only resolves when the checkout
    #     directory is literally named "EGL"
    # The single-argument FetchContent_Populate() previously used to apply
    # these is deprecated (CMP0169) and becomes an error with CMake >= 3.30,
    # so they run through PATCH_COMMAND instead.  Every replacement is
    # idempotent (its replacement text does not contain its own search text),
    # so re-running the patch on each configure is safe.
    set(MCEGL_PATCH_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/patch_mcegl.cmake)
    file(WRITE ${MCEGL_PATCH_SCRIPT}
    "set(_mcegl_root \"\${MCEGL_ROOT}\")\n"
    "file(READ \"\${_mcegl_root}/CMakeLists.txt\" _mcegl_c)\n"
    "string(REPLACE \"add_subdirectory(examples)\" \"# disabled by Example50: EGL examples need test asset paths\" _mcegl_c \"\${_mcegl_c}\")\n"
    "file(WRITE \"\${_mcegl_root}/CMakeLists.txt\" \"\${_mcegl_c}\")\n"
    "foreach(_mcegl_s egl_windows.cpp egl_windows_vk.cpp egl_windows_angle.cpp)\n"
    "  set(_mcegl_p \"\${_mcegl_root}/src/\${_mcegl_s}\")\n"
    "  if(EXISTS \"\${_mcegl_p}\")\n"
    "    file(READ \"\${_mcegl_p}\" _mcegl_c)\n"
    "    string(REPLACE \"../../EGL/include/EGL/\" \"../include/EGL/\" _mcegl_c \"\${_mcegl_c}\")\n"
    "    file(WRITE \"\${_mcegl_p}\" \"\${_mcegl_c}\")\n"
    "  endif()\n"
    "endforeach()\n")

    FetchContent_Declare(
        mcnopper_egl
        GIT_REPOSITORY https://github.com/McNopper/EGL.git
        GIT_TAG        v1.0.11
        PATCH_COMMAND ${CMAKE_COMMAND} -DMCEGL_ROOT=<SOURCE_DIR> -P ${MCEGL_PATCH_SCRIPT}
    )
endif()

FetchContent_MakeAvailable(mcnopper_egl)

# cgltf, stb_image and the GLUS_SHADER_DIR path are provided transitively by GLUS.

file(GLOB SOURCES "src/*.cpp" "src/*.c")
file(GLOB SHADERS "shader/*.glsl")
source_group("Shaders" FILES ${SHADERS})

add_executable(${PROJECT_NAME} ${SOURCES} ${SHADERS})

# EGL must come before opengl32 so its eglXxx symbols are picked up; the
# library publicly links Vulkan on Windows already.
target_link_libraries(${PROJECT_NAME} ${LIBRARIES_TO_LINK} GLUS EGL)
