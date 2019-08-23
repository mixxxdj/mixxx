# -*- coding: utf-8 -*-

import os
import subprocess

from . import util
from .mixxx import Dependence, Feature
import SCons.Script as SCons


class PortAudio(Dependence):

    def configure(self, build, conf):
        if not conf.CheckLib('portaudio'):
            raise Exception(
                'Did not find libportaudio.a, portaudio.lib, or the PortAudio-v19 development header files.')
        elif build.platform_is_linux:
            build.env.ParseConfig('pkg-config portaudio-2.0 --silence-errors --cflags --libs')

        if build.platform_is_windows and build.static_dependencies:
            conf.CheckLib('advapi32')

    def sources(self, build):
        return ['src/soundio/sounddeviceportaudio.cpp']


class PortMIDI(Dependence):

    def configure(self, build, conf):
        # Check for PortTime
        libs = ['porttime', 'libporttime']
        headers = ['porttime.h']

        # Depending on the library configuration PortTime might be statically
        # linked with PortMidi. We treat either presence of the lib or the
        # header as success.
        if not conf.CheckLib(libs) and not conf.CheckHeader(headers):
            raise Exception("Did not find PortTime or its development headers.")

        # Check for PortMidi
        libs = ['portmidi', 'libportmidi']
        headers = ['portmidi.h']
        if build.platform_is_windows and build.static_dependencies:
            conf.CheckLib('advapi32')
        if build.platform_is_windows:
            # We have this special branch here because on Windows we might want
            # to link PortMidi statically which we don't want to do on other
            # platforms.
            # TODO(rryan): Remove this? Don't want to break anyone but the
            # static/dynamic choice should be made by the whether the .a is an
            # import library for a shared library or a static library.
            libs.append('portmidi_s')

        if not conf.CheckLib(libs) or not conf.CheckHeader(headers):
            raise Exception("Did not find PortMidi or its development headers.")

    def sources(self, build):
        return ['src/controllers/midi/portmidienumerator.cpp',
                'src/controllers/midi/portmidicontroller.cpp']


class OpenGL(Dependence):

    def configure(self, build, conf):
        if build.platform_is_osx:
            build.env.AppendUnique(FRAMEWORKS='OpenGL')

        # Check for OpenGL (it's messy to do it for all three platforms).
        if (not conf.CheckLib('GL') and
                not conf.CheckLib('opengl32') and
                not conf.CheckCHeader('OpenGL/gl.h') and
                not conf.CheckCHeader('GL/gl.h')):
            raise Exception('Did not find OpenGL development files')

        if (not conf.CheckLib('GLU') and
                not conf.CheckLib('glu32') and
                not conf.CheckCHeader('OpenGL/glu.h')):
            raise Exception('Did not find GLU development files')


class SecurityFramework(Dependence):
    """The iOS/OS X security framework is used to implement sandboxing."""
    def configure(self, build, conf):
        if not build.platform_is_osx:
            return
        build.env.Append(CPPPATH='/System/Library/Frameworks/Security.framework/Headers/')
        build.env.Append(LINKFLAGS='-framework Security')


class CoreServices(Dependence):
    def configure(self, build, conf):
        if not build.platform_is_osx:
            return
        build.env.Append(CPPPATH='/System/Library/Frameworks/CoreServices.framework/Headers/')
        build.env.Append(LINKFLAGS='-framework CoreServices')

class Foundation(Dependence):
    def configure(self, build, conf):
        if not build.platform_is_osx:
            return
        build.env.Append(CPPPATH='/System/Library/Frameworks/Foundation.framework/Headers/')
        build.env.Append(LINKFLAGS='-framework Foundation')

class IOKit(Dependence):
    """Used for battery measurements and controlling the screensaver on OS X and iOS."""
    def configure(self, build, conf):
        if not build.platform_is_osx:
            return
        build.env.Append(
            CPPPATH='/System/Library/Frameworks/IOKit.framework/Headers/')
        build.env.Append(LINKFLAGS='-framework IOKit')

class UPower(Dependence):
    """UPower is used to get battery measurements on Linux and BSD."""
    def configure(self, build, conf):
        if not build.platform_is_linux and not build.platform_is_bsd:
            return
        build.env.ParseConfig(
                'pkg-config upower-glib --silence-errors --cflags --libs')

class OggVorbis(Dependence):

    def configure(self, build, conf):
        libs = ['libvorbisfile', 'vorbisfile']
        if not conf.CheckLib(libs):
            Exception('Did not find libvorbisfile.a, libvorbisfile.lib, '
                      'or the libvorbisfile development headers.')

        libs = ['libvorbis', 'vorbis']
        if not conf.CheckLib(libs):
            raise Exception(
                'Did not find libvorbis.a, libvorbis.lib, or the libvorbis development headers.')

        libs = ['libogg', 'ogg']
        if not conf.CheckLib(libs):
            raise Exception(
                'Did not find libogg.a, libogg.lib, or the libogg development headers')

        # libvorbisenc only exists on Linux, OSX and mingw32 on Windows. On
        # Windows with MSVS it is included in vorbisfile.dll. libvorbis and
        # libogg are included from build.py so don't add here.
        if not build.platform_is_windows or build.toolchain_is_gnu:
            vorbisenc_found = conf.CheckLib(['libvorbisenc', 'vorbisenc'])
            if not vorbisenc_found:
                raise Exception(
                    'Did not find libvorbisenc.a, libvorbisenc.lib, or the libvorbisenc development headers.')

    def sources(self, build):
        return ['src/sources/soundsourceoggvorbis.cpp']

class SndFile(Dependence):

    def configure(self, build, conf):
        if not conf.CheckLib(['sndfile', 'libsndfile', 'libsndfile-1']):
            raise Exception(
                "Did not find libsndfile or it\'s development headers")
        build.env.Append(CPPDEFINES='__SNDFILE__')
        if conf.CheckDeclaration('SFC_SET_COMPRESSION_LEVEL', '#include "sndfile.h"'):
            build.env.Append(CPPDEFINES='SFC_SUPPORTS_SET_COMPRESSION_LEVEL')

        if build.platform_is_windows and build.static_dependencies:
            build.env.Append(CPPDEFINES='FLAC__NO_DLL')
            conf.CheckLib('g72x')

    def sources(self, build):
        return ['src/sources/soundsourcesndfile.cpp']


class FLAC(Dependence):
    def configure(self, build, conf):
        if not conf.CheckHeader('FLAC/stream_decoder.h'):
            raise Exception('Did not find libFLAC development headers')
        libs = ['libFLAC', 'FLAC']
        if not conf.CheckLib(libs):
            raise Exception('Did not find libFLAC development libraries')

        if build.platform_is_windows and build.static_dependencies:
            build.env.Append(CPPDEFINES='FLAC__NO_DLL')

    def sources(self, build):
        return ['src/sources/soundsourceflac.cpp',]


