# Adding libremidi to your project

## Through CMake

Consider the following existing CMake project for you application:

```cmake
project(my_app)

add_executable(my_app src/main.cpp)
```

Then you can add libremidi either directly for instance through a git submodule: 

```cmake
project(my_app)

# example of folder structure
add_subdirectory(3rdparty/libremidi)

add_executable(my_app src/main.cpp)

target_link_libraries(my_app PRIVATE libremidi)
```

Or through FetchContent:

```cmake
project(my_app)

FetchContent_Declare(
    libremidi
    GIT_REPOSITORY https://github.com/celtera/libremidi
    GIT_TAG        main
)

FetchContent_MakeAvailable(libremidi)

add_executable(my_app src/main.cpp)

target_link_libraries(my_app PRIVATE libremidi)
```

## Through a custom build-system

If using a custom build-system, the main thing to be aware of that 
CMake does automatically for you is passing the relevant flags which will enable each backend
and linking with the correct libraries.

For instance on Linux with ALSA:

```sh
$ g++ \
  main.cpp \
  -std=c++20 \
  -I3rdparty/libremidi/include \
  -DLIBREMIDI_ALSA=1 \
  -DLIBREMIDI_HAS_UDEV=1
  -ldl
```

or on macOS with CoreMIDI:

```sh
$ clang++ \
  main.cpp \
  -std=c++20 \
  -I3rdparty/libremidi/include \
  -DLIBREMIDI_COREMIDI=1 \
  -framework CoreFoundation \
  -framework CoreAudio \
  -framework CoreMIDI
```

or on Win32 with WinMM:

```batch
> cl.exe ^
  main.cpp ^
  /std:c++latest ^
  /I c:\libs\3rdparty\libremidi\include ^
  /DLIBREMIDI_WINMM=1 ^
  /DUNICODE=1 ^
  /D_UNICODE=1 ^
  path/to/winmm.lib
```