# Setting up an IDE

Khepri uses [Conan 2](https://docs.conan.io/en/latest/conan_v2.html)'s layout and CMake generators to allow for seamless use with [Conan editables](https://docs.conan.io/2/tutorial/developing_packages/editable_packages.html) for multi-repo development.

## Visual Studio Code

### Install software
Install [Visual Studio Code](https://code.visualstudio.com/) and install at least the following extensions:
* [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
* [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)

### Run Conan
From a command line, go to the root of the Khepri repository and execute the following:
```
conan install . -s build_type=Debug -of build-debug
```
* "`-s build_type=Debug`" tells Conan to install a Debug configuration.
* "`-of build-debug`" tells Conan where to place all of the output files.

### Configure CMake Tools

Visual Studio Code's CMake extension can pick up the compiler settings from the Conan-generated preset file. To make sure this works, open the CMake Tool settings (via <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "CMake: Open CMake Tools Extension Settings") and ensure "Use CMake Presets" is set to "always" or "auto".

### Done

All done, now you can configure (<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "CMake: Configure"), build (<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "CMake: Build") and run tests (<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "CMake: Run Tests") as normal.

### Set up Intellisense (optional)

For Intellisense, the [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) can get the required information directly from the CMake Tools:
1. Open <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "C/C++: Change Configuration Provider..."
2. Select the current project folder.
3. Select "CMake Tools".

### Configure Clang-Tidy (optional)

If you want to run [Clang-Tidy](https://clang.llvm.org/extra/clang-tidy/) from within Visual Studio Code, the [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) supports it (via the <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> "C/C++: Run Code Analysis" options), but the extension must be configured:

1. Open the settings window (File > Preferences > Settings).
2. Go the "Folder" tab to edit project-specific settings.
5. Find and enable the "C_Cpp › Code Analysis › Clang Tidy: Use Build Path" setting.
3. Find and enable the "C_Cpp › Code Analysis › Clang Tidy: Enabled" setting.
4. Find and enable the "C_Cpp › Code Analysis: Run Automatically" setting (optional, if desired).
