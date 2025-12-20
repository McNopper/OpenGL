OpenGL
======

OpenGL 3 and OpenGL 4 with GLSL

## Build Instructions

### Prerequisites

**Required:**
- **CMake 3.14 or higher** - [Download](https://cmake.org/download/)
- **C/C++ Compiler**:
  - Windows: Visual Studio 2013+ (MSVC)
  - Linux: GCC or Clang
  - macOS: Xcode Command Line Tools
- **Git** - For fetching dependencies
- **OpenGL 3.2+** compatible graphics driver

### Verify Installation

Check if CMake is installed:
```bash
cmake --version
```
Should show version 3.14 or higher.

Check if Git is installed:
```bash
git --version
```

### Building

Dependencies (GLFW and GLEW) are automatically downloaded and built via CMake FetchContent.

**Windows (Visual Studio):**
```bash
mkdir build && cd build
cmake -A x64 ..
cmake --build . --config Release
```

**Linux / macOS:**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Executables will be in the `Binaries/` directory.

### Dependencies

All dependencies are automatically fetched and built:
- **GLFW 3.4** - Windowing and input (zlib/libpng License)
- **GLEW 2.2.0** - OpenGL Extension Wrangler (Modified BSD/MIT License)
- **OpenGL 3.2+** - System graphics library

See `THIRD-PARTY-LICENSES.txt` for full license information.

## Examples

======

Example01 - Basic window and OpenGL 3 initialization
	
Example02 - Rendering of a triangle

Example03 - Grey filter
	
Example04 - Perspective rendering of a cube

Example05 - Phong rendering of a sphere
	
Example06 - Texturing of a cube

Example07 - Normal mapping

Example08 - Environment/cube mapping

Example09 - GPU Particles

Example10 - Geometry shader
	
Example11 - Reflection and refraction

Example12 - Shadow mapping

Example13 - Simple tessellation (OpenGL 4.1)

Example14 - Terrain rendering (OpenGL 4.1)

Example15 - Water rendering

Example16 - Model loading and rendering

Example17 - Clipping planes and two sided rendering

Example18 - Using stencil buffer and clipping planes

Example19 - Render to texture and planar reflection

Example20 - Texture matrix, alpha blending and discarding

Example21 - Compute shader (OpenGL 4.3)

Example22 - Shadow volumes

Example23 - Displacement mapping (OpenGL 4.1, AMD hardware - not on Mac OS X - has artifacts but used to work)

Example24 - Erode effect using perlin noise

Example25 - Model with groups and materials

Example26 - Fur rendering

Example27 - Projection shadow for directional light

Example28 - Screen space ambient occlusion (SSAO) (OpenGL 4.1)

Example29 - CPU ray tracing

Example30 - GPU ray tracing using compute shader (OpenGL 4.3)

Example31 - Many lights using deferred shading (OpenGL 4.1)

Example32 - BRDF and IBL rendering (OpenGL 4.1)

Example33 - Real-Time BRDF and IBL rendering (OpenGL 4.1)

Example34 - Subsurface scattering

Example35 - Order independent transparency using depth peeling

Example36 - Order independent transparency using linked list (OpenGL 4.4, AMD hardware has artifacts)

Example37 - CPU ray marching

Example38 - Basic usage of program pipeline and separable programs(OpenGL 4.1)

Example39 - Basic usage of program pipeline, separable programs and shader subroutines (OpenGL 4.1, AMD hardware does not work properly)

Example40 - Cloth simulation using compute shader (OpenGL 4.3)

Example41 - Ocean wave height/normal map calculation with FFT using compute shader (OpenGL 4.3, NVIDIA hardware under Linux has artifacts)

Example42 - Fast Approximate Anti Aliasing - FXAA (OpenGL 4.3)

Example43 - Scene with several models having groups and materials

Example44 - Conservative rasterization

Example45 - GPU voxelization (OpenGL 4.4)
