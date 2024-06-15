from conan import ConanFile


class BackendRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("crowcpp-crow/1.0+5")
        self.requires("libpqxx/7.9.0")
        self.requires("openssl/3.2.2")
        self.requires("gtest/1.11.0")
    
    def build_requirements(self):
        self.tool_requires("cmake/3.22.6")