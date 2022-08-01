
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
config.name = 'APHOTIC_SHIELD'

# testFormat: The test format to use to interpret tests.
config.test_format = lit.formats.ShTest(True)

# suffixes: A list of file extensions to treat as test files. This is overriden
# by individual lit.local.cfg files in the test subdirectories.
config.suffixes = ['.cpp', '.rw']

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

# replaced %cxx_aphotic_shield by corresponding build invocation command.
config.substitutions.append( ("%expect_crash ", "not --crash ") )
config.aphotic_shield_include_dir = os.path.join(config.obj_root, '../src/runtime')
config.aphotic_shield_lib_dir = os.path.join(config.obj_root, 'lib')
config.substitutions.append(('%cxx_aphotic_shield',
    ('c++ -I%s -L%s '
     '-Wl,-whole-archive -laphotic_shield -Wl,-no-whole-archive')
    % (config.aphotic_shield_include_dir, config.aphotic_shield_lib_dir) ))
