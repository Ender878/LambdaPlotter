from conan import ConanFile
from conan.tools.cmake import cmake_layout

class LambdaPlotterConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("imgui/1.92.0")
        self.requires("glfw/3.4")
        self.requires("glad/0.1.36")
        self.requires("gtest/1.16.0")

    def layout(self):
        cmake_layout(self)
