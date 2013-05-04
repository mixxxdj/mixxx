# -*- coding: utf-8 -*-

import os
import util
from mixxx import Dependence, Feature
import SCons.Script as SCons

class PortAudio(Dependence):

    def configure(self, build, conf):
        libs = ['portaudio']
        if build.msvcdebug:
            libs = ['portaudiod','portaudio-debug']
        if not conf.CheckLib(libs):
            raise Exception('Did not find libportaudio.a, portaudio.lib, or the PortAudio-v19 development header files.')

        #Turn on PortAudio support in Mixxx
        build.env.Append(CPPDEFINES = '__PORTAUDIO__')

        if build.platform_is_windows and build.static_dependencies:
            conf.CheckLib('advapi32')

    def sources(self, build):
        return ['sounddeviceportaudio.cpp']

class PortMIDI(Dependence):

    def configure(self, build, conf):
        # Check for PortTime
        libs = ['porttime', 'libporttime']
        headers = ['porttime.h']
        if build.msvcdebug:
            libs = ['porttimed', 'porttime-debug']

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
            if build.msvcdebug:
                libs = ['portmidi_sd', 'portmidi_s-debug', 'portmidid', 'portmidi-debug']
            else:
                libs = ['portmidi_s','portmidi', 'libportmidi']
        if not conf.CheckLib(libs) or not conf.CheckHeader(headers):
            raise Exception("Did not find PortMidi or its development headers.")

    def sources(self, build):
        return ['controllers/midi/portmidienumerator.cpp', 'controllers/midi/portmidicontroller.cpp']

class OpenGL(Dependence):

    def configure(self, build, conf):
        # Check for OpenGL (it's messy to do it for all three platforms) XXX
        # this should *NOT* have hardcoded paths like this
        if (not conf.CheckLib('GL') and
            not conf.CheckLib('opengl32') and
            not conf.CheckCHeader('/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/gl.h') and
            not conf.CheckCHeader('GL/gl.h')):
            raise Exception('Did not find OpenGL development files')

        if (not conf.CheckLib('GLU') and
            not conf.CheckLib('glu32') and
            not conf.CheckCHeader('/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/glu.h')):
            raise Exception('Did not find GLU development files')

        if build.platform_is_osx:
            build.env.Append(CPPPATH='/Library/Frameworks/OpenGL.framework/Headers/')
            build.env.Append(LINKFLAGS='-framework OpenGL')

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
        if build.platform_is_windows:
            if build.msvcdebug:
                libs = ['libvorbisfile_static-debug','vorbisfile_static-debug','vorbisfile-debug','libvorbisfile-debug']
            else:
                libs = ['libvorbisfile', 'vorbisfile', 'libvorbisfile_static', 'vorbisfile_static']
        if not conf.CheckLib(libs):
            Exception('Did not find libvorbisfile.a, libvorbisfile.lib, '
                      'or the libvorbisfile development headers.')

        libs = ['libvorbis', 'vorbis']
        if build.platform_is_windows:
            if build.msvcdebug:
                libs = ['libvorbis_static-debug','vorbis_static-debug','libvorbis-debug','vorbis-debug']
            else:
                libs = ['libvorbis', 'vorbis', 'libvorbis_static', 'vorbis_static']
        if not conf.CheckLib(libs):
            raise Exception('Did not find libvorbis.a, libvorbis.lib, or the libvorbis development headers.')

        libs = ['libogg', 'ogg']
        if build.platform_is_windows:
            if build.msvcdebug:
                libs = ['libogg_static-debug','ogg_static-debug','ogg-debug','libogg-debug']
            else:
                libs = ['libogg', 'ogg', 'libogg_static', 'ogg_static']
        if not conf.CheckLib(libs):
            raise Exception('Did not find libogg.a, libogg.lib, or the libogg development headers')

    def sources(self, build):
        return ['soundsourceoggvorbis.cpp']


class SndFile(Dependence):

    def configure(self, build, conf):
        #if not conf.CheckLibWithHeader(['sndfile', 'libsndfile', 'libsndfile-1'], 'sndfile.h', 'C'):
        # TODO: check for debug version on Windows when one is available
        if not conf.CheckLib(['sndfile', 'libsndfile', 'libsndfile-1']):
            raise Exception("Did not find libsndfile or it\'s development headers")
        build.env.Append(CPPDEFINES = '__SNDFILE__')

    def sources(self, build):
        return ['soundsourcesndfile.cpp']

