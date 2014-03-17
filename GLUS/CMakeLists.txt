#
# GLUS CMake file
#
# Note: Set OpenGL=ES or OpenGL=ES2 for Windows OpenGL ES 3.0 or 2.0 simulation.
#       Raspberry Pi is default OpenGL ES 2.0.
#
# (c) Norbert Nopper
# 

cmake_minimum_required(VERSION 2.6)

project(GLUS)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# Mac OS X

	# Support only desktop
	
    SET(Processor "x64")
    SET(OperatingSystem "Darwin")
    SET(Compiler "GCC")

	set(ENV_DIR ${Processor}/${OperatingSystem}/${Compiler})
	
	include_directories(${GLUS_SOURCE_DIR}/../External/${ENV_DIR}/include)		

ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	# Linux

	IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
		# Raspberry Pi
		# ToDo Add i.MX6
		
		SET(OpenGL "ES2")
		
		add_definitions(-DGLUS_ES2)
		
		include_directories(/opt/vc/include /opt/vc/include/interface/vcos/pthreads /opt/vc/include/interface/vmcs_host/linux)		

	ELSE()
		# Support only desktop
	
		SET(Processor "x64")
		SET(OperatingSystem "Linux")
		SET(Compiler "GCC")
		
		set(ENV_DIR ${Processor}/${OperatingSystem}/${Compiler})

		# External directories
		include_directories(${GLUS_SOURCE_DIR}/../External/${ENV_DIR}/include)		
		
	ENDIF()
	
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Windows
	
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	
	IF(${OpenGL} MATCHES "ES2")
		# Desktop OpenGL ES 2.0

		add_definitions(-DGLUS_ES2)
	
		include_directories(C:\\Imagination\\PowerVR\\GraphicsSDK\\SDK_3.2\\Builds\\Include)	

	ELSEIF(${OpenGL} MATCHES "ES")
		# Desktop OpenGL ES 3.0

		add_definitions(-DGLUS_ES)
	
		include_directories(C:\\Imagination\\PowerVR\\GraphicsSDK\\SDK_3.2\\Builds\\Include)	

	ELSE()
		# Desktop

		SET(Processor "x86")
		SET(OperatingSystem "Windows")
		SET(Compiler "MSVC")
		
		set(ENV_DIR ${Processor}/${OperatingSystem}/${Compiler})
		
		include_directories(${GLUS_SOURCE_DIR}/../External/${ENV_DIR}/include)	
	
	ENDIF()
		
ENDIF()

# Files only valid for desktop OpenGL
list(APPEND ONLY_GL_C_FILES 	${GLUS_SOURCE_DIR}/src/glus_glfw.c
								${GLUS_SOURCE_DIR}/src/glus_perlin.c
								${GLUS_SOURCE_DIR}/src/glus_shaderprogram.c
								${GLUS_SOURCE_DIR}/src/glus_shape_adjacency.c
								${GLUS_SOURCE_DIR}/src/glus_version.c
)

# Files not for OpenGL ES 2.0 context
list(APPEND NOT_GLES2_C_FILES	${GLUS_SOURCE_DIR}/src/glus_line.c
								${GLUS_SOURCE_DIR}/src/glus_shape.c
								${GLUS_SOURCE_DIR}/src/glus_shape_texgen.c
								${GLUS_SOURCE_DIR}/src/glus_wavefront.c						
)

# Source files
file(GLOB C_FILES ${GLUS_SOURCE_DIR}/src/*.c)
file(GLOB ES_C_FILES ${GLUS_SOURCE_DIR}/src/*_es.c)
file(GLOB ES2_C_FILES ${GLUS_SOURCE_DIR}/src/*_es2.c)
file(GLOB VG_C_FILES ${GLUS_SOURCE_DIR}/src/*_vg.c)

# Header files
file(GLOB_RECURSE H_FILES ${GLUS_SOURCE_DIR}/src/*.h)

IF(${OpenGL} MATCHES "ES")

	list(REMOVE_ITEM C_FILES ${VG_C_FILES})

	IF(${OpenGL} MATCHES "ES2")
		# OpenGL ES 2.0
		
		list(REMOVE_ITEM C_FILES ${ONLY_GL_C_FILES} ${NOT_GLES2_C_FILES})

	ELSE()		
		# OpenGL ES 3.0
		
		list(REMOVE_ITEM C_FILES ${ONLY_GL_C_FILES} ${ES2_C_FILES})
	
	ENDIF()
	
	IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
		# Raspberry Pi
		# ToDo Add i.MX6
	
		list(REMOVE_ITEM C_FILES ${GLUS_SOURCE_DIR}/src/glus_windows_es.c)
		list(REMOVE_ITEM C_FILES ${GLUS_SOURCE_DIR}/src/glus_linux_es.c)
		list(REMOVE_ITEM C_FILES ${GLUS_SOURCE_DIR}/src/glus_imx6_es.c)
		
	ELSE()
		# Windows simulation
	
		list(REMOVE_ITEM C_FILES ${GLUS_SOURCE_DIR}/src/glus_raspberrypi_es.c)		
		list(REMOVE_ITEM C_FILES ${GLUS_SOURCE_DIR}/src/glus_linux_es.c)
		list(REMOVE_ITEM C_FILES ${GLUS_SOURCE_DIR}/src/glus_imx6_es.c)
		
	ENDIF()	
	
ELSE()
	# OpenGL 3 and 4
	
	list(REMOVE_ITEM C_FILES ${ES_C_FILES} ${ES2_C_FILES} ${VG_C_FILES})
	
ENDIF()

add_library(GLUS ${C_FILES} ${H_FILES})
