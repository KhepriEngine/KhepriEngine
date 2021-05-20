# Khepri Engine

Khepri is a C++ game engine library.

## Requirements

Khepri requires:

* a C++17-capable compiler
* [Conan](https://conan.io/) 1.46 or newer.
* [CMake](https://cmake.org/) 3.13 or newer.
* [Doxygen](https://www.doxygen.nl/).
* [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) 12.
* [ClangTidy](https://clang.llvm.org/extra/clang-tidy/) 10.

## Getting Started

Make sure that the requirements mentioned above are installed.

## Building

Building uses Conan to automatically install all required dependencies:
```
mkdir build && cd build
conan install ..
conan build ..
```

## Contributing
Please refer to the [Code of Conduct](CODE_OF_CONDUCT.md) and the [Contributing guidelines](CONTRIBUTING.md).
