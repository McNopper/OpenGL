# AGENTS.md - OpenGL Examples working contract

A self-contained collection of OpenGL examples, one folder per topic, from "draw
a triangle" through shadow mapping, deferred shading, SSAO, order-independent
transparency, tessellation, GPGPU particles, compute-shader cloth and ocean
simulation, voxelization and voxel cone tracing, to Gaussian splatting. Each
example is its own executable with its own `src/` and `shader/` directories.

**Stability mandate.** This is a long-established, widely read reference. The
examples are read as much as they are run, and screenshots of them are
published - so the rendered output is part of the contract.

- Never reformat. Formatting and style are authored, not derived; there is no
  formatter config here on purpose and tooling runs `FormatStyle: none`.
- Keep each example self-contained. Do not introduce cross-example
  dependencies, shared helper headers, or a common framework - the value of an
  example is that you can read one folder and understand the technique.
- Do not change rendered output for cosmetic reasons. If a fix changes what an
  example draws, say so explicitly and expect to re-capture its screenshot.
- Prefer the smallest diff that removes the defect. No drive-by refactors.

## Build (standalone)

```
cmake -S . -B build/ninja -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/ninja
```

Binaries land in `Binaries/`, which is also the runtime directory: examples
load assets by bare filename (`crate.tga`, `bunny.obj`, `doge2.hdr`), so **run
them with `Binaries/` as the working directory**. Shaders are loaded by
relative path from the source tree.

Dependencies are fetched at configure time (GLFW 3.4, GLEW 2.2.0, GLUS). GLUS
is preferred from a sibling `../GLUS` checkout and falls back to the GitHub
repository, so the multi-repo layout builds without edits.

Layout of one example:

| Path | Contents |
|---|---|
| `ExampleNN/src/main.c` | window, GL setup, render loop |
| `ExampleNN/src/*.c`, `*.h` | occasional helpers (wavefront loaders, water passes) |
| `ExampleNN/shader/*.glsl` | `*.vert`, `*.frag`, `*.geom`, `*.cont`, `*.eval`, `*.comp` |
| `ExampleNN/CMakeLists.txt` | one small template, globs `src/*.c` |
| `Binaries/` | assets and built binaries |
| `cmake/`, `tools/` | build policy and the clang-tidy lane |

## Finding and eliminating bugs

All tooling is optional at build time - a clean checkout with none of it
installed builds exactly as before.

| Layer | Command | Gate |
|---|---|---|
| Compiler warnings | part of every build | `ENABLE_WERROR=ON` makes them fatal |
| cppcheck | `cmake --build build/ninja --target cppcheck` | exits non-zero on findings |
| cppcheck (exhaustive) | `... --target cppcheck-strict` | opt-in |
| clang-tidy | `python tools/check_tidy.py --build-dir build/ninja` | `WarningsAsErrors` in `.clang-tidy` |
| Sanitizers | `-DENABLE_SANITIZER=address,undefined` then run the examples | runtime faults |

`check_tidy.py` accepts a path to narrow a run, e.g.
`python tools/check_tidy.py --build-dir build/ninja Example41`.

The warning level is applied per example target and **after** the dependency
setup, so GLFW/GLEW/GLUS keep building as before and every warning that appears
is ours. `ENABLE_WERROR` is **OFF** by default so a clean checkout keeps
building; turn it on once the baseline is clean.

**Suggested bug-elimination cycle.** Build with `ENABLE_SANITIZER=address,undefined`,
run each example once (the compute and image load/store ones are the risky
set), run `cppcheck`, run `check_tidy.py`. Fix in order of confidence: crashes
and out-of-bounds first, then shader undefined behaviour, then wrong results.
Re-run after each change. When anything broken is found at any point, drop back
to bugs immediately.

Suppression policy: `cppcheck.supp` stays short and justified. Prefer inline
`// cppcheck-suppress <id>` for one-off findings. Note that `unusedFunction` is
deliberately **not** suppressed here - these are applications, so dead
functions are exactly what the gate should catch.

## Defect classes this code is prone to

- **Shader/C interface drift.** `glVertexAttribPointer` stride/offset against
  the shader's `in` declarations, an element buffer's type against the index
  array type, a sampler uniform set to texture unit N with the texture bound to
  unit M, `glUniform*` setter type against the declared type. Check every
  pairing.
- **GLSL undefined behaviour.** Unwritten `gl_Position` components from a
  geometry shader, varyings not produced by the previous stage, integer varyings
  missing `flat`, tessellation levels never written.
- **Domain errors in shading math.** `normalize(vec3(0))`, `acos`/`asin`
  outside `[-1, 1]` (clamp the result of a `dot` of two normalized vectors),
  `pow` with a negative base (clamp the Schlick term to `>= 0` **and** `<= 1`
  before it reaches `mix`), `refract()` returning `vec3(0)` on total internal
  reflection before `normalize`.
- **Compute/image load-store.** An `image2D`/`uimage*` format qualifier
  mismatched with the texture's actual internal format, `imageStore` without a
  bounds guard against `imageSize()`, an execution `barrier()` used where a
  `memoryBarrier*()` is required, a missing `glMemoryBarrier` between a compute
  write and a later draw, and `local_size_*` versus `glDispatchCompute` counts.
- **Simulation singularities.** Divisions by a distance between two coincident
  particles, and `acos` of an unclamped dot at a contact apex - both produce
  NaN that spreads through the whole field via the constraint/relaxation passes.
- **Index-off-by-one in inverse mappings.** Voxelizers and grid round-trips
  (`gridSize - z` where `gridSize - 1 - z` is meant) silently shift by one and
  write one element out of bounds.
- **Unchecked loader returns.** An image/noise generator returning `GLUS_FALSE`
  still leaves the caller holding a struct it then uploads.
- **Conservative rasterization winding.** Triangle expansion is winding
  dependent; normalize the winding before pushing the edge planes or half the
  triangles shrink instead of expand.

## Conventions

- **Production code discipline**: clean code and clean architecture, **no
  hacks**. No content sniffing, magic numbers, hidden special cases,
  duplicated blocks, or indirections that silently die.
- **Work priority is a strict cycle**: **bugs first**, then refactoring or new
  features. When anything broken is found at any point, drop back to bugs
  immediately.
- **Dependency licence gate**: check the licence for closed-source commercial
  use before adding anything. MIT / BSD / zlib / Apache-2.0 / Unlicense pass;
  copyleft and non-commercial fail; unclear means ask. Record it in
  `THIRD-PARTY.md` in the same change.
- **Clean asset management**: assets in `Binaries/` use descriptive names
  (`ChessKing.tga`, `cm_pos_x.tga`, `grand_canyon_height.tga`) and are findable
  from the filesystem alone. Do not introduce opaque codes.
- **Comments explain *why*, not *what***. Spec citations and math derivations
  are worth keeping; do not narrate code.
- **Language**: English only, always.
- After any change, build clean, run the affected example, and re-run the
  analysis set. If rendered output changed, re-capture the screenshot.
