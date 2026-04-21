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
cmake ..
cmake --build .
```

**Linux / macOS:**
```bash
mkdir build && cd build
cmake ..
make
```

Executables will be in the `Binaries/` directory.

### Dependencies

All dependencies are automatically fetched and built:
- **GLFW 3.4** - Windowing and input
- **GLEW 2.2.0** - OpenGL Extension Wrangler
- **OpenGL 3.2+** - System graphics library

## Examples

All 46 examples demonstrate various OpenGL 3.x and 4.x features with GLSL shaders.

- [Example01 - Basic window and OpenGL 3 initialization](#example01---basic-window-and-opengl-3-initialization)
- [Example02 - Rendering of a triangle](#example02---rendering-of-a-triangle)
- [Example03 - Grey filter](#example03---grey-filter)
- [Example04 - Perspective rendering of a cube](#example04---perspective-rendering-of-a-cube)
- [Example05 - Phong rendering of a sphere](#example05---phong-rendering-of-a-sphere)
- [Example06 - Texturing of a cube](#example06---texturing-of-a-cube)
- [Example07 - Normal mapping](#example07---normal-mapping)
- [Example08 - Environment/cube mapping](#example08---environmentcube-mapping)
- [Example09 - GPU Particles](#example09---gpu-particles)
- [Example10 - Geometry shader](#example10---geometry-shader)
- [Example11 - Reflection and refraction](#example11---reflection-and-refraction)
- [Example12 - Shadow mapping](#example12---shadow-mapping)
- [Example13 - Simple tessellation (OpenGL 4.1)](#example13---simple-tessellation-opengl-41)
- [Example14 - Terrain rendering (OpenGL 4.1)](#example14---terrain-rendering-opengl-41)
- [Example15 - Water rendering](#example15---water-rendering)
- [Example16 - Model loading and rendering](#example16---model-loading-and-rendering)
- [Example17 - Clipping planes and two sided rendering](#example17---clipping-planes-and-two-sided-rendering)
- [Example18 - Using stencil buffer and clipping planes](#example18---using-stencil-buffer-and-clipping-planes)
- [Example19 - Render to texture and planar reflection](#example19---render-to-texture-and-planar-reflection)
- [Example20 - Texture matrix, alpha blending and discarding](#example20---texture-matrix-alpha-blending-and-discarding)
- [Example21 - Compute shader (OpenGL 4.3)](#example21---compute-shader-opengl-43)
- [Example22 - Shadow volumes](#example22---shadow-volumes)
- [Example23 - Displacement mapping (OpenGL 4.1)](#example23---displacement-mapping-opengl-41)
- [Example24 - Erode effect using perlin noise](#example24---erode-effect-using-perlin-noise)
- [Example25 - Model with groups and materials](#example25---model-with-groups-and-materials)
- [Example26 - Fur rendering](#example26---fur-rendering)
- [Example27 - Projection shadow for directional light](#example27---projection-shadow-for-directional-light)
- [Example28 - Screen space ambient occlusion (SSAO) (OpenGL 4.1)](#example28---screen-space-ambient-occlusion-ssao-opengl-41)
- [Example29 - CPU ray tracing](#example29---cpu-ray-tracing)
- [Example30 - GPU ray tracing using compute shader (OpenGL 4.3)](#example30---gpu-ray-tracing-using-compute-shader-opengl-43)
- [Example31 - Many lights using deferred shading (OpenGL 4.1)](#example31---many-lights-using-deferred-shading-opengl-41)
- [Example32 - BRDF and IBL rendering (OpenGL 4.1)](#example32---brdf-and-ibl-rendering-opengl-41)
- [Example33 - Real-Time BRDF and IBL rendering (OpenGL 4.1)](#example33---real-time-brdf-and-ibl-rendering-opengl-41)
- [Example34 - Subsurface scattering](#example34---subsurface-scattering)
- [Example35 - Order independent transparency using depth peeling](#example35---order-independent-transparency-using-depth-peeling)
- [Example36 - Order independent transparency using linked list (OpenGL 4.4)](#example36---order-independent-transparency-using-linked-list-opengl-44)
- [Example37 - CPU ray marching](#example37---cpu-ray-marching)
- [Example38 - Basic usage of program pipeline and separable programs (OpenGL 4.1)](#example38---basic-usage-of-program-pipeline-and-separable-programs-opengl-41)
- [Example39 - Basic usage of program pipeline, separable programs and shader subroutines (OpenGL 4.1)](#example39---basic-usage-of-program-pipeline-separable-programs-and-shader-subroutines-opengl-41)
- [Example40 - Cloth simulation using compute shader (OpenGL 4.3)](#example40---cloth-simulation-using-compute-shader-opengl-43)
- [Example41 - Ocean wave height/normal map calculation with FFT using compute shader (OpenGL 4.3)](#example41---ocean-wave-heightnormal-map-calculation-with-fft-using-compute-shader-opengl-43)
- [Example42 - Fast Approximate Anti Aliasing - FXAA (OpenGL 4.3)](#example42---fast-approximate-anti-aliasing---fxaa-opengl-43)
- [Example43 - Scene with several models having groups and materials](#example43---scene-with-several-models-having-groups-and-materials)
- [Example44 - Conservative rasterization](#example44---conservative-rasterization)
- [Example45 - GPU voxelization (OpenGL 4.4)](#example45---gpu-voxelization-opengl-44)
- [Example46 - Voxel cone tracing - Global illumination (OpenGL 4.4)](#example46---voxel-cone-tracing---global-illumination-opengl-44)

### Example01 - Basic window and OpenGL 3 initialization

![Example01](screenshots/Example01.png)

### Example02 - Rendering of a triangle

![Example02](screenshots/Example02.png)

### Example03 - Grey filter

![Example03](screenshots/Example03.png)

### Example04 - Perspective rendering of a cube

![Example04](screenshots/Example04.png)

### Example05 - Phong rendering of a sphere

![Example05](screenshots/Example05.png)

### Example06 - Texturing of a cube

![Example06](screenshots/Example06.png)

### Example07 - Normal mapping

![Example07](screenshots/Example07.png)

### Example08 - Environment/cube mapping

![Example08](screenshots/Example08.png)

### Example09 - GPU Particles

![Example09](screenshots/Example09.png)

### Example10 - Geometry shader

![Example10](screenshots/Example10.png)

### Example11 - Reflection and refraction

![Example11](screenshots/Example11.png)

### Example12 - Shadow mapping

![Example12](screenshots/Example12.png)

### Example13 - Simple tessellation (OpenGL 4.1)

![Example13](screenshots/Example13.png)

### Example14 - Terrain rendering (OpenGL 4.1)

![Example14](screenshots/Example14.png)

### Example15 - Water rendering

![Example15](screenshots/Example15.png)

### Example16 - Model loading and rendering

![Example16](screenshots/Example16.png)

### Example17 - Clipping planes and two sided rendering

![Example17](screenshots/Example17.png)

### Example18 - Using stencil buffer and clipping planes

![Example18](screenshots/Example18.png)

### Example19 - Render to texture and planar reflection

![Example19](screenshots/Example19.png)

### Example20 - Texture matrix, alpha blending and discarding

![Example20](screenshots/Example20.png)

### Example21 - Compute shader (OpenGL 4.3)

![Example21](screenshots/Example21.png)

### Example22 - Shadow volumes

![Example22](screenshots/Example22.png)

### Example23 - Displacement mapping (OpenGL 4.1)

![Example23](screenshots/Example23.png)

### Example24 - Erode effect using perlin noise

![Example24](screenshots/Example24.png)

### Example25 - Model with groups and materials

![Example25](screenshots/Example25.png)

### Example26 - Fur rendering

![Example26](screenshots/Example26.png)

### Example27 - Projection shadow for directional light

![Example27](screenshots/Example27.png)

### Example28 - Screen space ambient occlusion (SSAO) (OpenGL 4.1)

![Example28](screenshots/Example28.png)

### Example29 - CPU ray tracing

![Example29](screenshots/Example29.png)

### Example30 - GPU ray tracing using compute shader (OpenGL 4.3)

![Example30](screenshots/Example30.png)

### Example31 - Many lights using deferred shading (OpenGL 4.1)

![Example31](screenshots/Example31.png)

### Example32 - BRDF and IBL rendering (OpenGL 4.1)

![Example32](screenshots/Example32.png)

### Example33 - Real-Time BRDF and IBL rendering (OpenGL 4.1)

![Example33](screenshots/Example33.png)

### Example34 - Subsurface scattering

![Example34](screenshots/Example34.png)

### Example35 - Order independent transparency using depth peeling

![Example35](screenshots/Example35.png)

### Example36 - Order independent transparency using linked list (OpenGL 4.4)

![Example36](screenshots/Example36.png)

### Example37 - CPU ray marching

![Example37](screenshots/Example37.png)

### Example38 - Basic usage of program pipeline and separable programs (OpenGL 4.1)

![Example38](screenshots/Example38.png)

### Example39 - Basic usage of program pipeline, separable programs and shader subroutines (OpenGL 4.1)

![Example39](screenshots/Example39.png)

### Example40 - Cloth simulation using compute shader (OpenGL 4.3)

![Example40](screenshots/Example40.png)

### Example41 - Ocean wave height/normal map calculation with FFT using compute shader (OpenGL 4.3)

![Example41](screenshots/Example41.png)

### Example42 - Fast Approximate Anti Aliasing - FXAA (OpenGL 4.3)

![Example42](screenshots/Example42.png)

### Example43 - Scene with several models having groups and materials

![Example43](screenshots/Example43.png)

### Example44 - Conservative rasterization

![Example44](screenshots/Example44.png)

### Example45 - GPU voxelization (OpenGL 4.4)

![Example45](screenshots/Example45.png)

### Example46 - Voxel cone tracing - Global illumination (OpenGL 4.4)

Real-time global illumination using voxel cone tracing based on [Crassin et al. 2011](https://research.nvidia.com/sites/default/files/pubs/2011-09_Interactive-Indirect-Illumination/GIVoxels-pg2011-authors.pdf).
The Sponza scene is first voxelized into a 64³ RGBA8 3D texture with a dominant-axis geometry shader.
Mipmaps are generated for the voxel grid, then the final pass traces six diffuse cones plus a specular cone
per fragment to compute indirect illumination, ambient occlusion, and specular reflections.

**Comparison with the original paper:**

| Aspect | Paper | This implementation |
|---|---|---|
| Scene representation | Sparse Voxel Octree (SVO) — adaptive, high-resolution | Flat 64³ RGBA8 3D texture — simpler, lower resolution |
| Voxel filtering | Anisotropic pre-filtering: per-face directional radiance propagated up the octree | Isotropic `glGenerateMipmap` box filter |
| Radiance storage | Per-face (6 directions) for directional shadowing | Single isotropic value per voxel |
| Voxel write | Atomic accumulation when multiple triangles cover the same voxel | Plain `imageStore` (last-write-wins) |
| Specular cone aperture | Roughness-derived per material | Fixed half-angle (tan ≈ 0.07, ~4°) |

![Example46](screenshots/Example46.png)
