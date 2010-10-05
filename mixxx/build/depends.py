
import os
import util
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
                      'win32': 'C:\\qt\\4.5.1',
                      'win64': 'C:\\qt\\4.5.1'}

    def sources(self, build):
        return []

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


class MixxxCore(Dependence):

    def sources(self, build):
        return []

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
