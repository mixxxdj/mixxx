import logging
import platform
import sys
import os
import re
import SCons.Script as SCons

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

        if machine not in ['x86_64', 'i686', 'i586', 'i486', 'i386', 'powerpc', 'powerpc64']:
            raise Exception("invalid machine type")

        if toolchain not in ['gnu', 'msvs']:
            raise Exception('invalid toolchain type')

        if toolchain == 'msvs' and self.host_platform != 'windows':
            raise Exception('cannot use msvs toolchain on non-windows platform')

        self.platform = target
        self.machine = machine
        self.build = build
        self.toolchain = toolchain
        self.crosscompile = self.host_platform != self.platform

        self.machine_is_64bit = self.machine in ['x86_64', 'powerpc64']
        self.toolchain_is_gnu = self.toolchain == 'gnu'
        self.toolchain_is_msvs = self.toolchain == 'msvs'
        self.platform_is_posix = self.platform in ['linux', 'osx', 'bsd']
        self.platform_is_linux = self.platform == 'linux'
        self.platform_is_osx = self.platform == 'osx'
        self.platform_is_bsd = self.platform == 'bsd'
        self.platform_is_windows = self.platform == 'windows'

        self.bitwidth = 32
        if self.machine_is_64bit:
            self.bitwidth = 64

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
        qtdir = SCons.ARGUMENTS.get('qtdir',
                                    os.environ.get('QTDIR', default_qtdir))

        # Validate the specified qtdir exists
        if not os.path.exists(qtdir):
            logging.error("QT path does not exist or QT4 is not installed.")
            logging.error("Please specify your QT path by running 'scons qtdir=[path]'")
            SCons.Exit(1)
        # And that it doesn't contain qt3
        elif qtdir.find("qt3") != -1 or qtdir.find("qt/3") != -1:
            logging.error("Mixxx now requires QT4 instead of QT3 - please use your QT4 path with the qtdir build flag.")
            SCons.Exit(1)
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

        self.env = SCons.Environment(tools=tools, toolpath=toolpath, ENV=os.environ,
                                     **extra_arguments)

        # Global cache directory Put all project files in it so a rm -rf cache
        # will clean up the config
        if not self.env.has_key('CACHEDIR'):
            cachedir = str(SCons.Dir('#cache/'))
            if not os.path.isdir(cachedir):
                os.mkdir(cachedir)
            self.env['CACHEDIR'] = cachedir

    def detect_platform(self):
        return {'win32': 'windows',
                'cygwin': 'windows',
                'darwin': 'osx',
                'linux2': 'linux',
                'bsd': 'bsd',}.setdefault(sys.platform, 'invalid')

    def detect_machine(self):
        return platform.machine()

    def get_features(self):
        cachefile = os.path.join(str(self.env['CACHEDIR']), 'custom.py')

        ## Avoid spreading .sconsign files everywhere
        #env.SConsignFile(env['CACHEDIR']+'/scons_signatures')
        ## WARNING - We found that the above line causes SCons to randomly not find
        ##           dependencies for some reason. It might not happen right away, but
        ##           a good number of users found that it caused weird problems - Albert (May 15/08)


        vars = SCons.Variables(cachefile)
        vars.Add('prefix', 'Set to your install prefix', '/usr/local')
        vars.Add('qtdir', 'Set to your QT4 directory', '/usr/share/qt4')

        for feature_class in self.available_features:
            # Instantiate the feature
            feature = feature_class()

            # Add the feature to the feature list
            feature.add_options(self, vars)

        if not self.platform_is_windows:
            vars.Add('tuned', 'Set to 1 to optimize mixxx for this CPU (overrides "optimize")', 0)
            vars.Add('optimize', 'Set to:\n  1 for -O3 compiler optimizations\n  2 for Pentium 4 optimizations\n  3 for Intel Core optimizations\n  4 for Intel Core 2 optimizations\n  5 for Athlon-4/XP/MP optimizations\n  6 for K8/Opteron/AMD64 optimizations\n  7 for K8/Opteron/AMD64 w/ SSE3\n  8 for Celeron D (generic SSE/SSE2/SSE3) optimizations.', 1)

        else:
            if self.machine_is_64bit:
		vars.Add('tuned', 'Set to 1 to optimize mixxx for this CPU class', 0)
            vars.Add('optimize', 'Set to:\n  1 to maximize speed (/O2)\n  2 for maximum optimizations (/Ox)', 1)

        vars.Update(self.env)
        SCons.Help(vars.GenerateHelpText(self.env))

        #Save the options to cache
        vars.Save(cachefile, self.env)

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

    def satisfy(self, build):
        raise NotImplementedError()

    def description(self):
        raise NotImplementedError()

    def enabled(self, build):
        return False

    def add_options(self, build, vars):
        pass

