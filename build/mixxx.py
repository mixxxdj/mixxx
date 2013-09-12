# -*- coding: utf-8 -*-

from __future__ import with_statement

import logging
import platform
import sys
import os
import re
import shutil

import SCons
from SCons import Script

import util

class MixxxBuild(object):

    def __init__(self, target, machine, build, toolchain, available_features=[]):
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
                raise Exception('must specify toolchain on Windows (msvs or gnu)')
            else:
                toolchain = 'gnu'

        if build is None:
            build = 'debug'

        if not build in ['debug', 'release']:
            raise Exception("invalid build type")

        if target not in ['windows', 'osx', 'linux', 'bsd']:
            raise Exception("invalid target platform")

        if machine not in ['x86_64', 'x86', 'i686', 'i586',
                           'alpha', 'hppa', 'mips', 'mipsel', 's390',
                           'sparc', 'ia64', 'armel', 'armhf', 'hurd-i386',
                           'sh3', 'sh4',
                           'kfreebsd-amd64', 'kfreebsd-i386',
                           'i486', 'i386', 'powerpc', 'powerpc64',
                           'powerpcspe', 's390x',
                           'amd64', 'AMD64', 'EM64T', 'INTEL64']:
            raise Exception("invalid machine type")

        if toolchain not in ['gnu', 'msvs']:
            raise Exception('invalid toolchain type')

        if toolchain == 'msvs' and self.host_platform != 'windows':
            raise Exception('cannot use msvs toolchain on non-windows platform')

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
            if self.machine in ['powerpc', 'powerpc64']:
                self.machine = 'powerpc'
            else:
                self.machine = 'x86'
        elif flags_force64:
            if self.machine in ['powerpc', 'powerpc64']:
                self.machine = 'powerpc64'
            else:
                self.machine = 'x86_64'
        self.machine_is_64bit = self.machine in ['x86_64', 'powerpc64', 'AMD64', 'EM64T', 'INTEL64']
        self.bitwidth = 64 if self.machine_is_64bit else 32
        self.architecture_is_x86 = self.machine.lower() in ['x86', 'x86_64', 'i386', 'i486', 'i586', 'i686', 'EM64T', 'INTEL64']
        self.architecture_is_powerpc = self.machine.lower() in ['powerpc', 'powerpc64']

        self.build_dir = util.get_build_dir(self.platform, self.bitwidth)

        # Currently this only works for Windows
        self.static_dependencies = int(Script.ARGUMENTS.get('staticlibs', 0))
        self.msvcdebug = int(Script.ARGUMENTS.get('msvcdebug', 0))

        logging.info("Target Platform: %s" % self.platform)
        logging.info("Target Machine: %s" % self.machine)
        logging.info("Build: %s" % self.build)
        logging.info("Toolchain: %s" % self.toolchain)
        logging.info("Crosscompile: %s" % ("YES" if self.crosscompile else "NO"))
        if self.platform_is_windows:
            logging.info("Static dependencies: %s" % ("YES" if self.static_dependencies else "NO"))
            logging.info("MSVC Debug build: %s" % ("YES" if self.msvcdebug else "NO"))

        if self.crosscompile:
            logging.info("Host Platform: %s" % self.host_platform)
            logging.info("Host Machine: %s" % self.host_machine)

        if self.crosscompile and self.host_platform != 'linux':
            raise Exception('Cross-compiling on a non-Linux host not currently supported')

        tools = ['default']
        toolpath = ['#build/']
        extra_arguments = {}
        tools.append('qt4')
        tools.append('protoc')

        # Ugly hack to check the qtdir argument
        import depends
        default_qtdir = depends.Qt.DEFAULT_QTDIRS.get(self.platform, '')
        qtdir = Script.ARGUMENTS.get('qtdir',
                                    os.environ.get('QTDIR', default_qtdir))

        # Validate the specified qtdir exists
        if not os.path.exists(qtdir):
            logging.error("QT path does not exist or QT4 is not installed.")
            logging.error("Please specify your QT path by running 'scons qtdir=[path]'")
            Script.Exit(1)
        # And that it doesn't contain qt3
        elif qtdir.find("qt3") != -1 or qtdir.find("qt/3") != -1:
            logging.error("Mixxx now requires QT4 instead of QT3 - please use your QT4 path with the qtdir build flag.")
            Script.Exit(1)
        logging.info("Qt path: %s" % qtdir)

        # Previously this wasn't done for OSX, but I'm not sure why
        # -- rryan 6/8/2011
        extra_arguments['QTDIR'] = qtdir

        if self.platform == 'osx':
            tools.append('OSConsX')
            toolpath.append('#/build/osx/')
        if self.platform_is_windows and self.toolchain == 'msvs':
            toolpath.append('msvs')
            extra_arguments['VCINSTALLDIR'] = os.getenv('VCInstallDir') # TODO(XXX) Why?
            extra_arguments['QT_LIB'] = '' # TODO(XXX) Why?

        # Setup the appropriate toolchains for cross-compiling
        if self.crosscompile:
            if self.platform_is_windows:
                tools.append('crossmingw')
            if self.platform == 'osx':
                tools.append('crossosx')

        self.env = Script.Environment(tools=tools, toolpath=toolpath, ENV=os.environ,
                                     **extra_arguments)
        self.read_environment_variables()
        self.virtualize_build_dir()

        if self.toolchain_is_gnu:
            if flags_force32:
                self.env.Append(CCFLAGS = '-m32')
            elif flags_force64:
                self.env.Append(CCFLAGS = '-m64')

        if self.platform == 'osx':
            if self.machine == 'powerpc':
                self.env.Append(CCFLAGS = '-arch ppc')
                self.env.Append(LINKFLAGS = '-arch ppc')
            else:
                if self.bitwidth == 32:
                    self.env.Append(CCFLAGS = '-arch i386')
                    self.env.Append(LINKFLAGS = '-arch i386')
                elif self.bitwidth == 64:
                    self.env.Append(CCFLAGS = '-arch x86_64')
                    self.env.Append(LINKFLAGS = '-arch x86_64')

        if self.crosscompile:
            crosscompile_root = Script.ARGUMENTS.get('crosscompile_root', '')

            if crosscompile_root == '':
                print "Your build setup indicates this is a cross-compile, but you did not specify 'crosscompile_root', which is required."
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
        if sys.platform in ['linux2', 'linux3']:
            return 'linux'
        if sys.platform == 'darwin':
            return 'osx'
        logging.error("Couldn't determine platform. os.name: %s sys.platform: %s"
                      % (os.name, sys.platform))
        return 'invalid'

    def detect_machine(self):
        return platform.machine()

    def read_environment_variables(self):
        # Import environment variables from the terminal. Note that some
        # variables correspond to variables inside the SCons env with different
        # names, eg. the shell's "CFLAGS" ---> SCons' "CCFLAGS".
        if os.environ.has_key('CC'):
            self.env['CC'] = os.environ['CC']
        if os.environ.has_key('CFLAGS'):
            self.env['CFLAGS'] += SCons.Util.CLVar(os.environ['CFLAGS'])
        if os.environ.has_key('CXX'):
            self.env['CXX'] = os.environ['CXX']
        if os.environ.has_key('CXXFLAGS'):
            self.env['CXXFLAGS'] += SCons.Util.CLVar(os.environ['CXXFLAGS'])
        if os.environ.has_key('LDFLAGS'):
            self.env['LINKFLAGS'] += SCons.Util.CLVar(os.environ['LDFLAGS'])

        # Initialize this as a list, fixes a bug where first CPPDEFINE would get
        # mangled
        self.env['CPPDEFINES'] = []
        self.env['LIBS'] = []
        self.env['LIBPATH'] = []

    def get_cache_dir(self):
        # Global cache directory Put all project files in it so a rm -rf cache
        # will clean up the config
        if not self.env.has_key('CACHEDIR'):
            self.env['CACHEDIR'] = str(Script.Dir('#cache/'))
        if not os.path.isdir(self.env['CACHEDIR']):
            os.mkdir(self.env['CACHEDIR'])

        ## Avoid spreading .sconsign files everywhere
        #env.SConsignFile(env['CACHEDIR']+'/scons_signatures')
        ## WARNING - We found that the above line causes SCons to randomly not find
        ##           dependencies for some reason. It might not happen right away, but
        ##           a good number of users found that it caused weird problems - Albert (May 15/08)

        return str(self.env['CACHEDIR'])

    def install_options(self):
        cachefile = os.path.join(self.get_cache_dir(), 'custom.py')
        vars = Script.Variables(cachefile)
        vars.Add('prefix', 'Set to your install prefix', '/usr/local')
        vars.Add('virtualize', 'Dynamically swap out the build directory when switching Git branches.', 1)
        vars.Add('qtdir', 'Set to your QT4 directory', '/usr/share/qt4')
        if self.platform_is_windows:
            vars.Add('sqlitedll', 'Set to 1 to enable including QSQLite.dll.\
\n           Set to 0 if SQLite support is compiled into QtSQL.dll.', 1)
        vars.Add('target', 'Set the build target for cross-compiling (windows, osx, linux, bsd).', '')
        vars.Add('machine', 'Set the machine type for cross-compiling (x86_64, x86, powerpc, powerpc64).', '')
        vars.Add('toolchain', 'Specify the toolchain to use for building (gnu, msvs). Default is gnu.', 'gnu')
        vars.Add('crosscompile_root', 'Set the path to the root of a cross-compile sandbox.', '')
        vars.Add('force32', 'Force a 32-bit compile', 0)
        vars.Add('force64', 'Force a 64-bit compile', 0)

        for feature_class in self.available_features:
            # Instantiate the feature
            feature = feature_class()

            # Add the feature to the feature list
            feature.add_options(self, vars)

        vars.Update(self.env)
        Script.Help(vars.GenerateHelpText(self.env))

        #Save the options to cache
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
            return
        # TODO(rryan) what other branch name characters aren't allowed in
        # filenames?
        branch_name = re.sub('[/<>|"]', '_', branch_name).lower()

        cache_dir = self.get_cache_dir()
        branch_build_dir = os.path.join(cache_dir, branch_name)
        virtual_build_dir = os.path.join(branch_build_dir, self.build_dir)
        virtual_sconsign_file = os.path.join(branch_build_dir, 'sconsign.dblite')
        virtual_custom_file = os.path.join(branch_build_dir, 'custom.py')
        old_branch_build_dir = ''
        old_virtual_build_dir = ''
        old_virtual_sconsign_file = ''
        old_virtual_custom_file = ''

        # Clean up symlinks from our original method of virtualizing.
        if os.path.islink(self.build_dir):
            print "os.unlink", self.build_dir
            os.unlink(self.build_dir)

        sconsign_file = '.sconsign.dblite'
        sconsign_branch_file = '.sconsign.branch' #contains the branch name of last build 
        custom_file = 'cache/custom.py' #contains custom build flags
        sconsign_branch = ''
        is_branch_different = True
        if os.path.isfile(sconsign_branch_file):
            with open(sconsign_branch_file, 'r') as f:
                sconsign_branch = f.readline()
                sconsign_branch = sconsign_branch.strip()

        # check if there was a checkout of a different branch sine the last build
        is_branch_different = sconsign_branch != branch_name
        if not is_branch_different:
            # nothing to do 
            return

        if sconsign_branch:
            old_branch_build_dir = os.path.join(cache_dir, sconsign_branch)
            old_virtual_build_dir = os.path.join(old_branch_build_dir, self.build_dir)
            old_virtual_sconsign_file = os.path.join(old_branch_build_dir, 'sconsign.dblite')
            old_virtual_custom_file = os.path.join(old_branch_build_dir, 'custom.py')
            if os.path.isdir(self.build_dir):
                if os.path.isdir(old_virtual_build_dir):
                    raise Exception('%s already exists. '
                                    'build virtualization cannot continue. Please '
                                    'move or delete it.' % old_virtual_build_dir)       
                print "shutil.move", self.build_dir, old_virtual_build_dir
                #move build dir from last build to chache, named with the old branch name 
                shutil.move(self.build_dir, old_virtual_build_dir)

            if os.path.isfile(sconsign_file):
                print "shutil.move", sconsign_file, old_virtual_sconsign_file
                #move sconsdign-dblite as well
                shutil.move(sconsign_file, old_virtual_sconsign_file)

            if os.path.isfile(custom_file):
                print "shutil.move", custom_file, old_virtual_custom_file
                #and move custom.py
                shutil.move(custom_file, old_virtual_custom_file)

        # Now there should be no folder self.build_dir or file sconsign_file.
        if os.path.isdir(branch_build_dir):
            if os.path.isdir(virtual_build_dir):
                # found a build_dir in cache from a previous build 
                if os.path.isdir(self.build_dir):
                    raise Exception('%s exists without a .sconsign.branch file so '
                                    'build virtualization cannot continue. Please '
                                    'move or delete it.' % self.build_dir)
                print "shutil.move", virtual_build_dir, self.build_dir
                shutil.move(virtual_build_dir, self.build_dir)
            if os.path.isfile(virtual_sconsign_file):
                if os.path.isfile(sconsign_file):
                    raise Exception('%s exists without a .sconsign.branch file so '
                                    'build virtualization cannot continue. Please '
                                    'move or delete it.' % sconsign_file)
                print "shutil.move", virtual_sconsign_file, sconsign_file
                shutil.move(virtual_sconsign_file, sconsign_file)
            if os.path.isfile(virtual_custom_file):
                if os.path.isfile(custom_file):
                    raise Exception('%s exists without a .sconsign.branch file so '
                                    'build virtualization cannot continue. Please '
                                    'move or delete it.' % custom_file)
                print "shutil.move", virtual_custom_file, custom_file
                shutil.move(virtual_custom_file, custom_file)
        else:
            # no chached build dir found, assume this is a branch from the old branch 
            # if not, no problem because scons will rebuild all chaned files in any case    
            # copy the old_virtual_dir back
            if sconsign_branch: 
                if os.path.isdir(old_virtual_build_dir):
                    if os.path.isdir(self.build_dir):
                        raise Exception('%s exists without a .sconsign.branch file so '
                                        'build virtualization cannot continue. Please '
                                        'move or delete it.' % self.build_dir)
                    print "shutil.copytree", old_virtual_build_dir, self.build_dir
                    shutil.copytree(old_virtual_build_dir, self.build_dir)
                if os.path.isfile(old_virtual_sconsign_file):
                    if os.path.isfile(sconsign_file):
                        raise Exception('%s exists without a .sconsign.branch file so '
                                        'build virtualization cannot continue. Please '
                                        'move or delete it.' % sconsign_file)
                    print "shutil.copy", virtual_sconsign_file, sconsign_file
                    shutil.copy(old_virtual_sconsign_file, sconsign_file)
                if os.path.isfile(old_virtual_custom_file):
                    if os.path.isfile(sconsign_file):
                        raise Exception('%s exists without a .sconsign.branch file so '
                                        'build virtualization cannot continue. Please '
                                        'move or delete it.' % custom_file)
                    print "shutil.copy", virtual_custom_file, custom_file
                    shutil.copy(old_virtual_custom_file, custom_file)
      
            # create build dir in cache folder for later move       
            print "os.makedirs", branch_build_dir
            os.makedirs(branch_build_dir)

        with open(sconsign_branch_file, 'w+') as f:
            print 'touch', sconsign_branch_file
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

