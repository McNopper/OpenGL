cmake_minimum_required(VERSION 3.6)
project (OpenGLExamples)
 
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

# Set Variables for include and linking depending on platform

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# Mac OS X
	
	# Support only desktop
	
    SET(Processor "x64")
    SET(OperatingSystem "Darwin")
    SET(Compiler "GCC")	

ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	# Linux

	IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
		# Raspberry Pi and i.MX6
		
		IF(${SoC} MATCHES "iMX6")
			# i.MX6

			# Everything is in /usr/include

		ELSE()
			# Raspberry Pi
		ENDIF()
		
	ELSE()
		# Support only desktop
	
		SET(Processor "x64")
		SET(OperatingSystem "Linux")
		SET(Compiler "GCC")
				
	ENDIF()
	
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Windows
	
	SET(Processor "x86")
	SET(OperatingSystem "Windows")
	SET(Compiler "MSVC")
	set(LIBRARIES_TO_LINK OpenGL32.lib glew32s.lib glfw3.lib)
		
ENDIF()

# set GLFW valiables
# set(BUILD_SHARED_LIBS "Build shared libraries" OFF)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")

# set GLUS variables
set(ENV_DIR ${Processor}/${OperatingSystem}/${Compiler})
set(EXTERNAL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/External/${ENV_DIR}/include)
set(EXTERNAL_LIB_DIR ${CMAKE_SOURCE_DIR}/External/${ENV_DIR}/lib)

set(GLFW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/GLFW/include)

include_directories(${GLFW_INCLUDE_DIR} ${EXTERNAL_INCLUDE_DIR})
link_directories(${EXTERNAL_LIB_DIR} ${LIBRARY_OUTPUT_PATH})

add_subdirectory(GLUS)
add_subdirectory(GLFW)

file(GLOB EXAMPLES_LIST LIST_DIRECTORIES true RELATIVE ${CMAKE_SOURCE_DIR} "Example*")
foreach (EXAMPLE ${EXAMPLES_LIST})
	message(STATUS "Add Subdirectory: " ${EXAMPLE})
	add_subdirectory(${EXAMPLE})
endforeach()