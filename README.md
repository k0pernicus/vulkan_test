A simple project to play with Vulkan, on both macOS and Windows.

The windows version is still in work-in-progress.  
However, the program runs well on an Apple m1 chip.

Please to run the `build_shaders` script first, depending on the platform, to build the GLSL shaders to Spir-V.

This project uses GLFW for window, as Vulkan is platform-agnostic, and should be the only dependency you have to install.

To install Vulkan, please check on the web, depending on your OS.

This project has been built on XCode using C++ (clang compiler), but should be converted to VS project soon, to keep the compatibility with the Windows OS.
