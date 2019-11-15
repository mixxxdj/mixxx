# -*- coding: utf-8 -*-

import os
from . import util
from .mixxx import Feature
import SCons.Script as SCons
from . import depends

class HSS1394(Feature):
    def description(self):
        return "HSS1394 MIDI device support"

    def enabled(self, build):
        if build.platform_is_windows or build.platform_is_osx:
            build.flags['hss1394'] = util.get_flags(build.env, 'hss1394', 1)
        else:
            build.flags['hss1394'] = util.get_flags(build.env, 'hss1394', 0)
        if int(build.flags['hss1394']):
            return True
        return False

    def add_options(self, build, vars):
        if build.platform_is_windows or build.platform_is_osx:
            vars.Add('hss1394',
                     'Set to 1 to enable HSS1394 MIDI device support.', 1)

    def configure(self, build, conf):
        if not self.enabled(build):
            return
        if build.platform_is_windows or build.platform_is_osx:
#            if not conf.CheckHeader('HSS1394/HSS1394.h'):  # WTF this gives tons of cmath errors on MSVC
#                raise Exception('Did not find HSS1394 development headers')
            if not conf.CheckLib(['libhss1394', 'hss1394']):
                raise Exception('Did not find HSS1394 development library')

        build.env.Append(CPPDEFINES='__HSS1394__')

        if build.platform_is_windows and build.static_dependencies:
            conf.CheckLib('user32')

    def sources(self, build):
        return ['src/controllers/midi/hss1394controller.cpp',
                'src/controllers/midi/hss1394enumerator.cpp']


