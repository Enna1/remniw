
# Configuration file for the 'lit' test runner.

import os
import sys
import re
import platform
import subprocess

import lit.util
import lit.formats
from lit.llvm import llvm_config
from lit.llvm.subst import FindTool
from lit.llvm.subst import ToolSubst

# name: The name of this test suite.
config.name = 'REMNIW'

# testFormat: The test format to use to interpret tests.
config.test_format = lit.formats.ShTest(True)

# suffixes: A list of file extensions to treat as test files. This is overriden
# by individual lit.local.cfg files in the test subdirectories.
config.suffixes = ['.rw']

# excludes: A list of directories to exclude from the testsuite. The 'Inputs'
# subdirectories contain auxiliary inputs for various tests in their parent
# directories.
config.excludes = ['Inputs', 'CMakeLists.txt', 'README.txt', 'LICENSE.txt']

# test_source_root: The root path where tests are located.
config.test_source_root = os.path.dirname(__file__)

# test_exec_root: The root path where tests should be run.
config.test_exec_root = os.path.join(config.obj_root, 'test')

# replaced %remniw -emit-llvm by the path to the tool executable.
config.substitutions.append(('%remniw',
    os.path.join(config.obj_root, 'bin/remniw')))

# replaced %remniw-llc by the path to the tool executable.
config.substitutions.append(('%remniw-llc',
    os.path.join(config.obj_root, 'bin/remniw-llc')))

# replaced %riscv-cc by the path to
riscv_cc_path = None
if not riscv_cc_path:
  riscv_cc_path = lit.util.which('riscv64-unknown-linux-gnu-gcc')
if not riscv_cc_path:
  riscv_cc_path = lit.util.which('riscv64-linux-gnu-gcc')
config.substitutions.append(('%riscv-cc', ' %s %s ' % (riscv_cc_path, '-static')))
