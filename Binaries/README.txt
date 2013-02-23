How to make and run the examples:
---------------------------------

1. Please make sure, you have installed a Windows OpenGL 3.2 driver. For some examples you need a OpenGL 4.2 or OpenGL 4.3 driver. As Mac OS X only supports OpenGL 3.2, these examples do not work.
2. Install Eclipse IDE for C/C++ Developers and a GNU Compiler Collection for your operating system.
3. Extract this ZIP file and set the workspace-cpp folder as your Eclipse workspace.
4. Import each folder as a separate, existing project.
5. Set the build configuration in Eclipse to your operating system.
6. Build GLUS.
7. Build any of the examples.
8. The executables are located in the Binaries folder. Execute them directly or create a run configuration in Eclipse.

If you get build errors:
------------------------
- Please make sure, that you install all the needed header and libraries.
- Pre-build libraries are located in the External project folder. If you want, replace these libraries with your own build.

28.01.2013 Norbert Nopper (norbert@nopper.tv)

Changelog:
----------

28.01.2013 - Added PowerVR OpenGL ES 3.0 DLLs

21.12.2012 - Switched the environment textures to the correct sides
		   - see GLUS README.txt for more details

03.10.2012 - Minor bug fixes and improvements in GLUS
           - GLUS is now 1.0 Final!

31.08.2012 - Added latest version of GLEW (2.9.0)
           - Added more build configurations
           - Added OpenGL 4.3 features for GLUS
		   - Refactoring and bug fixing in GLUS. GLUS is now in 1.0 Beta!
		   - Added documenation(!) for GLUS
		   - Added compute shader example

02.08.2012 - Added latest version of GLFW (2.7.6)
		   - Added latest version of GLEW (2.8.0)
		   - Refactoring and bug fixing in GLUS. GLUS is now in 1.0 Alpha!
		   - Minor changes in the examples

30.04.2012 - Added latest version of GLFW (2.7.5)

28.04.2012 - Fixed bug: Modified GLSL code regarding OpenGL 3.2 conformance. Now the examples do run on NVIDIA without modifications
           - Fixed bug: Fixed a bug in Example 14. Used to pass one element, but defined an array. AMD driver did not complain, NVIDIA driver reported an error
           - Improved the code in Example 12 as descibed in OpenGL 4.0 Shading Language Cookbook

15.04.2012 - Fixed bug: Added casting of variables in the GLSL code of Example 14
		   - Fixed bug: Changed the plane matrix size from 3x3 to 4x4 in Example 18
		   - Improved wavefront object file loading. Now using dynamic memory for temporary variables

04.04.2012 - Added one more example
           - Added functions to GLUS
           - Fixed bugs in GLUS

24.03.2012 - Added one more example
           - Added latest version of GLFW (2.7.4)
           - Refactoring, added new functions and code optimizations
           - Switched to c99 for compilation

13.03.2012 - Minor bug fixing and optimization. Refactoring and new GLUS functions

07.03.2012 - Added the possibility, to disable the resizing of the window

03.03.2012 - Added two more examples
		   - Fixed a bug when creating the sphere geometry