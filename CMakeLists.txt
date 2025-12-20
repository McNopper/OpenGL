cmake_minimum_required(VERSION 3.14)
project (OpenGLExamples)

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

FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    GIT_TAG glew-cmake-2.2.0
)

FetchContent_GetProperties(glew)
if(NOT glew_POPULATED)
    FetchContent_Populate(glew)
    # Patch the CMakeLists.txt to use a modern CMake version
    file(READ "${glew_SOURCE_DIR}/CMakeLists.txt" GLEW_CMAKE_CONTENT)
    string(REPLACE "cmake_minimum_required(VERSION 2.8.12)" "cmake_minimum_required(VERSION 3.5)" GLEW_CMAKE_CONTENT "${GLEW_CMAKE_CONTENT}")
    file(WRITE "${glew_SOURCE_DIR}/CMakeLists.txt" "${GLEW_CMAKE_CONTENT}")
    add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR})
endif()

FetchContent_MakeAvailable(glfw)

# Set Variables for include and linking depending on platform
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(LIBRARIES_TO_LINK libglew_static glfw "-framework OpenGL")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(LIBRARIES_TO_LINK libglew_static glfw GL X11 pthread Xrandr Xi dl)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    SET(LIBRARIES_TO_LINK libglew_static glfw opengl32)
ENDIF()

include_directories(${glfw_SOURCE_DIR}/include ${glew_SOURCE_DIR}/include)

add_subdirectory(GLUS)

file(GLOB EXAMPLES_LIST LIST_DIRECTORIES true RELATIVE ${CMAKE_SOURCE_DIR} "Example*")
foreach (EXAMPLE ${EXAMPLES_LIST})
	message(STATUS "Add Subdirectory: " ${EXAMPLE})
	add_subdirectory(${EXAMPLE})
endforeach()