class FLAC(Dependence):
    def configure(self, build, conf):
        if not conf.CheckHeader('FLAC/stream_decoder.h'):
            raise Exception('Did not find libFLAC development headers')
        libs = ['libFLAC', 'FLAC']
        if build.platform_is_windows:
            if build.msvcdebug:
                libs = ['libFLAC-debug', 'FLAC-debug', 'libFLAC_static-debug', 'FLAC_static-debug']
            else:
                libs = ['libFLAC', 'FLAC', 'libFLAC_static', 'FLAC_static']
        if not conf.CheckLib(libs):
            raise Exception('Did not find libFLAC development libraries')

        if build.platform_is_windows and build.static_dependencies:
            build.env.Append(CPPDEFINES = 'FLAC__NO_DLL')

    def sources(self, build):
        return ['soundsourceflac.cpp',]

class Qt(Dependence):
    DEFAULT_QTDIRS = {'linux': '/usr/share/qt4',
                      'bsd': '/usr/local/lib/qt4',
                      'osx': '/Library/Frameworks',
                      'windows': 'C:\\qt\\4.6.0'}

    @staticmethod
    def find_framework_path(qtdir):
        for d in (os.path.join(qtdir, x) for x in ['', 'Frameworks', 'lib']):
            core = os.path.join(d,'QtCore.framework')
            if os.path.isdir(core):
                return d
        return None

    def satisfy(self):
        pass

    def configure(self, build, conf):
        # Emit various Qt defines
        build.env.Append(CPPDEFINES = ['QT_SHARED',
                                       'QT_TABLET_SUPPORT'])

        # Promo tracks is the only thing that uses webkit currently.
        use_qtwebkit = int(util.get_flags(build.env, 'promo', 0)) > 0

        # TODO(XXX) what is with the slightly differing modules used for each
        # platform here? Document the differences and make them all
        # programmatically driven from one list instead of hard-coded multiple
        # times.

        qt_modules = [
            'QtCore', 'QtGui', 'QtOpenGL', 'QtXml', 'QtSvg',
            'QtSql', 'QtScript', 'QtXmlPatterns', 'QtNetwork'
            #'QtUiTools', #'QtDesigner',
        ]

        if use_qtwebkit:
            qt_modules.append('QtWebKit')

        # Enable Qt include paths
        if build.platform_is_linux:
            if not conf.CheckForPKG('QtCore', '4.6'):
                raise Exception('QT >= 4.6 not found')

            #(This hopefully respects our qtdir=blah flag while linking now.)
            build.env.EnableQt4Modules(qt_modules,debug=False)

        elif build.platform_is_osx:
            qtdir = build.env['QTDIR']
            build.env.Append(
                LINKFLAGS=' '.join('-framework %s' % m for m in qt_modules)
            )
            framework_path = Qt.find_framework_path(qtdir)
            if not framework_path:
                raise Exception('Could not find frameworks in Qt directory: %s' % qtdir)
            # Necessary for raw includes of headers like #include <qobject.h>
            build.env.Append(CPPPATH = [os.path.join(framework_path, '%s.framework' % m, 'Headers')
                                        for m in qt_modules])
            # Framework path needs to be altered for CCFLAGS as well since a
            # header include of QtCore/QObject.h looks for a QtCore.framework on
            # the search path and a QObject.h in QtCore.framework/Headers.
            build.env.Append(CCFLAGS = ['-F%s' % os.path.join(framework_path)])
            build.env.Append(LINKFLAGS = ['-F%s' % os.path.join(framework_path)])

        # Setup Qt library includes for non-OSX
        if build.platform_is_linux or build.platform_is_bsd:
            build.env.Append(LIBS = 'QtCore')
            build.env.Append(LIBS = 'QtGui')
            build.env.Append(LIBS = 'QtOpenGL')
            build.env.Append(LIBS = 'QtXml')
            build.env.Append(LIBS = 'QtNetwork')
            build.env.Append(LIBS = 'QtScript')
            if use_qtwebkit:
                build.env.Append(LIBS = 'QtWebKit')
        elif build.platform_is_windows:
            build.env.Append(LIBPATH=['$QTDIR/lib'])
            # Since we use WebKit, that's only available dynamically
            qt_libs = ['QtCore4',
                       'QtGui4',
                       'QtOpenGL4',
                       'QtXml4',
                       'QtNetwork4',
                       'QtXmlPatterns4',
                       'QtSql4',
                       'QtScript4',]
            if use_qtwebkit:
                qt_libs.append('QtWebKit4')

            # Use the debug versions of the libs if we are building in debug mode.
            if build.msvcdebug:
                qt_libs = [lib.replace('4', 'd4') for lib in qt_libs]
            build.env.Append(LIBS=qt_libs)

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

        # Set Qt include paths for non-OSX
        if not build.platform_is_osx:
            include_paths = ['$QTDIR/include/QtCore',
                             '$QTDIR/include/QtGui',
                             '$QTDIR/include/QtOpenGL',
                             '$QTDIR/include/QtXml',
                             '$QTDIR/include/QtNetwork',
                             '$QTDIR/include/QtSql',
                             '$QTDIR/include/QtScript',
                             '$QTDIR/include/Qt']
            if use_qtwebkit:
                include_paths.append('$QTDIR/include/QtWebKit')
            build.env.Append(CPPPATH=include_paths)

        # Set the rpath for linux/bsd/osx.
        # This is not supported on OS X before the 10.5 SDK.
        using_104_sdk = (str(build.env["CCFLAGS"]).find("10.4") >= 0)
        compiling_on_104 = False
        if build.platform_is_osx:
            compiling_on_104 = (os.popen('sw_vers').readlines()[1].find('10.4') >= 0)
        if not build.platform_is_windows and not (using_104_sdk or compiling_on_104):
            qtdir = build.env['QTDIR']
            # TODO(XXX) should we use find_framework_path here or keep lib
            # hardcoded?
            framework_path = os.path.join(qtdir, 'lib')
            if os.path.isdir(framework_path):
                build.env.Append(LINKFLAGS = "-Wl,-rpath," + framework_path)
                build.env.Append(LINKFLAGS = "-L" + framework_path)

        #QtSQLite DLL
        if build.platform_is_windows:
            build.flags['sqlitedll'] = util.get_flags(build.env, 'sqlitedll', 1)


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
    SOUNDTOUCH_PATH = 'soundtouch-1.6.0'

    def sse_enabled(self, build):
        optimize = int(util.get_flags(build.env, 'optimize', 1))
        return (build.machine_is_64bit or
                (build.toolchain_is_msvs and optimize > 2) or
                (build.toolchain_is_gnu and optimize > 1))

    def sources(self, build):
        sources = ['engine/enginebufferscalest.cpp',
                   '#lib/%s/SoundTouch.cpp' % self.SOUNDTOUCH_PATH,
                   '#lib/%s/TDStretch.cpp' % self.SOUNDTOUCH_PATH,
                   '#lib/%s/RateTransposer.cpp' % self.SOUNDTOUCH_PATH,
                   '#lib/%s/AAFilter.cpp' % self.SOUNDTOUCH_PATH,
                   '#lib/%s/FIFOSampleBuffer.cpp' % self.SOUNDTOUCH_PATH,
                   '#lib/%s/FIRFilter.cpp' % self.SOUNDTOUCH_PATH,
                   '#lib/%s/PeakFinder.cpp' % self.SOUNDTOUCH_PATH,
                   '#lib/%s/BPMDetect.cpp' % self.SOUNDTOUCH_PATH]

        # SoundTouch CPU optimizations are only for x86
        # architectures. SoundTouch automatically ignores these files when it is
        # not being built for an architecture that supports them.
        cpu_detection = '#lib/%s/cpu_detect_x86_win.cpp' if build.toolchain_is_msvs else \
            '#lib/%s/cpu_detect_x86_gcc.cpp'
        sources.append(cpu_detection % self.SOUNDTOUCH_PATH)

        # Check if the compiler has SSE extention enabled
        # Allways the case on x64 (core instructions)
        if self.sse_enabled(build):
            sources.extend(
                ['#lib/%s/mmx_optimized.cpp' % self.SOUNDTOUCH_PATH,
                 '#lib/%s/sse_optimized.cpp' % self.SOUNDTOUCH_PATH,])
        return sources

    def configure(self, build, conf, env=None):
        if env is None:
            env = build.env
        if build.platform_is_windows:
            # Regardless of the bitwidth, ST checks for WIN32
            env.Append(CPPDEFINES = 'WIN32')
        env.Append(CPPPATH=['#lib/%s' % self.SOUNDTOUCH_PATH])

        # Check if the compiler has SSE extention enabled
        # Allways the case on x64 (core instructions)
        optimize = int(util.get_flags(env, 'optimize', 1))
        if self.sse_enabled(build):
            env.Append(CPPDEFINES='SOUNDTOUCH_ALLOW_X86_OPTIMIZATIONS')

