# Example 51 – 3D Gaussian Splatting

Implements the [KHR_gaussian_splatting](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_gaussian_splatting) glTF 2.0 extension using OpenGL 4.6 compute and instanced rendering.

![Example51](../screenshots/Example51.png)

## Features

- **Spec-compliant rendering**: View-space depth back-to-front sort, 3σ-clipped screen-space Gaussian quads, premultiplied alpha compositing.
- **GPU bitonic sort**: Adapted from Example 47. Sorts all splats by view-space depth each frame using a bitonic sort compute shader.
- **Spherical harmonics**: Full SH evaluation through band 3 (degree 0–3). Wigner-D rotation matrices rotate the SH coefficients when the node has a non-identity transform.
- **Dynamic configuration**: The glTF file path is an optional command-line argument. The SH degree and per-splat stride are detected automatically from the primitive attributes and injected as shader defines at compile time.

## Usage

```
Example51 [path/to/model.gltf]
```

Defaults to `../Binaries/lego.gltf` when no argument is given.

## Controls

| Key           | Action               |
|---------------|----------------------|
| ← / →         | Orbit horizontally   |
| ↑ / ↓         | Orbit vertically     |
| Page Up/Down  | Zoom in / out        |

## Technical Overview

### Rendering pipeline

1. **Depth pass** (`depth.comp.glsl`): Initialise the index buffer to `[0…N-1]` and compute the view-space depth of each splat mean.
2. **Sort pass** (`sort.comp.glsl`): Bitonic sort on `(depths, indices)` in descending order so the farthest splats are drawn first.
3. **Splat pass** (`splat.vert.glsl` / `splat.frag.glsl`): For each splat instance, project the 3D covariance to screen space (EWA approximation), eigendecompose the 2×2 result to orient a screen quad, evaluate the SH colour, and output `vec4(color·alpha, alpha)` for premultiplied alpha blending.

### Covariance projection

The world-space 3D covariance is `Σ = M·R·diag(s)·diag(s)·Rᵀ·Mᵀ` where `R` is the splat orientation quaternion, `s` the per-axis scale, and `M` the upper-left 3×3 of the node world matrix. The EWA Jacobian approximation converts it to a 2D screen-space covariance, which is eigendecomposed to produce the quad axes.

### Wigner-D SH rotation

For nodes with a non-identity transform the SH coefficients must be rotated into world space before evaluation. Band-1, 2, and 3 Wigner-D rotation matrices are built on the CPU and stored in the model SSBO. All matrices use column-major storage (OpenGL convention). The GLUS helpers (`glusSHBuildRotation1f`, `2f`, `3f`) are implemented in `GLUS/src/glus_sh.c`.

## References

- [KHR_gaussian_splatting extension specification](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_gaussian_splatting)
- [3D Gaussian Splatting for Real-Time Radiance Field Rendering](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/) (Kerbl et al., SIGGRAPH 2023)
- [antimatter15/splat](https://github.com/antimatter15/splat) – WebGL 3D Gaussian Splatting viewer
- [mkkellogg/GaussianSplats3D](https://github.com/mkkellogg/GaussianSplats3D) – Three.js-based 3D Gaussian Splatting
