"""Setup script for epsilon."""

import fnmatch
import os
import subprocess
import sys

from distutils import log
from distutils.dep_util import newer
from distutils.spawn import find_executable
from setuptools import setup, Extension, Command
from setuptools.command.build_ext import build_ext
from setuptools.command.build_py import build_py

PROTO_DIR = "proto"
PYTHON_DIR = "python"

PROTOC = find_executable("protoc")
if PROTOC is None:
    std.stderr.write("protoc is not installed")
    sys.exit(-1)
PROTOC_PREFIX = os.path.dirname(os.path.dirname(PROTOC))

class BuildPyCommand(build_py):
    def run(self):
        self.generate_protos(PROTO_DIR, PYTHON_DIR)
        build_py.run(self)

    def generate_protos(self, src_dir, dst_dir):
        for root, dirnames, filenames in os.walk(src_dir):
            for filename in fnmatch.filter(filenames, "*.proto"):
                src_name = os.path.join(root[len(src_dir)+1:], filename)
                self.generate_proto(src_dir, src_name, dst_dir)

    def generate_proto(self, src_dir, src_name, dst_dir):
        src = os.path.join(src_dir, src_name)
        dst = os.path.join(dst_dir, src_name.replace(".proto", "_pb2.py"))

        if not newer(src, dst):
            return

        if self.verbose >= 1:
            log.info("generating %s", dst)

        cmd = [PROTOC, "-I", PROTO_DIR, src, "--python_out=" + dst_dir]
        subprocess.check_call(cmd)

class BuildExtCommand(build_ext):
    def run(self):
        self.make()
        build_ext.run(self)

    def make(self):
        cmd = "make -j".split()
        subprocess.check_call(cmd)

class CleanCommand(Command):
    user_options = []
    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        cmd = ("rm -rf ./build ./build-cc ./dist ./python/*.egg-info " +
               "./python/epsilon/*_pb2.py")
        subprocess.check_call(cmd, shell=True)

solve = Extension(
    name = "epsilon._solve",
    sources = ["python/epsilon/solvemodule.cc"],
    language = "c++",
    extra_compile_args = ["-std=c++14"],
    extra_objects = ["build-cc/libepsilon.a"],
    include_dirs = [
        os.path.join(PROTOC_PREFIX, "include"),
        "build-cc",
        "src",
        "third_party/eigen",
    ],
    library_dirs = [
        os.path.join(PROTOC_PREFIX, "lib"),
    ],
    libraries = ["protobuf", "glog"],
)

setup(
    name = "epsilon",
    version = "1.0a1",
    author = "Matt Wytock",
    url = "https://github.com/mwytock/epsilon",
    author_email = "mwytock@gmail.com",
    packages = ["epsilon"],
    package_dir = {"": PYTHON_DIR},
    ext_modules = [solve],
    install_requires = [
        "cvxpy==0.2.28",
        "protobuf==3.0.0a3"
    ],
    cmdclass = {
        "build_ext": BuildExtCommand,
        "build_py": BuildPyCommand,
        "clean": CleanCommand
    }
)
