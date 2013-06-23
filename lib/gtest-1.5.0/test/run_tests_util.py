# Copyright 2008 Google Inc. All Rights Reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Provides facilities for running SCons-built Google Test/Mock tests."""


import optparse
import os
import re
import sets
import sys

try:
  # subrocess module is a preferable way to invoke subprocesses but it may
  # not be available on MacOS X 10.4.
  # Suppresses the 'Import not at the top of the file' lint complaint.
  # pylint: disable-msg=C6204
  import subprocess
except ImportError:
  subprocess = None

HELP_MSG = """Runs the specified tests for %(proj)s.

SYNOPSIS
       run_tests.py [OPTION]... [BUILD_DIR]... [TEST]...

DESCRIPTION
       Runs the specified tests (either binary or Python), and prints a
       summary of the results. BUILD_DIRS will be used to search for the
       binaries. If no TESTs are specified, all binary tests found in
       BUILD_DIRs and all Python tests found in the directory test/ (in the
       %(proj)s root) are run.

       TEST is a name of either a binary or a Python test. A binary test is
       an executable file named *_test or *_unittest (with the .exe
       extension on Windows) A Python test is a script named *_test.py or
       *_unittest.py.

OPTIONS
       -h, --help
              Print this help message.
       -c CONFIGURATIONS
              Specify build directories via build configurations.
              CONFIGURATIONS is either a comma-separated list of build
              configurations or 'all'. Each configuration is equivalent to
              adding 'scons/build/<configuration>/%(proj)s/scons' to BUILD_DIRs.
              Specifying -c=all is equivalent to providing all directories
              listed in KNOWN BUILD DIRECTORIES section below.
       -a
              Equivalent to -c=all
       -b
              Equivalent to -c=all with the exception that the script will not
              fail if some of the KNOWN BUILD DIRECTORIES do not exists; the
              script will simply not run the tests there. 'b' stands for
              'built directories'.

RETURN VALUE
       Returns 0 if all tests are successful; otherwise returns 1.

EXAMPLES
       run_tests.py
              Runs all tests for the default build configuration.
       run_tests.py -a
              Runs all tests with binaries in KNOWN BUILD DIRECTORIES.
       run_tests.py -b
              Runs all tests in KNOWN BUILD DIRECTORIES that have been
              built.
       run_tests.py foo/
              Runs all tests in the foo/ directory and all Python tests in
              the directory test. The Python tests are instructed to look
              for binaries in foo/.
       run_tests.py bar_test.exe test/baz_test.exe foo/ bar/
              Runs foo/bar_test.exe, bar/bar_test.exe, foo/baz_test.exe, and
              bar/baz_test.exe.
       run_tests.py foo bar test/foo_test.py
              Runs test/foo_test.py twice instructing it to look for its
              test binaries in the directories foo and bar,
              correspondingly.

KNOWN BUILD DIRECTORIES
      run_tests.py knows about directories where the SCons build script
      deposits its products. These are the directories where run_tests.py
      will be looking for its binaries. Currently, %(proj)s's SConstruct file
      defines them as follows (the default build directory is the first one
      listed in each group):
      On Windows:
              <%(proj)s root>/scons/build/win-dbg8/%(proj)s/scons/
              <%(proj)s root>/scons/build/win-opt8/%(proj)s/scons/
      On Mac:
              <%(proj)s root>/scons/build/mac-dbg/%(proj)s/scons/
              <%(proj)s root>/scons/build/mac-opt/%(proj)s/scons/
      On other platforms:
              <%(proj)s root>/scons/build/dbg/%(proj)s/scons/
              <%(proj)s root>/scons/build/opt/%(proj)s/scons/"""

IS_WINDOWS = os.name == 'nt'
IS_MAC = os.name == 'posix' and os.uname()[0] == 'Darwin'
IS_CYGWIN = os.name == 'posix' and 'CYGWIN' in os.uname()[0]

# Definition of CONFIGS must match that of the build directory names in the
# SConstruct script. The first list item is the default build configuration.
if IS_WINDOWS:
  CONFIGS = ('win-dbg8', 'win-opt8')
elif IS_MAC:
  CONFIGS = ('mac-dbg', 'mac-opt')
else:
  CONFIGS = ('dbg', 'opt')

if IS_WINDOWS or IS_CYGWIN:
  PYTHON_TEST_REGEX = re.compile(r'_(unit)?test\.py$', re.IGNORECASE)
  BINARY_TEST_REGEX = re.compile(r'_(unit)?test(\.exe)?$', re.IGNORECASE)
  BINARY_TEST_SEARCH_REGEX = re.compile(r'_(unit)?test\.exe$', re.IGNORECASE)
else:
  PYTHON_TEST_REGEX = re.compile(r'_(unit)?test\.py$')
  BINARY_TEST_REGEX = re.compile(r'_(unit)?test$')
  BINARY_TEST_SEARCH_REGEX = BINARY_TEST_REGEX