class HID(Feature):
    INTERNAL_LINK = False
    HIDAPI_INTERNAL_PATH = 'lib/hidapi-0.8.0-rc1'

    def description(self):
        return "HID controller support"

    def enabled(self, build):
        build.flags['hid'] = util.get_flags(build.env, 'hid', 1)
        if int(build.flags['hid']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('hid', 'Set to 1 to enable HID controller support.', 1)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        if build.platform_is_linux or build.platform_is_bsd:
            # Try using system lib
            if not conf.CheckLib(['hidapi-libusb', 'libhidapi-libusb']):
                # No System Lib found
                self.INTERNAL_LINK = True
                build.env.ParseConfig(
                    'pkg-config libusb-1.0 --silence-errors --cflags --libs')
                if (not conf.CheckLib(['libusb-1.0', 'usb-1.0']) or
                        not conf.CheckHeader('libusb-1.0/libusb.h')):
                    raise Exception(
                           'Did not find the libusb 1.0 development library or its header file')
            else:
                build.env.ParseConfig('pkg-config hidapi-libusb --silence-errors --cflags --libs')


            # Optionally add libpthread and librt. Some distros need this.
            conf.CheckLib(['pthread', 'libpthread'])
            conf.CheckLib(['rt', 'librt'])

            # -pthread tells GCC to do the right thing regardless of system
            build.env.Append(CCFLAGS='-pthread')
            build.env.Append(LINKFLAGS='-pthread')

        else:
            self.INTERNAL_LINK = True
            if build.platform_is_windows and not conf.CheckLib(['setupapi', 'libsetupapi']):
                raise Exception('Did not find the setupapi library, exiting.')
            elif build.platform_is_osx:
                build.env.AppendUnique(FRAMEWORKS=['IOKit', 'CoreFoundation'])

        build.env.Append(CPPDEFINES='__HID__')
        if self.INTERNAL_LINK:
            build.env.Append(
                 CPPPATH=[os.path.join('#' + self.HIDAPI_INTERNAL_PATH, 'hidapi')])

    def sources(self, build):
        sources = ['src/controllers/hid/hidcontroller.cpp',
                   'src/controllers/hid/hidenumerator.cpp',
                   'src/controllers/hid/hidcontrollerpresetfilehandler.cpp']

        if self.INTERNAL_LINK:
            if build.platform_is_windows:
                # Requires setupapi.lib which is included by the above check for
                # setupapi.
                sources.append(
                    os.path.join(self.HIDAPI_INTERNAL_PATH, "windows/hid.c"))
            elif build.platform_is_linux:
                # hidapi compiles the libusb implementation by default on Linux
                sources.append(
                    os.path.join(self.HIDAPI_INTERNAL_PATH, 'libusb/hid.c'))
            elif build.platform_is_osx:
                sources.append(
                    os.path.join(self.HIDAPI_INTERNAL_PATH, 'mac/hid.c'))

        return sources


class Bulk(Feature):
    def description(self):
        return "USB Bulk controller support"

    def enabled(self, build):
        # For now only make Bulk default on Linux only. Turn on for all
        # platforms after the 1.11.0 release.
        is_default = 1 if build.platform_is_linux else 0
        build.flags['bulk'] = util.get_flags(build.env, 'bulk', is_default)
        if int(build.flags['bulk']):
            return True
        return False

    def add_options(self, build, vars):
        is_default = 1 if build.platform_is_linux else 0
        vars.Add('bulk',
                 'Set to 1 to enable USB Bulk controller support.', is_default)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        build.env.ParseConfig(
            'pkg-config libusb-1.0 --silence-errors --cflags --libs')
        if (not conf.CheckLib(['libusb-1.0', 'usb-1.0']) or
                not conf.CheckHeader('libusb-1.0/libusb.h')):
            raise Exception(
                'Did not find the libusb 1.0 development library or its header file, exiting!')

        build.env.Append(CPPDEFINES='__BULK__')

    def sources(self, build):
        sources = ['src/controllers/bulk/bulkcontroller.cpp',
                   'src/controllers/bulk/bulkenumerator.cpp']
        if not int(build.flags['hid']):
            sources.append(
                'src/controllers/hid/hidcontrollerpresetfilehandler.cpp')
        return sources


class Mad(Feature):
    def description(self):
        return "MAD MP3 Decoder"

    def default(self, build):
        return 0 if build.platform_is_osx else 1

    def enabled(self, build):
        build.flags['mad'] = util.get_flags(build.env, 'mad',
                                            self.default(build))
        if int(build.flags['mad']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('mad', 'Set to 1 to enable MAD MP3 decoder support.',
                 self.default(build))

    def configure(self, build, conf):
        if not self.enabled(build):
            return
        if not conf.CheckLib(['libmad', 'mad']):
            raise Exception(
                'Did not find libmad.a, libmad.lib, or the libmad development header files - exiting!')
        if not conf.CheckLib(['libid3tag', 'id3tag', 'libid3tag-release']):
            raise Exception(
                'Did not find libid3tag.a, libid3tag.lib, or the libid3tag development header files - exiting!')
        build.env.Append(CPPDEFINES='__MAD__')

    def sources(self, build):
        return ['src/sources/soundsourcemp3.cpp']


class CoreAudio(Feature):

    def description(self):
        return "CoreAudio MP3/AAC Decoder"

    def default(self, build):
        return 1 if build.platform_is_osx else 0

    def enabled(self, build):
        build.flags['coreaudio'] = util.get_flags(
            build.env, 'coreaudio', self.default(build))
        if int(build.flags['coreaudio']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add(
            'coreaudio', 'Set to 1 to enable CoreAudio MP3/AAC decoder support.',
            self.default(build))

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        if not build.platform_is_osx:
            raise Exception('CoreAudio is only supported on OS X!')

        build.env.Append(CPPPATH='#lib/apple/')
        build.env.AppendUnique(FRAMEWORKS=['AudioToolbox', 'CoreFoundation'])
        build.env.Append(CPPDEFINES='__COREAUDIO__')

    def sources(self, build):
        return ['src/sources/soundsourcecoreaudio.cpp',
                'src/sources/v1/legacyaudiosourceadapter.cpp',
                'lib/apple/CAStreamBasicDescription.cpp']


class MediaFoundation(Feature):
    FLAG = 'mediafoundation'

    def description(self):
        return "Media Foundation AAC Decoder Plugin"

    def enabled(self, build):
        build.flags[self.FLAG] = util.get_flags(build.env, self.FLAG, 0)
        if int(build.flags[self.FLAG]):
            return True
        return False

    def add_options(self, build, vars):
        if build.platform_is_windows:
            vars.Add(
                self.FLAG, "Set to 1 to enable the Media Foundation AAC decoder plugin (Windows Vista with KB2117917 or Windows 7 required)", 0)

    def configure(self, build, conf):
        if not self.enabled(build):
            return
        if not build.platform_is_windows:
            raise Exception("Media Foundation is only supported on Windows!")
        if not conf.CheckLib('Ole32'):
            raise Exception('Did not find Ole32.lib - exiting!')
        if not conf.CheckLib(['Mfuuid']):
            raise Exception('Did not find Mfuuid.lib - exiting!')
        if not conf.CheckLib(['Mfplat']):
            raise Exception('Did not find Mfplat.lib - exiting!')
        if not conf.CheckLib(['Mfreadwrite']):  # Only available on Windows 7 and up, or properly updated Vista
            raise Exception('Did not find Mfreadwrite.lib - exiting!')
        build.env.Append(CPPDEFINES='__MEDIAFOUNDATION__')

    def sources(self, build):
        return ['src/sources/soundsourcemediafoundation.cpp']


class VinylControl(Feature):
    def description(self):
        return "Vinyl Control"

    def enabled(self, build):
        build.flags['vinylcontrol'] = util.get_flags(build.env,
                                                     'vinylcontrol', 0)
        # Existence of the macappstore option forces vinylcontrol off due to
        # licensing issues.
        if 'macappstore' in build.flags and int(build.flags['macappstore']):
            return False
        if int(build.flags['vinylcontrol']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('vinylcontrol', 'Set to 1 to enable vinyl control support', 1)

    def configure(self, build, conf):
        if not self.enabled(build):
            return
        build.env.Append(CPPDEFINES='__VINYLCONTROL__')
        build.env.Append(CPPPATH='#lib/xwax')

    def sources(self, build):
        sources = ['src/vinylcontrol/vinylcontrol.cpp',
                   'src/vinylcontrol/vinylcontrolxwax.cpp',
                   'src/preferences/dialog/dlgprefvinyl.cpp',
                   'src/vinylcontrol/vinylcontrolsignalwidget.cpp',
                   'src/vinylcontrol/vinylcontrolmanager.cpp',
                   'src/vinylcontrol/vinylcontrolprocessor.cpp',
                   'src/vinylcontrol/steadypitch.cpp',
                   'src/engine/controls/vinylcontrolcontrol.cpp', ]
        if build.platform_is_windows:
            sources.append("lib/xwax/timecoder_win32.cpp")
            sources.append("lib/xwax/lut_win32.cpp")
        else:
            sources.append("lib/xwax/timecoder.c")
            sources.append("lib/xwax/lut.c")

        return sources


class ModPlug(Feature):
    def description(self):
        return "Modplug module decoder plugin"

    def enabled(self, build):
        # Default to enabled on but only throw an error if it was explicitly
        # requested and is not available.
        if 'modplug' in build.flags:
            return int(build.flags['modplug']) > 0
        build.flags['modplug'] = util.get_flags(build.env, 'modplug', 1)
        if int(build.flags['modplug']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('modplug',
                 'Set to 1 to enable libmodplug based module tracker support.',
                 1)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        # Only block the configure if modplug was explicitly requested.
        explicit = 'modplug' in SCons.ARGUMENTS

        if not conf.CheckHeader('libmodplug/modplug.h'):
            if explicit:
                raise Exception('Could not find libmodplug development headers.')
            else:
                build.flags['modplug'] = 0
            return

        if not conf.CheckLib(['modplug', 'libmodplug'], autoadd=True):
            if explicit:
                raise Exception('Could not find libmodplug shared library.')
            else:
                build.flags['modplug'] = 0
            return

        build.env.Append(CPPDEFINES='__MODPLUG__')

    def sources(self, build):
        depends.Qt.uic(build)('src/preferences/dialog/dlgprefmodplugdlg.ui')
        return ['src/sources/soundsourcemodplug.cpp',
                'src/preferences/dialog/dlgprefmodplug.cpp']


class FAAD(Feature):
    def description(self):
        return "FAAD AAC audio file decoder plugin"

    def default(self, build):
        return 1 if build.platform_is_linux else 0

    def enabled(self, build):
        build.flags['faad'] = util.get_flags(build.env, 'faad', self.default(build))
        if int(build.flags['faad']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('faad',
                 'Set to 1 to enable building the FAAD AAC decoder plugin.', 0)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        build.env.Append(CPPDEFINES='__FAAD__')
        have_mp4v2_h = conf.CheckHeader('mp4v2/mp4v2.h')
        if have_mp4v2_h:
            build.env.Append(CPPDEFINES = '__MP4V2__')
        have_mp4 = conf.CheckLib(['mp4v2', 'libmp4v2', 'mp4'])

        if not have_mp4:
            raise Exception(
                'Could not find libmp4, libmp4v2 or the libmp4v2 development headers.')

    def sources(self, build):
        return ['src/sources/soundsourcem4a.cpp',
        		'src/sources/libfaadloader.cpp']


class WavPack(Feature):
    def description(self):
        return "WavPack audio file support plugin"

    def enabled(self, build):
        build.flags['wv'] = util.get_flags(build.env, 'wv', 0)
        if int(build.flags['wv']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('wv',
                 'Set to 1 to enable building the WavPack support plugin.', 0)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        build.env.Append(CPPDEFINES='__WV__')
        if not conf.CheckLib(['wavpack', 'wv']):
            raise Exception(
                'Could not find libwavpack, libwv or its development headers.')

    def sources(self, build):
        return ['src/sources/soundsourcewv.cpp']


class ColorDiagnostics(Feature):
    def description(self):
        return "Color Diagnostics"

    def enabled(self, build):
        build.flags['color'] = util.get_flags(build.env, 'color', 0)
        return bool(int(build.flags['color']))

    def add_options(self, build, vars):
        vars.Add('color', "Set to 1 to enable Clang color diagnostics.", 0)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        if not build.compiler_is_clang:
            raise Exception('Color diagnostics are only available using clang.')

        build.env.Append(CCFLAGS='-fcolor-diagnostics')


class Sanitizers(Feature):
    # Known sanitizers, their names, and their -fsanitize=foo argument.
    SANITIZERS = [('asan', 'AddressSanitizer', 'address'),
                  ('ubsan', 'UndefinedBehaviorSanitizer', 'undefined'),
                  ('tsan', 'ThreadSanitizer', 'thread')]
    def description(self):
        return "Clang Sanitizers (asan, ubsan, tsan, etc.)"

    def enabled(self, build):
        any_enabled = False
        for keyword, _, _ in Sanitizers.SANITIZERS:
            build.flags[keyword] = util.get_flags(build.env, keyword, 0)
            any_enabled = any_enabled or bool(int(build.flags[keyword]))
        return any_enabled

    def add_options(self, build, vars):
        for keyword, name, _ in Sanitizers.SANITIZERS:
            vars.Add(keyword, "Set to 1 to enable the Clang %s." % name, 0)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        if not build.compiler_is_clang:
            raise Exception('Sanitizers are only available when using clang.')

        sanitizers = []
        for keyword, _, fsanitize in Sanitizers.SANITIZERS:
            if bool(int(build.flags[keyword])):
                sanitizers.append(fsanitize)

        # The Optimize feature below checks whether we are enabled and prevents
        # -fomit-frame-pointer if any sanitizer is enabled.
        build.env.Append(CCFLAGS="-fsanitize=%s" % ','.join(sanitizers))
        build.env.Append(LINKFLAGS="-fsanitize=%s" % ','.join(sanitizers))


class PerfTools(Feature):
    def description(self):
        return "Google PerfTools"

    def enabled(self, build):
        build.flags['perftools'] = util.get_flags(build.env, 'perftools', 0)
        build.flags['perftools_profiler'] = util.get_flags(
            build.env, 'perftools_profiler', 0)
        if int(build.flags['perftools']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add(
            "perftools", "Set to 1 to enable linking against libtcmalloc and Google's performance tools. You must install libtcmalloc from google-perftools to use this option.", 0)
        vars.Add("perftools_profiler", "Set to 1 to enable linking against libprofiler, Google's CPU profiler. You must install libprofiler from google-perftools to use this option.", 0)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        if not conf.CheckLib('tcmalloc'):
            raise Exception('Could not find tcmalloc. Please install it or compile Mixxx with perftools=0.')

        if int(build.flags['perftools_profiler']) and not conf.CheckLib('profiler'):
            raise Exception('Could not find the google-perftools profiler. Please install it or compile Mixxx with perftools_profiler=0.')


class AsmLib(Feature):
    def description(self):
        return "Agner Fog\'s ASMLIB"

    def enabled(self, build):
        if build.build_is_debug:
            return False
        build.flags['asmlib'] = util.get_flags(build.env, 'asmlib', 0)
        if int(build.flags['asmlib']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add(
            'asmlib', 'Set to 1 to enable linking against Agner Fog\'s hand-optimized asmlib, found at http://www.agner.org/optimize/', 0)

    def configure(self, build, conf):
        if build.build_is_debug:
            self.status = "Disabled (due to debug build)"
            return
        if not self.enabled(build):
            return

        build.env.Append(LIBPATH='lib/asmlib')
        if build.platform_is_linux:
            #Use ASMLIB's functions instead of the compiler's
            build.env.Append(CCFLAGS='-fno-builtin')
            build.env.Prepend(LIBS='":libaelf%so.a"' % build.bitwidth)
        elif build.platform_is_osx:
            #Use ASMLIB's functions instead of the compiler's
            build.env.Append(CCFLAGS='-fno-builtin')
            build.env.Prepend(LIBS='":libamac%so.a"' % build.bitwidth)
        elif build.platform_is_windows:
            #Use ASMLIB's functions instead of the compiler's
            build.env.Append(CCFLAGS='/Oi-')
            build.env.Prepend(LIBS='libacof%so' % build.bitwidth)


class BuildTime(Feature):
    def description(self):
        return "Use __DATE__ and __TIME__"

    def enabled(self, build):
        build.flags['buildtime'] = util.get_flags(build.env, 'buildtime', 1)
        if int(build.flags['buildtime']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add(
            'buildtime', 'Set to 0 to disable build time (__DATE__ and __TIME__) usage.', 1)

    def configure(self, build, conf):
        # Distributions like openSUSE use tools (e. g. build-compare) to detect
        # whether a built binary differs from a former build to avoid unneeded
        # publishing of packages.
        # If __DATE__ and __TIME__ are used the built binary differs always but
        # the tools cannot detect the root and publish a new package although
        # the only change is caused by __DATE__ and __TIME__.
        # So let distributions disable __DATE__ and __TIME__ via buildtime=0.
        if not self.enabled(build):
            build.env.Append(CPPDEFINES='DISABLE_BUILDTIME')


class Verbose(Feature):
    def description(self):
        return "Verbose compilation output"

    def enabled(self, build):
        build.flags['verbose'] = util.get_flags(build.env, 'verbose', 1)
        if int(build.flags['verbose']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('verbose', 'Compile files verbosely.', 1)

    def configure(self, build, conf):
        if not self.enabled(build):
            build.env['CCCOMSTR'] = '[CC] $SOURCE'
            build.env['CXXCOMSTR'] = '[CXX] $SOURCE'
            build.env['ASCOMSTR'] = '[AS] $SOURCE'
            build.env['ARCOMSTR'] = '[AR] $TARGET'
            build.env['RANLIBCOMSTR'] = '[RANLIB] $TARGET'
            build.env['LDMODULECOMSTR'] = '[LD] $TARGET'
            build.env['LINKCOMSTR'] = '[LD] $TARGET'

            build.env['QT5_LUPDATECOMSTR'] = '[LUPDATE] $SOURCE'
            build.env['QT5_LRELEASECOMSTR'] = '[LRELEASE] $SOURCE'
            build.env['QT5_QRCCOMSTR'] = '[QRC] $SOURCE'
            build.env['QT5_UICCOMSTR'] = '[UIC5] $SOURCE'
            build.env['QT5_MOCCOMSTR'] = '[MOC] $SOURCE'


class Profiling(Feature):
    def description(self):
        return "profiling (e.g. gprof) support"

    def enabled(self, build):
        build.flags['profiling'] = util.get_flags(build.env, 'profiling', 0)
        if int(build.flags['profiling']):
            if build.platform_is_linux or build.platform_is_osx or build.platform_is_bsd:
                return True
        return False

    def add_options(self, build, vars):
        if not build.platform_is_windows:
            vars.Add('profiling',
                     '(DEVELOPER) Set to 1 to enable profiling using gprof (Linux). Disables -fomit-frame-pointer.', 0)

    def configure(self, build, conf):
        if not self.enabled(build):
            return
        if build.platform_is_linux or build.platform_is_bsd:
            build.env.Append(CCFLAGS='-pg')
            build.env.Append(LINKFLAGS='-pg')


class TestSuite(Feature):
    def description(self):
        return "Mixxx Test Suite"

    def enabled(self, build):
        build.flags['test'] = (util.get_flags(build.env, 'test', 0) or
                               'test' in SCons.COMMAND_LINE_TARGETS or
                               'mixxx-test' in SCons.COMMAND_LINE_TARGETS)
        if int(build.flags['test']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('test', 'Set to 1 to build Mixxx test fixtures.', 0)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

    def sources(self, build):
        # Build the gtest library, but don't return any sources.

        # Clone our main environment so we don't change any settings in the
        # Mixxx environment
        test_env = build.env.Clone()

        # -pthread tells GCC to do the right thing regardless of system
        if build.toolchain_is_gnu:
            test_env.Append(CCFLAGS='-pthread')
            test_env.Append(LINKFLAGS='-pthread')

        test_env.Append(CPPPATH="#lib/gtest-1.7.0/include")
        gtest_dir = test_env.Dir("lib/gtest-1.7.0")

        env = test_env
        SCons.Export('env')
        SCons.Export('build')
        env.SConscript(env.File('SConscript', gtest_dir))

        # build and configure gmock
        test_env.Append(CPPPATH="#lib/gmock-1.7.0/include")
        gmock_dir = test_env.Dir("lib/gmock-1.7.0")
        env.SConscript(env.File('SConscript', gmock_dir))

        # Build the benchmark library
        test_env.Append(CPPPATH="#lib/benchmark/include")
        benchmark_dir = test_env.Dir("lib/benchmark")
        env.SConscript(env.File('SConscript', benchmark_dir))

        return []


class LiveBroadcasting(Feature):
    def description(self):
        return "Live Broadcasting Support"

    def enabled(self, build):
        build.flags['shoutcast'] = util.get_flags(build.env, 'shoutcast', 1)
        if int(build.flags['shoutcast']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('shoutcast', 'Set to 1 to enable live broadcasting support', 1)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        libshout_found = conf.CheckLib(['libshout', 'shout'])
        build.env.Append(CPPDEFINES='__BROADCAST__')

        if not libshout_found:
            raise Exception('Could not find libshout or its development headers. Please install it or compile Mixxx without Shoutcast support using the shoutcast=0 flag.')

        if build.platform_is_windows and build.static_dependencies:
            conf.CheckLib('winmm')
            conf.CheckLib('ws2_32')
            conf.CheckLib('gdi32')

    def sources(self, build):
        depends.Qt.uic(build)('src/preferences/dialog/dlgprefbroadcastdlg.ui')
        return ['src/preferences/dialog/dlgprefbroadcast.cpp',
                'src/broadcast/broadcastmanager.cpp',
                'src/engine/sidechain/shoutconnection.cpp']


class Opus(Feature):
    def description(self):
        return "Opus (RFC 6716) support"

    def enabled(self, build):
        # Default Opus to on but only throw an error if it was explicitly
        # requested.
        if 'opus' in build.flags:
            return int(build.flags['opus']) > 0
        build.flags['opus'] = util.get_flags(build.env, 'opus', 1)
        if int(build.flags['opus']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('opus', 'Set to 1 to enable Opus (RFC 6716) support \
                           (supported are Opus 1.0 and above and Opusfile 0.2 and above)', 1)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        # Only block the configure if opus was explicitly requested.
        explicit = 'opus' in SCons.ARGUMENTS

        # Support for Opus (RFC 6716)
        # More info http://http://www.opus-codec.org/
        if not conf.CheckLib(['opusfile', 'libopusfile']) or not conf.CheckLib(['opus', 'libopus']):
            if explicit:
                raise Exception('Could not find opus or libopusfile.')
            else:
                build.flags['opus'] = 0
            return

        if build.platform_is_windows and build.static_dependencies:
            for opus_lib in ['celt', 'silk_common', 'silk_float']:
                if not conf.CheckLib(opus_lib):
                    raise Exception('Missing opus static library %s -- exiting' % opus_lib)

        if build.platform_is_linux or build.platform_is_bsd:
            build.env.ParseConfig('pkg-config opusfile opus --silence-errors --cflags --libs')

        build.env.Append(CPPDEFINES='__OPUS__')

    def sources(self, build):
        return ['src/sources/soundsourceopus.cpp',
                'src/encoder/encoderopus.cpp']


class FFmpeg(Feature):
    def description(self):
        return "FFmpeg 4.x support"

    def enabled(self, build):
        build.flags['ffmpeg'] = util.get_flags(build.env, 'ffmpeg', 0)
        if int(build.flags['ffmpeg']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('ffmpeg', 'Set to 1 to enable FFmpeg 4.x support', 0)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        # FFmpeg is multimedia library that can be found http://ffmpeg.org/
        if build.platform_is_linux or build.platform_is_osx \
                or build.platform_is_bsd:
            # Check for libavcodec, libavformat
            if not conf.CheckForPKG('libavcodec', '58'):
                raise Exception('Missing libavcodec or it\'s too old! It can '
                                'be separated from main package so check your '
                                'operating system packages.')
            if not conf.CheckForPKG('libavformat', '58'):
                raise Exception('Missing libavformat or it\'s too old! '
                                'It can be separated from main package so '
                                'check your operating system packages.')
            if not conf.CheckForPKG('libswresample', '3.1'):
                raise Exception('Missing libswresample or it\'s too old! '
                                'It can be separated from main package so '
                                'check your operating system packages.')

            # Needed to build new FFmpeg
            build.env.Append(CCFLAGS='-D__STDC_CONSTANT_MACROS')
            build.env.Append(CCFLAGS='-D__STDC_LIMIT_MACROS')
            build.env.Append(CCFLAGS='-D__STDC_FORMAT_MACROS')

            # Grabs the libs and cflags for FFmpeg
            build.env.ParseConfig('pkg-config libswresample --silence-errors \
                                   --cflags --libs')
            build.env.ParseConfig('pkg-config libavcodec --silence-errors \
                                  --cflags --libs')
            build.env.ParseConfig('pkg-config libavformat --silence-errors \
                                   --cflags --libs')
            build.env.ParseConfig('pkg-config libavutil --silence-errors \
                                   --cflags --libs')

            build.env.Append(CPPDEFINES='__FFMPEG__')
            self.status = "Enabled"

        else:
            raise Exception('Building with FFmpeg 4.x is not supported'
                            'for your platform')

    def sources(self, build):
        return ['src/sources/soundsourceffmpeg.cpp']


class Optimize(Feature):
    LEVEL_OFF = 'off'
    LEVEL_PORTABLE = 'portable'
    LEVEL_NATIVE = 'native'
    LEVEL_LEGACY = 'legacy'
    LEVEL_FASTBUILD = 'fastbuild'
    LEVEL_DEFAULT = LEVEL_PORTABLE

    def description(self):
        return "Optimization and Tuning"

    @staticmethod
    def get_optimization_level(build):
        optimize_level = build.env.get('optimize', None)
        if optimize_level is None:
            optimize_level = SCons.ARGUMENTS.get('optimize',
                                                 Optimize.LEVEL_DEFAULT)

        try:
            optimize_integer = int(optimize_level)
            if optimize_integer == 0:
                optimize_level = Optimize.LEVEL_OFF
            elif optimize_integer == 1:
                # Level 1 was a legacy (compiler optimizations only) build.
                optimize_level = Optimize.LEVEL_LEGACY
            elif optimize_integer in xrange(2, 10):
                # Levels 2 through 9 map to portable.
                optimize_level = Optimize.LEVEL_PORTABLE
        except:
            pass

        # Support common aliases for off.
        if optimize_level in ('none', 'disable', 'disabled'):
            optimize_level = Optimize.LEVEL_OFF

        if optimize_level not in (Optimize.LEVEL_OFF, Optimize.LEVEL_PORTABLE,
                                  Optimize.LEVEL_NATIVE, Optimize.LEVEL_LEGACY,
                                  Optimize.LEVEL_FASTBUILD):
            raise Exception("optimize={} is not supported. "
                            "Use portable, native, legacy or off"
                            .format(optimize_level))
        return optimize_level

    def enabled(self, build):
        build.flags['optimize'] = Optimize.get_optimization_level(build)
        return build.flags['optimize'] != Optimize.LEVEL_OFF

    def add_options(self, build, vars):
        vars.Add(
            'optimize', 'Set to:\n' \
                        '  portable: sse2 CPU (>= Pentium 4)\n' \
                        '  fastbuild: portable, but without costly optimization steps\n' \
                        '  native: optimized for the CPU of this system\n' \
                        '  legacy: pure i386 code' \
                        '  off: no optimization' \
                        , Optimize.LEVEL_DEFAULT)

    def build_status(self, level, text=None):
        if text is None:
            return level
        return '%s: %s' % (level, text)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        optimize_level = build.flags['optimize']

        if optimize_level == Optimize.LEVEL_OFF:
            self.status = self.build_status(optimize_level, "no optimization")
            return

        if build.toolchain_is_msvs:
            fastbuild_enabled = optimize_level == Optimize.LEVEL_FASTBUILD

            # /GL : http://msdn.microsoft.com/en-us/library/0zza0de8.aspx
            # !!! /GL is incompatible with /ZI, which is set by mscvdebug
            build.env.Append(CCFLAGS='/GL-' if fastbuild_enabled else '/GL')

            # Use the fastest floating point math library
            # http://msdn.microsoft.com/en-us/library/e7s85ffb.aspx
            # http://msdn.microsoft.com/en-us/library/ms235601.aspx
            build.env.Append(CCFLAGS='/fp:fast')

            # Do link-time code generation (and don't show a progress indicator
            # -- this relies on ANSI control characters and tends to overwhelm
            # Jenkins logs) Should we turn on PGO ?
            # http://msdn.microsoft.com/en-us/library/xbf3tbeh.aspx
            build.env.Append(LINKFLAGS='/LTCG:OFF' if fastbuild_enabled else '/LTCG:NOSTATUS')

            # Suggested for unused code removal
            # http://msdn.microsoft.com/en-us/library/ms235601.aspx
            # http://msdn.microsoft.com/en-us/library/xsa71f43.aspx
            # http://msdn.microsoft.com/en-us/library/bxwfs976.aspx
            build.env.Append(CCFLAGS='/Gy')
            build.env.Append(LINKFLAGS='/OPT:REF')
            build.env.Append(LINKFLAGS='/OPT:ICF')

            # Don't worry about aligning code on 4KB boundaries
            # build.env.Append(LINKFLAGS = '/OPT:NOWIN98')
            # ALBERT: NOWIN98 is not supported in MSVC 2010.

            # http://msdn.microsoft.com/en-us/library/59a3b321.aspx
            # In general, you should pick /O2 over /Ox
            build.env.Append(CCFLAGS='/O2')

            if optimize_level == Optimize.LEVEL_PORTABLE or fastbuild_enabled:
                # fastbuild/portable-binary: sse2 CPU (>= Pentium 4)
                self.status = self.build_status(optimize_level,
                                                "sse2 CPU (>= Pentium 4)")
                # SSE and SSE2 are core instructions on x64
                # and consequently raise a warning message from compiler with this flag on x64.
                if not build.machine_is_64bit:
                    build.env.Append(CCFLAGS='/arch:SSE2')
                build.env.Append(CPPDEFINES=['__SSE__', '__SSE2__'])
            elif optimize_level == Optimize.LEVEL_NATIVE:
                self.status = self.build_status(
                    optimize_level, "tuned for this CPU (%s)" % build.machine)
                build.env.Append(CCFLAGS='/favor:' + build.machine)
            elif optimize_level == Optimize.LEVEL_LEGACY:
                self.status = self.build_status(optimize_level,
                                                "pure i386 code")
            else:
                # Not possible to reach this code if enabled is written
                # correctly.
                raise Exception("optimize={} is not supported. "
                                "Use portable, native, legacy or off"
                                .format(optimize_level))

            # SSE and SSE2 are core instructions on x64
            if build.machine_is_64bit:
                build.env.Append(CPPDEFINES=['__SSE__', '__SSE2__'])

        elif build.toolchain_is_gnu:
            # Portable is fast enough on GNU.
            if optimize_level == Optimize.LEVEL_FASTBUILD:
                optimize_level = Optimize.LEVEL_PORTABLE

            # Common flags to all optimizations.
            # -ffast-math will prevent a performance penalty by denormals
            # (floating point values almost Zero are treated as Zero)
            # unfortunately that work only on 64 bit CPUs or with sse2 enabled

            # the following optimisation flags makes the engine code ~3 times
            # faster, measured on a Atom CPU.
            build.env.Append(CCFLAGS='-O3')
            build.env.Append(CCFLAGS='-ffast-math')
            build.env.Append(CCFLAGS='-funroll-loops')

            # set -fomit-frame-pointer when we don't profile and are not using
            # Clang sanitizers.
            # Note: It is only included in -O on machines where it does not
            # interfere with debugging
            if not int(build.flags['profiling']) and not Sanitizers().enabled(build):
                build.env.Append(CCFLAGS='-fomit-frame-pointer')

            if optimize_level == Optimize.LEVEL_PORTABLE:
                # portable: sse2 CPU (>= Pentium 4)
                if build.architecture_is_x86:
                    self.status = self.build_status(optimize_level,
                                                    "sse2 CPU (>= Pentium 4)")
                    build.env.Append(CCFLAGS='-mtune=generic')
                    # -mtune=generic pick the most common, but compatible options.
                    # on arm platforms equivalent to -march=arch
                    if not build.machine_is_64bit:
                        # the sse flags are not set by default on 32 bit builds
                        # but are not supported on arm builds
                        build.env.Append(CCFLAGS='-msse2')
                        build.env.Append(CCFLAGS='-mfpmath=sse')
                    # TODO(rryan): macOS can use SSE3, and possibly SSE 4.1 once
                    # we require macOS 10.12.
                    # https://stackoverflow.com/questions/45917280/mac-osx-minumum-support-sse-version
                elif build.architecture_is_arm:
                    self.status = self.build_status(optimize_level)
                    build.env.Append(CCFLAGS='-mfloat-abi=hard')
                    build.env.Append(CCFLAGS='-mfpu=neon')
                else:
                    self.status = self.build_status(optimize_level)
                # this sets macros __SSE2_MATH__ __SSE_MATH__ __SSE2__ __SSE__
                # This should be our default build for distribution
                # It's a little sketchy, but turning on SSE2 will gain
                # 100% performance in our filter code and allows us to
                # turns on denormal zeroing.
                # We don't really support CPU's earlier than Pentium 4,
                # which is the class of CPUs this decision affects.
                # The downside of this is that we aren't truly
                # i386 compatible, so builds that claim 'i386' will crash.
                # -- rryan 2/2011
                # Note: SSE2 is a core part of x64 CPUs
            elif optimize_level == Optimize.LEVEL_NATIVE:
                self.status = self.build_status(
                    optimize_level, "tuned for this CPU (%s)" % build.machine)
                build.env.Append(CCFLAGS='-march=native')
                # http://en.chys.info/2010/04/what-exactly-marchnative-means/
                # Note: requires gcc >= 4.2.0
                # macros like __SSE2_MATH__ __SSE_MATH__ __SSE2__ __SSE__
                # are set automatically
                if build.architecture_is_x86 and not build.machine_is_64bit:
                    # For 32 bit builds using gcc < 5.0, the mfpmath=sse is
                    # not set by default (not supported on arm builds)
                    # If -msse is not implicitly set, it falls back to mfpmath=387
                    # and a compiler warning is issued (tested with gcc 4.8.4)
                    build.env.Append(CCFLAGS='-mfpmath=sse')
                elif build.architecture_is_arm:
                    self.status = self.build_status(optimize_level)
                    build.env.Append(CCFLAGS='-mfloat-abi=hard')
                    build.env.Append(CCFLAGS='-mfpu=neon')
            elif optimize_level == Optimize.LEVEL_LEGACY:
                if build.architecture_is_x86:
                    self.status = self.build_status(
                        optimize_level, "pure i386 code")
                    build.env.Append(CCFLAGS='-mtune=generic')
                    # -mtune=generic pick the most common, but compatible options.
                    # on arm platforms equivalent to -march=arch
                else:
                    self.status = self.build_status(optimize_level)
            else:
                # Not possible to reach this code if enabled is written
                # correctly.
                raise Exception("optimize={} is not supported. "
                                "Use portable, native, fastbuild, legacy or off"
                                .format(optimize_level))

            # what others do:
            # soundtouch uses just -O3 and -msse in Ubuntu Trusty
            # rubberband uses just -O2 in Ubuntu Trusty
            # fftw3 (used by rubberband) in Ubuntu Trusty
            # -O3 -fomit-frame-pointer -mtune=native -malign-double
            # -fstrict-aliasing -fno-schedule-insns -ffast-math


class MacAppStoreException(Feature):
    def description(self):
        return "Build for Mac App Store"

    def enabled(self, build):
        build.flags['macappstore'] = util.get_flags(build.env,
                                                    'macappstore', 0)
        if int(build.flags['macappstore']):
            # Existence of the macappstore option forces vinylcontrol off due to
            # licensing issues.
            build.flags['vinylcontrol'] = 0
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('macappstore', 'Set to 1 to indicate the build is for the Mac App Store', 0)

    def configure(self, build, conf):
        if not self.enabled(build):
            return
        build.env.Append(CPPDEFINES='__MACAPPSTORE__')

class LocaleCompare(Feature):
    def description(self):
        return "Locale Aware Compare for SQLite"

    def default(self, build):
        return 1 if build.platform_is_linux else 0

    def enabled(self, build):
        build.flags['localecompare'] = util.get_flags(build.env, 'localecompare',
                                                      self.default(build))
        if int(build.flags['localecompare']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('localecompare',
                 'Set to 1 to enable Locale Aware Compare support for SQLite.',
                 self.default(build))

    def configure(self, build, conf):
        if not self.enabled(build):
            return
        if int(util.get_flags(build.env, 'qt_sqlite_plugin', 0)):
            raise Exception('WARNING: localecompare is not compatible with the Qt SQLite plugin')
        if not conf.CheckLib(['sqlite3']):
            raise Exception('Missing libsqlite3 -- exiting!')
        build.env.Append(CPPDEFINES='__SQLITE3__')

class Lilv(Feature):
    def description(self):
        return "Lilv library for LV2 support"

    def enabled(self, build):
        build.flags['lilv'] = util.get_flags(build.env, 'lilv', 0)
        if int(build.flags['lilv']):
            return True
        return False

    def add_options(self, build, vars):
        default = 1
        # We do not have lilv set up in the Windows build server environment (yet)
        if build.platform_is_windows:
            default = 0
        vars.Add('lilv', 'Set to 1 to enable Lilv library for LV2 support', default)

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        if build.platform_is_linux or build.platform_is_osx \
                or build.platform_is_bsd:
            # Check for liblilv-0
            if not conf.CheckLib('lilv-0'):
                raise Exception('Missing liblilv-0 (needs at least 0.5)')

            build.env.Append(CPPDEFINES='__LILV__')

    def sources(self, build):
        return ['src/effects/lv2/lv2backend.cpp',
                'src/effects/lv2/lv2effectprocessor.cpp',
                'src/effects/lv2/lv2manifest.cpp',
                'src/preferences/dialog/dlgpreflv2.cpp']

class Battery(Feature):
    def description(self):
        return "Battery meter support."

    def enabled(self, build):
        build.flags['battery'] = util.get_flags(build.env, 'battery', 1)
        if int(build.flags['battery']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('battery',
                 'Set to 1 to enable battery meter support.')

    def configure(self, build, conf):
        if not self.enabled(build):
            return

        build.env.Append(CPPDEFINES='__BATTERY__')

    def sources(self, build):
        if build.platform_is_windows:
            return ["src/util/battery/batterywindows.cpp"]
        elif build.platform_is_osx:
            return ["src/util/battery/batterymac.cpp"]
        elif build.platform_is_linux or build.platform_is_bsd:
            return ["src/util/battery/batterylinux.cpp"]
        else:
            raise Exception('Battery support is not implemented for the target platform.')

    def depends(self, build):
        return [depends.IOKit, depends.UPower]

class QtKeychain(Feature):
    def description(self):
        return "Secure credentials storage support for Live Broadcasting profiles"

    def enabled(self, build):
        build.flags['qtkeychain'] = util.get_flags(build.env, 'qtkeychain', 0)
        if int(build.flags['qtkeychain']):
            return True
        return False

    def add_options(self, build, vars):
        vars.Add('qtkeychain', 'Set to 1 to enable secure credentials storage support for Live Broadcasting profiles', 0)

    def configure(self, build, conf):
        if not self.enabled(build):
            return
        if not conf.CheckLib('qt5keychain'):
            raise Exception("Could not find qt5keychain.")
        build.env.Append(CPPDEFINES='__QTKEYCHAIN__')
