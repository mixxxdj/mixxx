# -*- coding: utf-8 -*-

import os
import util
from mixxx import Dependence, Feature
import SCons.Script as SCons


class PortAudio(Dependence):

    def configure(self, build, conf):
        if not conf.CheckLib('portaudio'):
            raise Exception(
                'Did not find libportaudio.a, portaudio.lib, or the PortAudio-v19 development header files.')

        # Turn on PortAudio support in Mixxx
        build.env.Append(CPPDEFINES='__PORTAUDIO__')

        if build.platform_is_windows and build.static_dependencies:
            conf.CheckLib('advapi32')

    def sources(self, build):
        return ['sounddeviceportaudio.cpp']


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
        return ['controllers/midi/portmidienumerator.cpp', 'controllers/midi/portmidicontroller.cpp']


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


class OggVorbis(Dependence):

    def configure(self, build, conf):
#        if build.platform_is_windows and build.machine_is_64bit:
            # For some reason this has to be checked this way on win64,
            # otherwise it looks for the dll lib which will cause a conflict
            # later
#            if not conf.CheckLib('vorbisfile_static'):
#                raise Exception('Did not find vorbisfile_static.lib or the libvorbisfile development headers.')
#        else:
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
        return ['soundsourceoggvorbis.cpp']

class SndFile(Dependence):

    def configure(self, build, conf):
        # if not conf.CheckLibWithHeader(['sndfile', 'libsndfile', 'libsndfile-1'], 'sndfile.h', 'C'):
        # TODO: check for debug version on Windows when one is available
        if not conf.CheckLib(['sndfile', 'libsndfile', 'libsndfile-1']):
            raise Exception(
                "Did not find libsndfile or it\'s development headers")
        build.env.Append(CPPDEFINES='__SNDFILE__')

    def sources(self, build):
        return ['soundsourcesndfile.cpp']


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
        return ['soundsourceflac.cpp', ]


