OpenGL Examples
---------------

## Running the Examples

After building, executables are located in the `Binaries/` directory.

**Windows:**
```bash
cd Binaries
Example01.exe
```

**Linux / macOS:**
```bash
cd Binaries
./Example01
```

## Build Information

Executables are built using CMake and link statically with:
- GLFW 3.4
- GLEW 2.2.0
- GLUS (project's utility library, which in turn bundles cgltf and stb_image)

Dependencies are automatically downloaded during CMake configuration.

Further information:

- See main README.md in project root for build instructions
- See the GLUS repository (https://github.com/McNopper/GLUS) for GLUS library details
