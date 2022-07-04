A simple project to play with Vulkan, on both macOS and Windows, with C++17.

The program runs well on both Windows 10/11 (using Visual Studio 2022), and on M1 macs (using latest Xcode build).

Please to run the `build_shaders` script first, depending on the platform, to build the GLSL shaders to Spir-V.

This project uses GLFW to create / manage / interact with windows, as Vulkan is platform-agnostic, and should be the only dependency you have to install.

To install Vulkan, please check on the web, depending on your OS.

This project has been built on XCode using C++ (clang / MSVC compiler), but should be converted to VS project soon, to keep the compatibility with the Windows OS.