class Qt(Dependence):
    DEFAULT_QT4DIRS = {'linux': '/usr/share/qt4',
                       'bsd': '/usr/local/lib/qt4',
                       'osx': '/Library/Frameworks',
                       'windows': 'C:\\qt\\4.6.0'}

    DEFAULT_QT5DIRS64 = {'linux': '/usr/lib/x86_64-linux-gnu/qt5',
                         'osx': '/Library/Frameworks',
                         'windows': 'C:\\qt\\5.0.1'}

    DEFAULT_QT5DIRS32 = {'linux': '/usr/lib/i386-linux-gnu/qt5',
                         'osx': '/Library/Frameworks',
                         'windows': 'C:\\qt\\5.0.1'}

    @staticmethod
    def qt5_enabled(build):
        return int(util.get_flags(build.env, 'qt5', 0))

    @staticmethod
    def uic(build):
        qt5 = Qt.qt5_enabled(build)
        return build.env.Uic5 if qt5 else build.env.Uic4

    @staticmethod
    def find_framework_path(qtdir):
        for d in (os.path.join(qtdir, x) for x in ['', 'Frameworks', 'lib']):
            core = os.path.join(d, 'QtCore.framework')
            if os.path.isdir(core):
                return d
        return None

    @staticmethod
    def enabled_modules(build):
        qt5 = Qt.qt5_enabled(build)
        qt_modules = [
            'QtCore', 'QtGui', 'QtOpenGL', 'QtXml', 'QtSvg',
            'QtSql', 'QtScript', 'QtXmlPatterns', 'QtNetwork',
            'QtTest', 'QtScriptTools'
        ]
        if qt5:
            qt_modules.extend(['QtWidgets', 'QtConcurrent'])
        return qt_modules

    def satisfy(self):
        pass

    def configure(self, build, conf):
        qt_modules = Qt.enabled_modules(build)

        qt5 = Qt.qt5_enabled(build)
        # Emit various Qt defines
        build.env.Append(CPPDEFINES=['QT_SHARED',
                                     'QT_TABLET_SUPPORT'])
        if qt5:
            # Enable qt4 support.
            build.env.Append(CPPDEFINES='QT_DISABLE_DEPRECATED_BEFORE')

        # Set qt_sqlite_plugin flag if we should package the Qt SQLite plugin.
        build.flags['qt_sqlite_plugin'] = util.get_flags(
            build.env, 'qt_sqlite_plugin', 0)

        # Enable Qt include paths
        if build.platform_is_linux:
            if qt5 and not conf.CheckForPKG('Qt5Core', '5.0'):
                raise Exception('Qt >= 5.0 not found')
            elif not qt5 and not conf.CheckForPKG('QtCore', '4.6'):
                raise Exception('QT >= 4.6 not found')

            # This automatically converts QtXXX to Qt5XXX where appropriate.
            if qt5:
                build.env.EnableQt5Modules(qt_modules, debug=False)
            else:
                build.env.EnableQt4Modules(qt_modules, debug=False)

            if qt5:
                # Note that -reduce-relocations is enabled by default in Qt5.
                # So we must build the code with position independent code
                build.env.Append(CCFLAGS='-fPIE')

        elif build.platform_is_bsd:
            build.env.Append(LIBS=qt_modules)
            include_paths = ['$QTDIR/include/%s' % module
                             for module in qt_modules]
            build.env.Append(CPPPATH=include_paths)
        elif build.platform_is_osx:
            qtdir = build.env['QTDIR']
            build.env.Append(
                LINKFLAGS=' '.join('-framework %s' % m for m in qt_modules)
            )
            framework_path = Qt.find_framework_path(qtdir)
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

            # Copied verbatim from qt4.py and qt5.py.
            # TODO(rryan): Get our fixes merged upstream so we can use qt4.py
            # and qt5.py for OS X.
            qt4_module_defines = {
                'QtScript'   : ['QT_SCRIPT_LIB'],
                'QtSvg'      : ['QT_SVG_LIB'],
                'Qt3Support' : ['QT_QT3SUPPORT_LIB','QT3_SUPPORT'],
                'QtSql'      : ['QT_SQL_LIB'],
                'QtXml'      : ['QT_XML_LIB'],
                'QtOpenGL'   : ['QT_OPENGL_LIB'],
                'QtGui'      : ['QT_GUI_LIB'],
                'QtNetwork'  : ['QT_NETWORK_LIB'],
                'QtCore'     : ['QT_CORE_LIB'],
            }
            qt5_module_defines = {
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

            module_defines = qt5_module_defines if qt5 else qt4_module_defines
            for module in qt_modules:
                build.env.AppendUnique(CPPDEFINES=module_defines.get(module, []))

            if qt5:
                build.env["QT5_MOCCPPPATH"] = build.env["CPPPATH"]
            else:
                build.env["QT4_MOCCPPPATH"] = build.env["CPPPATH"]
        elif build.platform_is_windows:
            # This automatically converts QtCore to QtCore[45][d] where
            # appropriate.
            if qt5:
                build.env.EnableQt5Modules(qt_modules,
                                           debug=build.build_is_debug)
            else:
                build.env.EnableQt4Modules(qt_modules,
                                           debug=build.build_is_debug)

            # if build.static_dependencies:
                # # Pulled from qt-4.8.2-source\mkspecs\win32-msvc2010\qmake.conf
                # # QtCore
                # build.env.Append(LIBS = 'kernel32')
                # build.env.Append(LIBS = 'user32') # QtGui, QtOpenGL, libHSS1394
                # build.env.Append(LIBS = 'shell32')
                # build.env.Append(LIBS = 'uuid')
                # build.env.Append(LIBS = 'ole32') # QtGui,
                # build.env.Append(LIBS = 'advapi32') # QtGui, portaudio, portmidi
                # build.env.Append(LIBS = 'ws2_32')   # QtGui, QtNetwork, libshout
                # # QtGui
                # build.env.Append(LIBS = 'gdi32') #QtOpenGL
                # build.env.Append(LIBS = 'comdlg32')
                # build.env.Append(LIBS = 'oleaut32')
                # build.env.Append(LIBS = 'imm32')
                # build.env.Append(LIBS = 'winmm')
                # build.env.Append(LIBS = 'winspool')
                # # QtOpenGL
                # build.env.Append(LIBS = 'glu32')
                # build.env.Append(LIBS = 'opengl32')

        # Set the rpath for linux/bsd/osx.
        # This is not supported on OS X before the 10.5 SDK.
        using_104_sdk = (str(build.env["CCFLAGS"]).find("10.4") >= 0)
        compiling_on_104 = False
        if build.platform_is_osx:
            compiling_on_104 = (
                os.popen('sw_vers').readlines()[1].find('10.4') >= 0)
        if not build.platform_is_windows and not (using_104_sdk or compiling_on_104):
            qtdir = build.env['QTDIR']
            # TODO(XXX) should we use find_framework_path here or keep lib
            # hardcoded?
            framework_path = os.path.join(qtdir, 'lib')
            if os.path.isdir(framework_path):
                build.env.Append(LINKFLAGS="-Wl,-rpath," + framework_path)
                build.env.Append(LINKFLAGS="-L" + framework_path)


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

        return [build.env.StaticObject('#lib/fidlib-0.9.10/fidlib.c',
                                       CPPDEFINES=symbol)]

    def configure(self, build, conf):
        build.env.Append(CPPPATH='#lib/fidlib-0.9.10/')


class ReplayGain(Dependence):

    def sources(self, build):
        return ["#lib/replaygain/replaygain.cpp"]

    def configure(self, build, conf):
        build.env.Append(CPPPATH="#lib/replaygain")


class SoundTouch(Dependence):
    SOUNDTOUCH_PATH = 'soundtouch-1.8.0'

    def sources(self, build):
        return ['engine/enginebufferscalest.cpp',
                '#lib/%s/AAFilter.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/BPMDetect.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/FIFOSampleBuffer.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/FIRFilter.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/InterpolateCubic.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/InterpolateLinear.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/InterpolateShannon.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/PeakFinder.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/RateTransposer.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/SoundTouch.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/TDStretch.cpp' % self.SOUNDTOUCH_PATH,
                # SoundTouch CPU optimizations are only for x86
                # architectures. SoundTouch automatically ignores these files
                # when it is not being built for an architecture that supports
                # them.
                '#lib/%s/cpu_detect_x86.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/mmx_optimized.cpp' % self.SOUNDTOUCH_PATH,
                '#lib/%s/sse_optimized.cpp' % self.SOUNDTOUCH_PATH]

    def configure(self, build, conf, env=None):
        if env is None:
            env = build.env
        env.Append(CPPPATH=['#lib/%s' % self.SOUNDTOUCH_PATH])

        # Prevents circular import.
        from features import Optimize

        # If we do not want optimizations then disable them.
        optimize = (build.flags['optimize'] if 'optimize' in build.flags
                    else Optimize.get_optimization_level())
        if optimize == Optimize.LEVEL_OFF:
            env.Append(CPPDEFINES='SOUNDTOUCH_DISABLE_X86_OPTIMIZATIONS')


class RubberBand(Dependence):
    def sources(self, build):
        sources = ['engine/enginebufferscalerubberband.cpp', ]
        return sources

    def configure(self, build, conf, env=None):
        if env is None:
            env = build.env
        if not conf.CheckLib(['rubberband', 'librubberband']):
            raise Exception(
                "Could not find librubberband or its development headers.")


class TagLib(Dependence):
    def configure(self, build, conf):
        libs = ['tag']
        if not conf.CheckLib(libs):
            raise Exception(
                "Could not find libtag or its development headers.")

        # Karmic seems to have an issue with mp4tag.h where they don't include
        # the files correctly. Adding this folder ot the include path should fix
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
            if not conf.CheckLib(['fftw', 'libfftw', 'fftw3', 'libfftw3']):
                raise Exception(
                    "Could not find fftw3 or its development headers.")


class ProtoBuf(Dependence):
    def configure(self, build, conf):
        libs = ['libprotobuf-lite', 'protobuf-lite', 'libprotobuf', 'protobuf']
        if build.platform_is_windows:
            if not build.static_dependencies:
                build.env.Append(CPPDEFINES='PROTOBUF_USE_DLLS')
        if not conf.CheckLib(libs):
            raise Exception(
                "Could not find libprotobuf or its development headers.")


class MixxxCore(Feature):

    def description(self):
        return "Mixxx Core Features"

    def enabled(self, build):
        return True

    def sources(self, build):
        sources = ["mixxxkeyboard.cpp",

                   "configobject.cpp",
                   "control/control.cpp",
                   "control/controlbehavior.cpp",
                   "control/controlmodel.cpp",
                   "controlobject.cpp",
                   "controlobjectslave.cpp",
                   "controlobjectthread.cpp",
                   "controlaudiotaperpot.cpp",
                   "controlpotmeter.cpp",
                   "controllinpotmeter.cpp",
                   "controllogpotmeter.cpp",
                   "controleffectknob.cpp",
                   "controlpushbutton.cpp",
                   "controlindicator.cpp",
                   "controlttrotary.cpp",

                   "preferences/dlgpreferencepage.cpp",
                   "dlgpreferences.cpp",
                   "dlgprefsound.cpp",
                   "dlgprefsounditem.cpp",
                   "controllers/dlgprefcontroller.cpp",
                   "controllers/dlgcontrollerlearning.cpp",
                   "controllers/dlgprefcontrollers.cpp",
                   "dlgpreflibrary.cpp",
                   "dlgprefcontrols.cpp",
                   "dlgprefwaveform.cpp",
                   "dlgprefautodj.cpp",
                   "dlgprefkey.cpp",
                   "dlgprefreplaygain.cpp",
                   "dlgprefnovinyl.cpp",
                   "dlgabout.cpp",
                   "dlgprefeq.cpp",
                   "dlgprefeffects.cpp",
                   "dlgprefcrossfader.cpp",
                   "dlgtagfetcher.cpp",
                   "dlgtrackinfo.cpp",
                   "dlganalysis.cpp",
                   "dlgautodj.cpp",
                   "dlghidden.cpp",
                   "dlgmissing.cpp",
                   "dlgdevelopertools.cpp",
                   "dlgcoverartfullsize.cpp",

                   "effects/effectmanifest.cpp",
                   "effects/effectmanifestparameter.cpp",

                   "effects/effectchain.cpp",
                   "effects/effect.cpp",
                   "effects/effectparameter.cpp",

                   "effects/effectrack.cpp",
                   "effects/effectchainslot.cpp",
                   "effects/effectslot.cpp",
                   "effects/effectparameterslotbase.cpp",
                   "effects/effectparameterslot.cpp",
                   "effects/effectbuttonparameterslot.cpp",

                   "effects/effectsmanager.cpp",
                   "effects/effectchainmanager.cpp",
                   "effects/effectsbackend.cpp",

                   "effects/native/nativebackend.cpp",
                   "effects/native/bitcrushereffect.cpp",
                   "effects/native/linkwitzriley8eqeffect.cpp",
                   "effects/native/bessel4lvmixeqeffect.cpp",
                   "effects/native/bessel8lvmixeqeffect.cpp",
                   "effects/native/graphiceqeffect.cpp",
                   "effects/native/flangereffect.cpp",
                   "effects/native/filtereffect.cpp",
                   "effects/native/moogladder4filtereffect.cpp",
                   "effects/native/reverbeffect.cpp",
                   "effects/native/echoeffect.cpp",
                   "effects/native/reverb/Reverb.cc",

                   "engine/effects/engineeffectsmanager.cpp",
                   "engine/effects/engineeffectrack.cpp",
                   "engine/effects/engineeffectchain.cpp",
                   "engine/effects/engineeffect.cpp",

                   "engine/sync/basesyncablelistener.cpp",
                   "engine/sync/enginesync.cpp",
                   "engine/sync/synccontrol.cpp",
                   "engine/sync/internalclock.cpp",

                   "engine/engineworker.cpp",
                   "engine/engineworkerscheduler.cpp",
                   "engine/enginebuffer.cpp",
                   "engine/enginebufferscale.cpp",
                   "engine/enginebufferscalelinear.cpp",
                   "engine/enginefilterbiquad1.cpp",
                   "engine/enginefiltermoogladder4.cpp",
                   "engine/enginefilterbessel4.cpp",
                   "engine/enginefilterbessel8.cpp",
                   "engine/enginefilterbutterworth4.cpp",
                   "engine/enginefilterbutterworth8.cpp",
                   "engine/enginefilterlinkwitzriley4.cpp",
                   "engine/enginefilterlinkwitzriley8.cpp",
                   "engine/enginefilter.cpp",
                   "engine/engineobject.cpp",
                   "engine/enginepregain.cpp",
                   "engine/enginechannel.cpp",
                   "engine/enginemaster.cpp",
                   "engine/enginedelay.cpp",
                   "engine/enginevumeter.cpp",
                   "engine/enginesidechaincompressor.cpp",
                   "engine/sidechain/enginesidechain.cpp",
                   "engine/enginexfader.cpp",
                   "engine/enginemicrophone.cpp",
                   "engine/enginedeck.cpp",
                   "engine/engineaux.cpp",
                   "engine/channelmixer_autogen.cpp",

                   "engine/enginecontrol.cpp",
                   "engine/ratecontrol.cpp",
                   "engine/positionscratchcontroller.cpp",
                   "engine/loopingcontrol.cpp",
                   "engine/bpmcontrol.cpp",
                   "engine/keycontrol.cpp",
                   "engine/cuecontrol.cpp",
                   "engine/quantizecontrol.cpp",
                   "engine/clockcontrol.cpp",
                   "engine/readaheadmanager.cpp",
                   "engine/enginetalkoverducking.cpp",
                   "cachingreader.cpp",
                   "cachingreaderworker.cpp",

                   "analyserrg.cpp",
                   "analyserqueue.cpp",
                   "analyserwaveform.cpp",
                   "analyserkey.cpp",

                   "controllers/controller.cpp",
                   "controllers/controllerengine.cpp",
                   "controllers/controllerenumerator.cpp",
                   "controllers/controllerlearningeventfilter.cpp",
                   "controllers/controllermanager.cpp",
                   "controllers/controllerpresetfilehandler.cpp",
                   "controllers/controllerpresetinfo.cpp",
                   "controllers/controlpickermenu.cpp",
                   "controllers/controllermappingtablemodel.cpp",
                   "controllers/controllerinputmappingtablemodel.cpp",
                   "controllers/controlleroutputmappingtablemodel.cpp",
                   "controllers/delegates/controldelegate.cpp",
                   "controllers/delegates/midichanneldelegate.cpp",
                   "controllers/delegates/midiopcodedelegate.cpp",
                   "controllers/delegates/midibytedelegate.cpp",
                   "controllers/delegates/midioptionsdelegate.cpp",
                   "controllers/learningutils.cpp",
                   "controllers/midi/midimessage.cpp",
                   "controllers/midi/midiutils.cpp",
                   "controllers/midi/midicontroller.cpp",
                   "controllers/midi/midicontrollerpresetfilehandler.cpp",
                   "controllers/midi/midienumerator.cpp",
                   "controllers/midi/midioutputhandler.cpp",
                   "controllers/qtscript-bytearray/bytearrayclass.cpp",
                   "controllers/qtscript-bytearray/bytearrayprototype.cpp",
                   "controllers/softtakeover.cpp",

                   "main.cpp",
                   "mixxx.cpp",
                   "mixxxapplication.cpp",
                   "errordialoghandler.cpp",
                   "upgrade.cpp",

                   "soundsource.cpp",
                   "soundsourcetaglib.cpp",

                   "sharedglcontext.cpp",
                   "widget/controlwidgetconnection.cpp",
                   "widget/wbasewidget.cpp",
                   "widget/wwidget.cpp",
                   "widget/wwidgetgroup.cpp",
                   "widget/wwidgetstack.cpp",
                   "widget/wsizeawarestack.cpp",
                   "widget/wlabel.cpp",
                   "widget/wtracktext.cpp",
                   "widget/wnumber.cpp",
                   "widget/wnumberdb.cpp",
                   "widget/wnumberpos.cpp",
                   "widget/wnumberrate.cpp",
                   "widget/wknob.cpp",
                   "widget/wknobcomposed.cpp",
                   "widget/wdisplay.cpp",
                   "widget/wvumeter.cpp",
                   "widget/wpushbutton.cpp",
                   "widget/weffectpushbutton.cpp",
                   "widget/wslidercomposed.cpp",
                   "widget/wstatuslight.cpp",
                   "widget/woverview.cpp",
                   "widget/woverviewlmh.cpp",
                   "widget/woverviewhsv.cpp",
                   "widget/woverviewrgb.cpp",
                   "widget/wspinny.cpp",
                   "widget/wskincolor.cpp",
                   "widget/wsearchlineedit.cpp",
                   "widget/wpixmapstore.cpp",
                   "widget/wimagestore.cpp",
                   "widget/hexspinbox.cpp",
                   "widget/wtrackproperty.cpp",
                   "widget/wstarrating.cpp",
                   "widget/weffectchain.cpp",
                   "widget/weffect.cpp",
                   "widget/weffectparameter.cpp",
                   "widget/weffectbuttonparameter.cpp",
                   "widget/weffectparameterbase.cpp",
                   "widget/wtime.cpp",
                   "widget/wkey.cpp",
                   "widget/wcombobox.cpp",
                   "widget/wsplitter.cpp",
                   "widget/wcoverart.cpp",
                   "widget/wcoverartlabel.cpp",
                   "widget/wcoverartmenu.cpp",
                   "widget/wsingletoncontainer.cpp",

                   "network.cpp",
                   "musicbrainz/tagfetcher.cpp",
                   "musicbrainz/gzip.cpp",
                   "musicbrainz/crc.c",
                   "musicbrainz/acoustidclient.cpp",
                   "musicbrainz/chromaprinter.cpp",
                   "musicbrainz/musicbrainzclient.cpp",

                   "rotary.cpp",
                   "widget/wtracktableview.cpp",
                   "widget/wtracktableviewheader.cpp",
                   "widget/wlibrarysidebar.cpp",
                   "widget/wlibrary.cpp",
                   "widget/wlibrarytableview.cpp",
                   "widget/wanalysislibrarytableview.cpp",
                   "widget/wlibrarytextbrowser.cpp",
                   "library/trackcollection.cpp",
                   "library/basesqltablemodel.cpp",
                   "library/basetrackcache.cpp",
                   "library/columncache.cpp",
                   "library/librarytablemodel.cpp",
                   "library/searchquery.cpp",
                   "library/searchqueryparser.cpp",
                   "library/analysislibrarytablemodel.cpp",
                   "library/missingtablemodel.cpp",
                   "library/hiddentablemodel.cpp",
                   "library/proxytrackmodel.cpp",
                   "library/coverart.cpp",
                   "library/coverartcache.cpp",

                   "library/playlisttablemodel.cpp",
                   "library/libraryfeature.cpp",
                   "library/analysisfeature.cpp",
                   "library/autodj/autodjfeature.cpp",
                   "library/autodj/autodjprocessor.cpp",
                   "library/dao/directorydao.cpp",
                   "library/mixxxlibraryfeature.cpp",
                   "library/baseplaylistfeature.cpp",
                   "library/playlistfeature.cpp",
                   "library/setlogfeature.cpp",

                   "library/browse/browsetablemodel.cpp",
                   "library/browse/browsethread.cpp",
                   "library/browse/browsefeature.cpp",
                   "library/browse/foldertreemodel.cpp",

                   "library/recording/recordingfeature.cpp",
                   "dlgrecording.cpp",
                   "recording/recordingmanager.cpp",
                   "engine/sidechain/enginerecord.cpp",

                   # External Library Features
                   "library/baseexternallibraryfeature.cpp",
                   "library/baseexternaltrackmodel.cpp",
                   "library/baseexternalplaylistmodel.cpp",
                   "library/rhythmbox/rhythmboxfeature.cpp",

                   "library/banshee/bansheefeature.cpp",
                   "library/banshee/bansheeplaylistmodel.cpp",
                   "library/banshee/bansheedbconnection.cpp",

                   "library/itunes/itunesfeature.cpp",
                   "library/traktor/traktorfeature.cpp",

                   "library/cratefeature.cpp",
                   "library/sidebarmodel.cpp",
                   "library/legacylibraryimporter.cpp",
                   "library/library.cpp",

                   "library/scanner/libraryscanner.cpp",
                   "library/scanner/libraryscannerdlg.cpp",
                   "library/scanner/scannertask.cpp",
                   "library/scanner/importfilestask.cpp",
                   "library/scanner/recursivescandirectorytask.cpp",

                   "library/dao/cratedao.cpp",
                   "library/cratetablemodel.cpp",
                   "library/dao/cuedao.cpp",
                   "library/dao/cue.cpp",
                   "library/dao/trackdao.cpp",
                   "library/dao/playlistdao.cpp",
                   "library/dao/libraryhashdao.cpp",
                   "library/dao/settingsdao.cpp",
                   "library/dao/analysisdao.cpp",

                   "library/librarycontrol.cpp",
                   "library/schemamanager.cpp",
                   "library/songdownloader.cpp",
                   "library/starrating.cpp",
                   "library/stardelegate.cpp",
                   "library/stareditor.cpp",
                   "library/bpmdelegate.cpp",
                   "library/previewbuttondelegate.cpp",
                   "library/coverartdelegate.cpp",
                   "audiotagger.cpp",

                   "library/treeitemmodel.cpp",
                   "library/treeitem.cpp",

                   "library/parser.cpp",
                   "library/parserpls.cpp",
                   "library/parserm3u.cpp",
                   "library/parsercsv.cpp",

                   "soundsourceproxy.cpp",

                   "widget/wwaveformviewer.cpp",

                   "waveform/waveform.cpp",
                   "waveform/waveformfactory.cpp",
                   "waveform/waveformwidgetfactory.cpp",
                   "waveform/vsyncthread.cpp",
                   "waveform/guitick.cpp",
                   "waveform/renderers/waveformwidgetrenderer.cpp",
                   "waveform/renderers/waveformrendererabstract.cpp",
                   "waveform/renderers/waveformrenderbackground.cpp",
                   "waveform/renderers/waveformrendermark.cpp",
                   "waveform/renderers/waveformrendermarkrange.cpp",
                   "waveform/renderers/waveformrenderbeat.cpp",
                   "waveform/renderers/waveformrendererendoftrack.cpp",
                   "waveform/renderers/waveformrendererpreroll.cpp",

                   "waveform/renderers/waveformrendererfilteredsignal.cpp",
                   "waveform/renderers/waveformrendererhsv.cpp",
                   "waveform/renderers/waveformrendererrgb.cpp",
                   "waveform/renderers/qtwaveformrendererfilteredsignal.cpp",
                   "waveform/renderers/qtwaveformrenderersimplesignal.cpp",
                   "waveform/renderers/glwaveformrendererfilteredsignal.cpp",
                   "waveform/renderers/glwaveformrenderersimplesignal.cpp",
                   "waveform/renderers/glslwaveformrenderersignal.cpp",
                   "waveform/renderers/glvsynctestrenderer.cpp",
                   "waveform/renderers/glwaveformrendererrgb.cpp",

                   "waveform/renderers/waveformsignalcolors.cpp",

                   "waveform/renderers/waveformrenderersignalbase.cpp",
                   "waveform/renderers/waveformmark.cpp",
                   "waveform/renderers/waveformmarkset.cpp",
                   "waveform/renderers/waveformmarkrange.cpp",

                   "waveform/widgets/waveformwidgetabstract.cpp",
                   "waveform/widgets/emptywaveformwidget.cpp",
                   "waveform/widgets/softwarewaveformwidget.cpp",
                   "waveform/widgets/hsvwaveformwidget.cpp",
                   "waveform/widgets/rgbwaveformwidget.cpp",
                   "waveform/widgets/qtwaveformwidget.cpp",
                   "waveform/widgets/qtsimplewaveformwidget.cpp",
                   "waveform/widgets/glwaveformwidget.cpp",
                   "waveform/widgets/glsimplewaveformwidget.cpp",
                   "waveform/widgets/glvsynctestwidget.cpp",

                   "waveform/widgets/glslwaveformwidget.cpp",

                   "waveform/widgets/glrgbwaveformwidget.cpp",

                   "skin/imginvert.cpp",
                   "skin/imgloader.cpp",
                   "skin/imgcolor.cpp",
                   "skin/skinloader.cpp",
                   "skin/legacyskinparser.cpp",
                   "skin/colorschemeparser.cpp",
                   "skin/tooltips.cpp",
                   "skin/skincontext.cpp",
                   "skin/svgparser.cpp",
                   "skin/pixmapsource.cpp",

                   "sampleutil.cpp",
                   "trackinfoobject.cpp",
                   "track/beatgrid.cpp",
                   "track/beatmap.cpp",
                   "track/beatfactory.cpp",
                   "track/beatutils.cpp",
                   "track/keys.cpp",
                   "track/keyfactory.cpp",
                   "track/keyutils.cpp",

                   "baseplayer.cpp",
                   "basetrackplayer.cpp",
                   "deck.cpp",
                   "sampler.cpp",
                   "previewdeck.cpp",
                   "playermanager.cpp",
                   "samplerbank.cpp",
                   "sounddevice.cpp",
                   "soundmanager.cpp",
                   "soundmanagerconfig.cpp",
                   "soundmanagerutil.cpp",
                   "dlgprefrecord.cpp",
                   "playerinfo.cpp",
                   "visualplayposition.cpp",

                   "encoder/encoder.cpp",
                   "encoder/encodermp3.cpp",
                   "encoder/encodervorbis.cpp",

                   "tapfilter.cpp",

                   "util/pa_ringbuffer.c",
                   "util/sleepableqthread.cpp",
                   "util/statsmanager.cpp",
                   "util/stat.cpp",
                   "util/statmodel.cpp",
                   "util/time.cpp",
                   "util/timer.cpp",
                   "util/performancetimer.cpp",
                   "util/threadcputimer.cpp",
                   "util/version.cpp",
                   "util/rlimit.cpp",
                   "util/valuetransformer.cpp",
                   "util/sandbox.cpp",
                   "util/file.cpp",
                   "util/mac.cpp",
                   "util/task.cpp",
                   "util/experiment.cpp",
                   "util/xml.cpp",

                   '#res/mixxx.qrc'
                   ]

        proto_args = {
            'PROTOCPROTOPATH': ['src'],
            'PROTOCPYTHONOUTDIR': '',  # set to None to not generate python
            'PROTOCOUTDIR': build.build_dir,
            'PROTOCCPPOUTFLAGS': '',
            #'PROTOCCPPOUTFLAGS': "dllexport_decl=PROTOCONFIG_EXPORT:"
        }
        proto_sources = SCons.Glob('proto/*.proto')
        proto_objects = [build.env.Protoc([], proto_source, **proto_args)[0]
                         for proto_source in proto_sources]
        sources.extend(proto_objects)

        # Uic these guys (they're moc'd automatically after this) - Generates
        # the code for the QT UI forms.
        ui_files = [
            'controllers/dlgcontrollerlearning.ui',
            'controllers/dlgprefcontrollerdlg.ui',
            'controllers/dlgprefcontrollersdlg.ui',
            'dlgaboutdlg.ui',
            'dlganalysis.ui',
            'dlgautodj.ui',
            'dlgcoverartfullsize.ui',
            'dlgdevelopertoolsdlg.ui',
            'dlghidden.ui',
            'dlgmissing.ui',
            'dlgprefbeatsdlg.ui',
            'dlgprefcontrolsdlg.ui',
            'dlgprefwaveformdlg.ui',
            'dlgprefautodjdlg.ui',
            'dlgprefcrossfaderdlg.ui',
            'dlgprefkeydlg.ui',
            'dlgprefeqdlg.ui',
            'dlgprefeffectsdlg.ui',
            'dlgpreferencesdlg.ui',
            'dlgprefnovinyldlg.ui',
            'dlgpreflibrarydlg.ui',
            'dlgprefrecorddlg.ui',
            'dlgprefreplaygaindlg.ui',
            'dlgprefsounddlg.ui',
            'dlgprefsounditem.ui',
            'dlgprefvinyldlg.ui',
            'dlgrecording.ui',
            'dlgtagfetcher.ui',
            'dlgtrackinfo.ui',
        ]
        map(Qt.uic(build), ui_files)

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

        if build.build_is_debug:
            build.env.Append(CPPDEFINES='MIXXX_BUILD_DEBUG')
        elif build.build_is_release:
            build.env.Append(CPPDEFINES='MIXXX_BUILD_RELEASE')

            # In a release build we want to disable all Q_ASSERTs in Qt headers
            # that we include. We can't define QT_NO_DEBUG because that would
            # mean turning off QDebug output. qt_noop() is what Qt defines
            # Q_ASSERT to be when QT_NO_DEBUG is defined.
            build.env.Append(CPPDEFINES="'Q_ASSERT(x)=qt_noop()'")

        if int(SCons.ARGUMENTS.get('debug_assertions_fatal', 0)):
            build.env.Append(CPPDEFINES='MIXXX_DEBUG_ASSERTIONS_FATAL')

        if build.toolchain_is_gnu:
            # Default GNU Options
            build.env.Append(CCFLAGS='-pipe')
            build.env.Append(CCFLAGS='-Wall')
            build.env.Append(CCFLAGS='-Wextra')

            # Always generate debugging info.
            build.env.Append(CCFLAGS='-g')
        elif build.toolchain_is_msvs:
            # Validate the specified winlib directory exists
            mixxx_lib_path = SCons.ARGUMENTS.get(
                'winlib', '..\\..\\..\\mixxx-win32lib-msvc100-release')
            if not os.path.exists(mixxx_lib_path):
                print mixxx_lib_path
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

            # Ugh, MSVC-only hack :( see
            # http://www.qtforum.org/article/17883/problem-using-qstring-
            # fromstdwstring.html
            build.env.Append(CXXFLAGS='/Zc:wchar_t-')

            # Build with multiple processes. TODO(XXX) make this configurable.
            # http://msdn.microsoft.com/en-us/library/bb385193.aspx
            build.env.Append(CCFLAGS='/MP')

            # Generate debugging information for compilation units and
            # executables linked regardless of whether we are creating a debug
            # build. Having PDB files for our releases is helpful for debugging.
            build.env.Append(LINKFLAGS='/DEBUG')
            build.env.Append(CCFLAGS='/Zi')

            if build.build_is_debug:
                # Important: We always build Mixxx with the Multi-Threaded DLL
                # runtime because Mixxx loads DLLs at runtime. Since this is a
                # debug build, use the debug version of the MD runtime.
                build.env.Append(CCFLAGS='/MDd')
                # Enable the Mixxx debug console (see main.cpp).
                build.env.Append(CPPDEFINES='DEBUGCONSOLE')
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

        elif build.platform_is_linux:
            build.env.Append(CPPDEFINES='__LINUX__')

            # Check for pkg-config >= 0.15.0
            if not conf.CheckForPKGConfig('0.15.0'):
                raise Exception('pkg-config >= 0.15.0 not found.')

        elif build.platform_is_osx:
            # Stuff you may have compiled by hand
            if os.path.isdir('/usr/local/include'):
                build.env.Append(LIBPATH=['/usr/local/lib'])
                build.env.Append(CPPPATH=['/usr/local/include'])

            # Non-standard libpaths for fink and certain (most?) darwin ports
            if os.path.isdir('/sw/include'):
                build.env.Append(LIBPATH=['/sw/lib'])
                build.env.Append(CPPPATH=['/sw/include'])

            # Non-standard libpaths for darwin ports
            if os.path.isdir('/opt/local/include'):
                build.env.Append(LIBPATH=['/opt/local/lib'])
                build.env.Append(CPPPATH=['/opt/local/include'])

        elif build.platform_is_bsd:
            build.env.Append(CPPDEFINES='__BSD__')
            build.env.Append(CPPPATH=['/usr/include',
                                      '/usr/local/include',
                                      '/usr/X11R6/include/'])
            build.env.Append(LIBPATH=['/usr/lib/',
                                      '/usr/local/lib',
                                      '/usr/X11R6/lib'])
            build.env.Append(LIBS='pthread')
            # why do we need to do this on OpenBSD and not on Linux?  if we
            # don't then CheckLib("vorbisfile") fails
            build.env.Append(LIBS=['ogg', 'vorbis'])

        # Define for things that would like to special case UNIX (Linux or BSD)
        if build.platform_is_bsd or build.platform_is_linux:
            build.env.Append(CPPDEFINES='__UNIX__')

        # Add the src/ directory to the include path
        build.env.Append(CPPPATH=['.'])

        # Set up flags for config/track listing files
        if build.platform_is_linux or \
                build.platform_is_bsd:
            mixxx_files = [
                ('SETTINGS_PATH', '.mixxx/'),
                ('SETTINGS_FILE', 'mixxx.cfg')]
        elif build.platform_is_osx:
            mixxx_files = [
                ('SETTINGS_PATH', 'Library/Application Support/Mixxx/'),
                ('SETTINGS_FILE', 'mixxx.cfg')]
        elif build.platform_is_windows:
            mixxx_files = [
                ('SETTINGS_PATH', 'Local Settings/Application Data/Mixxx/'),
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
        return [SoundTouch, ReplayGain, PortAudio, PortMIDI, Qt, TestHeaders,
                FidLib, SndFile, FLAC, OggVorbis, OpenGL, TagLib, ProtoBuf,
                Chromaprint, RubberBand, SecurityFramework, CoreServices]

    def post_dependency_check_configure(self, build, conf):
        """Sets up additional things in the Environment that must happen
        after the Configure checks run."""
        if build.platform_is_windows:
            if build.toolchain_is_msvs:
                if not build.static_dependencies or build.build_is_debug:
                    build.env.Append(LINKFLAGS=['/nodefaultlib:LIBCMT.lib',
                                                '/nodefaultlib:LIBCMTd.lib'])

                build.env.Append(LINKFLAGS='/entry:mainCRTStartup')
                # Makes the program not launch a shell first.
                # Minimum platform version 5.01 for XP x86 and 5.02 for XP x64.
                if build.machine_is_64bit:
                    build.env.Append(LINKFLAGS='/subsystem:windows,5.02')
                else:
                    build.env.Append(LINKFLAGS='/subsystem:windows,5.01')
                # Force MSVS to generate a manifest (MSVC2010)
                build.env.Append(LINKFLAGS='/manifest')
            elif build.toolchain_is_gnu:
                # Makes the program not launch a shell first
                build.env.Append(LINKFLAGS='--subsystem,windows')
                build.env.Append(LINKFLAGS='-mwindows')
