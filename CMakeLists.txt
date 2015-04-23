CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

SET (CMAKE_C_COMPILER /usr/bin/gcc)
SET (CMAKE_CXX_COMPILER /usr/bin/g++)
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -ansi -g -fPIC")

SET (NUI nui) # the final library name
SET (K4L k4l) # the project name

PROJECT ("${K4L}")

FIND_PACKAGE (Threads)
FIND_PACKAGE (PkgConfig REQUIRED)

PKG_CHECK_MODULES (OPENCV REQUIRED opencv)
PKG_CHECK_MODULES (LIBFREENECT REQUIRED libfreenect)
PKG_CHECK_MODULES (LUA5.1 REQUIRED lua5.1)

# Version number
SET (K4L_MAJOR_VERSION 1)
SET (K4L_MINOR_VERSION 0)

# Add include directory inside source tree to the search path for include k4l.h
INCLUDE_DIRECTORIES ("${PROJECT_SOURCE_DIR}/include")

# Configure header file
CONFIGURE_FILE (
	"${PROJECT_SOURCE_DIR}/include/${K4L}internal.h.in"
	"${PROJECT_BINARY_DIR}/include/${K4L}internal.h"
)

# Add include directory inside binary tree to the search path for include k4linternal.h
INCLUDE_DIRECTORIES ("${PROJECT_BINARY_DIR}/include")

# Add library
ADD_LIBRARY ("${NUI}" SHARED "src/${K4L}.cpp" src/wrapper.c)

TARGET_LINK_LIBRARIES ("${NUI}" "${CMAKE_THREAD_LIBS_INIT}" "${OPENCV_LIBRARIES}" "${LIBFREENECT_LIBRARIES}" "${LUA5.1_LIBRARIES}")

# Make sure the compiler can find include files for our library
# when other libraries or executables link to k4l
TARGET_INCLUDE_DIRECTORIES ("${NUI}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
