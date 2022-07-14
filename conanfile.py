from conans import ConanFile, tools
from conan.tools.cmake import CMake, cmake_layout, CMakeToolchain
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

    generators = "CMakeDeps"

    requires = [
        ("assimp/[>=5.0 <6.0]"),
        ("cxxopts/[>=2.0 <3.0]"),
        ("diligent-core/2.5.1"),
        ("fmt/9.0.0"),
        ("freetype/[>=2.0 <3.0]"),
        ("glfw/[>=3.0 <4.0]"),
        ("gsl-lite/0.36.0"),
    ]

    build_requires = [
        ("gtest/[>=1.0 <2.0]"),
    ]

    exports_sources = "CMakeLists.txt", "cmake/*", "include/*", "src/*", "tests/*", "tools/*"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        # Propagate version info into CMake
        version_info = self._parse_version(self.version)
        if version_info:
            tc.variables['KHEPRI_VERSION_MAJOR'] = version_info['major']
            tc.variables['KHEPRI_VERSION_MINOR'] = version_info['minor']
            tc.variables['KHEPRI_VERSION_PATCH'] = version_info['patch']
            tc.variables['KHEPRI_VERSION_COMMIT'] = version_info['commit']
            tc.variables['KHEPRI_VERSION_CLEAN'] = str(version_info['clean']).lower()
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["Khepri"]
        if self.settings.os == "Windows":
            self.cpp_info.system_libs = ["dxgi", "d3d11", "d3dcompiler"]
            if self.settings.build_type == "Debug":
                self.cpp_info.system_libs.append("DbgHelp")

    @staticmethod
    def _parse_version(version):
        result = re.match(r"(\d+).(\d+).(\d+)\+([a-fA-F0-9]+)(\.dirty)?", version)
        if result:
            return {
                'major': int(result.group(1)),
                'minor': int(result.group(2)),
                'patch': int(result.group(3)),
                'commit': result.group(4),
                'clean': (result.group(5) is None)
            }
