GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities:
-----------------------------------------------------

How to build GLUS and the examples:

1. Install Eclipse IDE for C/C++ Developers and a GNU Compiler Collection for your operating system.
   For building with Visual C++ 2013, CMake and the Windows SDK 8.1 are needed. 
2. Import GLUS as an existing project.
3. Set the build configuration in Eclipse to your operating system.
4. Build GLUS.
5. Import an example as an existing project.
6. Set the build configuration of the example in Eclipse to your operating system.
7. Build the example.
8. The executable is located in the Binaries or VG_Binaries folder.

If you get build errors:

- Please make sure, that you install all the needed header and libraries.
- Pre-build libraries are located in the External project folder. If you want, replace these libraries with your own build.
- Libraries for the Raspberry PI platform are not included.
- Libraries for the i.MX6 platform are not included.
- Libraries for OpenGL ES and OpenVG are not included.

SDKs and Libraries:

- GLEW 1.12.0 http://glew.sourceforge.net/
- GLFW 3.1    http://www.glfw.org/

OpenGL ES SDKs and Libraries:

- OpenGL ES 3.1: PowerVR SDK 3.4 http://community.imgtec.com/developers/powervr/
- OpenGL ES 3.0: PowerVR SDK 3.4 http://community.imgtec.com/developers/powervr/
				 Mali OpenGL ES 3.0 Emulator http://malideveloper.arm.com/develop-for-mali/tools/opengl-es-3-0-emulator/
- OpenGL ES 2.0: PowerVR SDK 3.4 http://community.imgtec.com/developers/powervr/
				 Mali OpenGL ES 2.0 Emulator http://malideveloper.arm.com/develop-for-mali/tools/opengl-es-2-0-emulator/
				 ANGLE http://code.google.com/p/angleproject/
				 AMD OpenGL ES 2.0 SDK http://developer.amd.com/tools-and-sdks/graphics-development/amd-opengl-es-sdk/

OpenVG SDKs and Libraries:

- OpenVG 1.1: Sample Implementation http://www.khronos.org/registry/vg/

SDKs/Libraries/Images used for Raspberry Pi:

- GCC Toolchain for Raspberry Pi https://github.com/raspberrypi/tools
- SDL 1.2 http://www.libsdl.org/download-1.2.php
- Raspbian "wheezy" http://www.raspberrypi.org/downloads

SDKs/Libraries/Images used for i.MX6:

- i.MX 6 Series Software and Development Tool Resources http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=IMX6_SW


Build configuration naming:

[CPU]_[GPU]_[OS]_[OpenGL]_[Compiler]_[Configuration]

CPU:								ARMv6, ARMv7, x64, x86
GPU/Emulator (Optional):			AMD, ANGLE, Khronos, Mali, PowerVR, VC4, Vivante  
OS: 								Darwin, Linux, Windows
OpenGL/OpenGL ES/OpenVG (Optional):	GLES2, GLES3, GLES31, VG11
Compiler:							GCC, MinGW
Configuration:						Debug, Release

e.g. x86__Windows__MinGW_Debug or ARMv6_VC4_Linux_GLES2_GCC_Release


Yours Norbert Nopper


Changelog:

01.05.2015 - Removed issue after running cppcheck.

02.04.2015 - Improved keyboard and touch screen detection on i.MX6.

11.03.2015 - Improved function robustness, when extracting angles from matrices and quaternions.

05.03.2015 - Fixed bug during EGL context creation, when previous commands did fail.

19.02.2015 - Added wavefront object line loading.
		   - Fixed bug in wavefront loading, that last triangle/line was added twice.

18.02.2015 - Improved color channel mapping for OpenGL ES and OpenVG.
		   - Improved basic FPS profiler.

12.02.2015 - Fixed loading and cleanup code for wavefront object scenes.

11.02.2015 - Renamed log error output in OpenVG: From glGetError() to vgGetError().
		   - Added a field of view check, when creating a perspective matrix. Now, no crash can happen.

10.02.2015 - Added possibility to pass surface attribute list. This allows e.g. to create a single buffered windows surface.

04.02.2015 - Added TGA image converting to a premultiplied TGA image.

29.01.2015 - Updated to GLEW 1.12.0.

28.01-2015 - Updated touch input code for i.MX6 running on Yocto Linux 3.10.53-1.1.0.

19.01.2015 - Updated to GLFW 3.1.

06.11.2014 - Fixed bug in wavefront loading: When object file was not found, clean up did crash.

31.10.2014 - Fixed bug in EGL initialization: EGLContext had EGLDisplay type.
		   - Fixed bug in TGA save: Count and size were swapped. 

24.10.2014 - Fixed fullscreen mode under desktop Linux using OpenGL ES or OpenVG.

21.10.2014 - Fixed keyboard events on i.MX6. Added touch display input to simulate mouse events on i.MX6. 

20.10.2014 - Fixed issue in wavefront scene loading. Added OpenGL Example43 plus assets.

		   - Updated paths and links in the projects. 

17.10.2014 - Added wavefront scene loading and improved loader at all. Minor bug fixing. 

10.10.2014 * Released GLUS 2.0. GLUS is now official part of the OpenGL SDK libraries: https://www.opengl.org/sdk/libs/GLUS/

15.09.2014 - Added emissive texture to wavefront object file loader. 

10.09.2014 - Removed doxygen warnings. Submitting GLUS for the OpenGL SDK today.

09.09.2014 - Added proper cleanup in GLUS, if invalid values are passed during window creation under desktop OpenGL.

04.09.2014 - Added automatic frame recording for e.g. movie clip generation in Blender.

01.09.2014 - Added TGA image color format converter.

29.08.2014 - Reorganisation and improvement of READMEs.

27.08.2014 - Cleaning up and refactoring code inside GLUS 2.0 draft version. No interface changes.
		   - Renamed files for Raspberry Pi and i.MX6.

26.08.2014 - Draft version of GLUS 2.0.
