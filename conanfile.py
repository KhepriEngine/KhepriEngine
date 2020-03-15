from conans import ConanFile, CMake, tools
import re

class KhepriEngineConan(ConanFile):
    name = "khepri"
    license = "MIT"
    homepage = "https://github.com/KhepriEngine/KhepriEngine"
    url = "https://github.com/KhepriEngine/KhepriEngine"
    description = "A general-purpose modular game engine for C++"

    def set_version(self):
        git = tools.Git(folder=self.recipe_folder)

        self.version = "0.0.0"
        try:
            GIT_SHORT_HASH_LENGH=12
            latest_tag = git.run("describe --tags --abbrev=0").strip()
            result = re.match(r"v?(\d+.\d+.\d+)", latest_tag)
            if result:
                self.version = "{}".format(result.group(1))
        except Exception:
            pass

        try:
            # Store the short Git version because Conan has a limit of the length of the version string
            self.version += "+{}".format(git.get_revision()[:GIT_SHORT_HASH_LENGH])
            if not git.is_pristine():
                self.version += ".dirty"
        except Exception:
            pass

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    generators = "cmake_find_package"

    requires = [
        ("fmt/[>=6.0 <7.0]"),
        ("glfw/[>=3.0 <4.0]"),
        ("gsl-lite/0.36.0"),
        ("vulkan-headers/[>=1.3]"),
    ]

    exports_sources = "CMakeLists.txt", "include/*", "src/*"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["Khepri"]