def _GetGtestBuildDir(injected_os, script_dir, config):
  """Calculates path to the Google Test SCons build directory."""

  return injected_os.path.normpath(injected_os.path.join(script_dir,
                                                         'scons/build',
                                                         config,
                                                         'gtest/scons'))


def _GetConfigFromBuildDir(build_dir):
  """Extracts the configuration name from the build directory."""

  # We don't want to depend on build_dir containing the correct path
  # separators.
  m = re.match(r'.*[\\/]([^\\/]+)[\\/][^\\/]+[\\/]scons[\\/]?$', build_dir)
  if m:
    return m.group(1)
  else:
    print >>sys.stderr, ('%s is an invalid build directory that does not '
                         'correspond to any configuration.' % (build_dir,))
    return ''


# All paths in this script are either absolute or relative to the current
# working directory, unless otherwise specified.
class TestRunner(object):
  """Provides facilities for running Python and binary tests for Google Test."""

  def __init__(self,
               script_dir,
               build_dir_var_name='GTEST_BUILD_DIR',
               injected_os=os,
               injected_subprocess=subprocess,
               injected_build_dir_finder=_GetGtestBuildDir):
    """Initializes a TestRunner instance.

    Args:
      script_dir:                File path to the calling script.
      build_dir_var_name:        Name of the env variable used to pass the
                                 the build directory path to the invoked
                                 tests.
      injected_os:               standard os module or a mock/stub for
                                 testing.
      injected_subprocess:       standard subprocess module or a mock/stub
                                 for testing
      injected_build_dir_finder: function that determines the path to
                                 the build directory.
    """

    self.os = injected_os
    self.subprocess = injected_subprocess
    self.build_dir_finder = injected_build_dir_finder
    self.build_dir_var_name = build_dir_var_name
    self.script_dir = script_dir

  def _GetBuildDirForConfig(self, config):
    """Returns the build directory for a given configuration."""

    return self.build_dir_finder(self.os, self.script_dir, config)

  def _Run(self, args):
    """Runs the executable with given args (args[0] is the executable name).

    Args:
      args: Command line arguments for the process.

    Returns:
      Process's exit code if it exits normally, or -signal if the process is
      killed by a signal.
    """

    if self.subprocess:
      return self.subprocess.Popen(args).wait()
    else:
      return self.os.spawnv(self.os.P_WAIT, args[0], args)

  def _RunBinaryTest(self, test):
    """Runs the binary test given its path.

    Args:
      test: Path to the test binary.

    Returns:
      Process's exit code if it exits normally, or -signal if the process is
      killed by a signal.
    """

    return self._Run([test])

  def _RunPythonTest(self, test, build_dir):
    """Runs the Python test script with the specified build directory.

    Args:
      test: Path to the test's Python script.
      build_dir: Path to the directory where the test binary is to be found.

    Returns:
      Process's exit code if it exits normally, or -signal if the process is
      killed by a signal.
    """

    old_build_dir = self.os.environ.get(self.build_dir_var_name)

    try:
      self.os.environ[self.build_dir_var_name] = build_dir

      # If this script is run on a Windows machine that has no association
      # between the .py extension and a python interpreter, simply passing
      # the script name into subprocess.Popen/os.spawn will not work.
      print 'Running %s . . .' % (test,)
      return self._Run([sys.executable, test])

    finally:
      if old_build_dir is None:
        del self.os.environ[self.build_dir_var_name]
      else:
        self.os.environ[self.build_dir_var_name] = old_build_dir

  def _FindFilesByRegex(self, directory, regex):
    """Returns files in a directory whose names match a regular expression.

    Args:
      directory: Path to the directory to search for files.
      regex: Regular expression to filter file names.

    Returns:
      The list of the paths to the files in the directory.
    """

    return [self.os.path.join(directory, file_name)
            for file_name in self.os.listdir(directory)
            if re.search(regex, file_name)]

  # TODO(vladl@google.com): Implement parsing of scons/SConscript to run all
  # tests defined there when no tests are specified.
  # TODO(vladl@google.com): Update the docstring after the code is changed to
  # try to test all builds defined in scons/SConscript.
  def GetTestsToRun(self,
                    args,
                    named_configurations,
                    built_configurations,
                    available_configurations=CONFIGS,
                    python_tests_to_skip=None):
    """Determines what tests should be run.

    Args:
      args: The list of non-option arguments from the command line.
      named_configurations: The list of configurations specified via -c or -a.
      built_configurations: True if -b has been specified.
      available_configurations: a list of configurations available on the
                            current platform, injectable for testing.
      python_tests_to_skip: a collection of (configuration, python test name)s
                            that need to be skipped.

    Returns:
      A tuple with 2 elements: the list of Python tests to run and the list of
      binary tests to run.
    """

    if named_configurations == 'all':
      named_configurations = ','.join(available_configurations)

    normalized_args = [self.os.path.normpath(arg) for arg in args]

    # A final list of build directories which will be searched for the test
    # binaries. First, add directories specified directly on the command
    # line.
    build_dirs = filter(self.os.path.isdir, normalized_args)

    # Adds build directories specified via their build configurations using
    # the -c or -a options.
    if named_configurations:
      build_dirs += [self._GetBuildDirForConfig(config)
                     for config in named_configurations.split(',')]

    # Adds KNOWN BUILD DIRECTORIES if -b is specified.
    if built_configurations:
      build_dirs += [self._GetBuildDirForConfig(config)
                     for config in available_configurations
                     if self.os.path.isdir(self._GetBuildDirForConfig(config))]

    # If no directories were specified either via -a, -b, -c, or directly, use
    # the default configuration.
    elif not build_dirs:
      build_dirs = [self._GetBuildDirForConfig(available_configurations[0])]

    # Makes sure there are no duplications.
    build_dirs = sets.Set(build_dirs)

    errors_found = False
    listed_python_tests = []  # All Python tests listed on the command line.
    listed_binary_tests = []  # All binary tests listed on the command line.

    test_dir = self.os.path.normpath(self.os.path.join(self.script_dir, 'test'))

    # Sifts through non-directory arguments fishing for any Python or binary
    # tests and detecting errors.
    for argument in sets.Set(normalized_args) - build_dirs:
      if re.search(PYTHON_TEST_REGEX, argument):
        python_path = self.os.path.join(test_dir,
                                        self.os.path.basename(argument))
        if self.os.path.isfile(python_path):
          listed_python_tests.append(python_path)
        else:
          sys.stderr.write('Unable to find Python test %s' % argument)
          errors_found = True
      elif re.search(BINARY_TEST_REGEX, argument):
        # This script also accepts binary test names prefixed with test/ for
        # the convenience of typing them (can use path completions in the
        # shell).  Strips test/ prefix from the binary test names.
        listed_binary_tests.append(self.os.path.basename(argument))
      else:
        sys.stderr.write('%s is neither test nor build directory' % argument)
        errors_found = True

    if errors_found:
      return None

    user_has_listed_tests = listed_python_tests or listed_binary_tests

    if user_has_listed_tests:
      selected_python_tests = listed_python_tests
    else:
      selected_python_tests = self._FindFilesByRegex(test_dir,
                                                     PYTHON_TEST_REGEX)

    # TODO(vladl@google.com): skip unbuilt Python tests when -b is specified.
    python_test_pairs = []
    for directory in build_dirs:
      for test in selected_python_tests:
        config = _GetConfigFromBuildDir(directory)
        file_name = os.path.basename(test)
        if python_tests_to_skip and (config, file_name) in python_tests_to_skip:
          print ('NOTE: %s is skipped for configuration %s, as it does not '
                 'work there.' % (file_name, config))
        else:
          python_test_pairs.append((directory, test))

    binary_test_pairs = []
    for directory in build_dirs:
      if user_has_listed_tests:
        binary_test_pairs.extend(
            [(directory, self.os.path.join(directory, test))
             for test in listed_binary_tests])
      else:
        tests = self._FindFilesByRegex(directory, BINARY_TEST_SEARCH_REGEX)
        binary_test_pairs.extend([(directory, test) for test in tests])

    return (python_test_pairs, binary_test_pairs)

  def RunTests(self, python_tests, binary_tests):
    """Runs Python and binary tests and reports results to the standard output.

    Args:
      python_tests: List of Python tests to run in the form of tuples
                    (build directory, Python test script).
      binary_tests: List of binary tests to run in the form of tuples
                    (build directory, binary file).

    Returns:
      The exit code the program should pass into sys.exit().
    """

    if python_tests or binary_tests:
      results = []
      for directory, test in python_tests:
        results.append((directory,
                        test,
                        self._RunPythonTest(test, directory) == 0))
      for directory, test in binary_tests:
        results.append((directory,
                        self.os.path.basename(test),
                        self._RunBinaryTest(test) == 0))

      failed = [(directory, test)
                for (directory, test, success) in results
                if not success]
      print
      print '%d tests run.' % len(results)
      if failed:
        print 'The following %d tests failed:' % len(failed)
        for (directory, test) in failed:
          print '%s in %s' % (test, directory)
        return 1
      else:
        print 'All tests passed!'
    else:  # No tests defined
      print 'Nothing to test - no tests specified!'

    return 0


def ParseArgs(project_name, argv=None, help_callback=None):
  """Parses the options run_tests.py uses."""

  # Suppresses lint warning on unused arguments.  These arguments are
  # required by optparse, even though they are unused.
  # pylint: disable-msg=W0613
  def PrintHelp(option, opt, value, parser):
    print HELP_MSG % {'proj': project_name}
    sys.exit(1)

  parser = optparse.OptionParser()
  parser.add_option('-c',
                    action='store',
                    dest='configurations',
                    default=None)
  parser.add_option('-a',
                    action='store_const',
                    dest='configurations',
                    default=None,
                    const='all')
  parser.add_option('-b',
                    action='store_const',
                    dest='built_configurations',
                    default=False,
                    const=True)
  # Replaces the built-in help with ours.
  parser.remove_option('-h')
  parser.add_option('-h', '--help',
                    action='callback',
                    callback=help_callback or PrintHelp)
  return parser.parse_args(argv)