class Qt(Dependence):
    DEFAULT_QT5DIRS64 = {'linux': '/usr/lib/x86_64-linux-gnu/qt5',
                         'bsd': '/usr/local/lib/qt5',
                         'osx': '/Library/Frameworks',
                         'windows': 'C:\\qt\\5.11.1'}

    DEFAULT_QT5DIRS32 = {'linux': '/usr/lib/i386-linux-gnu/qt5',
                         'bsd': '/usr/local/lib/qt5',
                         'osx': '/Library/Frameworks',
                         'windows': 'C:\\qt\\5.11.1'}

    @staticmethod
    def uic(build):
        return build.env.Uic5

    @staticmethod
    def find_framework_libdir(qtdir):
        # Try pkg-config on Linux
        pkg_config_cmd = ['pkg-config', '--variable=libdir', 'Qt5Core']
        try:
            output = subprocess.check_output(pkg_config_cmd)
        except OSError:
            # pkg-config is not installed
            pass
        except subprocess.CalledProcessError:
            # pkg-config failed to find Qt5Core
            pass
        else:
            core = output.decode('utf-8').rstrip()
            if os.path.isdir(core):
                return core

        for d in (os.path.join(qtdir, x) for x in ['', 'Frameworks', 'lib']):
            core = os.path.join(d, 'QtCore.framework')
            if os.path.isdir(core):
                return d
        return None

    @staticmethod
    def enabled_modules(build):
        qt_modules = [
            # Keep alphabetized.
            'QtConcurrent',
            'QtCore',
            'QtGui',
            'QtNetwork',
            'QtOpenGL',
            'QtScript',
            'QtScriptTools',
            'QtSql',
            'QtSvg',
            'QtTest',
            'QtWidgets',
            'QtXml',
        ]
        if build.platform_is_windows:
            qt_modules.extend([
                # Keep alphabetized.
                'QtAccessibilitySupport',
                'QtEventDispatcherSupport',
                'QtFontDatabaseSupport',
                'QtThemeSupport',
                'QtWindowsUIAutomationSupport',
            ])
        return qt_modules

    @staticmethod
    def enabled_imageformats(build):
        qt_imageformats = [
            # Keep alphabetized.
            'qdds',
            'qgif',
            'qicns',
            'qico',
            'qjp2',
            'qjpeg',
            'qmng',
            'qsvg',
            'qtga',
            'qtiff',
            'qwbmp',
            'qwebp',
        ]
        return qt_imageformats

    def satisfy(self):
        pass

    def configure(self, build, conf):
        qt_modules = Qt.enabled_modules(build)

        # Emit various Qt defines
        build.env.Append(CPPDEFINES=['QT_TABLET_SUPPORT'])

        if build.static_qt:
            build.env.Append(CPPDEFINES='QT_NODLL')
        else:
            build.env.Append(CPPDEFINES='QT_SHARED')

        # Set qt_sqlite_plugin flag if we should package the Qt SQLite plugin.
        build.flags['qt_sqlite_plugin'] = util.get_flags(
            build.env, 'qt_sqlite_plugin', 0)

        # Link in SQLite library if Qt is compiled statically
        if build.platform_is_windows and build.static_dependencies \
           and build.flags['qt_sqlite_plugin'] == 0 :
            conf.CheckLib('sqlite3');

        # Enable Qt include paths
        if build.platform_is_linux or build.platform_is_bsd:
            if not conf.CheckForPKG('Qt5Core', '5.0'):
                raise Exception('Qt >= 5.0 not found')

            if not conf.CheckLib('Qt5X11Extras'):
                raise Exception('Could not find Qt5X11Extras or its development headers')

            qt_modules.extend(['QtDBus'])
            # This automatically converts QtXXX to Qt5XXX where appropriate.
            build.env.EnableQt5Modules(qt_modules, debug=False)

            if build.architecture_is_x86:
                # Note that -reduce-relocations is enabled by default in Qt5.
                # So we must build the Mixxx *executable* with position
                # independent code. -pie / -fPIE must not be used, and Clang
                # -flto must not be used when producing ELFs (i.e. on Linux).
                # http://lists.qt-project.org/pipermail/development/2012-January/001418.html
                # https://github.com/qt/qtbase/blob/c5307203f5c0b0e588cc93e70764c090dd4c2ce0/dist/changes-5.4.2#L37-L45
                # https://codereview.qt-project.org/#/c/111787/
                # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65886#c30
                build.env.Append(CCFLAGS='-fPIC')

        elif build.platform_is_osx:
            qtdir = build.env['QTDIR']
            build.env.Append(
                LINKFLAGS=' '.join('-framework %s' % m for m in qt_modules)
            )
            framework_path = Qt.find_framework_libdir(qtdir)
            if not framework_path:
                raise Exception(
                    'Could not find frameworks in Qt directory: %s' % qtdir)
            # Necessary for raw includes of headers like #include <qobject.h>
            build.env.Append(CPPPATH=[os.path.join(framework_path, '%s.framework' % m, 'Headers')
                                      for m in qt_modules])
            # Framework path needs to be altered for CCFLAGS as well since a
            # header include of QtCore/QObject.h looks for a QtCore.framework on
            # the search path and a QObject.h in QtCore.framework/Headers.
            build.env.Append(CCFLAGS=['-F%s' % os.path.join(framework_path)])
            build.env.Append(LINKFLAGS=['-F%s' % os.path.join(framework_path)])

            # Copied verbatim from qt5.py.
            # TODO(rryan): Get our fixes merged upstream so we can use qt5.py for OS X.
            module_defines = {
                'QtScript'   : ['QT_SCRIPT_LIB'],
                'QtSvg'      : ['QT_SVG_LIB'],
                'QtSql'      : ['QT_SQL_LIB'],
                'QtXml'      : ['QT_XML_LIB'],
                'QtOpenGL'   : ['QT_OPENGL_LIB'],
                'QtGui'      : ['QT_GUI_LIB'],
                'QtNetwork'  : ['QT_NETWORK_LIB'],
                'QtCore'     : ['QT_CORE_LIB'],
                'QtWidgets'  : ['QT_WIDGETS_LIB'],
            }
            for module in qt_modules:
                build.env.AppendUnique(CPPDEFINES=module_defines.get(module, []))
            build.env["QT5_MOCCPPPATH"] = build.env["CPPPATH"]
        elif build.platform_is_windows:
            # This automatically converts QtCore to QtCore[45][d] where
            # appropriate.
            build.env.EnableQt5Modules(qt_modules,
                                       staticdeps=build.static_qt,
                                       debug=build.build_is_debug)

            if build.static_qt:
                # Pulled from qt-4.8.2-source\mkspecs\win32-msvc2010\qmake.conf
                # QtCore
                build.env.Append(LIBS = 'kernel32')
                build.env.Append(LIBS = 'user32') # QtGui, QtOpenGL, libHSS1394
                build.env.Append(LIBS = 'shell32')
                build.env.Append(LIBS = 'uuid')
                build.env.Append(LIBS = 'ole32') # QtGui,
                build.env.Append(LIBS = 'advapi32') # QtGui, portaudio, portmidi
                build.env.Append(LIBS = 'ws2_32')   # QtGui, QtNetwork, libshout
                # QtGui
                build.env.Append(LIBS = 'gdi32') #QtOpenGL, libshout
                build.env.Append(LIBS = 'comdlg32')
                build.env.Append(LIBS = 'oleaut32')
                build.env.Append(LIBS = 'imm32')
                build.env.Append(LIBS = 'winmm')
                build.env.Append(LIBS = 'winspool')
                # QtOpenGL
                build.env.Append(LIBS = 'glu32')
                build.env.Append(LIBS = 'opengl32')

                # QtNetwork openssl-linked
                build.env.Append(LIBS = 'crypt32')

                # New libraries required by Qt5.
                build.env.Append(LIBS = 'dwmapi')  # qtwindows
                build.env.Append(LIBS = 'iphlpapi')  # qt5network
                build.env.Append(LIBS = 'libEGL')  # qt5opengl
                build.env.Append(LIBS = 'libGLESv2')  # qt5opengl
                build.env.Append(LIBS = 'mpr')  # qt5core
                build.env.Append(LIBS = 'netapi32')  # qt5core
                build.env.Append(LIBS = 'userenv')  # qt5core
                build.env.Append(LIBS = 'uxtheme')  # ?
                build.env.Append(LIBS = 'version')  # ?
                build.env.Append(LIBS = 'wtsapi32') # ?

                build.env.Append(LIBS = 'qtfreetype')
                build.env.Append(LIBS = 'qtharfbuzz')
                build.env.Append(LIBS = 'qtlibpng')
                build.env.Append(LIBS = 'qtpcre2')

                # NOTE(rryan): If you are adding a plugin here, you must also
                # update src/mixxxapplication.cpp to define a Q_IMPORT_PLUGIN
                # for it. Not all imageformats plugins are built as .libs when
                # building Qt statically on Windows. Check the build environment
                # to see exactly what's available as a standalone .lib vs linked
                # into Qt .libs by default.

                # iconengines plugins
                build.env.Append(LIBPATH=[
                    os.path.join(build.env['QTDIR'],'plugins/iconengines')])
                build.env.Append(LIBS = 'qsvgicon')

                # imageformats plugins
                build.env.Append(LIBPATH=[
                    os.path.join(build.env['QTDIR'],'plugins/imageformats')])
                build.env.Append(LIBS = 'qico')
                build.env.Append(LIBS = 'qsvg')
                build.env.Append(LIBS = 'qtga')
                build.env.Append(LIBS = 'qgif')
                build.env.Append(LIBS = 'qjpeg')

                # platform plugins (new in Qt5 for Windows)
                build.env.Append(LIBPATH=[
                    os.path.join(build.env['QTDIR'],'plugins/platforms')])
                build.env.Append(LIBS = 'qwindows')

                # styles (new in Qt5 for Windows)
                build.env.Append(LIBPATH=[
                    os.path.join(build.env['QTDIR'],'plugins/styles')])
                build.env.Append(LIBS = 'qwindowsvistastyle')

                # sqldrivers (new in Qt5? or did we just start enabling them)
                build.env.Append(LIBPATH=[
                    os.path.join(build.env['QTDIR'],'plugins/sqldrivers')])
                build.env.Append(LIBS = 'qsqlite')

        # Set the rpath for linux/bsd/macOS.
        if not build.platform_is_windows:
            qtdir = build.env['QTDIR']
            libdir_path = Qt.find_framework_libdir(qtdir)
            if os.path.isdir(libdir_path):
                build.env.Append(LINKFLAGS=['-Wl,-rpath,%s' % libdir_path])
                build.env.Append(LINKFLAGS="-L" + libdir_path)

        # Mixxx requires C++14 support
        if build.platform_is_windows:
            # MSVC
            build.env.Append(CXXFLAGS='/std:c++14')
        else:
            # GCC/Clang
            build.env.Append(CXXFLAGS='-std=c++14')


class TestHeaders(Dependence):
    def configure(self, build, conf):
        build.env.Append(CPPPATH="#lib/gtest-1.7.0/include")

