import logging
import platform
import sys
import os
import re

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
            toolchain = 'gnu'

        if build is None:
            build = 'debug'

        if not build in ['debug', 'release']:
            raise Exception("invalid build type")

        if target not in ['windows', 'osx', 'linux', 'bsd']:
            raise Exception("invalid target platform")

        if machine not in ['x86_64', 'x86', 'i686', 'i586', 'i486', 'i386', 'powerpc', 'powerpc64']:
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

        self.toolchain = toolchain
        self.toolchain_is_gnu = self.toolchain == 'gnu'
        self.toolchain_is_msvs = self.toolchain == 'msvs'

        self.crosscompile = self.host_platform != self.platform

        flags_force32 = int(Script.ARGUMENTS.get('force32', 0))
        flags_force64 = int(Script.ARGUMENTS.get('force64', 0))
        if self.toolchain == 'gnu':
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
        self.machine_is_64bit = self.machine in ['x86_64', 'powerpc64']

        self.bitwidth = 32
        if self.machine_is_64bit:
            self.bitwidth = 64

        self.build_dir = util.get_build_dir(self.platform, self.bitwidth)

        logging.info("Target Platform: %s" % self.platform)
        logging.info("Target Machine: %s" % self.machine)
        logging.info("Build: %s" % self.build)
        logging.info("Toolchain: %s" % self.toolchain)
        logging.info("Crosscompile: %s" % ("YES" if self.crosscompile else "NO"))
        if self.crosscompile:
            logging.info("Host Platform: %s" % self.host_platform)
            logging.info("Host Machine: %s" % self.host_machine)

        if self.crosscompile and self.host_platform != 'linux':
            raise Exception('Cross-compiling on a non-Linux host not currently supported')

        tools = ['default']
        toolpath = ['#build/']
        extra_arguments = {}

        if self.platform in ('linux', 'bsd', 'osx'):
            tools.append('qt4')

        # Ugly hack to check the qtdir argument
        import depends
        default_qtdir = depends.Qt.DEFAULT_QTDIRS[self.platform]
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

        if self.platform == 'osx':
            tools.append('OSConsX')
            toolpath.append('#/build/osx/')
        if self.platform in ['windows', 'linux', 'bsd']:
            extra_arguments['QTDIR'] = qtdir
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

        if self.toolchain_is_gnu:
            if flags_force32:
                self.env.Append(CCFLAGS = '-m32')
            elif flags_force64:
                self.env.Append(CCFLAGS = '-m64')

        self.install_options()

    def detect_platform(self):
        return {'win32': 'windows',
                'cygwin': 'windows',
                'darwin': 'osx',
                'linux2': 'linux',
                'bsd': 'bsd',}.setdefault(sys.platform, 'invalid')

    def detect_machine(self):
        return platform.machine()

    def read_environment_variables(self):
        # Import environment variables from the terminal. Note that some
        # variables correspond to variables inside the SCons env with different
        # names, eg. the shell's "CFLAGS" ---> SCons' "CCFLAGS".
        if os.environ.has_key('CC'):
            self.env['CC'] = os.environ['CC']
        if os.environ.has_key('CFLAGS'):
            self.env['CCFLAGS'] += SCons.Util.CLVar(os.environ['CFLAGS'])
        if os.environ.has_key('CXX'):
            self.env['CXX'] = os.environ['CXX']
        if os.environ.has_key('CXXFLAGS'):
            self.env['CXXFLAGS'] += SCons.Util.CLVar(os.environ['CXXFLAGS'])
        if os.environ.has_key('LDFLAGS'):
            self.env['LINKFLAGS'] += SCons.Util.CLVar(os.environ['LDFLAGS'])

        # Initialize this as a list, fixes a bug where first CPPDEFINE would get
        # mangled
        self.env['CPPDEFINES'] = []

    def install_options(self):
        # Global cache directory Put all project files in it so a rm -rf cache
        # will clean up the config
        if not self.env.has_key('CACHEDIR'):
            self.env['CACHEDIR'] = str(Script.Dir('#cache/'))
        if not os.path.isdir(self.env['CACHEDIR']):
            os.mkdir(self.env['CACHEDIR'])

        cachefile = os.path.join(str(self.env['CACHEDIR']), 'custom.py')

        ## Avoid spreading .sconsign files everywhere
        #env.SConsignFile(env['CACHEDIR']+'/scons_signatures')
        ## WARNING - We found that the above line causes SCons to randomly not find
        ##           dependencies for some reason. It might not happen right away, but
        ##           a good number of users found that it caused weird problems - Albert (May 15/08)

        vars = Script.Variables(cachefile)
        vars.Add('prefix', 'Set to your install prefix', '/usr/local')
        vars.Add('qtdir', 'Set to your QT4 directory', '/usr/share/qt4')

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

