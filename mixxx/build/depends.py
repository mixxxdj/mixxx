
import os
import util
from mixxx import Dependence, Feature
import SCons.Script as SCons

class PortAudio(Dependence):

    def configure(self, build, conf):
        if not conf.CheckLib('portaudio'):
            raise Exception('Did not find libportaudio.a, portaudio.lib, or the PortAudio-v19 development header files.')

        #Turn on PortAudio support in Mixxx
        build.env.Append(CPPDEFINES = '__PORTAUDIO__');

    def sources(self, build):
        return ['sounddeviceportaudio.cpp']

class PortMIDI(Dependence):

    def configure(self, build, conf):
        #Check for PortTime
        if not conf.CheckLib(['porttime', 'libporttime']) and \
                not conf.CheckHeader(['porttime.h']):
            raise Exception("Did not find PortTime or its development headers.")
        if not conf.CheckLib(['portmidi', 'libportmidi']) and \
                not conf.CheckHeader(['portmidi.h']):
            raise Exception('Did not find PortMidi or its development headers.')

        # WHY!? Supposedly we need this for PortMIDI.
        if build.platform_is_windows:
            build.env.Append(LIBS='advapi32')

    def sources(self, build):
        return ['midi/midideviceportmidi.cpp']

class OpenGL(Dependence):

    def configure(self, build, conf):
        # Check for OpenGL (it's messy to do it for all three platforms) XXX
        # this should *NOT* have hardcoded paths like this
        if (not conf.CheckLib('GL') and
            not conf.CheckLib('opengl32') and
            not conf.CheckCHeader('/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/gl.h') and
            not conf.CheckCHeader('GL/gl.h')):
            raise Exception('Did not find OpenGL development files, exiting!')

        if (not conf.CheckLib('GLU') and
            not conf.CheckLib('glu32') and
            not conf.CheckCHeader('/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/glu.h')):
            raise Exception('Did not find GLU development files, exiting!')

        if build.platform_is_osx:
            build.env.Append(CPPPATH='/Library/Frameworks/OpenGL.framework/Headers/')
            build.env.Append(LINKFLAGS='-framework OpenGL')

class OggVorbis(Dependence):

    def configure(self, build, conf):
        if build.platform_is_windows and build.machine_is_64bit:
            # For some reason this has to be checked this way on win64,
            # otherwise it looks for the dll lib which will cause a conflict
            # later
            if not conf.CheckLib('vorbisfile_static'):
                raise Exception('Did not find vorbisfile_static.lib or the libvorbisfile development headers.')
        else:
            if not conf.CheckLib('vorbisfile'):
		Exception('Did not find libvorbisfile.a, libvorbisfile.lib, or the libvorbisfile development headers.')

        if not conf.CheckLib('vorbis'):
            raise Exception('Did not find libvorbis.a, libvorbis.lib, or the libvorbisfile development headers.')

        if not conf.CheckLib('ogg'):
            raise Exception('Did not find libogg.a, libogg.lib, or the libogg development headers, exiting!')

    def sources(self, build):
        return ['soundsourceoggvorbis.cpp']

class Mad(Dependence):

    def configure(self, build, conf):
        if not conf.CheckLib(['mad','libmad']):
            raise Exception('Did not find libmad.a, libmad.lib, or the libmad development header files - exiting!')
        if not conf.CheckLib(['id3tag','libid3tag-release']):
            raise Exception('Did not find libid3tag.a, libid3tag.lib, or the libid3tag development header files - exiting!')

    def sources(self, build):
        return ['soundsourcemp3.cpp']


class SndFile(Dependence):

    def configure(self, build, conf):
        #if not conf.CheckLibWithHeader(['sndfile', 'libsndfile'], 'sndfile.h', 'C'):
        if not conf.CheckLib(['sndfile', 'libsndfile']):
            raise Exception("Did not find libsndfile or it\'s development headers, exiting!")
        build.env.Append(CPPDEFINES = '__SNDFILE__')

    def sources(self, build):
        return ['soundsourcesndfile.cpp']

