# -*- coding: utf-8 -*-

from __future__ import with_statement

import logging
import os
import platform
import re
import shlex
import shutil
import subprocess
import sys

import SCons
from SCons import Script

from . import util


class MixxxBuild(object):

    def __init__(self, target, machine, build, toolchain, available_features):
        self.available_features = available_features
        self.host_platform = self.detect_platform()
        self.host_machine = self.detect_machine()
        self.flags = {}

        if target is None:
            target = self.host_platform

        if machine is None:
            machine = self.host_machine

        if toolchain is None:
            if self.host_platform == 'windows':
                raise Exception(
                    'must specify toolchain on Windows (msvs or gnu)')
            else:
                toolchain = 'gnu'

        if build is None:
            build = 'debug'

        if not build in ['debug', 'release']:
            raise Exception("invalid build type")

        if target not in ['windows', 'osx', 'linux', 'bsd']:
            raise Exception("invalid target platform")

        if machine.lower() not in ['x86_64', 'x86', 'i686', 'i586',
                                   'alpha', 'hppa', 's390',
                                   'sparc', 'ia64', 'armel', 'armhf', 'hurd-i386',
                                   'armv5tel', 'armv5tejl', 'armv6l', 'armv6hl',
                                   'armv7l', 'armv7hl', 'armv7hnl',
                                   'sh3', 'sh4',
                                   'kfreebsd-amd64', 'kfreebsd-i386',
                                   'i486', 'i386', 'ppc', 'ppc64', 'powerpc',
                                   'powerpc64', 'powerpcspe', 's390x',
                                   'amd64', 'em64t', 'intel64', 'arm64',
                                   'ppc64el', 'ppc64le', 'm68k', 
                                   'mips', 'mipsel', 'mipsr6', 'mipsr6el',
                                   'mips64', 'mips64r6', 'mips64el', 'mips64r6el', 
                                   'mipsn32', 'mipsn32el', 'mipsn32r6', 'mipsn32r6el',
                                   'mipsisa32r6', 'mipsisa32r6el', 'mipsisa64r6', 'mipsisa64r6el',
                                   'aarch64']:
            raise Exception("invalid machine type")

        if toolchain not in ['gnu', 'msvs']:
            raise Exception('invalid toolchain type')

        if toolchain == 'msvs' and self.host_platform != 'windows':
            raise Exception(
                'cannot use msvs toolchain on non-windows platform')

        self.platform = target
        self.platform_is_posix = self.platform in ['linux', 'osx', 'bsd']
        self.platform_is_linux = self.platform == 'linux'
        self.platform_is_osx = self.platform == 'osx'
        self.platform_is_bsd = self.platform == 'bsd'
        self.platform_is_windows = self.platform == 'windows'

        self.machine = machine
        self.build = build
        self.build_is_debug = build == 'debug'
        self.build_is_release = build == 'release'

        self.toolchain = toolchain
        self.toolchain_is_gnu = self.toolchain == 'gnu'
        self.toolchain_is_msvs = self.toolchain == 'msvs'

        self.crosscompile = self.host_platform != self.platform

        flags_force32 = int(Script.ARGUMENTS.get('force32', 0))
        flags_force64 = int(Script.ARGUMENTS.get('force64', 0))
        if flags_force32 and flags_force64:
            logging.error('Both force32 and force64 cannot be enabled at once')
            Script.Exit(1)

        if flags_force32:
            if self.machine in ['powerpc', 'powerpc64', 'ppc', 'ppc64']:
                self.machine = 'powerpc'
            else:
                self.machine = 'x86'
        elif flags_force64:
            if self.machine in ['powerpc', 'powerpc64', 'ppc', 'ppc64']:
                self.machine = 'powerpc64'
            else:
                self.machine = 'x86_64'
        self.machine_is_64bit = self.machine.lower(
            ) in ['x86_64', 'powerpc64', 'ppc64', 'amd64', 'em64t', 'intel64']
        self.bitwidth = 64 if self.machine_is_64bit else 32
        self.architecture_is_x86 = self.machine.lower(
            ) in ['x86', 'x86_64', 'i386', 'i486', 'i586', 'i686', 'em64t',
                  'intel64', 'amd64']
        self.architecture_is_powerpc = self.machine.lower(
            ) in ['powerpc', 'powerpc64', 'ppc', 'ppc64']
        self.architecture_is_arm = self.machine.lower().startswith('arm')

        self.build_dir = util.get_build_dir(self.platform, self.bitwidth)

        # Currently this only works for Windows
        self.static_dependencies = int(Script.ARGUMENTS.get('staticlibs', 0))
        self.static_qt = int(Script.ARGUMENTS.get('staticqt', 0))

        self.bundle_pdbs = self.platform_is_windows and (
            self.build_is_debug or Script.ARGUMENTS.get('bundle_pdbs', '') in ('yes', 'y', '1'))

        self.scons_version = (
            'unknown' if SCons.__version__.startswith('__VERSION')
            else '%s (%s, %s)' % (
                SCons.__version__,
                SCons.__build__,
                SCons.__date__,
        ))
        logging.info("SCons version: %s" % self.scons_version)
        logging.info("Python version: %s" % sys.version.replace('\n', ''))
        logging.info("Target Platform: %s" % self.platform)
        logging.info("Target Machine: %s" % self.machine)
        logging.info("Build: %s" % self.build)
        logging.info("Toolchain: %s" % self.toolchain)
        logging.info(
            "Crosscompile: %s" % ("YES" if self.crosscompile else "NO"))
        if self.platform_is_windows:
            self.winlib_path = Script.ARGUMENTS.get('winlib', '')
            logging.info("Winlib Path: %s" % self.winlib_path)
            logging.info("Static dependencies: %s" % (
                "YES" if self.static_dependencies else "NO"))
            logging.info("Static Qt: %s" % (
                "YES" if self.static_qt else "NO"))

        if self.crosscompile:
            logging.info("Host Platform: %s" % self.host_platform)
            logging.info("Host Machine: %s" % self.host_machine)

        if self.crosscompile and self.host_platform != 'linux':
            raise Exception(
                'Cross-compiling on a non-Linux host not currently supported')

        tools = ['default', 'qt5', 'protoc']
        toolpath = ['#build/']
        extra_arguments = {}
        from . import depends
        if self.machine_is_64bit:
            default_qtdir = depends.Qt.DEFAULT_QT5DIRS64.get(self.platform, '')
        else:
            default_qtdir = depends.Qt.DEFAULT_QT5DIRS32.get(self.platform, '')

        # Try fallback to pkg-config on Linux
        if not os.path.isdir(default_qtdir) and self.platform == 'linux':
            pkg_config_cmd = ['pkg-config', '--variable=includedir', 'Qt5Core']
            try:
                output = subprocess.check_output(pkg_config_cmd)
            except OSError:
                # pkg-config is not installed
                pass
            except subprocess.CalledProcessError:
                # pkg-config failed to find Qt5Core
                pass
            else:
                default_qtdir = output.decode('utf-8').rstrip()

        # Ugly hack to check the qtdir argument
        qtdir = Script.ARGUMENTS.get(
            'qtdir', os.environ.get('QTDIR', default_qtdir)).rstrip()

        # Validate the specified qtdir exists
        if not os.path.isdir(qtdir):
            logging.error("Qt path (%s) does not exist or Qt is not installed." % qtdir)
            logging.error(
                "Please specify your Qt path by running 'scons qtdir=[path]'")
            Script.Exit(1)
        # And that it doesn't contain qt3 or qt4
        elif 'qt3' in qtdir or 'qt/3' in qtdir or 'qt4' in qtdir:
            logging.error("Mixxx now requires Qt 5. Please set the qtdir build flag to the path to your Qt 5 installation.")
            Script.Exit(1)
        logging.info("Qt path: %s" % qtdir)
        extra_arguments['QTDIR'] = qtdir

        if self.platform_is_osx:
            tools.append('OSConsX')
            toolpath.append('#/build/osx/')
        if self.platform_is_windows and self.toolchain_is_msvs:
            # NOTE(rryan): Don't use the SCons mssdk tool since it does not
            # support x64.
            # In SConscript.env we use the MSVS tool to let you generate a
            # Visual Studio solution. Consider removing this.
            tools.extend(['msvs', 'signtool'])
            toolpath.append('#/build/windows/')
            # SCons's built-in Qt tool attempts to link 'qt' into your binary if
            # you don't do this.
            extra_arguments['QT_LIB'] = ''
            # Causes SCons to bypass MSVC environment detection altogether
            # and depend on environment variables.
            # TODO(rryan): Expose SCons MSVC auto-detection options.
            extra_arguments['MSVC_USE_SCRIPT'] = None

        # Setup the appropriate toolchains for cross-compiling
        if self.crosscompile:
            if self.platform_is_windows:
                tools.append('crossmingw')
            if self.platform_is_osx:
                tools.append('crossosx')

        self.env = Script.Environment(
            tools=tools, toolpath=toolpath, ENV=os.environ,
            **extra_arguments)
        self.read_environment_variables()

        # Now that environment variables have been read, we can detect the compiler.
        if self.toolchain_is_msvs:
            self.compiler_is_gcc = False
            self.compiler_is_clang = False
        else:
            cc_version_cmd = shlex.split(self.env['CC']) + ['--version']
            cc_version = subprocess.check_output(cc_version_cmd).decode('utf-8')
            self.compiler_is_gcc = 'gcc' in cc_version.lower()
            self.compiler_is_clang = 'clang' in cc_version.lower()

            # Determine the major compiler version (only GCC)
            if self.compiler_is_gcc:
                self.gcc_major_version = None
                gcc_version_cmd = shlex.split(self.env['CC']) + ['-dumpversion']
                gcc_version = subprocess.check_output(gcc_version_cmd).decode('utf-8')
                # If match is None we don't know the version.
                if not gcc_version is None:
                    version_split = gcc_version.split('.')
                    if version_split:
                        self.gcc_major_version = int(version_split[0])

        self.virtualize_build_dir()

        if self.toolchain_is_gnu:
            if flags_force32:
                self.env.Append(CCFLAGS='-m32')
            elif flags_force64:
                self.env.Append(CCFLAGS='-m64')

        self.setup_platform_sdk()

        if self.platform_is_osx:
            if self.architecture_is_powerpc:
                self.env.Append(CCFLAGS='-arch ppc')
                self.env.Append(LINKFLAGS='-arch ppc')
            else:
                if self.bitwidth == 32:
                    self.env.Append(CCFLAGS='-arch i386')
                    self.env.Append(LINKFLAGS='-arch i386')
                elif self.bitwidth == 64:
                    self.env.Append(CCFLAGS='-arch x86_64')
                    self.env.Append(LINKFLAGS='-arch x86_64')

        if self.crosscompile:
            crosscompile_root = Script.ARGUMENTS.get('crosscompile_root', '')

            if crosscompile_root == '':
                print("Your build setup indicates this is a cross-compile, but you did not specify 'crosscompile_root', which is required.")
                Script.Exit(1)

            crosscompile_root = os.path.abspath(crosscompile_root)
            self.env.Append(CPPPATH=os.path.join(crosscompile_root, 'include'))
            self.env.Append(LIBPATH=os.path.join(crosscompile_root, 'lib'))
            self.env.Append(LIBPATH=os.path.join(crosscompile_root, 'bin'))

        self.install_options()

    def detect_platform(self):
        if os.name == 'nt' or sys.platform == 'win32':
            return 'windows'
        # Should cover {Net,Open,Free,DragonFly}BSD, but only tested on OpenBSD
        if 'bsd' in sys.platform:
            return 'bsd'
        if sys.platform.startswith('linux'):
            return 'linux'
        if sys.platform.startswith('darwin'):
            return 'osx'
        logging.error("Couldn't determine platform. os.name: %s sys.platform: %s"
                      % (os.name, sys.platform))
        return 'invalid'

    def detect_machine(self):
        return platform.machine()

    def setup_platform_sdk(self):
        if self.platform_is_windows:
            self.setup_windows_platform_sdk()
        elif self.platform_is_osx:
            self.setup_osx_platform_sdk()

    def setup_windows_platform_sdk(self):
        mssdk_dir = Script.ARGUMENTS.get('mssdk_dir', None)
        if mssdk_dir is None:
            print("Skipping Windows SDK setup because no SDK path was specified.")
            print("Specify the path to your platform SDK with mssdk_dir.")
            return
        env_update_tuples = []
        include_path = os.path.join(mssdk_dir, 'Include')

        if not os.path.exists(include_path):
            raise Exception('No "Include" subfolder exists in the specified mssdk_dir.')
        env_update_tuples.append(('INCLUDE', include_path))
        mfc_path = os.path.join(include_path, 'mfc')
        if os.path.exists(mfc_path):
            env_update_tuples.append(('INCLUDE', mfc_path))
        atl_path = os.path.join(include_path, 'atl')
        if os.path.exists(atl_path):
            env_update_tuples.append(('INCLUDE', atl_path))

        bin_path = os.path.join(mssdk_dir, 'Bin')
        if self.machine_is_64bit:
            bin_path = os.path.join(bin_path, 'x64')
        if not os.path.exists(bin_path):
            raise Exception('No "Bin" subfolder exists in the specified mssdk_dir.')
        env_update_tuples.append(('PATH', bin_path))

        lib_path = os.path.join(mssdk_dir, 'Lib')
        if self.machine_is_64bit:
            lib_path = os.path.join(lib_path, 'x64')
        if not os.path.exists(lib_path):
            raise Exception('No "Lib" subfolder exists in the specified mssdk_dir.')
        env_update_tuples.append(('LIB', lib_path))
        env_update_tuples.append(('LIBPATH', lib_path))

        for variable, directory in env_update_tuples:
            self.env.PrependENVPath(variable, directory)


    def setup_osx_platform_sdk(self):
        sysroot = Script.ARGUMENTS.get('sysroot', '')
        if sysroot:
            self.env.Append(CCFLAGS=['-isysroot', sysroot])

        # If no sysroot was specified, pick one automatically. The only platform
        # we pick one automatically on is OS X.
        if self.platform_is_osx:
            if '-isysroot' in self.env['CXXFLAGS'] or '-isysroot' in self.env['CFLAGS']:
                print("Skipping OS X automatic sysroot selection because -isysroot is in your CCFLAGS.")
                return

            print("Automatically detecting Mac OS X SDK.")


            # Returns a version like "10.8.0". We strip off the last ".0".
            osx_min_version = util.get_osx_min_version()
            assert osx_min_version.endswith('.0')
            osx_min_version = osx_min_version[:len(osx_min_version) - 2]
            osx_stdlib = 'libc++'

            print("XCode developer directory:" + os.popen('xcode-select -p').readline().strip())

            available_sdks = []
            macosx_matcher = re.compile(r'^MacOSX\d+\.\d+\.sdk.*\((.*)\)$')
            for line in os.popen('xcodebuild -version -sdk'):
                match = macosx_matcher.match(line)
                if not match:
                    continue
                version = match.group(1)
                print("Found OS X SDK:"+ version)
                available_sdks.append(version)

            def version_sorter(version):
                assert version.startswith('macosx')
                major_version, minor_version = version.replace('macosx', '').split('.')
                return int(major_version), int(minor_version)

            # Use the latest SDK.
            for sdk in sorted(available_sdks, reverse=True, key=version_sorter):
                sdk_path = os.popen(
                    'xcodebuild -version -sdk %s Path' % sdk).readline().strip()
                if sdk_path:
                    print("Automatically selected OS X SDK:" + sdk_path)

                    common_flags = ['-isysroot', sdk_path,
                                    '-mmacosx-version-min=%s' % osx_min_version,
                                    '-stdlib=%s' % osx_stdlib]
                    link_flags = [
                        '-Wl,-syslibroot,' + sdk_path,
                    ]
                    self.env.Append(CCFLAGS=common_flags)
                    self.env.Append(LINKFLAGS=common_flags + link_flags)
                    return

                print("Could not find a supported Mac OS X SDK.")
                print("Make sure that XCode is installed, you have installed "
                      "the command line tools, and have selected an SDK path with "
                      "xcode-select.")

    def read_environment_variables(self):
        # Import environment variables from the terminal. Note that some
        # variables correspond to variables inside the SCons env with different
        # names, eg. the shell's "CFLAGS" ---> SCons' "CCFLAGS".
        if 'CC' in os.environ:
            self.env['CC'] = os.environ['CC']
        if 'CFLAGS' in os.environ:
            self.env['CCFLAGS'] += SCons.Util.CLVar(os.environ['CFLAGS'])
        if 'CXX' in os.environ:
            self.env['CXX'] = os.environ['CXX']
        if 'CXXFLAGS' in os.environ:
            self.env['CXXFLAGS'] += SCons.Util.CLVar(os.environ['CXXFLAGS'])
        if 'LDFLAGS' in os.environ:
            self.env['LINKFLAGS'] += SCons.Util.CLVar(os.environ['LDFLAGS'])

        # Allow installation directories to be specified.
        prefix = Script.ARGUMENTS.get('prefix', '/usr/local')
        if 'LIBDIR' in os.environ:
            self.env['LIBDIR'] = os.path.relpath(os.environ['LIBDIR'], prefix)
        if 'BINDIR' in os.environ:
            self.env['BINDIR'] = os.path.relpath(os.environ['BINDIR'], prefix)
        if 'SHAREDIR' in os.environ:
            self.env['SHAREDIR'] = \
                os.path.relpath(os.environ['SHAREDIR'], prefix)

        # Initialize this as a list, fixes a bug where first CPPDEFINE would get
        # mangled
        self.env['CPPDEFINES'] = []
        self.env['LIBS'] = []
        self.env['LIBPATH'] = []

    def get_cache_dir(self):
        # Global cache directory Put all project files in it so a rm -rf cache
        # will clean up the config
        if 'CACHEDIR' not in self.env:
            self.env['CACHEDIR'] = str(Script.Dir('#cache/'))
        if not os.path.isdir(self.env['CACHEDIR']):
            os.mkdir(self.env['CACHEDIR'])

        ## Avoid spreading .sconsign files everywhere
        # env.SConsignFile(env['CACHEDIR']+'/scons_signatures')
        ## WARNING - We found that the above line causes SCons to randomly not find
        ##           dependencies for some reason. It might not happen right away, but
        # a good number of users found that it caused weird problems - Albert
        # (May 15/08)

        return str(self.env['CACHEDIR'])

    def install_options(self):
        cachefile = os.path.join(self.get_cache_dir(), 'custom.py')
        vars = Script.Variables(cachefile)
        vars.Add('prefix', 'Set to your install prefix', '/usr/local')
        vars.Add('virtualize',
                 'Dynamically swap out the build directory when switching Git branches.', 1)
        vars.Add('qtdir', 'Set to your Qt 5 directory', '/usr/share/qt5')
        vars.Add('qt_sqlite_plugin', 'Set to 1 to package the Qt SQLite plugin.'
                 '\n           Set to 0 if SQLite support is compiled into QtSQL.', 0)
        vars.Add('target',
                 'Set the build target for cross-compiling (windows, osx, linux, bsd).', '')
        vars.Add(
            'machine', 'Set the machine type for cross-compiling (x86_64, x86, powerpc, powerpc64).', '')
        vars.Add('toolchain',
                 'Specify the toolchain to use for building (gnu, msvs). Default is gnu.', 'gnu')
        vars.Add('crosscompile_root',
                 'Set the path to the root of a cross-compile sandbox.', '')
        vars.Add('force32', 'Force a 32-bit compile', 0)
        vars.Add('force64', 'Force a 64-bit compile', 0)
        vars.Add('sysroot', 'Specify a custom sysroot', '')
        vars.Add('debug_assertions_fatal',
                 'Whether debug assertions are fatal.', False)

        for feature_class in self.available_features:
            # Instantiate the feature
            feature = feature_class()

            # Add the feature to the feature list
            feature.add_options(self, vars)

        vars.Update(self.env)
        Script.Help(vars.GenerateHelpText(self.env))

        # Save the options to cache
        vars.Save(cachefile, self.env)

    def virtualize_build_dir(self):
        # WARNING: Do not use SCons self.env.SConsignFile to change the location
        # of .sconsign.dblite or turn build_dir into a symlink. It will mostly
        # seem to work fine but eventually cause strange build issues (not
        # re-building a necessary object file, etc.) and cause instability.
        # See also: asantoni's warning in get_cache_dir. rryan 6/2013
        should_virtualize = int(Script.ARGUMENTS.get('virtualize', 1))
        if not should_virtualize:
            return

        branch_name = util.get_branch_name()
        if not branch_name:
            # happens in case of tarball builds, detached HEADs have
            # branch_name = '(no branch)'
            return

        # TODO(rryan) what other branch name characters aren't allowed in
        # filenames?
        branch_name = re.sub('[/<>|"]', '_', branch_name).lower()

        cache_dir = self.get_cache_dir()
        branch_build_dir = os.path.join(cache_dir, branch_name)
        virtual_build_dir = os.path.join(branch_build_dir, self.build_dir)
        virtual_sconsign_file = os.path.join(
            branch_build_dir, 'sconsign.dblite')
        virtual_sconf_temp_dir = os.path.join(
            branch_build_dir, 'sconf_temp')
        virtual_custom_file = os.path.join(branch_build_dir, 'custom.py')
        old_branch_build_dir = ''
        old_virtual_build_dir = ''
        old_virtual_sconsign_file = ''
        old_virtual_sconf_temp_dir = ''
        old_virtual_custom_file = ''

        # Clean up symlinks from our original method of virtualizing.
        if os.path.islink(self.build_dir):
            print("os.unlink " + self.build_dir)
            os.unlink(self.build_dir)

        sconsign_file = '.sconsign.dblite'
        sconsign_branch_file = '.sconsign.branch'  # contains the branch name of last build
        sconf_temp_dir = '.sconf_temp'  # contains the configure test files
        custom_file = 'cache/custom.py'  # contains custom build flags
        sconsign_branch = ''
        is_branch_different = True
        if os.path.isfile(sconsign_branch_file):
            with open(sconsign_branch_file, 'r') as f:
                sconsign_branch = f.readline()
                sconsign_branch = sconsign_branch.strip()

        # Check if there was a checkout of a different branch since the last
        # build.
        is_branch_different = sconsign_branch != branch_name
        if not is_branch_different:
            # nothing to do
            return

        print("branch has changed %s -> %s" % (sconsign_branch, branch_name))

        if sconsign_branch:
            old_branch_build_dir = os.path.join(cache_dir, sconsign_branch)
            old_virtual_build_dir = os.path.join(
                old_branch_build_dir, self.build_dir)
            old_virtual_sconsign_file = os.path.join(
                old_branch_build_dir, 'sconsign.dblite')
            old_virtual_sconf_temp_dir = os.path.join(
                old_branch_build_dir, 'sconf_temp')
            old_virtual_custom_file = os.path.join(
                old_branch_build_dir, 'custom.py')
            if os.path.isdir(self.build_dir):
                if os.path.isdir(old_virtual_build_dir):
                    raise Exception("%s already exists. "
                                    "build virtualization cannot continue. Please "
                                    "move or delete it." % old_virtual_build_dir)
                print("shutil.move %s -> %s" % (self.build_dir, old_virtual_build_dir))
                # move build dir from last build to cache, named with the old
                # branch name
                shutil.move(self.build_dir, old_virtual_build_dir)

            if os.path.isfile(sconsign_file):
                print("shutil.move %s -> %s" % (sconsign_file, old_virtual_sconsign_file))
                # move sconsdign.dblite as well
                shutil.move(sconsign_file, old_virtual_sconsign_file)

            if os.path.isdir(sconf_temp_dir):
                print("shutil.move %s -> %s" % (sconf_temp_dir, old_virtual_sconf_temp_dir))
                # move sconf_temp dir as well
                shutil.move(sconf_temp_dir, old_virtual_sconf_temp_dir)

            if os.path.isfile(custom_file):
                print("shutil.move %s -> %s" % (custom_file, old_virtual_custom_file))
                # and move custom.py
                shutil.move(custom_file, old_virtual_custom_file)

            # all files are saved now so remove .sconsign.branch file
            # to avoid a new copy after an exception below
            os.remove(sconsign_branch_file)

        # Now there should be no folder self.build_dir, .sconsign.dblite, or
        # .sconf_temp directory.
        if os.path.isdir(branch_build_dir):
            if os.path.isdir(virtual_build_dir):
                # found a build_dir in cache from a previous build
                if os.path.isdir(self.build_dir):
                    raise Exception('%s exists without a .sconsign.branch file so '
                                    'build virtualization cannot continue. Please '
                                    'move or delete it.' % self.build_dir)
                print("shutil.move %s -> %s" % (virtual_build_dir, self.build_dir))
                shutil.move(virtual_build_dir, self.build_dir)
            if os.path.isfile(virtual_sconsign_file):
                if os.path.isfile(sconsign_file):
                    raise Exception('%s exists without a .sconsign.branch file so '
                                    'build virtualization cannot continue. Please '
                                    'move or delete it.' % sconsign_file)
                print("shutil.move %s -> %s" % (virtual_sconsign_file, sconsign_file))
                shutil.move(virtual_sconsign_file, sconsign_file)
            if os.path.isdir(virtual_sconf_temp_dir):
                if os.path.isdir(sconf_temp_dir):
                    raise Exception('%s exists without a .sconsign.branch file so '
                                    'build virtualization cannot continue. Please '
                                    'move or delete it.' % sconf_temp_dir)
                print("shutil.move %s -> %s" % (virtual_sconf_temp_dir, sconf_temp_dir))
                shutil.move(virtual_sconf_temp_dir, sconf_temp_dir)
            if os.path.isfile(virtual_custom_file):
                if os.path.isfile(custom_file):
                    raise Exception('%s exists without a .sconsign.branch file so '
                                    'build virtualization cannot continue. Please '
                                    'move or delete it.' % custom_file)
                print("shutil.move %s -> %s" % (virtual_custom_file, custom_file))
                shutil.move(virtual_custom_file, custom_file)
        else:
            # no cached build dir found, assume this is a branch from the old branch
            # if not, no problem because scons will rebuild all changed files in any case
            # copy the old_virtual_dir back
            if sconsign_branch:
                if os.path.isdir(old_virtual_build_dir):
                    if os.path.isdir(self.build_dir):
                        raise Exception('%s exists without a .sconsign.branch file so '
                                        'build virtualization cannot continue. Please '
                                        'move or delete it.' % self.build_dir)
                    print("shutil.copytree %s -> %s" % (old_virtual_build_dir, self.build_dir))
                    shutil.copytree(old_virtual_build_dir, self.build_dir)
                if os.path.isfile(old_virtual_sconsign_file):
                    if os.path.isfile(sconsign_file):
                        raise Exception('%s exists without a .sconsign.branch file so '
                                        'build virtualization cannot continue. Please '
                                        'move or delete it.' % sconsign_file)
                    print("shutil.copy %s -> %s" % (old_virtual_sconsign_file, sconsign_file))
                    shutil.copy(old_virtual_sconsign_file, sconsign_file)
                if os.path.isdir(old_virtual_sconf_temp_dir):
                    if os.path.isdir(sconf_temp_dir):
                        raise Exception('%s exists without a .sconsign.branch file so '
                                        'build virtualization cannot continue. Please '
                                        'move or delete it.' % sconf_temp_dir)
                    print("shutil.copytree %s -> %s" % (old_virtual_sconf_temp_dir, sconf_temp_dir))
                    shutil.copytree(old_virtual_sconf_temp_dir, sconf_temp_dir)
                if os.path.isfile(old_virtual_custom_file):
                    if os.path.isfile(custom_file):
                        raise Exception('%s exists without a .sconsign.branch file so '
                                        'build virtualization cannot continue. Please '
                                        'move or delete it.' % custom_file)
                    print("shutil.copy %s -> %s" % (old_virtual_custom_file, custom_file))
                    shutil.copy(old_virtual_custom_file, custom_file)

            # create build dir in cache folder for later move
            print("os.makedirs" + branch_build_dir)
            os.makedirs(branch_build_dir)

        with open(sconsign_branch_file, 'w+') as f:
            print("touch" + sconsign_branch_file)
            f.write(branch_name)

    def get_features(self):
        return self.available_features


class Dependence(object):

    def _get_name(self):
        return self.__class__.__name__
    name = property(_get_name)

    def sources(self, build):
        return []

    def satisfy(self):
        pass

    def depends(self, build):
        return []

    def configure(self, build, conf):
        pass

    def post_dependency_check_configure(self, build, conf):
        pass


class Feature(Dependence):

    def _get_name(self):
        return self.__class__.__name__
    name = property(_get_name)
    status = ""

    def satisfy(self, build):
        raise NotImplementedError()

    def description(self):
        raise NotImplementedError()

    def enabled(self, build):
        return False

    def add_options(self, build, vars):
        pass
