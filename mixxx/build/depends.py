
import os
from mixxx import Dependence
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
                      'win32': 'C:\\qt\\4.5.1',
                      'win64': 'C:\\qt\\4.5.1'}

    def sources(self, build):
        return []

    def satisfy(self):
        pass

    def configure(self, build, conf):
        default_qtdir = self.DEFAULT_QTDIRS[build.platform]

        qtdir = SCons.ARGUMENTS.get('qtdir',
                                    os.environ.get('QTDIR', default_qtdir))

        if not os.path.exists(qtdir):
            raise Exception("Error: QT path does not exist or QT4 is not installed. Please specify your QT path by running 'scons qtdir=[path]'")
        elif qtdir.find("qt3") != -1 or qtdir.find("qt/3") != -1:
            raise Exception("Error: Mixxx now requires QT4 instead of QT3 - please use your QT4 path with the qtdir build flag.")

        build.env['QTDIR'] = qtdir

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


# NOT USED RIGHT NOW
class SoundTouch(Dependence):
    SOUNDTOUCH_PATH = 'soundtouch-1.4.1'

    def sources(self, build):
        sources =  ['engine/enginebufferscalest.cpp',
                    '#lib/%s/SoundTouch.cpp' % self.SOUNDTOUCH_PATH,
                    '#lib/%s/TDStretch.cpp' % self.SOUNDTOUCH_PATH,
                    '#lib/%s/RateTransposer.cpp' % self.SOUNDTOUCH_PATH,
                    '#lib/%s/AAFilter.cpp' % self.SOUNDTOUCH_PATH,
                    '#lib/%s/FIFOSampleBuffer.cpp' % self.SOUNDTOUCH_PATH,
                    '#lib/%s/FIRFilter.cpp' % self.SOUNDTOUCH_PATH,
                    '#lib/%s/PeakFinder.cpp' % self.SOUNDTOUCH_PATH,
                    '#lib/%s/BPMDetect.cpp' % self.SOUNDTOUCH_PATH]
        if build.platform_is_windows:
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

    def configure(self, build, conf):
        if build.platform_is_windows:
            build.env.Append(CPPDEFINES = 'WIN'+build.bitwidth)
        build.env.Append(CPPPATH=['#lib/%s' % self.SOUNDTOUCH_PATH])


# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
# THIS IS UNUSED
# THIS IS UNUSED
# THIS IS UNUSED  It will be used later! It's not used now!
# THIS IS UNUSED
# THIS IS UNUSED
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
class MixxxCore(Dependence):

    def sources(self, build):
        return ['input.cpp',
                'trackplaylistlist.cpp',
                'mixxxkeyboard.cpp',
                'configobject.cpp',
                'controlobjectthread.cpp',
                'controlobjectthreadwidget.cpp',
                'controlobjectthreadmain.cpp',
                'controlevent.cpp',
                'controllogpotmeter.cpp',
                'controlobject.cpp',
                'controlnull.cpp',
                'controlpotmeter.cpp',
                'controlpushbutton.cpp',
                'controlttrotary.cpp',
                'controlbeat.cpp',

                'dlgpreferences.cpp',
                'dlgprefsound.cpp',
                'dlgprefmidibindings.cpp',
                'dlgprefplaylist.cpp',
                'dlgprefnomidi.cpp',
                'dlgprefcontrols.cpp',
                'dlgbpmtap.cpp',
                'dlgprefbpm.cpp',
                'dlgbpmscheme.cpp',
                'dlgabout.cpp',
                'dlgprefeq.cpp',
                'dlgprefcrossfader.cpp',
                'dlgmidilearning.cpp',

                'engine/enginebuffercue.cpp',
                'engine/enginebuffer.cpp',
                'engine/enginebufferscale.cpp',
		'engine/enginebufferscaledummy.cpp',
                'engine/enginebufferscalelinear.cpp',
		'engine/enginebufferscalereal.cpp',
                'engine/engineclipping.cpp',
                'engine/enginefilterblock.cpp',
                'engine/enginefilteriir.cpp',
                'engine/enginefilter.cpp',
                'engine/engineobject.cpp',
                'engine/enginepregain.cpp',
                'engine/enginevolume.cpp',
                'engine/enginechannel.cpp',
                'engine/enginemaster.cpp',
                'engine/enginedelay.cpp',
                'engine/engineflanger.cpp',
                'engine/enginespectralfwd.cpp',
                'engine/enginevumeter.cpp',
                'engine/enginevinylsoundemu.cpp',
                'engine/enginesidechain.cpp',
                'engine/enginefilterbutterworth8.cpp',
                'engine/enginexfader.cpp',

                'analyserqueue.cpp',
		'analyserwavesummary.cpp',
		'analyserbpm.cpp',
		'analyserwaveform.cpp',

                'main.cpp',
                'midiobject.cpp',
                'midimapping.cpp',
                'midiobjectnull.cpp',
                'mididevicehandler.cpp',
                'midiinputmappingtablemodel.cpp',
                'midioutputmappingtablemodel.cpp',
                'midichanneldelegate.cpp',
                'midistatusdelegate.cpp',
                'midinodelegate.cpp',
                'midioptiondelegate.cpp',
                'controlgroupdelegate.cpp',
                'controlvaluedelegate.cpp',
                'midimessage.cpp',
                'mixxxcontrol.cpp',
                'mixxx.cpp',
                'mixxxview.cpp',
                'errordialog.cpp',
                'upgrade.cpp',

                'soundsource.cpp',
                'soundsourcemp3.cpp',
                'soundsourceoggvorbis.cpp',

                'widget/wwidget.cpp',
                'widget/wlabel.cpp',
                'widget/wnumber.cpp',
                'widget/wnumberpos.cpp',
                'widget/wnumberrate.cpp',
                'widget/wnumberbpm.cpp',
                'widget/wknob.cpp',
                'widget/wdisplay.cpp',
                'widget/wvumeter.cpp',
                'widget/wpushbutton.cpp',
                'widget/wslidercomposed.cpp',
                'widget/wslider.cpp',
                'widget/wstatuslight.cpp',
		'widget/woverview.cpp',
		'widget/wskincolor.cpp',
		'widget/wabstractcontrol.cpp',
                'widget/wsearchlineedit.cpp',
		'widget/wpixmapstore.cpp',
                'widget/hexspinbox.cpp',

                'mathstuff.cpp',
                'readerextract.cpp',
                'readerextractwave.cpp',
                'readerevent.cpp',
                'rtthread.cpp',
                'windowkaiser.cpp',
                'probabilityvector.cpp',
                'reader.cpp',
                'peaklist.cpp',
                'rotary.cpp',
                'track.cpp',
                'trackcollection.cpp',
                'trackplaylist.cpp',
                'wtracktableview.cpp',
                'wtracktablemodel.cpp',
                'wpromotracksmodel.cpp',
                'proxymodel.cpp',
                'xmlparse.cpp',
                'trackimporter.cpp',
                'parser.cpp',
                'parserpls.cpp',
                'parserm3u.cpp',
                'bpm/bpmscheme.cpp',
                'soundsourceproxy.cpp',
                'widget/wvisualsimple.cpp',
                'widget/wwaveformviewer.cpp',
                'widget/wglwaveformviewer.cpp',
                'waveformviewerfactory.cpp',
                'waveform/waveformrenderer.cpp',
		'waveform/waveformrenderbackground.cpp',
		'waveform/waveformrendersignal.cpp',
		'waveform/waveformrendersignalpixmap.cpp',
		'waveform/waveformrendermark.cpp	',
                'waveform/waveformrenderbeat.cpp',
                'imginvert.cpp',
                'imgloader.cpp',
                'imgcolor.cpp',
                'trackinfoobject.cpp',
                'midiledhandler.cpp',
                'sounddevice.cpp',
                'soundmanager.cpp',
                'dlgprefrecord.cpp',
                'recording/enginerecord.cpp',
                'recording/writeaudiofile.cpp',
                'wtracktablefilter.cpp',
                'wplaylistlistmodel.cpp',
                'libraryscanner.cpp',
                'libraryscannerdlg.cpp',
                'playerinfo.cpp',
                'segmentation.cpp']

    def configure(self, build, conf):
        if build.platform_is_windows:
            build.env.Append(CPPPATH='#lib/ladspa')
            if build.machine == 'x86_64':
                mixxx_lib_path = '#/../mixxx-win64lib'
            else:
                mixxx_lib_path = '#/../mixxx-winlib'

            build.env.Append(CPPPATH=mixxx_lib_path)
            build.env.Append(LIBPATH=mixxx_lib_path)
            if build.toolchain_is_msvs:
                build.env.Append(LINKFLAGS = ['/nodefaultlib:libc.lib',
                                              '/nodefaultlib:libcd.lib',
                                              '/entry:mainCRTStartup'])
                build.env.Append(LINKFLAGS = ['/nodefaultlib:LIBCMT.lib',
                                              '/nodefaultlib:LIBCMTD.lib'])
                #Ugh, MSVC-only hack :( see
                #http://www.qtforum.org/article/17883/problem-using-qstring-fromstdwstring.html
                build.env.Append(CXXFLAGS = '/Zc:wchar_t-')
            elif build.toolchain_is_gnu:
               build.env.Append(LINKFLAGS = [])

        elif build.platform_is_linux:
            pass
        elif build.platform_is_osx:
            #Non-standard libpaths for fink and certain (most?) darwin ports
            build.env.Append(LIBPATH = ['/sw/lib'])
            build.env.Append(CPPPATH = ['/sw/include'])

            #Non-standard libpaths for darwin ports
            build.env.Append(LIBPATH = ['/opt/local/lib'])
            build.env.Append(CPPPATH = ['/opt/local/include'])

        elif build.platform_is_bsd:
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

    def depends(self):
        return [SoundTouch, KissFFT, PortAudio, PortMIDI, Qt, FidLib, Mad, OggVorbis, OpenGL]
