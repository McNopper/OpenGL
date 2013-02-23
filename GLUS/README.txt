How to build GLUS:
------------------

1. Install Eclipse IDE for C/C++ Developers and a GNU Compiler Collection for your operating system.
2. Extract this ZIP file and set the workspace-cpp folder as your Eclipse workspace.
3. Import each folder as a separate, existing project.
4. Set the build configuration in Eclipse to your operating system.
6. Build GLUS.

If you get build errors:
------------------------
- Please make sure, that you install all the needed header and libraries.
- Pre-build libraries are located in the External project folder. If you want, replace these libraries with your own build.
- Libraries for the Raspberry PI platform are not included.

23.02.2013 Norbert Nopper (norbert@nopper.tv)

SDKs / Libraries / Images used:
-------------------------------

- OpenGL ES 3.0: PowerVR SDK http://www.imgtec.com/powervr/insider/sdkdownloads/
- OpenGL ES 2.0: PowerVR SDK http://www.imgtec.com/powervr/insider/sdkdownloads/
- GCC Toolchain for Raspberry Pi https://github.com/raspberrypi/tools
- Raspbian “wheezy” http://www.raspberrypi.org/downloads

Changelog:
----------

22.02.2013 - Added GLUS for Raspberry Pi

13.02.2013 - Added GLUS for OpenGL ES 2.0

28.01.2013 - Added GLUS for OpenGL ES 3.0

21.12.2012 - Added latest GLFW (2.7.7)
		   - Added function to copy shapes
		   - Fixed out of bounds in euler angles gathering
		   - Added GLFW terminate function when creation of windows fails

03.10.2012 - Minor bug fixes and improvements in GLUS
           - GLUS is now 1.0 Final!

31.08.2012 - Added latest version of GLEW (2.9.0)
           - Added more build configurations
           - Added OpenGL 4.3 features for GLUS
		   - Refactoring and bug fixing in GLUS. GLUS is now in 1.0 Beta!
		   - Added documenation(!) for GLUS
