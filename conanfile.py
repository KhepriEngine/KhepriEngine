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
        ("assimp/[>=5.0 <6.0]"),
        ("cxxopts/[>=2.0 <3.0]"),
        ("fmt/[>=6.0 <7.0]"),
        ("glfw/[>=3.0 <4.0]"),
        ("gsl-lite/0.36.0"),
        ("vulkan-headers/[>=1.3]"),
    ]

    build_requires = [
        ("gtest/[>=1.0 <2.0]"),
    ]

    exports_sources = "CMakeLists.txt", "cmake/*", "include/*", "src/*", "tests/*", "tools/*"

    def build(self):
        cmake = CMake(self)

        # Propagate version info into CMake
        version_info = self._parse_version(self.version)
        if version_info:
            cmake.definitions['KHEPRI_VERSION_MAJOR'] = version_info['major']
            cmake.definitions['KHEPRI_VERSION_MINOR'] = version_info['minor']
            cmake.definitions['KHEPRI_VERSION_PATCH'] = version_info['patch']
            cmake.definitions['KHEPRI_VERSION_COMMIT'] = version_info['commit']
            cmake.definitions['KHEPRI_VERSION_CLEAN'] = str(version_info['clean']).lower()

        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["Khepri"]

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
