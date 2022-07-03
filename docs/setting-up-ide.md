# Setting up an IDE

Khepri uses [Conan 2.0](https://docs.conan.io/en/latest/conan_v2.html)'s layout and CMake generators to allow for seamless use with [Conan editables](https://docs.conan.io/en/latest/developing_packages/editable_packages.html) for multi-repo development.

> **NOTE**: at the time of writing, [Conan workspaces](https://docs.conan.io/en/latest/developing_packages/workspaces.html) are not mature enough to support Khepri.

## Visual Studio Code

### Install software
Install [Visual Studio Code](https://code.visualstudio.com/) and install at least the following extensions:
* [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
* [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)

### Run Conan
From a command line, go to the root of the Khepri repository and execute the following:
```
conan install . -s build_type=Debug --install-folder cmake-build-debug -c tools.cmake.cmaketoolchain:generator=Ninja
```
* "`-s build_type=Debug`" tells Conan to install a Debug configuration.
* "`--install-folder cmake-build-debug`" tells Conan where to place the output files. `cmake-build-debug` (or `cmake-build-release` for release builds) is STRONGLY recommended as the dependency information is written to that folder regardless of what is specified here.
* "`-c tools.cmake.cmaketoolchain:generator=Ninja`" makes Conan's CMake tools uses Ninja as build system, allowing the generation of a [`compile_commands.json`](https://clang.llvm.org/docs/JSONCompilationDatabase.html) file that Visual Studio Code can use to [configure Intellisense](https://code.visualstudio.com/docs/cpp/faq-cpp#_how-do-i-get-intellisense-to-work-correctly).

Next, Visual Studio Code has to be configured.

### Define a custom kit
By default, Visual Studio Code's CMake extension scans the system for compilers and stores their configuration as Kits. However, Conan already stored that information in its generated toolchain file (derived from the active [Conan profile](https://docs.conan.io/en/latest/reference/profiles.html). CMake must use this configuration to guarantee that our project is built with the same compiler settings as the project's dependencies were. So, we must define a project-local CMake kit that provides the Conan-generated toolchain to CMake.

1. Create `.vscode/cmake-kits.json` in the repository with the following contents:
    ```
    [
        {
            "name": "Conan",
            "toolchainFile": "${workspaceFolder}/cmake-build-${variant:buildType}/conan/conan_toolchain.cmake",
            "visualStudio": "e43a52b5",
            "visualStudioArchitecture": "x64"
        }
    ]
    ```
    The value for "`toolchainFile`" must be specified exactly as above.

    Only if building using a compiler from Visual Studio, then "`visualStudio`" and "`visualStudioArchitecture`" are necessary for Visual Studio Code to run CMake in the proper Visual Studio environment. To find the values for these properties for your machine it's recommended to let CMake scan for kits (i.e. run <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "CMake: Scan for Kits") and then copy the value from the desired kit (via <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "CMake: Edit User-Local CMake Kits").

    It's **crucial** to select the same Visual Studio environment as was set in the Conan profile.

2. Restart Visual Studio Code so it loads the new kit.
3. Select the "Conan" kit as the active kit (via <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "CMake: Select a Kit").

### Override the default build directory
By default, Visual Studio Code's CMake extension configures CMake to build in the `build` folder. Since we use `cmake-build-debug` et al., we must tell Visual Studio Code about this.

1. Open the settings window (File > Preferences > Settings).
2. Go to the "Folder" tab to edit project-specific settings.
3. Find the "CMake: Build Directory" setting.
4. Input `${workspaceFolder}/cmake-build-${variant:buildType}`

### Define the build variants
By default, Visual Studio Code defines build variants that are slightly incompatible with Conan's generated files, so we must tell Visual Studio Code what to use.

1. Create a file `.vscode/cmake-variants.yaml` in the repository with the following contents:
    ```
    buildType:
      default: debug
      choices:
        debug:
          short: debug
          long: Add debug information.
          buildType: Debug
        release:
          short: release
          long: Optimize generated code.
          buildType: Release
        relwithdebinfo:
          short: relwithdebinfo
          long: Optimize generated code and add debug information.
          buildType: RelWithDebInfo
        minsizerel:
          short: minsizerel
          long: Optimize generated code and minimize the size.
          buildType: MinSizeRel
    ```
2. Select "debug" (or whatever variant you're building) as the active variant (via <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "CMake: Select Variant").

    This will also trigger a CMake configuration.

### Done

All done, now you can configure (<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "CMake: Configure"), build (<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "CMake: Build") and run tests (<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "CMake: Run Tests") as normal.

### Set up Intellisense (optional)

For Intellisense, Visual Studio Code expects a `compile_commands.json` file. It can find this on its own after the first build, but the downside is that it's then set to first build variant that you build (e.g. `cmake-build-debug/compile_commands.json`). To make IntelliSense work for _all_ build variants, we must tell Visual Studio Code to create a _merged_ `compile_commands.json` after running CMake and use that one, instead.

1. Open the settings window (File > Preferences > Settings).
2. Go the "Folder" tab to edit project-specific settings.
3. Find the "CMake: Merged Compile Commands" setting.
4. Input `${workspaceFolder}/cmake-build-merged/compile_commands.json`. This will instruct Visual Studio Code to merge all compile commands that it finds into `build/compile_commands.json`.
5. Next, open the project's C/C++ properties (<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "C/C++: Edit Configurations (UI)").
6. In the "Advanced Settings" section find the setting "Compile Commands".
7. Input "`${workspaceFolder}/cmake-build-merged/compile_commands.json`". It may complain this file does not exist (yet).

### Configure Clang-Tidy (optional)

If you want to run [Clang-Tidy](https://clang.llvm.org/extra/clang-tidy/) from within Visual Studio Code, the [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) supports it (via the <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "C/C++: Run Code Analysis" options), but the extension must be configured:

1. Open the settings window (File > Preferences > Settings).
2. Go the "Folder" tab to edit project-specific settings.
5. Find and enable the "C_Cpp › Code Analysis › Clang Tidy: Use Build Path" setting.
3. Find and enable the "C_Cpp › Code Analysis › Clang Tidy: Enabled" setting.
4. Find and enable the "C_Cpp › Code Analysis: Run Automatically" setting (optional, if desired).
