# Khepri Engine

[![build](https://github.com/KhepriEngine/KhepriEngine/actions/workflows/main.yml/badge.svg)](https://github.com/KhepriEngine/KhepriEngine/actions/workflows/main.yml?query=branch%3Amain)

Khepri is a C++ game engine library. It provides C++ developers a set of tools to easily and quickly develop games. Its goals are:
* **High-quality**. Khepri's code aims to be as well-tested as possible, making sure its suite of tools is of high quality.
* **Easy to use**. Khepri's components are well documented and can be plugged into new or existing projects with ease.
* **Modular & replaceable**. Khepri provides sane default implementations of various subsystems (rendering, audio, state management, configuration, etc), but each is replaceable.

## Requirements

Contributing to Khepri requires:

* a C++17-capable compiler
* [Conan](https://conan.io/) 1.46 or newer.
* [CMake](https://cmake.org/) 3.15 or newer.
* [Doxygen](https://www.doxygen.nl/).
* [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) 12.
* [ClangTidy](https://clang.llvm.org/extra/clang-tidy/) 10.

## Getting Started

Make sure that the requirements mentioned above are installed.

## Building

Building uses Conan to automatically install all required dependencies.
CMake uses a _multi-configuration generator_ for Visual Studio which ignores `CMAKE_BUILD_TYPE` and allows specifying the build type at build time, rather than configuration time:
```
mkdir build && cd build
conan install .. -s build_type=Release
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake
cmake --build . --config Release
```

After building, run the tests with CTest:
```
ctest
```

## Contributing
Please refer to the [Code of Conduct](CODE_OF_CONDUCT.md) and the [Contributing guidelines](CONTRIBUTING.md).
