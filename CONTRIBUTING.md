# Contributing

## Getting started
Please read the [Code of Conduct](CODE_OF_CONDUCT.md) before you begin.

## Filing issues
Please consider the following before creating a new issue:
* Raise only **one issue per issue**. If you want to report multiple issues, create multiple issues.
* Carefully consider the type of your issue.
  * A **bug** is a (possible) malfunction in the software; it behaves differently from documented under certain conditions. Crashes, hangs, wrong results, are all bugs. Phrase the issue in terms of what goes wrong.
  * A feature request is something you'd like the software to have or do.
* When in doubt, contact the [project maintainers](MAINTAINERS.md) for advice.

## Making changes
The following section is for people who wish to contribute by making changes to the software.

### Development environment
The project's main development environment is Windows, using [Conan](https://conan.io/) for package management and [Visual Studio](https://visualstudio.microsoft.com/) as compiler.

### Requirements

Contributing to Khepri requires:
* a C++17-capable compiler (Visual Studio is recommended on Windows)
* [Conan](https://conan.io/) 2.5 or newer.
* [CMake](https://cmake.org/) 3.23 or newer.
* [Doxygen](https://www.doxygen.nl/).
* [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) 12.
* [ClangTidy](https://clang.llvm.org/extra/clang-tidy/) 10.

### Checking out the code
To start, first clone the repository using `git clone`.

### Installing Git hooks
Every PR is checked for certain code quality criteria. Some of these checks are also available as Git pre-commit hooks. It is recommended to install them after checking out the code:
```
scripts/install-git-hooks.sh
```

### Check out a branch
Before working on a change, check out a new branch:
```
git checkout -b name-of-branch
```
If your change spans into submodules, don't forget to create a branch in the submodules as well. These must be committed and pushed separately. They will also require their own pull requests.
Make sure to update the submodule reference after the submodule change has been merged into the `main` branch. Essentially, when committing and creating PRs, start at the lowest submodule in the tree and work your way up.

### Coding standards
Please refer to the [Coding standards](CODING_STANDARDS.md).

### Build the code
Building uses Conan to automatically install all required dependencies and set up CMake presets:
```
conan install . -s build_type=Release -of build-release
cmake --preset conan-default
cmake --build --preset conan-release
```
Or, to work with a debug build:
```
conan install . -s "&:build_type=Debug" -s build_type=Release -of build-debug
cmake --preset conan-default
cmake --build --preset conan-debug
```
This will build Khepri in Debug mode, but use Release builds for its dependencies.

### Run tests
After building, run the tests with CTest:
```
ctest --preset conan-release
```
Or, for the debug build:
```
ctest --preset conan-debug
```

### Create a Pull Request
After you made your changes and made sure all tests (both old and new) pass, you can commit your code with `git commit`, push it and create a pull request.

Follow the following rules:
* Only do one thing per pull request: either fix a bug, refactor code, improve performance, implement a new feature, etc. Do not combine multiple of these activities in a single pull request.
* **Squash** your changes into a single commit. This reduces noise in the repository history.
* This project requires [signed commits](https://docs.github.com/en/authentication/managing-commit-signature-verification/signing-commits).
* Format your commit message according to [Conventional Commits](https://www.conventionalcommits.org/).
* Write a Good Commit Message:
  * When fixing a bug, explain what the bug was, how it occurred and how it was fixed.
  * When implementing a feature, explain the feature at high-level, referring to the new namespaces, classes or methods if applicable. Mention known limitations, if any.
  * When improving performance, explain what caused the bad performance, and how it was improved.
  * And so on...

### Versioning
This project uses [Semantic Versioning](https://semver.org/). The [project maintainers](MAINTAINERS.md) are in charge of releasing versions. Contact them if you urgently require a new version.