class Qt(Dependence):
    DEFAULT_QTDIRS = {'linux': '/usr/share/qt4',
                      'bsd': '/usr/local/lib/qt4',
                      'osx': '/usr/lib/Qt-4.5',
                      'windows': 'C:\\qt\\4.5.1'}

    def satisfy(self):
        pass

    def configure(self, build, conf):
        if not conf.CheckForPKG('QtCore', '4.3'):
            raise Exception('QT >= 4.3 not found')

        # Emit various Qt defines
        build.env.Append(CPPDEFINES = ['QT3_SUPPORT',
                                       'QT3_SUPPORT_WARNINGS',
                                       'QT_THREAD_SUPPORT',
                                       'QT_SHARED',
                                       'QT_TABLET_SUPPORT'])

        # Enable Qt includep paths
        if build.platform_is_linux:
            #Try using David's qt4.py's Qt4-module finding thingy instead of pkg-config.
            #(This hopefully respects our qtdir=blah flag while linking now.)
            build.env.EnableQt4Modules(['QtCore',
                                        'QtGui',
                                        'QtOpenGL',
                                        'Qt3Support',
                                        'QtXml',
                                        'QtSvg',
                                        'QtSql',
                                        'QtScript',
                                        'QtXmlPatterns',
                                        'QtWebKit'
                                        #'QtUiTools',
                                        #'QtDesigner',
                                        ],
                                       debug=False)
        elif build.platform_is_osx:
            build.env.Append(LINKFLAGS = '-framework QtCore -framework QtOpenGL -framework Qt3Support -framework QtGui -framework QtSql -framework QtXml -framework QtXmlPatterns  -framework QtNetwork -framework QtSql -framework QtScript -framework QtWebKit')
            build.env.Append(CPPPATH = ['/Library/Frameworks/QtCore.framework/Headers/',
				'/Library/Frameworks/QtOpenGL.framework/Headers/',
				'/Library/Frameworks/Qt3Support.framework/Headers/',
				'/Library/Frameworks/QtGui.framework/Headers/',
				'/Library/Frameworks/QtXml.framework/Headers/',
				'/Library/Frameworks/QtNetwork.framework/Headers/',
				'/Library/Frameworks/QtSql.framework/Headers/',
				'/Library/Frameworks/QtWebKit.framework/Headers/',
				'/Library/Frameworks/QtScript.framework/Headers/'])

        # Setup Qt library includes for non-OSX
        if build.platform_is_linux or build.platform_is_bsd:
            build.env.Append(LIBS = 'Qt3Support')
            build.env.Append(LIBS = 'QtXml')
            build.env.Append(LIBS = 'QtGui')
            build.env.Append(LIBS = 'QtCore')
            build.env.Append(LIBS = 'QtNetwork')
            build.env.Append(LIBS = 'QtOpenGL')
            build.env.Append(LIBS = 'QtWebKit')
            build.env.Append(LIBS = 'QtScript')
        elif build.platform_is_windows:
            build.env.Append(LIBS = 'Qt3Support4');
            build.env.Append(LIBS = 'QtXml4');
            build.env.Append(LIBS = 'QtXmlPatterns4');
            build.env.Append(LIBS = 'QtSql4');
            build.env.Append(LIBS = 'QtGui4');
            build.env.Append(LIBS = 'QtCore4');
            build.env.Append(LIBS = 'QtWebKit4');
            build.env.Append(LIBS = 'QtNetwork4')
            build.env.Append(LIBS = 'QtOpenGL4');

        # Set Qt include paths for non-OSX
        if not build.platform_is_osx:
            build.env.Append(CPPPATH=['$QTDIR/include/Qt3Support',
                                      '$QTDIR/include/QtCore',
                                      '$QTDIR/include/QtGui',
                                      '$QTDIR/include/QtXml',
                                      '$QTDIR/include/QtNetwork',
                                      '$QTDIR/include/QtSql',
                                      '$QTDIR/include/QtOpenGL',
                                      '$QTDIR/include/QtWebKit',
                                      '$QTDIR/include/Qt'])

        # Set the rpath for linux/bsd/osx. TODO(XXX) is this supposed to be done
        # for OSX?
        if not build.platform_is_windows:
            build.env.Append(LINKFLAGS = "-Wl,-rpath,$QTDIR/lib")



