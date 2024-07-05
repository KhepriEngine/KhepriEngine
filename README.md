# Khepri Engine

[![build](https://github.com/KhepriEngine/KhepriEngine/actions/workflows/main.yml/badge.svg)](https://github.com/KhepriEngine/KhepriEngine/actions/workflows/main.yml?query=branch%3Amain)

Khepri is a C++ game engine library. It provides C++ developers a set of tools to easily and quickly develop games. Its goals are:
* **High-quality**. Khepri's code aims to be as well-tested as possible, making sure its suite of tools is of high quality.
* **Easy to use**. Khepri's components are well documented and can be plugged into new or existing projects with ease.
* **Modular & replaceable**. Khepri provides sane default implementations of various subsystems (rendering, audio, state management, configuration, etc), but each is replaceable.

## Requirements

Contributing to Khepri requires:

* a C++17-capable compiler
* [Conan](https://conan.io/) 2.5 or newer.
* [CMake](https://cmake.org/) 3.23 or newer.
* [Doxygen](https://www.doxygen.nl/).
* [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) 12.
* [ClangTidy](https://clang.llvm.org/extra/clang-tidy/) 10.

## Getting Started

Make sure that the requirements mentioned above are installed.

## Building

Building uses Conan to automatically install all required dependencies and set up CMake presets:
```
conan install . -s build_type=Release -of build-release
cmake --preset conan-default
cmake --build --preset conan-release
```

After building, run the tests with CTest:
```
ctest --preset conan-release
```

To debug Khepri, run the following:
```
conan install . -s "&:build_type=Debug" -s build_type=Release -of build-debug
cmake --preset conan-default
cmake --build --preset conan-debug
ctest --preset conan-debug
```
This will build Khepri in Debug mode, but use Release builds for its dependencies.

## Contributing
Please refer to the [Code of Conduct](CODE_OF_CONDUCT.md) and the [Contributing guidelines](CONTRIBUTING.md).