class FidLib(Dependence):
    def sources(self, build):
        symbol = None
        if build.platform_is_windows:
            if build.toolchain_is_msvs:
                symbol = 'T_MSVC'
            elif build.crosscompile:
                # Not sure why, but fidlib won't build with mingw32msvc and
                # T_MINGW
                symbol = 'T_LINUX'
            elif build.toolchain_is_gnu:
                symbol = 'T_MINGW'
        else:
            symbol = 'T_LINUX'

        return [build.env.StaticObject('lib/fidlib/fidlib.c',
                                       CPPDEFINES=symbol)]

    def configure(self, build, conf):
        build.env.Append(CPPPATH='#lib/fidlib/')


class ReplayGain(Dependence):

    def sources(self, build):
        return ["lib/replaygain/replaygain.cpp"]

    def configure(self, build, conf):
        build.env.Append(CPPPATH="#lib/replaygain")


class Ebur128Mit(Dependence):
    INTERNAL_PATH = 'lib/libebur128'
    INTERNAL_LINK = False

    def sources(self, build):
        if self.INTERNAL_LINK:
            return ['%s/ebur128/ebur128.c' % self.INTERNAL_PATH]

    def configure(self, build, conf, env=None):
        if env is None:
            env = build.env
        if not conf.CheckLib(['ebur128', 'libebur128']):
            self.INTERNAL_LINK = True;
            env.Append(CPPPATH=['#%s/ebur128' % self.INTERNAL_PATH])
            import sys
            if not conf.CheckHeader('sys/queue.h') or sys.platform.startswith('openbsd'):
                # OpenBSD's queue.h lacks the STAILQ_* macros
                env.Append(CPPPATH=['#%s/ebur128/queue' % self.INTERNAL_PATH])


class SoundTouch(Dependence):
    SOUNDTOUCH_INTERNAL_PATH = 'lib/soundtouch'
    INTERNAL_LINK = True

    def sources(self, build):
        if self.INTERNAL_LINK:
            env = build.env.Clone()
            soundtouch_dir = env.Dir(self.SOUNDTOUCH_INTERNAL_PATH)
            SCons.Export('env')
            SCons.Export('build')
            env.SConscript(env.File('SConscript', soundtouch_dir))

            build.env.Append(LIBPATH=self.SOUNDTOUCH_INTERNAL_PATH)
            build.env.Append(LIBS=['soundtouch'])
        return ['src/engine/bufferscalers/enginebufferscalest.cpp']

    def configure(self, build, conf, env=None):
        if env is None:
            env = build.env

        if build.platform_is_linux or build.platform_is_bsd:
            # Try using system lib
            if conf.CheckForPKG('soundtouch', '2.0.0'):
                # System Lib found
                if not conf.CheckLib(['SoundTouch']):
                    raise Exception(
                        "Could not find libSoundTouch or its development headers.")
                build.env.ParseConfig('pkg-config soundtouch --silence-errors --cflags --libs')
                self.INTERNAL_LINK = False

        if self.INTERNAL_LINK:
            env.Append(CPPPATH=['#' + self.SOUNDTOUCH_INTERNAL_PATH])

            # Prevents circular import.
            from .features import Optimize

            # If we do not want optimizations then disable them.
            optimize = (build.flags['optimize'] if 'optimize' in build.flags
                        else Optimize.get_optimization_level(build))
            if optimize == Optimize.LEVEL_OFF:
                env.Append(CPPDEFINES='SOUNDTOUCH_DISABLE_X86_OPTIMIZATIONS')

class RubberBand(Dependence):
    def sources(self, build):
        sources = ['src/engine/bufferscalers/enginebufferscalerubberband.cpp', ]
        return sources

    def configure(self, build, conf, env=None):
        if env is None:
            env = build.env
        if not conf.CheckLib(['rubberband', 'librubberband']):
            raise Exception(
                "Could not find librubberband or its development headers.")


class QueenMaryDsp(Dependence):
    def sources(self, build):
        return [
            #"#lib/qm-dsp/base/KaiserWindow.cpp",
            "#lib/qm-dsp/base/Pitch.cpp",
            #"#lib/qm-dsp/base/SincWindow.cpp",
            "#lib/qm-dsp/dsp/chromagram/Chromagram.cpp",
            "#lib/qm-dsp/dsp/chromagram/ConstantQ.cpp",
            "#lib/qm-dsp/dsp/keydetection/GetKeyMode.cpp",
            #"#lib/qm-dsp/dsp/mfcc/MFCC.cpp",
            "#lib/qm-dsp/dsp/onsets/DetectionFunction.cpp",
            "#lib/qm-dsp/dsp/onsets/PeakPicking.cpp",
            "#lib/qm-dsp/dsp/phasevocoder/PhaseVocoder.cpp",
            "#lib/qm-dsp/dsp/rateconversion/Decimator.cpp",
            #"#lib/qm-dsp/dsp/rateconversion/DecimatorB.cpp",
            #"#lib/qm-dsp/dsp/rateconversion/Resampler.cpp",
            #"#lib/qm-dsp/dsp/rhythm/BeatSpectrum.cpp",
            #"#lib/qm-dsp/dsp/segmentation/ClusterMeltSegmenter.cpp",
            #"#lib/qm-dsp/dsp/segmentation/Segmenter.cpp",
            #"#lib/qm-dsp/dsp/segmentation/cluster_melt.c",
            #"#lib/qm-dsp/dsp/segmentation/cluster_segmenter.c",
            "#lib/qm-dsp/dsp/signalconditioning/DFProcess.cpp",
            "#lib/qm-dsp/dsp/signalconditioning/FiltFilt.cpp",
            "#lib/qm-dsp/dsp/signalconditioning/Filter.cpp",
            "#lib/qm-dsp/dsp/signalconditioning/Framer.cpp",
            "#lib/qm-dsp/dsp/tempotracking/DownBeat.cpp",
            "#lib/qm-dsp/dsp/tempotracking/TempoTrack.cpp",
            "#lib/qm-dsp/dsp/tempotracking/TempoTrackV2.cpp",
            "#lib/qm-dsp/dsp/tonal/ChangeDetectionFunction.cpp",
            "#lib/qm-dsp/dsp/tonal/TCSgram.cpp",
            "#lib/qm-dsp/dsp/tonal/TonalEstimator.cpp",
            "#lib/qm-dsp/dsp/transforms/FFT.cpp",
            #"#lib/qm-dsp/dsp/wavelet/Wavelet.cpp",
            "#lib/qm-dsp/ext/kissfft/kiss_fft.c",
            "#lib/qm-dsp/ext/kissfft/tools/kiss_fftr.c",
            #"#lib/qm-dsp/hmm/hmm.c",
            "#lib/qm-dsp/maths/Correlation.cpp",
            #"#lib/qm-dsp/maths/CosineDistance.cpp",
            "#lib/qm-dsp/maths/KLDivergence.cpp",
            "#lib/qm-dsp/maths/MathUtilities.cpp",
            #"#lib/qm-dsp/maths/pca/pca.c",
            #"#lib/qm-dsp/thread/Thread.cpp"
        ]

    def configure(self, build, conf):
        build.env.Append(CPPPATH="#lib/qm-dsp")
        build.env.Append(CPPPATH="#lib/qm-dsp/include")
        build.env.Append(CPPDEFINES='kiss_fft_scalar=double')
        if not build.platform_is_windows:
            build.env.Append(CPPDEFINES='USE_PTHREADS')


class TagLib(Dependence):
    def configure(self, build, conf):
        libs = ['tag']
        if not conf.CheckLib(libs):
            raise Exception(
                "Could not find libtag or its development headers.")

        # Karmic seems to have an issue with mp4tag.h where they don't include
        # the files correctly. Adding this folder to the include path should fix
        # it, though might cause issues. This is safe to remove once we
        # deprecate Karmic support. rryan 2/2011
        build.env.Append(CPPPATH='/usr/include/taglib/')

        if build.platform_is_windows and build.static_dependencies:
            build.env.Append(CPPDEFINES='TAGLIB_STATIC')


class Chromaprint(Dependence):
    def configure(self, build, conf):
        if not conf.CheckLib(['chromaprint', 'libchromaprint', 'chromaprint_p', 'libchromaprint_p']):
            raise Exception(
                "Could not find libchromaprint or its development headers.")
        if build.platform_is_windows and build.static_dependencies:
            build.env.Append(CPPDEFINES='CHROMAPRINT_NODLL')

            # On Windows, we link chromaprint with FFTW3.
            if not conf.CheckLib(['fftw', 'libfftw', 'fftw3', 'libfftw3', 'libfftw-3.3']):
                raise Exception(
                    "Could not find fftw3 or its development headers.")


