# hmp: Handmade Pong #

A proof of concept game application that uses the framework established by the
[Handmade Hero][hmh] project to create a simulation of the classic [Pong][pong] game.
The code in this repository breaks a few principles of the "handmade" framework
by using various libraries (listed below) for better application portability.

## Libraries ##

### Required ###

- [SDL (v2.0)](https://www.libsdl.org/)
- [SDL TTF (v2.0)](https://www.libsdl.org/projects/SDL_ttf/)
- [OpenGL (v3.2)](https://www.opengl.org/)
- [GLM (v0.9.9.3)](https://glm.g-truc.net)
- [C++ Standard Library (v3.4)](https://en.cppreference.com/w/cpp/header)

### Optional ###

- [libpng (v1.6.34)](http://www.libpng.org/pub/png/libpng.html)
- [libffmpeg (v3.4.4)](https://ffmpeg.org/)

## Install ##

The source code for this application was built and tested on an Ubuntu 16.04/18.04
Linux machine, and the steps below reflect the installation required for this
environment. To replicate this process on an alternate distribution, either
install the libraries and their dependencies from source or by using the
distro's package management system (a la `apt` for Ubuntu).

Here are the commands required to install all third-part dependencies:

1. `sudo apt-get install clang-3.8 cmake`: Install build tools (i.e. a C++ compiler and a build manager).
1. `sudo apt-get install libsdl2-2.0-0 libsdl2-dev libsdl2-ttf-2.0-0 libsdl2-ttf-dev`: Install the window management library SDL and its supplemental text library SDL-TTF.
1. `sudo apt-get install libpng-dev`: (Optional) Install image/video processing libraries to facilitate in-situ application render outputs.
1. `./etc/build_{platform}.sh`: Build the demo application, which will be deployed to the `./build/install/` directory.


[pong]: https://en.wikipedia.org/wiki/Pong
[hmh]: https://handmadehero.org/
