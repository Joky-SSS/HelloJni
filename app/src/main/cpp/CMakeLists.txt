# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

#寻找swig
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set(JAVA_GEN_PACKAGE "com.example.swigjni")
string(REPLACE "." "/" JAVA_GEN_SUBDIR ${JAVA_GEN_PACKAGE})
set(JAVA_GEN_DIR ${Project_SOURCE_DIR}/../java/${JAVA_GEN_SUBDIR})

#文件输出目录
set(CMAKE_SWIG_OUTDIR ${JAVA_GEN_DIR})

set(CMAKE_SWIG_FLAGS -c++ -package ${JAVA_GEN_PACKAGE})
#在c ++模式下调用SWIG

#set_property(SOURCE see.i PROPERTY CPLUSPLUS ON)

#swig_add_library(See LANGUAGE java SOURCES See.i)




# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp
        )

set(MY_LOG_TAG \"hello-jni\")
set(MY_LOG_LEVEL MY_LOG_LEVEL_VERBOSE)
#IF ($(APP_OPITM) STREQUAL release)
#    set(MY_LOG_LEVEL MY_LOG_LEVEL_ERROR)
#else ()
#    set(MY_LOG_LEVEL MY_LOG_LEVEL_VERBOSE)
#endif ()

add_definitions(-DMY_LOG_TAG=${MY_LOG_TAG})
add_definitions(-DMY_LOG_LEVEL=${MY_LOG_LEVEL})

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})