class ProtoBuf(Dependence):
    def configure(self, build, conf):
        libs = ['libprotobuf-lite', 'protobuf-lite', 'libprotobuf', 'protobuf']
        if build.platform_is_windows:
            if not build.static_dependencies:
                build.env.Append(CPPDEFINES='PROTOBUF_USE_DLLS')
        # SCons is supposed to check this for us by calling 'exists' in build/protoc.py.
        protoc_binary = build.env['PROTOC']
        if build.env.WhereIs(protoc_binary) is None:
            raise Exception("Can't locate '%s' the protobuf compiler." % protoc_binary)
        if not conf.CheckLib(libs):
            raise Exception(
                "Could not find libprotobuf or its development headers.")

class FpClassify(Dependence):

    def enabled(self, build):
        return build.toolchain_is_gnu

    # This is a wrapper around the fpclassify function that prevents inlining
    # It is compiled without optimization and allows to use these function
    # from -ffast-math optimized objects
    def sources(self, build):
        # add this file without fast-math flag
        env = build.env.Clone()
        if '-ffast-math' in env['CCFLAGS']:
                env['CCFLAGS'].remove('-ffast-math')
        return env.Object('src/util/fpclassify.cpp')

class QtScriptByteArray(Dependence):
    def configure(self, build, conf):
        build.env.Append(CPPPATH='#lib/qtscript-bytearray')

    def sources(self, build):
        return ['lib/qtscript-bytearray/bytearrayclass.cpp',
                'lib/qtscript-bytearray/bytearrayprototype.cpp']

class PortAudioRingBuffer(Dependence):
    def configure(self, build, conf):
        build.env.Append(CPPPATH='#lib/portaudio')

    def sources(self, build):
        return ['lib/portaudio/pa_ringbuffer.c']

class Reverb(Dependence):
    def configure(self, build, conf):
        build.env.Append(CPPPATH='#lib/reverb')

    def sources(self, build):
        return ['lib/reverb/Reverb.cc']

class LAME(Dependence):
    def configure(self, build, conf):
        if not conf.CheckLib(['libmp3lame', 'libmp3lame-static']):
            raise Exception("Could not find libmp3lame.")

