from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.files import copy, rmdir
from conan.tools.layout import basic_layout
import os

class analyzerConan(ConanFile):
    name = "analyzer"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"
    
    default_options = {
        # Базовые настройки
        "boost/*:shared": False,
        "boost/*:header_only": False,

        "boost/*:without_program_options": False,
        
        "boost/*:without_atomic": True,
        "boost/*:without_chrono": True,
        "boost/*:without_container": True,
        "boost/*:without_context": True,
        "boost/*:without_contract": True,
        "boost/*:without_coroutine": True,
        "boost/*:without_date_time": True,
        "boost/*:without_exception": True,
        "boost/*:without_fiber": True,
        "boost/*:without_filesystem": True,
        "boost/*:without_graph": True,
        "boost/*:without_graph_parallel": True,
        "boost/*:without_iostreams": True,
        "boost/*:without_json": True,
        "boost/*:without_locale": True,
        "boost/*:without_log": True,
        "boost/*:without_math": True,
        "boost/*:without_mpi": True,
        "boost/*:without_nowide": True,
        "boost/*:without_random": True,
        "boost/*:without_regex": True,
        "boost/*:without_serialization": True,
        "boost/*:without_stacktrace": True,
        "boost/*:without_system": True,
        "boost/*:without_test": True,
        "boost/*:without_thread": True,
        "boost/*:without_timer": True,
        "boost/*:without_type_erasure": True,
        "boost/*:without_wave": True,
        "boost/*:without_url": True,
        "boost/*:without_python": True
    }
    
    def requirements(self):
        self.requires("boost/1.83.0")
        self.requires("gtest/1.13.0")
        self.requires("range-v3/0.12.0")
        self.tool_requires("cmake/3.30.0")
    
    def layout(self):
        basic_layout(self, src_folder=".", build_folder="build")
    
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        
        tc = CMakeToolchain(self)
        tc.generate()
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