class FidLib(Dependence):

    def sources(self, build):
        symbol = None
        if build.platform_is_windows:
            if build.toolchain_is_msvs:
                symbol = 'T_MSVC'
            elif build.toolchain_is_gnu:
                symbol = 'T_MINGW'
        else:
            symbol = 'T_LINUX'

        return [build.env.StaticObject('#lib/fidlib-0.9.9/fidlib.c',
                                       CPPDEFINES=symbol)]

    def configure(self, build, conf):
        build.env.Append(CPPPATH='#lib/fidlib-0.9.9/')

class KissFFT(Dependence):

    def sources(self, build):
        return ["#lib/kissfft/kiss_fft.c"]

    def configure(self, build, conf):
        build.env.Append(CPPPATH="#lib/kissfft")


class SoundTouch(Dependence):
    SOUNDTOUCH_PATH = 'soundtouch-1.4.1'

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
        if build.platform_is_windows and build.toolchain_is_msvs:
            if build.machine == 'x86_64':
                sources.append(
                    '#lib/%s/cpu_detect_x64_win.cpp' % self.SOUNDTOUCH_PATH)
            elif build.machine == 'x86':
                sources.append(
                    '#lib/%s/cpu_detect_x86_win.cpp' % self.SOUNDTOUCH_PATH)
            else:
                raise Exception("Unhandled CPU configuration for SoundTouch")
        elif build.toolchain_is_gnu:
            if build.machine == 'x86_64':
                sources.append(
                    '#lib/%s/cpu_detect_x64_gcc.cpp' % self.SOUNDTOUCH_PATH)
            else:
                sources.append(
                    '#lib/%s/cpu_detect_x86_gcc.cpp' % self.SOUNDTOUCH_PATH)
        else:
            raise Exception("Unhandled CPU configuration for SoundTouch")

        # TODO(XXX) when we figure out a better way to represent features, fix
        # this.
        optimize = int(util.get_flags(build.env, 'optimize', 1))
        if build.machine_is_64bit or \
                (build.toolchain_is_msvs and optimize > 1) or \
                (build.toolchain_is_gnu and optimize > 2):
            sources.extend(
                ['#lib/%s/mmx_optimized.cpp' % self.SOUNDTOUCH_PATH,
                 '#lib/%s/sse_optimized.cpp' % self.SOUNDTOUCH_PATH,
                 ])
        if build.toolchain_is_msvs and not build.machine_is_64bit:
            sources.append('#lib/%s/3dnow_win.cpp' % self.SOUNDTOUCH_PATH)
        else:
            # TODO(XXX) the docs refer to a 3dnow_gcc, but we don't seem to have
            # it.
            pass

        return sources

    def configure(self, build, conf):
        if build.platform_is_windows:
            build.env.Append(CPPDEFINES = 'WIN'+build.bitwidth)
        build.env.Append(CPPPATH=['#lib/%s' % self.SOUNDTOUCH_PATH])

        # TODO(XXX) when we figure out a better way to represent features, fix
        # this.
        optimize = int(util.get_flags(build.env, 'optimize', 1))
        if build.machine_is_64bit or \
                (build.toolchain_is_msvs and optimize > 1) or \
                (build.toolchain_is_gnu and optimize > 2):
            build.env.Append(CPPDEFINES='ALLOW_X86_OPTIMIZATIONS')


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
                   "controlpushbutton.cpp",
                   "controlttrotary.cpp",
                   "controlbeat.cpp",

                   "dlgpreferences.cpp",
                   "dlgprefsound.cpp",
                   "dlgprefmidibindings.cpp",
                   "dlgprefplaylist.cpp",
                   "dlgprefnomidi.cpp",
                   "dlgprefcontrols.cpp",
                   "dlgprefbpm.cpp",
                   "dlgbpmscheme.cpp",
                   "dlgabout.cpp",
                   "dlgprefeq.cpp",
                   "dlgprefcrossfader.cpp",
                   "dlgmidilearning.cpp",
                   "dlgtrackinfo.cpp",
                   "dlgprepare.cpp",
                   "dlgautodj.cpp",

                   "engine/engineworker.cpp",
                   "engine/engineworkerscheduler.cpp",
                   "engine/enginebuffer.cpp",
                   "engine/enginebufferscale.cpp",
                   "engine/enginebufferscaledummy.cpp",
                   "engine/enginebufferscalelinear.cpp",
                   "engine/enginebufferscalereal.cpp",
                   "engine/engineclipping.cpp",
                   "engine/enginefilterblock.cpp",
                   "engine/enginefilteriir.cpp",
                   "engine/enginefilter.cpp",
                   "engine/engineobject.cpp",
                   "engine/enginepregain.cpp",
                   "engine/enginevolume.cpp",
                   "engine/enginechannel.cpp",
                   "engine/enginemaster.cpp",
                   "engine/enginedelay.cpp",
                   "engine/engineflanger.cpp",
                   "engine/enginevumeter.cpp",
                   "engine/enginevinylsoundemu.cpp",
                   "engine/enginesidechain.cpp",
                   "engine/enginefilterbutterworth8.cpp",
                   "engine/enginexfader.cpp",
                   "engine/enginecontrol.cpp",
                   "engine/ratecontrol.cpp",
                   "engine/loopingcontrol.cpp",
                   "engine/bpmcontrol.cpp",
                   "engine/cuecontrol.cpp",
                   "engine/readaheadmanager.cpp",
                   "cachingreader.cpp",

                   "analyserqueue.cpp",
                   "analyserwavesummary.cpp",
                   "analyserbpm.cpp",
                   "analyserwaveform.cpp",

                   "midi/mididevice.cpp",
                   "midi/mididevicemanager.cpp",
                   "midi/midimapping.cpp",
                   "midi/midiinputmappingtablemodel.cpp",
                   "midi/midioutputmappingtablemodel.cpp",
                   "midi/midichanneldelegate.cpp",
                   "midi/midistatusdelegate.cpp",
                   "midi/midinodelegate.cpp",
                   "midi/midioptiondelegate.cpp",
                   "midi/midimessage.cpp",
                   "midi/midiledhandler.cpp",

                   "main.cpp",
                   "controlgroupdelegate.cpp",
                   "controlvaluedelegate.cpp",
                   "mixxxcontrol.cpp",
                   "mixxx.cpp",
                   "mixxxview.cpp",
                   "errordialoghandler.cpp",
                   "upgrade.cpp",

                   "soundsource.cpp",

                   "widget/wwidget.cpp",
                   "widget/wlabel.cpp",
                   "widget/wnumber.cpp",
                   "widget/wnumberpos.cpp",
                   "widget/wnumberrate.cpp",
                   "widget/wnumberbpm.cpp",
                   "widget/wknob.cpp",
                   "widget/wdisplay.cpp",
                   "widget/wvumeter.cpp",
                   "widget/wpushbutton.cpp",
                   "widget/wslidercomposed.cpp",
                   "widget/wslider.cpp",
                   "widget/wstatuslight.cpp",
                   "widget/woverview.cpp",
                   "widget/wskincolor.cpp",
                   "widget/wabstractcontrol.cpp",
                   "widget/wsearchlineedit.cpp",
                   "widget/wpixmapstore.cpp",
                   "widget/hexspinbox.cpp",

                   "mathstuff.cpp",

                   "rotary.cpp",
                   "widget/wtracktableview.cpp",
                   "widget/wtracktableviewheader.cpp",
                   "widget/wlibrarysidebar.cpp",
                   "widget/wlibrary.cpp",
                   "widget/wlibrarytableview.cpp",
                   "widget/wpreparelibrarytableview.cpp",
                   "widget/wpreparecratestableview.cpp",
                   "widget/wbrowsetableview.cpp",
                   "widget/wlibrarytextbrowser.cpp",
                   "library/preparecratedelegate.cpp",
                   "library/trackcollection.cpp",
                   "library/basesqltablemodel.cpp",
                   "library/librarytablemodel.cpp",
                   "library/preparelibrarytablemodel.cpp",
                   "library/browsetablemodel.cpp",
                   "library/missingtablemodel.cpp",
                   "library/proxytrackmodel.cpp",
                   "library/abstractxmltrackmodel.cpp",
                   "library/rhythmboxtrackmodel.cpp",
                   "library/rhythmboxplaylistmodel.cpp",
                   "library/itunestrackmodel.cpp",
                   "library/itunesplaylistmodel.cpp",
                   "library/playlisttablemodel.cpp",
                   "library/libraryfeature.cpp",
                   "library/preparefeature.cpp",
                   "library/autodjfeature.cpp",
                   "library/mixxxlibraryfeature.cpp",
                   "library/playlistfeature.cpp",
                   "library/rhythmboxfeature.cpp",
                   "library/itunesfeature.cpp",
                   "library/browsefeature.cpp",
                   "library/cratefeature.cpp",
                   "library/browsefilter.cpp",
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
                   "library/librarymidicontrol.cpp",
                   "library/schemamanager.cpp",
                   "library/promotracksfeature.cpp",
                   "library/featuredartistswebview.cpp",
                   "library/bundledsongswebview.cpp",

                   "xmlparse.cpp",
                   "parser.cpp",
                   "parserpls.cpp",
                   "parserm3u.cpp",

                   "bpm/bpmscheme.cpp",

                   "soundsourceproxy.cpp",

                   "widget/wvisualsimple.cpp",
                   "widget/wwaveformviewer.cpp",
                   "widget/wglwaveformviewer.cpp",
                   "waveformviewerfactory.cpp",
                   "waveform/renderobject.cpp",
                   "waveform/waveformrenderer.cpp",
                   "waveform/waveformrenderbackground.cpp",
                   "waveform/waveformrendersignal.cpp",
                   "waveform/waveformrendersignaltiles.cpp",
                   "waveform/waveformrendersignalpixmap.cpp",
                   "waveform/waveformrendermark.cpp",
                   "waveform/waveformrendermarkrange.cpp",
                   "waveform/waveformrenderbeat.cpp",

                   "imginvert.cpp",
                   "imgloader.cpp",
                   "imgcolor.cpp",

                   "trackinfoobject.cpp",
                   "player.cpp",
                   "sounddevice.cpp",
                   "soundmanager.cpp",
                   "dlgprefrecord.cpp",
                   "playerinfo.cpp",

                   "recording/enginerecord.cpp",
                   "recording/encoder.cpp",

                   "segmentation.cpp",
                   ]

        # Uic these guys (they're moc'd automatically after this) - Generates
        # the code for the QT UI forms
        build.env.Uic4('dlgpreferencesdlg.ui')
        build.env.Uic4('dlgprefsounddlg.ui')
        build.env.Uic4('dlgprefmidibindingsdlg.ui')
        build.env.Uic4('dlgprefplaylistdlg.ui')
        build.env.Uic4('dlgprefnomididlg.ui')
        build.env.Uic4('dlgprefcontrolsdlg.ui')
        build.env.Uic4('dlgprefeqdlg.ui')
        build.env.Uic4('dlgprefcrossfaderdlg.ui')
        build.env.Uic4('dlgprefbpmdlg.ui')
        build.env.Uic4('dlgbpmschemedlg.ui')
        # build.env.Uic4('dlgbpmtapdlg.ui')
        build.env.Uic4('dlgprefvinyldlg.ui')
        build.env.Uic4('dlgprefrecorddlg.ui')
        build.env.Uic4('dlgaboutdlg.ui')
        build.env.Uic4('dlgmidilearning.ui')
        build.env.Uic4('dlgtrackinfo.ui')
        build.env.Uic4('dlgprepare.ui')
        build.env.Uic4('dlgautodj.ui')

        # Add the QRC file which compiles in some extra resources (prefs icons,
        # etc.)
        build.env.Qrc('#res/mixxx.qrc')
        sources.append("#res/qrc_mixxx.cc")

        if build.platform_is_windows:
            # Add Windows resource file with icons and such
            build.env.RES('#src/mixxx.rc')
	    # Tobias Rafreider: What is the purpose of the following line, if
	    # the file doesn't exist?
            #
            # I think this file is auto-generated on Windows, as qrc_mixxx.cc is
            # auto-generated above. Leaving uncommented.
            sources.append("mixxx.res")

        return sources

    def configure(self, build, conf):
        # Evaluate this define. There are a lot of different things around the
        # codebase that use different defines. (AMD64, x86_64, x86, i386, i686,
        # EM64T). We need to unify them together.
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
            if build.machine == 'x86_64':
                mixxx_lib_path = '#/../mixxx-win%slib-msvc' % build.bitwidth
            else:
                mixxx_lib_path = '#/../mixxx-win%slib-msvc' % build.bitwidth

            # Set include and library paths to work with this
            build.env.Append(CPPPATH=mixxx_lib_path)
            build.env.Append(LIBPATH=mixxx_lib_path)

            build.env.Append(LINKFLAGS = ['/nodefaultlib:libc.lib',
                                          '/nodefaultlib:libcd.lib',
                                          '/entry:mainCRTStartup'])
            build.env.Append(LINKFLAGS = ['/nodefaultlib:LIBCMT.lib',
                                          '/nodefaultlib:LIBCMTD.lib'])
            #Ugh, MSVC-only hack :( see
            #http://www.qtforum.org/article/17883/problem-using-qstring-fromstdwstring.html
            build.env.Append(CXXFLAGS = '/Zc:wchar_t-')

            # Still needed?
            build.env.Append(CPPPATH=[
                    "$VCINSTALLDIR/include/atl",
                    "C:/Program Files/Microsoft Platform SDK/Include/atl"])

        if build.platform_is_windows:
            build.env.Append(CPPDEFINES='__WINDOWS__')
            # Need this on Windows until we have UTF16 support in Mixxx
            build.env.Append(CPPDEFINES='UNICODE')
            build.env.Append(CPPDEFINES='WIN'+build.bitwidth)

            #Needed for Midi stuff, should be able to remove since PortMIDI
            build.env.Append(LIBS = 'WinMM');
            #Tobias Rafreider: libshout won't compile if you uncomment this
	    #build.env.Append(LIBS = 'ogg_static')
	    #build.env.Append(LIBS = 'vorbis_static')
	    #build.env.Append(LIBS = 'vorbisfile_static')
            build.env.Append(LIBS = 'imm32')
            build.env.Append(LIBS = 'wsock32')
            build.env.Append(LIBS = 'delayimp')
            build.env.Append(LIBS = 'winspool')
            build.env.Append(LIBS = 'shell32')

            # Makes the program not launch a shell first
            if build.toolchain_is_msvs:
                build.env.Append(LINKFLAGS = '/subsystem:windows')
            elif build.toolchain_is_gnu:
                build.env.Append(LINKFLAGS = '-subsystem,windows')

        elif build.platform_is_linux:
            build.env.Append(CPPDEFINES='__LINUX__')

            #Check for pkg-config >= 0.15.0
            if not conf.CheckForPKGConfig('0.15.0'):
		raise Exception('pkg-config >= 0.15.0 not found.')


        elif build.platform_is_osx:
            #Non-standard libpaths for fink and certain (most?) darwin ports
            build.env.Append(LIBPATH = ['/sw/lib'])
            build.env.Append(CPPPATH = ['/sw/include'])

            #Non-standard libpaths for darwin ports
            build.env.Append(LIBPATH = ['/opt/local/lib'])
            build.env.Append(CPPPATH = ['/opt/local/include'])

            # TODO(XXX) ALBERT PLEASE TEST WHICH FRAMEWORKS ARE REQUIRED
            # if not conf.CheckCXXHeader('/System/Library/Frameworks/CoreMIDI.framework/Headers/CoreMIDI.h'):
	    #     raise Exception('Did not find CoreMIDI framework, exiting! (Please install it)')
            # else:
	    #     build.env.Append(LINKFLAGS = '-framework CoreMIDI -framework CoreFoundation -framework CoreAudio -framework Carbon -framework QuickTime -framework AudioToolbox -framework AudioUnit') #Have to add the rest of these frameworks somewhere..

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

            # We should not have to do either of these.
            #build.env.Append(LIBS = 'sndfile')
            #build.env.Append(LIBS = 'vorbisfile')

        # I disagree with this. rryan 9/2010
        #env.Append(CPPPATH = ['.', '../', '../../']) #Fun fun fun with paths

        # Add the src/ directory to the include path
        build.env.Append(CPPPATH = ['.'])


        # Set up flags for config/track listing files
        if build.platform_is_linux or \
                build.platform_is_bsd or \
                build.platform_is_osx:
            mixxx_files = [
                ('SETTINGS_PATH','.mixxx/'),
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
            share_path = os.path.join(SCons.ARGUMENTS.get('prefix', '/usr/local'), 'share/mixxx')
            build.env.Append(CPPDEFINES=('UNIX_SHARE_PATH', r'\"%s\"' % share_path))

    def depends(self, build):
        return [SoundTouch, KissFFT, PortAudio, PortMIDI, Qt,
                FidLib, Mad, SndFile, OggVorbis, OpenGL]
