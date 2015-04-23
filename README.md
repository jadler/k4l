kinect4love (k4l)
=================

Integration of Kinect motion sensor and the framework for development of Love2d games.

# Build Instructions

To build k4l, you'll need

- [OpenCV](http://code.opencv.org/projects/opencv/wiki) >= 2.4.8
- [CMake](http://cmake.org) >= 2.6
- [Lua](http://www.lua.org/) == 5.1
- [libfreenect](https://github.com/OpenKinect/libfreenect) >= 0.5

## Build

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/usr ..
    make
    make install
