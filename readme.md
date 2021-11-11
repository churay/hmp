# `hmp`: Handmade Pong #

`hmp` is a [Pong][hmp-pong] clone written in C++ that adheres to many of the
precepts laid out by Casey Muratori in his ["Handmade Hero"][hmp-hmh] project, i.e.
deliberate hardware usage and minimization of dependence on third-part libraries.
This implementation breaks many "handmade" principles by incorporating the C++
standard library and SDL to achieve some level of platform-independence, but
adheres to the philosophy for most core and platform-independent systems.

The main features of that distinguish this implementation are:

- Support for loop-live code editing, i.e. testing code updates while a
  live application input loop is running to dynamically adjust application
  parameters (including logic!).
- Robust rendering capabilities, including efficient block-based text rendering
  and support for aspect ratio-respecting graphical elements.
- Rudimentary sound support via primitive sound waves (e.g. square, sawtooth)
  and a basic audio synthesizer.
- Robust input configuration support, including arbitrary remapping of different
  input types (e.g. booleans, 1D scales, 2D scales, etc.).

## Demos ##

### Basic Gameplay ###

![`hmp` game demo](https://github.com/churay/hmp/raw/master/doc/demo.gif)

### Input Capture/Replay ###

![`hmp` replay demo](https://github.com/churay/hmp/raw/master/doc/replay.gif)

### Basic Loop-live Editing ###

![`hmp` llce demo 1](https://github.com/churay/hmp/raw/master/doc/llce1.gif)

### Advanced Loop-live Editing ###

![`hmp` llce demo 2](https://github.com/churay/hmp/raw/master/doc/llce2.gif)

## Install/Test Instructions ##

To build and run on Ubuntu 16.04/18.04+, execute the following commands:

```
# Install third-party dependencies.
$ sudo apt-get install clang-6.0 cmake libsdl2-2.0-0 libsdl2-dev libsdl2-ttf-2.0-0 libsdl2-ttf-dev
# (Optional) Install image/video capture software.
$ sudo apt-get install ffmpeg libpng-dev

# Edit the build configuration, then configure/build the code.
$ vim ./etc/build_$PLATFORM.sh
$ ./etc/build_$PLATFORM.sh

# (Optional) Disable ASLR to allow for loop-live code editing.
$ ./etc/debug_env.sh

# Run the application.
$ ./llce.out
```

### Dependencies ###

- [C++17-compatible Compiler](https://en.cppreference.com/w/cpp/17)
- [CMake v3.10](https://cmake.org/)
- [OpenGL v3.2](https://www.opengl.org/)
- [GLM v0.9.9.3](https://glm.g-truc.net)
- [SDL v2.0](https://www.libsdl.org/)
- [SDL TTF v2.0](https://www.libsdl.org/projects/SDL_ttf/)
- [(Optional) libpng v1.6.34](http://www.libpng.org/pub/png/libpng.html)
- [(Optional) ffmpeg v3.4.4](https://ffmpeg.org/)

## License ##

This project is licensed under [the MIT License][hmp-license].


[hmp-pong]: https://en.wikipedia.org/wiki/Pong
[hmp-hmh]: https://handmadehero.org/
[hmp-license]: https://raw.githubusercontent.com/churay/hmp/master/liscense.txt