class MixxxCore(Feature):

    def description(self):
        return "Mixxx Core Features"

    def enabled(self, build):
        return True

    def sources(self, build):
        sources = ["src/control/control.cpp",
                   "src/control/controlaudiotaperpot.cpp",
                   "src/control/controlbehavior.cpp",
                   "src/control/controleffectknob.cpp",
                   "src/control/controlindicator.cpp",
                   "src/control/controllinpotmeter.cpp",
                   "src/control/controllogpotmeter.cpp",
                   "src/control/controlmodel.cpp",
                   "src/control/controlobject.cpp",
                   "src/control/controlobjectscript.cpp",
                   "src/control/controlpotmeter.cpp",
                   "src/control/controlproxy.cpp",
                   "src/control/controlpushbutton.cpp",
                   "src/control/controlttrotary.cpp",
                   "src/control/controlencoder.cpp",

                   "src/controllers/dlgcontrollerlearning.cpp",
                   "src/controllers/dlgprefcontroller.cpp",
                   "src/controllers/dlgprefcontrollers.cpp",
                   "src/dialog/dlgabout.cpp",
                   "src/dialog/dlgdevelopertools.cpp",

                   "src/preferences/configobject.cpp",
                   "src/preferences/dialog/dlgprefautodj.cpp",
                   "src/preferences/dialog/dlgprefdeck.cpp",
                   "src/preferences/dialog/dlgprefcrossfader.cpp",
                   "src/preferences/dialog/dlgprefeffects.cpp",
                   "src/preferences/dialog/dlgprefeq.cpp",
                   "src/preferences/dialog/dlgpreferences.cpp",
                   "src/preferences/dialog/dlgprefinterface.cpp",
                   "src/preferences/dialog/dlgpreflibrary.cpp",
                   "src/preferences/dialog/dlgprefnovinyl.cpp",
                   "src/preferences/dialog/dlgprefrecord.cpp",
                   "src/preferences/dialog/dlgprefreplaygain.cpp",
                   "src/preferences/dialog/dlgprefsound.cpp",
                   "src/preferences/dialog/dlgprefsounditem.cpp",
                   "src/preferences/dialog/dlgprefwaveform.cpp",
                   "src/preferences/dialog/dlgprefbeats.cpp",
                   "src/preferences/dialog/dlgprefkey.cpp",
                   "src/preferences/settingsmanager.cpp",
                   "src/preferences/replaygainsettings.cpp",
                   "src/preferences/broadcastsettings.cpp",
                   "src/preferences/broadcastsettings_legacy.cpp",
                   "src/preferences/broadcastsettingsmodel.cpp",
                   "src/preferences/effectsettingsmodel.cpp",
                   "src/preferences/broadcastprofile.cpp",
                   "src/preferences/upgrade.cpp",
                   "src/preferences/dlgpreferencepage.cpp",

                   "src/effects/effectmanifest.cpp",
                   "src/effects/effectmanifestparameter.cpp",

                   "src/effects/effectchain.cpp",
                   "src/effects/effect.cpp",
                   "src/effects/effectparameter.cpp",

                   "src/effects/effectrack.cpp",
                   "src/effects/effectchainslot.cpp",
                   "src/effects/effectslot.cpp",
                   "src/effects/effectparameterslotbase.cpp",
                   "src/effects/effectparameterslot.cpp",
                   "src/effects/effectbuttonparameterslot.cpp",
                   "src/effects/effectsmanager.cpp",
                   "src/effects/effectchainmanager.cpp",
                   "src/effects/effectsbackend.cpp",

                   "src/effects/builtin/builtinbackend.cpp",
                   "src/effects/builtin/bitcrushereffect.cpp",
                   "src/effects/builtin/balanceeffect.cpp",
                   "src/effects/builtin/linkwitzriley8eqeffect.cpp",
                   "src/effects/builtin/bessel4lvmixeqeffect.cpp",
                   "src/effects/builtin/bessel8lvmixeqeffect.cpp",
                   "src/effects/builtin/threebandbiquadeqeffect.cpp",
                   "src/effects/builtin/biquadfullkilleqeffect.cpp",
                   "src/effects/builtin/loudnesscontoureffect.cpp",
                   "src/effects/builtin/graphiceqeffect.cpp",
                   "src/effects/builtin/parametriceqeffect.cpp",
                   "src/effects/builtin/flangereffect.cpp",
                   "src/effects/builtin/filtereffect.cpp",
                   "src/effects/builtin/moogladder4filtereffect.cpp",
                   "src/effects/builtin/reverbeffect.cpp",
                   "src/effects/builtin/echoeffect.cpp",
                   "src/effects/builtin/autopaneffect.cpp",
                   "src/effects/builtin/phasereffect.cpp",
                   "src/effects/builtin/metronomeeffect.cpp",
                   "src/effects/builtin/tremoloeffect.cpp",

                   "src/engine/effects/engineeffectsmanager.cpp",
                   "src/engine/effects/engineeffectrack.cpp",
                   "src/engine/effects/engineeffectchain.cpp",
                   "src/engine/effects/engineeffect.cpp",

                   "src/engine/sync/basesyncablelistener.cpp",
                   "src/engine/sync/enginesync.cpp",
                   "src/engine/sync/synccontrol.cpp",
                   "src/engine/sync/internalclock.cpp",

                   "src/engine/engineworker.cpp",
                   "src/engine/engineworkerscheduler.cpp",
                   "src/engine/enginebuffer.cpp",
                   "src/engine/bufferscalers/enginebufferscale.cpp",
                   "src/engine/bufferscalers/enginebufferscalelinear.cpp",
                   "src/engine/channels/engineaux.cpp",
                   "src/engine/channels/enginechannel.cpp",
                   "src/engine/channels/enginedeck.cpp",
                   "src/engine/channels/enginemicrophone.cpp",
                   "src/engine/filters/enginefilterbiquad1.cpp",
                   "src/engine/filters/enginefiltermoogladder4.cpp",
                   "src/engine/filters/enginefilterbessel4.cpp",
                   "src/engine/filters/enginefilterbessel8.cpp",
                   "src/engine/filters/enginefilterbutterworth4.cpp",
                   "src/engine/filters/enginefilterbutterworth8.cpp",
                   "src/engine/filters/enginefilterlinkwitzriley2.cpp",
                   "src/engine/filters/enginefilterlinkwitzriley4.cpp",
                   "src/engine/filters/enginefilterlinkwitzriley8.cpp",
                   "src/engine/filters/enginefilter.cpp",
                   "src/engine/engineobject.cpp",
                   "src/engine/enginepregain.cpp",
                   "src/engine/enginemaster.cpp",
                   "src/engine/enginedelay.cpp",
                   "src/engine/enginevumeter.cpp",
                   "src/engine/enginesidechaincompressor.cpp",
                   "src/engine/sidechain/enginesidechain.cpp",
                   "src/engine/sidechain/networkoutputstreamworker.cpp",
                   "src/engine/sidechain/networkinputstreamworker.cpp",
                   "src/engine/enginexfader.cpp",
                   "src/engine/channelmixer_autogen.cpp",
                   "src/engine/positionscratchcontroller.cpp",
                   "src/engine/controls/bpmcontrol.cpp",
                   "src/engine/controls/clockcontrol.cpp",
                   "src/engine/controls/cuecontrol.cpp",
                   "src/engine/controls/enginecontrol.cpp",
                   "src/engine/controls/keycontrol.cpp",
                   "src/engine/controls/loopingcontrol.cpp",
                   "src/engine/controls/quantizecontrol.cpp",
                   "src/engine/controls/ratecontrol.cpp",
                   "src/engine/readaheadmanager.cpp",
                   "src/engine/enginetalkoverducking.cpp",
                   "src/engine/cachingreader/cachingreader.cpp",
                   "src/engine/cachingreader/cachingreaderchunk.cpp",
                   "src/engine/cachingreader/cachingreaderworker.cpp",

                   "src/analyzer/trackanalysisscheduler.cpp",
                   "src/analyzer/analyzerthread.cpp",
                   "src/analyzer/analyzerwaveform.cpp",
                   "src/analyzer/analyzergain.cpp",
                   "src/analyzer/analyzerbeats.cpp",
                   "src/analyzer/analyzerkey.cpp",
                   "src/analyzer/analyzerebur128.cpp",
                   "src/analyzer/analyzersilence.cpp",
                   "src/analyzer/plugins/analyzersoundtouchbeats.cpp",
                   "src/analyzer/plugins/analyzerqueenmarybeats.cpp",
                   "src/analyzer/plugins/analyzerqueenmarykey.cpp",
                   "src/analyzer/plugins/buffering_utils.cpp",

                   "src/controllers/controller.cpp",
                   "src/controllers/controllerdebug.cpp",
                   "src/controllers/controllerengine.cpp",
                   "src/controllers/controllerenumerator.cpp",
                   "src/controllers/controllerlearningeventfilter.cpp",
                   "src/controllers/controllermanager.cpp",
                   "src/controllers/controllerpresetfilehandler.cpp",
                   "src/controllers/controllerpresetinfo.cpp",
                   "src/controllers/controllerpresetinfoenumerator.cpp",
                   "src/controllers/controlpickermenu.cpp",
                   "src/controllers/controllermappingtablemodel.cpp",
                   "src/controllers/controllerinputmappingtablemodel.cpp",
                   "src/controllers/controlleroutputmappingtablemodel.cpp",
                   "src/controllers/delegates/controldelegate.cpp",
                   "src/controllers/delegates/midichanneldelegate.cpp",
                   "src/controllers/delegates/midiopcodedelegate.cpp",
                   "src/controllers/delegates/midibytedelegate.cpp",
                   "src/controllers/delegates/midioptionsdelegate.cpp",
                   "src/controllers/learningutils.cpp",
                   "src/controllers/midi/midimessage.cpp",
                   "src/controllers/midi/midiutils.cpp",
                   "src/controllers/midi/midicontroller.cpp",
                   "src/controllers/midi/midicontrollerpresetfilehandler.cpp",
                   "src/controllers/midi/midienumerator.cpp",
                   "src/controllers/midi/midioutputhandler.cpp",
                   "src/controllers/softtakeover.cpp",
                   "src/controllers/keyboard/keyboardeventfilter.cpp",
                   "src/controllers/colorjsproxy.cpp",

                   "src/main.cpp",
                   "src/mixxx.cpp",
                   "src/mixxxapplication.cpp",
                   "src/errordialoghandler.cpp",

                   "src/sources/audiosource.cpp",
                   "src/sources/audiosourcestereoproxy.cpp",
                   "src/sources/metadatasourcetaglib.cpp",
                   "src/sources/soundsource.cpp",
                   "src/sources/soundsourceproviderregistry.cpp",
                   "src/sources/soundsourceproxy.cpp",

                   "src/widget/controlwidgetconnection.cpp",
                   "src/widget/wbasewidget.cpp",
                   "src/widget/wwidget.cpp",
                   "src/widget/wwidgetgroup.cpp",
                   "src/widget/wwidgetstack.cpp",
                   "src/widget/wsizeawarestack.cpp",
                   "src/widget/wlabel.cpp",
                   "src/widget/wtracktext.cpp",
                   "src/widget/wnumber.cpp",
                   "src/widget/wbeatspinbox.cpp",
                   "src/widget/wnumberdb.cpp",
                   "src/widget/wnumberpos.cpp",
                   "src/widget/wnumberrate.cpp",
                   "src/widget/wknob.cpp",
                   "src/widget/wknobcomposed.cpp",
                   "src/widget/wdisplay.cpp",
                   "src/widget/wvumeter.cpp",
                   "src/widget/wpushbutton.cpp",
                   "src/widget/weffectpushbutton.cpp",
                   "src/widget/wslidercomposed.cpp",
                   "src/widget/wstatuslight.cpp",
                   "src/widget/woverview.cpp",
                   "src/widget/woverviewlmh.cpp",
                   "src/widget/woverviewhsv.cpp",
                   "src/widget/woverviewrgb.cpp",
                   "src/widget/wspinny.cpp",
                   "src/widget/wskincolor.cpp",
                   "src/widget/wsearchlineedit.cpp",
                   "src/widget/wpixmapstore.cpp",
                   "src/widget/paintable.cpp",
                   "src/widget/wimagestore.cpp",
                   "src/widget/hexspinbox.cpp",
                   "src/widget/wtrackproperty.cpp",
                   "src/widget/wstarrating.cpp",
                   "src/widget/weffectchain.cpp",
                   "src/widget/weffect.cpp",
                   "src/widget/weffectselector.cpp",
                   "src/widget/weffectparameter.cpp",
                   "src/widget/weffectparameterknob.cpp",
                   "src/widget/weffectparameterknobcomposed.cpp",
                   "src/widget/weffectbuttonparameter.cpp",
                   "src/widget/weffectparameterbase.cpp",
                   "src/widget/wtime.cpp",
                   "src/widget/wrecordingduration.cpp",
                   "src/widget/wkey.cpp",
                   "src/widget/wbattery.cpp",
                   "src/widget/wcombobox.cpp",
                   "src/widget/wsplitter.cpp",
                   "src/widget/wcoverart.cpp",
                   "src/widget/wcoverartlabel.cpp",
                   "src/widget/wcoverartmenu.cpp",
                   "src/widget/wsingletoncontainer.cpp",
                   "src/widget/wmainmenubar.cpp",

                   "src/musicbrainz/network.cpp",
                   "src/musicbrainz/tagfetcher.cpp",
                   "src/musicbrainz/gzip.cpp",
                   "src/musicbrainz/crc.c",
                   "src/musicbrainz/acoustidclient.cpp",
                   "src/musicbrainz/chromaprinter.cpp",
                   "src/musicbrainz/musicbrainzclient.cpp",

                   "src/widget/wtracktableview.cpp",
                   "src/widget/wtracktableviewheader.cpp",
                   "src/widget/wlibrarysidebar.cpp",
                   "src/widget/wlibrary.cpp",
                   "src/widget/wlibrarytableview.cpp",
                   "src/widget/wanalysislibrarytableview.cpp",
                   "src/widget/wlibrarytextbrowser.cpp",

                   "src/database/mixxxdb.cpp",
                   "src/database/schemamanager.cpp",

                   "src/library/trackcollection.cpp",
                   "src/library/basesqltablemodel.cpp",
                   "src/library/basetrackcache.cpp",
                   "src/library/columncache.cpp",
                   "src/library/librarytablemodel.cpp",
                   "src/library/searchquery.cpp",
                   "src/library/searchqueryparser.cpp",
                   "src/library/analysislibrarytablemodel.cpp",
                   "src/library/missingtablemodel.cpp",
                   "src/library/hiddentablemodel.cpp",
                   "src/library/proxytrackmodel.cpp",
                   "src/library/coverart.cpp",
                   "src/library/coverartcache.cpp",
                   "src/library/coverartutils.cpp",

                   "src/library/crate/cratestorage.cpp",
                   "src/library/crate/cratefeature.cpp",
                   "src/library/crate/cratefeaturehelper.cpp",
                   "src/library/crate/cratetablemodel.cpp",

                   "src/library/playlisttablemodel.cpp",
                   "src/library/libraryfeature.cpp",
                   "src/library/analysisfeature.cpp",
                   "src/library/autodj/autodjfeature.cpp",
                   "src/library/autodj/autodjprocessor.cpp",
                   "src/library/dao/directorydao.cpp",
                   "src/library/mixxxlibraryfeature.cpp",
                   "src/library/baseplaylistfeature.cpp",
                   "src/library/playlistfeature.cpp",
                   "src/library/setlogfeature.cpp",
                   "src/library/autodj/dlgautodj.cpp",
                   "src/library/dlganalysis.cpp",
                   "src/library/dlgcoverartfullsize.cpp",
                   "src/library/dlghidden.cpp",
                   "src/library/dlgmissing.cpp",
                   "src/library/dlgtagfetcher.cpp",
                   "src/library/dlgtrackinfo.cpp",
                   "src/library/dlgtrackmetadataexport.cpp",

                   "src/library/browse/browsetablemodel.cpp",
                   "src/library/browse/browsethread.cpp",
                   "src/library/browse/browsefeature.cpp",
                   "src/library/browse/foldertreemodel.cpp",

                   "src/library/export/trackexportdlg.cpp",
                   "src/library/export/trackexportwizard.cpp",
                   "src/library/export/trackexportworker.cpp",

                   "src/library/recording/recordingfeature.cpp",
                   "src/library/recording/dlgrecording.cpp",
                   "src/recording/recordingmanager.cpp",
                   "src/engine/sidechain/enginerecord.cpp",

                   # External Library Features
                   "src/library/baseexternallibraryfeature.cpp",
                   "src/library/baseexternaltrackmodel.cpp",
                   "src/library/baseexternalplaylistmodel.cpp",
                   "src/library/rhythmbox/rhythmboxfeature.cpp",

                   "src/library/banshee/bansheefeature.cpp",
                   "src/library/banshee/bansheeplaylistmodel.cpp",
                   "src/library/banshee/bansheedbconnection.cpp",

                   "src/library/itunes/itunesfeature.cpp",
                   "src/library/traktor/traktorfeature.cpp",

                   "src/library/sidebarmodel.cpp",
                   "src/library/library.cpp",

                   "src/library/scanner/libraryscanner.cpp",
                   "src/library/scanner/libraryscannerdlg.cpp",
                   "src/library/scanner/scannertask.cpp",
                   "src/library/scanner/importfilestask.cpp",
                   "src/library/scanner/recursivescandirectorytask.cpp",

                   "src/library/dao/cuedao.cpp",
                   "src/library/dao/trackdao.cpp",
                   "src/library/dao/playlistdao.cpp",
                   "src/library/dao/libraryhashdao.cpp",
                   "src/library/dao/settingsdao.cpp",
                   "src/library/dao/analysisdao.cpp",
                   "src/library/dao/autodjcratesdao.cpp",

                   "src/library/librarycontrol.cpp",
                   "src/library/songdownloader.cpp",
                   "src/library/starrating.cpp",
                   "src/library/stardelegate.cpp",
                   "src/library/stareditor.cpp",
                   "src/library/bpmdelegate.cpp",
                   "src/library/previewbuttondelegate.cpp",
                   "src/library/coverartdelegate.cpp",
                   "src/library/locationdelegate.cpp",
                   "src/library/tableitemdelegate.cpp",

                   "src/library/treeitemmodel.cpp",
                   "src/library/treeitem.cpp",

                   "src/library/parser.cpp",
                   "src/library/parserpls.cpp",
                   "src/library/parserm3u.cpp",
                   "src/library/parsercsv.cpp",

                   "src/widget/wwaveformviewer.cpp",

                   "src/waveform/sharedglcontext.cpp",
                   "src/waveform/waveform.cpp",
                   "src/waveform/waveformfactory.cpp",
                   "src/waveform/waveformwidgetfactory.cpp",
                   "src/waveform/vsyncthread.cpp",
                   "src/waveform/guitick.cpp",
                   "src/waveform/visualsmanager.cpp",
                   "src/waveform/visualplayposition.cpp",
                   "src/waveform/renderers/waveformwidgetrenderer.cpp",
                   "src/waveform/renderers/waveformrendererabstract.cpp",
                   "src/waveform/renderers/waveformrenderbackground.cpp",
                   "src/waveform/renderers/waveformrendermark.cpp",
                   "src/waveform/renderers/waveformrendermarkrange.cpp",
                   "src/waveform/renderers/waveformrenderbeat.cpp",
                   "src/waveform/renderers/waveformrendererendoftrack.cpp",
                   "src/waveform/renderers/waveformrendererpreroll.cpp",

                   "src/waveform/renderers/waveformrendererfilteredsignal.cpp",
                   "src/waveform/renderers/waveformrendererhsv.cpp",
                   "src/waveform/renderers/waveformrendererrgb.cpp",
                   "src/waveform/renderers/qtwaveformrendererfilteredsignal.cpp",
                   "src/waveform/renderers/qtwaveformrenderersimplesignal.cpp",
                   "src/waveform/renderers/qtvsynctestrenderer.cpp",

                   "src/waveform/renderers/waveformsignalcolors.cpp",

                   "src/waveform/renderers/waveformrenderersignalbase.cpp",
                   "src/waveform/renderers/waveformmark.cpp",
                   "src/waveform/renderers/waveformmarkproperties.cpp",
                   "src/waveform/renderers/waveformmarkset.cpp",
                   "src/waveform/renderers/waveformmarkrange.cpp",
                   "src/waveform/renderers/glwaveformrenderersimplesignal.cpp",
                   "src/waveform/renderers/glwaveformrendererrgb.cpp",
                   "src/waveform/renderers/glwaveformrendererfilteredsignal.cpp",
                   "src/waveform/renderers/glslwaveformrenderersignal.cpp",
                   "src/waveform/renderers/glvsynctestrenderer.cpp",

                   "src/waveform/widgets/waveformwidgetabstract.cpp",
                   "src/waveform/widgets/emptywaveformwidget.cpp",
                   "src/waveform/widgets/softwarewaveformwidget.cpp",
                   "src/waveform/widgets/hsvwaveformwidget.cpp",
                   "src/waveform/widgets/rgbwaveformwidget.cpp",
                   "src/waveform/widgets/qthsvwaveformwidget.cpp",
                   "src/waveform/widgets/qtrgbwaveformwidget.cpp",
                   "src/waveform/widgets/qtwaveformwidget.cpp",
                   "src/waveform/widgets/qtsimplewaveformwidget.cpp",
                   "src/waveform/widgets/qtvsynctestwidget.cpp",
                   "src/waveform/widgets/glwaveformwidget.cpp",
                   "src/waveform/widgets/glsimplewaveformwidget.cpp",
                   "src/waveform/widgets/glvsynctestwidget.cpp",

                   "src/waveform/widgets/glslwaveformwidget.cpp",

                   "src/waveform/widgets/glrgbwaveformwidget.cpp",

                   "src/skin/imginvert.cpp",
                   "src/skin/imgloader.cpp",
                   "src/skin/imgcolor.cpp",
                   "src/skin/skinloader.cpp",
                   "src/skin/legacyskinparser.cpp",
                   "src/skin/colorschemeparser.cpp",
                   "src/skin/tooltips.cpp",
                   "src/skin/skincontext.cpp",
                   "src/skin/svgparser.cpp",
                   "src/skin/pixmapsource.cpp",
                   "src/skin/launchimage.cpp",

                   "src/track/beatfactory.cpp",
                   "src/track/beatgrid.cpp",
                   "src/track/beatmap.cpp",
                   "src/track/beatutils.cpp",
                   "src/track/beats.cpp",
                   "src/track/bpm.cpp",
                   "src/track/cue.cpp",
                   "src/track/keyfactory.cpp",
                   "src/track/keys.cpp",
                   "src/track/keyutils.cpp",
                   "src/track/playcounter.cpp",
                   "src/track/replaygain.cpp",
                   "src/track/track.cpp",
                   "src/track/globaltrackcache.cpp",
                   "src/track/trackfile.cpp",
                   "src/track/trackmetadata.cpp",
                   "src/track/trackmetadatataglib.cpp",
                   "src/track/tracknumbers.cpp",
                   "src/track/albuminfo.cpp",
                   "src/track/trackinfo.cpp",
                   "src/track/trackrecord.cpp",
                   "src/track/trackref.cpp",

                   "src/mixer/auxiliary.cpp",
                   "src/mixer/baseplayer.cpp",
                   "src/mixer/basetrackplayer.cpp",
                   "src/mixer/deck.cpp",
                   "src/mixer/microphone.cpp",
                   "src/mixer/playerinfo.cpp",
                   "src/mixer/playermanager.cpp",
                   "src/mixer/previewdeck.cpp",
                   "src/mixer/sampler.cpp",
                   "src/mixer/samplerbank.cpp",

                   "src/soundio/sounddevice.cpp",
                   "src/soundio/sounddevicenetwork.cpp",
                   "src/engine/sidechain/enginenetworkstream.cpp",
                   "src/soundio/soundmanager.cpp",
                   "src/soundio/soundmanagerconfig.cpp",
                   "src/soundio/soundmanagerutil.cpp",

                   "src/encoder/encoder.cpp",
                   "src/encoder/encoderbroadcastsettings.cpp",
                   "src/encoder/encoderflacsettings.cpp",
                   "src/encoder/encodermp3.cpp",
                   "src/encoder/encodermp3settings.cpp",
                   "src/encoder/encodersndfileflac.cpp",
                   "src/encoder/encodervorbis.cpp",
                   "src/encoder/encodervorbissettings.cpp",
                   "src/encoder/encoderwave.cpp",
                   "src/encoder/encoderwavesettings.cpp",
                   'src/encoder/encoderopussettings.cpp',

                   "src/util/sleepableqthread.cpp",
                   "src/util/statsmanager.cpp",
                   "src/util/stat.cpp",
                   "src/util/statmodel.cpp",
                   "src/util/dnd.cpp",
                   "src/util/duration.cpp",
                   "src/util/time.cpp",
                   "src/util/timer.cpp",
                   "src/util/performancetimer.cpp",
                   "src/util/threadcputimer.cpp",
                   "src/util/version.cpp",
                   "src/util/rlimit.cpp",
                   "src/util/battery/battery.cpp",
                   "src/util/valuetransformer.cpp",
                   "src/util/sandbox.cpp",
                   "src/util/file.cpp",
                   "src/util/mac.cpp",
                   "src/util/task.cpp",
                   "src/util/experiment.cpp",
                   "src/util/xml.cpp",
                   "src/util/tapfilter.cpp",
                   "src/util/movinginterquartilemean.cpp",
                   "src/util/console.cpp",
                   "src/util/color/color.cpp",
                   "src/util/db/dbconnection.cpp",
                   "src/util/db/dbconnectionpool.cpp",
                   "src/util/db/dbconnectionpooler.cpp",
                   "src/util/db/dbconnectionpooled.cpp",
                   "src/util/db/dbid.cpp",
                   "src/util/db/fwdsqlquery.cpp",
                   "src/util/db/fwdsqlqueryselectresult.cpp",
                   "src/util/db/sqllikewildcardescaper.cpp",
                   "src/util/db/sqlqueryfinisher.cpp",
                   "src/util/db/sqlstringformatter.cpp",
                   "src/util/db/sqltransaction.cpp",
                   "src/util/sample.cpp",
                   "src/util/samplebuffer.cpp",
                   "src/util/readaheadsamplebuffer.cpp",
                   "src/util/rotary.cpp",
                   "src/util/logger.cpp",
                   "src/util/logging.cpp",
                   "src/util/cmdlineargs.cpp",
                   "src/util/audiosignal.cpp",
                   "src/util/widgethider.cpp",
                   "src/util/autohidpi.cpp",
                   "src/util/screensaver.cpp",
                   "src/util/indexrange.cpp",
                   "src/util/desktophelper.cpp",
                   "src/util/widgetrendertimer.cpp",
                   "src/util/workerthread.cpp",
                   "src/util/workerthreadscheduler.cpp",
                   "src/util/color/predefinedcolor.cpp"
                   ]

        proto_args = {
            'PROTOCPROTOPATH': ['src'],
            'PROTOCPYTHONOUTDIR': '',  # set to None to not generate python
            'PROTOCOUTDIR': os.path.join(build.build_dir, 'src'),
            'PROTOCCPPOUTFLAGS': '',
            #'PROTOCCPPOUTFLAGS': "dllexport_decl=PROTOCONFIG_EXPORT:"
        }
        proto_sources = SCons.Glob('src/proto/*.proto')
        proto_objects = [build.env.Protoc([], proto_source, **proto_args)[0]
                         for proto_source in proto_sources]
        sources.extend(proto_objects)

        # Uic these guys (they're moc'd automatically after this) - Generates
        # the code for the QT UI forms.
        ui_files = [
            'src/controllers/dlgcontrollerlearning.ui',
            'src/controllers/dlgprefcontrollerdlg.ui',
            'src/controllers/dlgprefcontrollersdlg.ui',
            'src/dialog/dlgaboutdlg.ui',
            'src/dialog/dlgdevelopertoolsdlg.ui',
            'src/library/autodj/dlgautodj.ui',
            'src/library/dlganalysis.ui',
            'src/library/dlgcoverartfullsize.ui',
            'src/library/dlghidden.ui',
            'src/library/dlgmissing.ui',
            'src/library/dlgtagfetcher.ui',
            'src/library/dlgtrackinfo.ui',
            'src/library/export/dlgtrackexport.ui',
            'src/library/recording/dlgrecording.ui',
            'src/preferences/dialog/dlgprefautodjdlg.ui',
            'src/preferences/dialog/dlgprefbeatsdlg.ui',
            'src/preferences/dialog/dlgprefdeckdlg.ui',
            'src/preferences/dialog/dlgprefcrossfaderdlg.ui',
            'src/preferences/dialog/dlgpreflv2dlg.ui',
            'src/preferences/dialog/dlgprefeffectsdlg.ui',
            'src/preferences/dialog/dlgprefeqdlg.ui',
            'src/preferences/dialog/dlgpreferencesdlg.ui',
            'src/preferences/dialog/dlgprefinterfacedlg.ui',
            'src/preferences/dialog/dlgprefkeydlg.ui',
            'src/preferences/dialog/dlgpreflibrarydlg.ui',
            'src/preferences/dialog/dlgprefnovinyldlg.ui',
            'src/preferences/dialog/dlgprefrecorddlg.ui',
            'src/preferences/dialog/dlgprefreplaygaindlg.ui',
            'src/preferences/dialog/dlgprefsounddlg.ui',
            'src/preferences/dialog/dlgprefsounditem.ui',
            'src/preferences/dialog/dlgprefvinyldlg.ui',
            'src/preferences/dialog/dlgprefwaveformdlg.ui',
        ]

        # In Python 3.x, map() returns a "map object" (instead of a list),
        # which is evaluated on-demand rather than at once. To invoke uic
        # for all *.ui files at once, we need to cast it to a list here.
        list(map(Qt.uic(build), ui_files))

        if build.platform_is_windows:
            # Add Windows resource file with icons and such
            # force manifest file creation, apparently not necessary for all
            # people but necessary for this committers handicapped windows
            # installation -- bkgood
            if build.toolchain_is_msvs:
                build.env.Append(LINKFLAGS="/MANIFEST")
        elif build.platform_is_osx:
            # Need extra room for code signing (App Store)
            build.env.Append(LINKFLAGS="-Wl,-headerpad,ffff")
            build.env.Append(LINKFLAGS="-Wl,-headerpad_max_install_names")

        return sources

    def configure(self, build, conf):
        # Evaluate this define. There are a lot of different things around the
        # codebase that use different defines. (AMD64, x86_64, x86, i386, i686,
        # EM64T). We need to unify them together.
        if not build.machine == 'alpha':
            build.env.Append(CPPDEFINES=build.machine)

        # TODO(rryan): Quick hack to get the build number in title bar. Clean up
        # later.
        if int(SCons.ARGUMENTS.get('build_number_in_title_bar', 0)):
            build.env.Append(CPPDEFINES='MIXXX_BUILD_NUMBER_IN_TITLE_BAR')

        if build.build_is_debug:
            build.env.Append(CPPDEFINES='MIXXX_BUILD_DEBUG')
        elif build.build_is_release:
            build.env.Append(CPPDEFINES='MIXXX_BUILD_RELEASE')
            # Disable assert.h assertions in release mode. Some libraries use
            # this as a signal for when to enable code that should be disabled
            # in release mode.
            build.env.Append(CPPDEFINES='NDEBUG')

            # In a release build we want to disable all Q_ASSERTs in Qt headers
            # that we include. We can't define QT_NO_DEBUG because that would
            # mean turning off QDebug output. qt_noop() is what Qt defined
            # Q_ASSERT to be when QT_NO_DEBUG is defined in Qt 5.9 and earlier.
            # Now it is defined as static_cast<void>(false&&(x)) to support use
            # in constexpr functions. We still use qt_noop on Windows since we
            # can't specify static_cast<void>(false&&(x)) in a commandline
            # macro definition, but it seems VS 2015 isn't bothered by the use
            # qt_noop here, so we can keep it.
            if build.platform_is_windows:
                build.env.Append(CPPDEFINES="'Q_ASSERT(x)=qt_noop()'")
            else:
                build.env.Append(CPPDEFINES="'Q_ASSERT(x)=static_cast<void>(false&&(x))'")

        if int(SCons.ARGUMENTS.get('debug_assertions_fatal', 0)):
            build.env.Append(CPPDEFINES='MIXXX_DEBUG_ASSERTIONS_FATAL')

        if build.toolchain_is_gnu:
            # Default GNU Options
            build.env.Append(CCFLAGS='-pipe')
            build.env.Append(CCFLAGS='-Wall')
            build.env.Append(CCFLAGS='-Wextra')

            if build.compiler_is_gcc and build.gcc_major_version >= 9:
                # Avoid many warnings from GCC 9 about implicitly defined copy assignment
                # operators that are deprecated for classes with a user-provided copy
                # constructor. This affects both Qt 5.12 and Mixxx.
                build.env.Append(CXXFLAGS='-Wno-deprecated-copy')

            if build.compiler_is_clang:
                # Quiet down Clang warnings about inconsistent use of override
                # keyword until Qt fixes qt_metacall.
                build.env.Append(CCFLAGS='-Wno-inconsistent-missing-override')

                # Do not warn about use of the deprecated 'register' keyword
                # since it produces noise from libraries we depend on using it.
                build.env.Append(CCFLAGS='-Wno-deprecated-register')

                # Warn about implicit fallthrough.
                build.env.Append(CCFLAGS='-Wimplicit-fallthrough')

                # Enable thread-safety analysis.
                # http://clang.llvm.org/docs/ThreadSafetyAnalysis.html
                build.env.Append(CCFLAGS='-Wthread-safety')

            # Always generate debugging info.
            build.env.Append(CCFLAGS='-g')
        elif build.toolchain_is_msvs:
            # Validate the specified winlib directory exists
            mixxx_lib_path = build.winlib_path
            if not os.path.exists(mixxx_lib_path):
                raise Exception("Winlib path does not exist! Please specify your winlib directory"
                                "path by running 'scons winlib=[path]'")
                Script.Exit(1)

            # Set include and library paths to work with this
            build.env.Append(CPPPATH=[mixxx_lib_path,
                                      os.path.join(mixxx_lib_path, 'include')])
            build.env.Append(LIBPATH=[mixxx_lib_path, os.path.join(mixxx_lib_path, 'lib')])

            # Find executables (e.g. protoc) in the winlib path
            build.env.AppendENVPath('PATH', mixxx_lib_path)
            build.env.AppendENVPath('PATH', os.path.join(mixxx_lib_path, 'bin'))

            # Valid values of /MACHINE are: {ARM|EBC|X64|X86}
            # http://msdn.microsoft.com/en-us/library/5wy54dk2.aspx
            if build.architecture_is_x86:
                if build.machine_is_64bit:
                    build.env.Append(LINKFLAGS='/MACHINE:X64')
                else:
                    build.env.Append(LINKFLAGS='/MACHINE:X86')
            elif build.architecture_is_arm:
                build.env.Append(LINKFLAGS='/MACHINE:ARM')
            else:
                raise Exception('Invalid machine type for Windows build.')

            # Build with multiple processes. TODO(XXX) make this configurable.
            # http://msdn.microsoft.com/en-us/library/bb385193.aspx
            build.env.Append(CCFLAGS='/MP')

            # Generate debugging information for compilation units and
            # executables linked if we are creating a debug build or bundling
            # PDBs is enabled.  Having PDB files for our releases is helpful for
            # debugging, but increases link times and memory usage
            # significantly.
            if build.build_is_debug or build.bundle_pdbs:
                build.env.Append(LINKFLAGS='/DEBUG')
                build.env.Append(CCFLAGS='/Zi /Fd${TARGET}.pdb')

            if build.build_is_debug:
                # Important: We always build Mixxx with the Multi-Threaded DLL
                # runtime because Mixxx loads DLLs at runtime. Since this is a
                # debug build, use the debug version of the MD runtime.
                build.env.Append(CCFLAGS='/MDd')
            else:
                # Important: We always build Mixxx with the Multi-Threaded DLL
                # runtime because Mixxx loads DLLs at runtime.
                build.env.Append(CCFLAGS='/MD')

        if build.platform_is_windows:
            build.env.Append(CPPDEFINES='__WINDOWS__')
            # Restrict ATL to XP-compatible SDK functions.
            # TODO(rryan): Remove once we ditch XP support.
            build.env.Append(CPPDEFINES='_ATL_XP_TARGETING')
            build.env.Append(
                CPPDEFINES='_ATL_MIN_CRT')  # Helps prevent duplicate symbols
            # Need this on Windows until we have UTF16 support in Mixxx
            # use stl min max defines
            # http://connect.microsoft.com/VisualStudio/feedback/details/553420/std-cpp-max-and-std-cpp-min-not-available-in-visual-c-2010
            build.env.Append(CPPDEFINES='NOMINMAX')
            build.env.Append(CPPDEFINES='UNICODE')
            build.env.Append(
                CPPDEFINES='WIN%s' % build.bitwidth)  # WIN32 or WIN64
            # Tobias: Don't remove this line
            # I used the Windows API in foldertreemodel.cpp
            # to quickly test if a folder has subfolders
            build.env.Append(LIBS='shell32')

            # Causes the cmath headers to declare M_PI and friends.
            # http://msdn.microsoft.com/en-us/library/4hwaceh6.aspx
            # We could define this in our headers but then include order
            # matters since headers we don't control may include cmath first.
            build.env.Append(CPPDEFINES='_USE_MATH_DEFINES')

        elif build.platform_is_linux:
            build.env.Append(CPPDEFINES='__LINUX__')

            # Check for pkg-config >= 0.15.0
            if not conf.CheckForPKGConfig('0.15.0'):
                raise Exception('pkg-config >= 0.15.0 not found.')

            if not conf.CheckLib(['X11', 'libX11']):
                raise Exception(
                    "Could not find libX11 or its development headers.")

        elif build.platform_is_osx:
            # Stuff you may have compiled by hand
            if os.path.isdir('/usr/local/include'):
                build.env.Append(LIBPATH=['/usr/local/lib'])
                # Use -isystem instead of -I to avoid compiler warnings from
                # system libraries. This cuts down on Mixxx's compilation output
                # significantly when using Homebrew installed to /usr/local.
                build.env.Append(CCFLAGS=['-isystem', '/usr/local/include'])

        elif build.platform_is_bsd:
            build.env.Append(CPPDEFINES='__BSD__')
            build.env.Append(CPPPATH=['/usr/include',
                                      '/usr/local/include',
                                      '/usr/X11R6/include/'])
            build.env.Append(LIBPATH=['/usr/lib/',
                                      '/usr/local/lib',
                                      '/usr/X11R6/lib'])
            build.env.Append(LIBS='pthread')

        # Define for things that would like to special case UNIX (Linux or BSD)
        if build.platform_is_bsd or build.platform_is_linux:
            build.env.Append(CPPDEFINES='__UNIX__')

        # Add the src/ directory to the include path
        build.env.Append(CPPPATH=['src'])

        # Set up flags for config/track listing files
        # SETTINGS_PATH not needed for windows and MacOSX because we now use QDesktopServices::storageLocation(QDesktopServices::DataLocation)
        if build.platform_is_linux or \
                build.platform_is_bsd:
            mixxx_files = [
                # TODO(XXX) Trailing slash not needed anymore as we switches from String::append
                # to QDir::filePath elsewhere in the code. This is candidate for removal.
                ('SETTINGS_PATH', '.mixxx/'),
                ('SETTINGS_FILE', 'mixxx.cfg')]
        elif build.platform_is_osx:
            mixxx_files = [
                ('SETTINGS_FILE', 'mixxx.cfg')]
        elif build.platform_is_windows:
            mixxx_files = [
                ('SETTINGS_FILE', 'mixxx.cfg')]

        # Escape the filenames so they don't end up getting screwed up in the
        # shell.
        mixxx_files = [(k, r'\"%s\"' % v) for k, v in mixxx_files]
        build.env.Append(CPPDEFINES=mixxx_files)

        # Say where to find resources on Unix. TODO(XXX) replace this with a
        # RESOURCE_PATH that covers Win and OSX too:
        if build.platform_is_linux or build.platform_is_bsd:
            prefix = SCons.ARGUMENTS.get('prefix', '/usr/local')
            share_path = os.path.join (prefix, build.env.get(
                'SHAREDIR', default='share'), 'mixxx')
            build.env.Append(
                CPPDEFINES=('UNIX_SHARE_PATH', r'\"%s\"' % share_path))
            lib_path = os.path.join(prefix, build.env.get(
                'LIBDIR', default='lib'), 'mixxx')
            build.env.Append(
                CPPDEFINES=('UNIX_LIB_PATH', r'\"%s\"' % lib_path))

    def depends(self, build):
        return [SoundTouch, ReplayGain, Ebur128Mit, PortAudio, PortMIDI, Qt, TestHeaders,
                FidLib, SndFile, FLAC, OggVorbis, OpenGL, TagLib, ProtoBuf,
                Chromaprint, RubberBand, SecurityFramework, CoreServices, IOKit,
                QtScriptByteArray, Reverb, FpClassify, PortAudioRingBuffer, LAME,
                QueenMaryDsp]

    def post_dependency_check_configure(self, build, conf):
        """Sets up additional things in the Environment that must happen
        after the Configure checks run."""
        if build.platform_is_windows:
            if build.toolchain_is_msvs:
                if not build.static_dependencies or build.build_is_debug:
                    build.env.Append(LINKFLAGS=['/nodefaultlib:LIBCMT.lib',
                                                '/nodefaultlib:LIBCMTd.lib'])

                build.env.Append(LINKFLAGS='/entry:mainCRTStartup')
                # Makes the program not launch a shell first. 6.01 declares the
                # minimum version to be Windows 7.
                # https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/compiler-options/subsystemversion-compiler-option
                build.env.Append(LINKFLAGS='/subsystem:windows,6.01')
                # Force MSVS to generate a manifest (MSVC2010)
                build.env.Append(LINKFLAGS='/manifest')
            elif build.toolchain_is_gnu:
                # Makes the program not launch a shell first
                build.env.Append(LINKFLAGS='--subsystem,windows')
                build.env.Append(LINKFLAGS='-mwindows')
