GLUS - Modern OpenGL Utilities
===============================

GLUS is automatically built as part of the main project using CMake.

## Prerequisites

- **CMake 3.14 or higher** - [Download CMake](https://cmake.org/download/)
- **C/C++ Compiler**:
  - Windows: Visual Studio 2013 or newer (MSVC)
  - Linux: GCC or Clang
  - macOS: Xcode Command Line Tools (Clang)
- **Git** - For fetching dependencies
- **OpenGL 3.2+** compatible graphics driver

## Building

From the project root directory:

**Windows (Visual Studio):**
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

**Linux / macOS:**
```bash
mkdir build && cd build
cmake ..
make
```

## Dependencies

All dependencies are automatically fetched via CMake FetchContent:
- **GLFW 3.4** - Window and input management
- **GLEW 2.2.0** - OpenGL extension loading

No manual installation of external libraries required!

## Output

- Static library: `lib/GLUS.lib` (or `lib/libGLUS.a` on Linux/macOS)
- Examples: Compiled to `Binaries/` directory

## Dependencies (Auto-Downloaded)

- **GLFW 3.4** - https://github.com/glfw/glfw
- **GLEW 2.2.0** - https://github.com/nigels-com/glew

These are automatically downloaded and built via CMake FetchContent.

---

Yours Norbert Nopper

---

## Historical Changelog (Legacy Eclipse Build)

Note: This changelog covers the legacy Eclipse-based build system.
For current CMake build changes, see git commit history.

31.05.2016 - Updated to PowerVR SDK 2016 R1.2.

28.04.2016 - Fixed wrong function name in Android module. 

29.12.2015 - Updated to PowerVR SDK 4.0.

15.12.2015 - Updated to Yocto Linux 3.14.52-1.1.0, supporting also the i.MX6 Quad Plus.

15.10.2015 - Optimized/fixed own dynamic memory manager.

10.09.2015 - Optimized file abstraction.

08.09.2015 - Added basic Android support for OpenGL ES.

11.07.2015 - Improved, updated README.

14.05.2015 - Updated OpenGL ES libraries.

11.05.2015 - Added time stamp querying function.
           - Removed warning in OpenGL ES 2.0 code.

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