class TagLib(Dependence):
    def configure(self, build, conf):
        libs = ['tag']
        if build.msvcdebug:
            libs = ['tag-debug']
        if not conf.CheckLib(libs):
            raise Exception("Could not find libtag or its development headers.")

        # Karmic seems to have an issue with mp4tag.h where they don't include
        # the files correctly. Adding this folder ot the include path should fix
        # it, though might cause issues. This is safe to remove once we
        # deprecate Karmic support. rryan 2/2011
        build.env.Append(CPPPATH='/usr/include/taglib/')

        if build.platform_is_windows and build.static_dependencies:
            build.env.Append(CPPDEFINES = 'TAGLIB_STATIC')

class ProtoBuf(Dependence):
    def configure(self, build, conf):
        libs = ['libprotobuf-lite', 'protobuf-lite', 'libprotobuf', 'protobuf']
        if build.msvcdebug:
            libs = ['libprotobuf-lite-debug','protobuf-lite-debug','libprotobuf-debug','protobuf-debug']
        if not conf.CheckLib(libs):
            raise Exception("Could not find libprotobuf or its development headers.")

class MixxxCore(Feature):

    def description(self):
        return "Mixxx Core Features"

    def enabled(self, build):
        return True

    def sources(self, build):
        sources = ["mixxxkeyboard.cpp",

                   "configobject.cpp",
                   "controlobjectthread.cpp",
                   "controlobjectthreadwidget.cpp",
                   "controlobjectthreadmain.cpp",
                   "controlevent.cpp",
                   "controllogpotmeter.cpp",
                   "controlobject.cpp",
                   "controlnull.cpp",
                   "controlpotmeter.cpp",
                   "controllinpotmeter.cpp",
                   "controlpushbutton.cpp",
                   "controlttrotary.cpp",
                   "controlbeat.cpp",

                   "dlgpreferences.cpp",
                   "dlgprefsound.cpp",
                   "dlgprefsounditem.cpp",
                   "controllers/dlgprefcontroller.cpp",
                   "controllers/dlgprefmappablecontroller.cpp",
                   "controllers/dlgcontrollerlearning.cpp",
                   "controllers/dlgprefnocontrollers.cpp",
                   "dlgprefplaylist.cpp",
                   "dlgprefcontrols.cpp",
                   "dlgprefbpm.cpp",
                   "dlgprefreplaygain.cpp",
                   "dlgprefnovinyl.cpp",
                   "dlgbpmscheme.cpp",
                   "dlgabout.cpp",
                   "dlgprefeq.cpp",
                   "dlgprefcrossfader.cpp",
                   "dlgtrackinfo.cpp",
                   "dlgprepare.cpp",
                   "dlgautodj.cpp",
                   "dlghidden.cpp",
                   "dlgmissing.cpp",

                   "engine/engineworker.cpp",
                   "engine/engineworkerscheduler.cpp",
                   "engine/syncworker.cpp",
                   "engine/enginebuffer.cpp",
                   "engine/enginebufferscale.cpp",
                   "engine/enginebufferscaledummy.cpp",
                   "engine/enginebufferscalelinear.cpp",
                   "engine/engineclipping.cpp",
                   "engine/enginefilterblock.cpp",
                   "engine/enginefilteriir.cpp",
                   "engine/enginefilter.cpp",
                   "engine/engineobject.cpp",
                   "engine/enginepregain.cpp",
                   "engine/enginechannel.cpp",
                   "engine/enginemaster.cpp",
                   "engine/enginedelay.cpp",
                   "engine/engineflanger.cpp",
                   "engine/enginevumeter.cpp",
                   "engine/enginevinylsoundemu.cpp",
                   "engine/enginesidechain.cpp",
                   "engine/enginefilterbutterworth8.cpp",
                   "engine/enginexfader.cpp",
                   "engine/enginemicrophone.cpp",
                   "engine/enginedeck.cpp",

                   "engine/enginecontrol.cpp",
                   "engine/ratecontrol.cpp",
                   "engine/positionscratchcontroller.cpp",
                   "engine/loopingcontrol.cpp",
                   "engine/bpmcontrol.cpp",
                   "engine/cuecontrol.cpp",
                   "engine/quantizecontrol.cpp",
                   "engine/clockcontrol.cpp",
                   "engine/readaheadmanager.cpp",
                   "cachingreader.cpp",

                   "analyserrg.cpp",
                   "analyserqueue.cpp",
                   "analyserbpm.cpp",
                   "analyserwaveform.cpp",

                   "controllers/controller.cpp",
                   "controllers/controllerengine.cpp",
                   "controllers/controllerenumerator.cpp",
                   "controllers/controllerlearningeventfilter.cpp",
                   "controllers/controllermanager.cpp",
                   "controllers/controllerpresetfilehandler.cpp",
                   "controllers/controllerpresetinfo.cpp",
                   "controllers/midi/midicontroller.cpp",
                   "controllers/midi/midicontrollerpresetfilehandler.cpp",
                   "controllers/midi/midienumerator.cpp",
                   "controllers/midi/midioutputhandler.cpp",
                   "controllers/mixxxcontrol.cpp",
                   "controllers/qtscript-bytearray/bytearrayclass.cpp",
                   "controllers/qtscript-bytearray/bytearrayprototype.cpp",
                   "controllers/softtakeover.cpp",

                   "main.cpp",
                   "mixxx.cpp",
                   "errordialoghandler.cpp",
                   "upgrade.cpp",

                   "soundsource.cpp",

                   "sharedglcontext.cpp",
                   "widget/wwidget.cpp",
                   "widget/wwidgetgroup.cpp",
                   "widget/wwidgetstack.cpp",
                   "widget/wlabel.cpp",
                   "widget/wtracktext.cpp",
                   "widget/wnumber.cpp",
                   "widget/wnumberpos.cpp",
                   "widget/wnumberrate.cpp",
                   "widget/wknob.cpp",
                   "widget/wdisplay.cpp",
                   "widget/wvumeter.cpp",
                   "widget/wpushbutton.cpp",
                   "widget/wslidercomposed.cpp",
                   "widget/wslider.cpp",
                   "widget/wstatuslight.cpp",
                   "widget/woverview.cpp",
                   "widget/wspinny.cpp",
                   "widget/wskincolor.cpp",
                   "widget/wabstractcontrol.cpp",
                   "widget/wsearchlineedit.cpp",
                   "widget/wpixmapstore.cpp",
                   "widget/wimagestore.cpp",
                   "widget/hexspinbox.cpp",
                   "widget/wtrackproperty.cpp",
                   "widget/wtime.cpp",

                   "mathstuff.cpp",

                   "rotary.cpp",
                   "widget/wtracktableview.cpp",
                   "widget/wtracktableviewheader.cpp",
                   "widget/wlibrarysidebar.cpp",
                   "widget/wlibrary.cpp",
                   "widget/wlibrarytableview.cpp",
                   "widget/wpreparelibrarytableview.cpp",
                   "widget/wpreparecratestableview.cpp",
                   "widget/wlibrarytextbrowser.cpp",
                   "library/preparecratedelegate.cpp",
                   "library/trackcollection.cpp",
                   "library/basesqltablemodel.cpp",
                   "library/basetrackcache.cpp",
                   "library/librarytablemodel.cpp",
                   "library/searchqueryparser.cpp",
                   "library/preparelibrarytablemodel.cpp",
                   "library/missingtablemodel.cpp",
                   "library/hiddentablemodel.cpp",
                   "library/proxytrackmodel.cpp",

                   "library/playlisttablemodel.cpp",
                   "library/libraryfeature.cpp",
                   "library/preparefeature.cpp",
                   "library/autodjfeature.cpp",
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

                   # External Library Features
                   "library/baseexternallibraryfeature.cpp",
                   "library/baseexternaltrackmodel.cpp",
                   "library/baseexternalplaylistmodel.cpp",
                   "library/rhythmbox/rhythmboxfeature.cpp",
                   "library/itunes/itunesfeature.cpp",
                   "library/traktor/traktorfeature.cpp",

                   "library/cratefeature.cpp",
                   "library/sidebarmodel.cpp",
                   "library/libraryscanner.cpp",
                   "library/libraryscannerdlg.cpp",
                   "library/legacylibraryimporter.cpp",
                   "library/library.cpp",
                   "library/searchthread.cpp",

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
                   "library/bpmeditor.cpp",
                   "library/previewbuttondelegate.cpp",
                   "audiotagger.cpp",

                   "library/treeitemmodel.cpp",
                   "library/treeitem.cpp",

                   "xmlparse.cpp",
                   "library/parser.cpp",
                   "library/parserpls.cpp",
                   "library/parserm3u.cpp",
                   "library/parsercsv.cpp",

                   "bpm/bpmscheme.cpp",

                   "soundsourceproxy.cpp",

                   "widget/wwaveformviewer.cpp",

                   "waveform/waveform.cpp",
                   "waveform/waveformfactory.cpp",
                   "waveform/waveformwidgetfactory.cpp",
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
                   "waveform/renderers/qtwaveformrendererfilteredsignal.cpp",
                   "waveform/renderers/qtwaveformrenderersimplesignal.cpp",
                   "waveform/renderers/glwaveformrendererfilteredsignal.cpp",
                   "waveform/renderers/glwaveformrenderersimplesignal.cpp",
                   "waveform/renderers/glslwaveformrenderersignal.cpp",

                   "waveform/renderers/waveformsignalcolors.cpp",

                   "waveform/renderers/waveformrenderersignalbase.cpp",
                   "waveform/renderers/waveformmark.cpp",
                   "waveform/renderers/waveformmarkset.cpp",
                   "waveform/renderers/waveformmarkrange.cpp",

                   "waveform/widgets/waveformwidgetabstract.cpp",
                   "waveform/widgets/emptywaveformwidget.cpp",
                   "waveform/widgets/softwarewaveformwidget.cpp",
                   "waveform/widgets/hsvwaveformwidget.cpp",
                   "waveform/widgets/qtwaveformwidget.cpp",
                   "waveform/widgets/qtsimplewaveformwidget.cpp",
                   "waveform/widgets/glwaveformwidget.cpp",
                   "waveform/widgets/glsimplewaveformwidget.cpp",

                   "waveform/widgets/glslwaveformwidget.cpp",

                   "skin/imginvert.cpp",
                   "skin/imgloader.cpp",
                   "skin/imgcolor.cpp",
                   "skin/skinloader.cpp",
                   "skin/legacyskinparser.cpp",
                   "skin/colorschemeparser.cpp",
                   "skin/propertybinder.cpp",
                   "skin/tooltips.cpp",

                   "sampleutil.cpp",
                   "trackinfoobject.cpp",
                   "track/beatgrid.cpp",
                   "track/beatmap.cpp",
                   "track/beatfactory.cpp",
                   "track/beatutils.cpp",

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

                   "recording/enginerecord.cpp",
                   "recording/encoder.cpp",

                   "segmentation.cpp",
                   "tapfilter.cpp",

                   "util/pa_ringbuffer.c",
                   "util/sleepableqthread.cpp",
                   "util/statsmanager.cpp",
                   "util/stat.cpp",
                   "util/timer.cpp",
                   "util/performancetimer.cpp",

                   # Add the QRC file which compiles in some extra resources
                   # (prefs icons, etc.)
                   build.env.Qrc('#res/mixxx.qrc')
                   ]

        proto_args = {
            'PROTOCPROTOPATH': ['src'],
            'PROTOCPYTHONOUTDIR': '', # set to None to not generate python
            'PROTOCOUTDIR': build.build_dir,
            'PROTOCCPPOUTFLAGS': '',
            #'PROTOCCPPOUTFLAGS': "dllexport_decl=PROTOCONFIG_EXPORT:"
        }
        proto_sources = SCons.Glob('proto/*.proto')
        proto_objects = [build.env.Protoc([], proto_source, **proto_args)[0]
                        for proto_source in proto_sources]
        sources.extend(proto_objects)


        # Uic these guys (they're moc'd automatically after this) - Generates
        # the code for the QT UI forms
        build.env.Uic4('dlgpreferencesdlg.ui')
        build.env.Uic4('dlgprefsounddlg.ui')

        build.env.Uic4('controllers/dlgprefcontrollerdlg.ui')
        build.env.Uic4('controllers/dlgprefmappablecontrollerdlg.ui')
        build.env.Uic4('controllers/dlgcontrollerlearning.ui')
        build.env.Uic4('controllers/dlgprefnocontrollersdlg.ui')

        build.env.Uic4('dlgprefplaylistdlg.ui')
        build.env.Uic4('dlgprefcontrolsdlg.ui')
        build.env.Uic4('dlgprefeqdlg.ui')
        build.env.Uic4('dlgprefcrossfaderdlg.ui')
        build.env.Uic4('dlgprefbpmdlg.ui')
        build.env.Uic4('dlgprefreplaygaindlg.ui')
        build.env.Uic4('dlgprefbeatsdlg.ui')
        build.env.Uic4('dlgbpmschemedlg.ui')
        # build.env.Uic4('dlgbpmtapdlg.ui')
        build.env.Uic4('dlgprefvinyldlg.ui')
        build.env.Uic4('dlgprefnovinyldlg.ui')
        build.env.Uic4('dlgprefrecorddlg.ui')
        build.env.Uic4('dlgaboutdlg.ui')
        build.env.Uic4('dlgtrackinfo.ui')
        build.env.Uic4('dlgprepare.ui')
        build.env.Uic4('dlgautodj.ui')
        build.env.Uic4('dlgprefsounditem.ui')
        build.env.Uic4('dlgrecording.ui')
        build.env.Uic4('dlghidden.ui')
        build.env.Uic4('dlgmissing.ui')

        if build.platform_is_windows:
            # Add Windows resource file with icons and such
            # force manifest file creation, apparently not necessary for all
            # people but necessary for this committers handicapped windows
            # installation -- bkgood
            if build.toolchain_is_msvs:
                build.env.Append(LINKFLAGS="/MANIFEST")
        elif build.platform_is_osx:
            build.env.Append(LINKFLAGS="-headerpad=ffff"); #Need extra room for code signing (App Store)
            build.env.Append(LINKFLAGS="-headerpad_max_install_names"); #Need extra room for code signing (App Store)

        return sources

    def configure(self, build, conf):
        # Evaluate this define. There are a lot of different things around the
        # codebase that use different defines. (AMD64, x86_64, x86, i386, i686,
        # EM64T). We need to unify them together.
        if not build.machine=='alpha':
            build.env.Append(CPPDEFINES=build.machine)

        if build.toolchain_is_gnu:
            # Default GNU Options
            # TODO(XXX) always generate debugging info?
            build.env.Append(CCFLAGS = '-pipe')
            build.env.Append(CCFLAGS = '-Wall')
            build.env.Append(CCFLAGS = '-Wextra')
            build.env.Append(CCFLAGS = '-g')

            # Check that g++ is present (yeah, SCONS is a bit dumb here)
            if os.system("which g++ > /dev/null"): #Checks for non-zero return code
                raise Exception("Did not find g++.")
        elif build.toolchain_is_msvs:
            # Validate the specified winlib directory exists
            mixxx_lib_path = SCons.ARGUMENTS.get('winlib', '..\\..\\..\\mixxx-win32lib-msvc100-release')
            if not os.path.exists(mixxx_lib_path):
                print mixxx_lib_path
                raise Exception("Winlib path does not exist! Please specify your winlib directory"
                                "path by running 'scons winlib=[path]'")
                Script.Exit(1)
            #mixxx_lib_path = '#/../../mixxx-win%slib-msvc100-release' % build.bitwidth

            # Set include and library paths to work with this
            build.env.Append(CPPPATH=mixxx_lib_path)
            build.env.Append(LIBPATH=mixxx_lib_path)

            #Ugh, MSVC-only hack :( see
            #http://www.qtforum.org/article/17883/problem-using-qstring-fromstdwstring.html
            build.env.Append(CXXFLAGS = '/Zc:wchar_t-')

            # Still needed?
            build.env.Append(CPPPATH=[
                    "$VCINSTALLDIR/include/atl",
                    "C:/Program Files/Microsoft Platform SDK/Include/atl"])

        if build.platform_is_windows:
            build.env.Append(CPPDEFINES='__WINDOWS__')
            build.env.Append(CPPDEFINES='_ATL_MIN_CRT') #Helps prevent duplicate symbols
            # Need this on Windows until we have UTF16 support in Mixxx
            build.env.Append(CPPDEFINES='UNICODE')
            build.env.Append(CPPDEFINES='WIN%s' % build.bitwidth) # WIN32 or WIN64
            # Tobias: Don't remove this line
            # I used the Windows API in foldertreemodel.cpp
            # to quickly test if a folder has subfolders
            build.env.Append(LIBS = 'shell32');


        elif build.platform_is_linux:
            build.env.Append(CPPDEFINES='__LINUX__')

            #Check for pkg-config >= 0.15.0
            if not conf.CheckForPKGConfig('0.15.0'):
                raise Exception('pkg-config >= 0.15.0 not found.')


        elif build.platform_is_osx:
            #Stuff you may have compiled by hand
            if os.path.isdir('/usr/local/include'):
                build.env.Append(LIBPATH = ['/usr/local/lib'])
                build.env.Append(CPPPATH = ['/usr/local/include'])

            #Non-standard libpaths for fink and certain (most?) darwin ports
            if os.path.isdir('/sw/include'):
                build.env.Append(LIBPATH = ['/sw/lib'])
                build.env.Append(CPPPATH = ['/sw/include'])

            #Non-standard libpaths for darwin ports
            if os.path.isdir('/opt/local/include'):
                build.env.Append(LIBPATH = ['/opt/local/lib'])
                build.env.Append(CPPPATH = ['/opt/local/include'])

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
        build.env.Append(CPPPATH = ['.'])

        # Set up flags for config/track listing files
        if build.platform_is_linux or \
                build.platform_is_bsd:
            mixxx_files = [
                ('SETTINGS_PATH','.mixxx/'),
                ('BPMSCHEME_FILE','mixxxbpmscheme.xml'),
                ('SETTINGS_FILE', 'mixxx.cfg'),
                ('TRACK_FILE', 'mixxxtrack.xml')]
        elif build.platform_is_osx:
            mixxx_files = [
                ('SETTINGS_PATH','Library/Application Support/Mixxx/'),
                ('BPMSCHEME_FILE','mixxxbpmscheme.xml'),
                ('SETTINGS_FILE', 'mixxx.cfg'),
                ('TRACK_FILE', 'mixxxtrack.xml')]
        elif build.platform_is_windows:
            mixxx_files = [
                ('SETTINGS_PATH','Local Settings/Application Data/Mixxx/'),
                ('BPMSCHEME_FILE', 'mixxxbpmscheme.xml'),
                ('SETTINGS_FILE', 'mixxx.cfg'),
                ('TRACK_FILE', 'mixxxtrack.xml')]
        # Escape the filenames so they don't end up getting screwed up in the
        # shell.
        mixxx_files = [(k, r'\"%s\"' % v) for k,v in mixxx_files]
        build.env.Append(CPPDEFINES=mixxx_files)

        # Say where to find resources on Unix. TODO(XXX) replace this with a
        # RESOURCE_PATH that covers Win and OSX too:
        if build.platform_is_linux or build.platform_is_bsd:
            prefix = SCons.ARGUMENTS.get('prefix', '/usr/local')
            share_path = os.path.join(prefix, 'share/mixxx')
            build.env.Append(CPPDEFINES=('UNIX_SHARE_PATH', r'\"%s\"' % share_path))
            lib_path = os.path.join(prefix, 'lib/mixxx')
            build.env.Append(CPPDEFINES=('UNIX_LIB_PATH', r'\"%s\"' % lib_path))

    def depends(self, build):
        return [SoundTouch, ReplayGain, PortAudio, PortMIDI, Qt,
                FidLib, SndFile, FLAC, OggVorbis, OpenGL, TagLib, ProtoBuf]

    def post_dependency_check_configure(self, build, conf):
        """Sets up additional things in the Environment that must happen
        after the Configure checks run."""
        if build.platform_is_windows:
            if build.toolchain_is_msvs:
                if not build.static_dependencies or build.msvcdebug:
                    build.env.Append(LINKFLAGS = ['/nodefaultlib:LIBCMT.lib',
                                                  '/nodefaultlib:LIBCMTd.lib'])

                build.env.Append(LINKFLAGS = '/entry:mainCRTStartup')
                # Makes the program not launch a shell first
                build.env.Append(LINKFLAGS = '/subsystem:windows')
                build.env.Append(LINKFLAGS = '/manifest') #Force MSVS to generate a manifest (MSVC2010)
            elif build.toolchain_is_gnu:
                # Makes the program not launch a shell first
                build.env.Append(LINKFLAGS = '--subsystem,windows')
                build.env.Append(LINKFLAGS = '-mwindows')
