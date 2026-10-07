cmake_minimum_required(VERSION 3.14)
project (OpenGLExamples VERSION 3.2.3)

# Build policy + static analysis. Registered ONLY when this is the top-level
# project: GLUS is vendored through FetchContent and Example50 vendors EGL the
# same way, and a second `cppcheck` custom target would make CMake fail that
# build outright. None of this is required to build the examples - with the
# tools absent and every option at its default the build is unchanged.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
	include(cppcheck)
	include(warnings)
endif()

include(FetchContent)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Binaries)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

# Force single output directory (no Debug/Release subdirectories)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Binaries)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/Binaries)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib)
endforeach()

# Fetch GLFW from GitHub
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

# Fetch GLEW from GitHub
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(ONLY_LIBS ON CACHE BOOL "" FORCE)

# glew-cmake still declares cmake_minimum_required(VERSION 2.8.12), which modern
# CMake refuses. The single argument FetchContent_Populate() that used to patch
# it is deprecated (CMP0169) and becomes an error with CMake >= 3.30, so the
# patch is applied through PATCH_COMMAND instead. The replacement is idempotent,
# therefore it is safe to re-run on every configure.
set(GLEW_PATCH_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/patch_glew.cmake)
file(WRITE ${GLEW_PATCH_SCRIPT}
"file(READ \"\${GLEW_CMAKELISTS}\" GLEW_CMAKE_CONTENT)\n"
"string(REPLACE \"cmake_minimum_required(VERSION 2.8.12)\" \"cmake_minimum_required(VERSION 3.10)\" GLEW_CMAKE_CONTENT \"\${GLEW_CMAKE_CONTENT}\")\n"
"string(REPLACE \"find_library(AGL_LIBRARY AGL)\" \"# AGL is not part of modern macOS SDKs; GL symbols resolve through the OpenGL framework\" GLEW_CMAKE_CONTENT \"\${GLEW_CMAKE_CONTENT}\")\n"
"file(WRITE \"\${GLEW_CMAKELISTS}\" \"\${GLEW_CMAKE_CONTENT}\")\n")

FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    GIT_TAG glew-cmake-2.2.0
    PATCH_COMMAND ${CMAKE_COMMAND} -DGLEW_CMAKELISTS=<SOURCE_DIR>/CMakeLists.txt -P ${GLEW_PATCH_SCRIPT}
)

FetchContent_MakeAvailable(glew glfw)

# Set Variables for include and linking depending on platform
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(LIBRARIES_TO_LINK libglew_static glfw "-framework OpenGL")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(LIBRARIES_TO_LINK libglew_static glfw GL X11 pthread Xrandr Xi dl)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    SET(LIBRARIES_TO_LINK libglew_static glfw opengl32)
ENDIF()

include_directories(${glfw_SOURCE_DIR}/include ${glew_SOURCE_DIR}/include)

# GLUS: prefer the sibling local checkout (multi-repo development layout), and
# fall back to the published release when it is not present.
if(EXISTS "${CMAKE_SOURCE_DIR}/../GLUS/CMakeLists.txt")
    FetchContent_Declare(
        glus
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/../GLUS
    )
else()
    FetchContent_Declare(
        glus
        GIT_REPOSITORY https://github.com/McNopper/GLUS.git
        GIT_TAG        v1.1.3
    )
endif()
FetchContent_MakeAvailable(glus)

# GLUS exposes its shader directory via the GLUS_SHADER_DIR compile definition,
# so examples load the glTF / IBL shaders straight from the GLUS source tree.

file(GLOB EXAMPLES_LIST LIST_DIRECTORIES true RELATIVE ${CMAKE_SOURCE_DIR} "Example*")
foreach (EXAMPLE ${EXAMPLES_LIST})
	message(STATUS "Add Subdirectory: " ${EXAMPLE})
	add_subdirectory(${EXAMPLE})
endforeach()

# Warnings are applied per example target and only here - AFTER the dependency
# setup above - so GLFW, GLEW, GLUS and the EGL that Example50 vendors all
# keep building exactly as before, and every warning that appears is ours. At
# the default settings this loop adds no flags at all.
if(COMMAND project_enable_warnings)
	foreach(_ex IN LISTS EXAMPLES_LIST)
		get_filename_component(_ex_target ${_ex} NAME)
		if(TARGET ${_ex_target})
			project_enable_warnings(${_ex_target})
		endif()
	endforeach()
endif()