#
# Options (select one audio driver and one midi driver)
#

# PortAudio (Working good. Linux OSS, Windows, MacOS X)
SOURCES += playerportaudio.cpp
HEADERS += playerportaudio.h
DEFINES += __PORTAUDIO__
unix:!macx:SOURCES += ../lib/portaudio-v18/ringbuffer.c ../lib/portaudio-v18/pa_lib.c ../lib/portaudio-v18/pa_convert.c ../lib/portaudio-v18/pa_unix.c ../lib/portaudio-v18/pa_unix_oss.c
unix:!macx:HEADERS += ../lib/portaudio-v18/ringbuffer.h ../lib/portaudio-v18/portaudio.h ../lib/portaudio-v18/pa_host.h ../lib/portaudio-v18/pa_unix.h
win32:SOURCES += ../lib/portaudio-v18/pa_lib.c ../lib/portaudio-v18/dsound_wrapper.c ../lib/portaudio-v18/pa_dsound.c
win32:HEADERS += ../lib/portaudio-v18/portaudio.h ../lib/portaudio-v18/pa_host.h
win32:LIBS += dsound.lib
macx:SOURCES += ../lib/portaudio-v18/ringbuffer.c ../lib/portaudio-v18/pa_lib.c ../lib/portaudio-v18/pa_mac_core.c ../lib/portaudio-v18/pa_convert.c
macx:HEADERS += ../lib/portaudio-v18/ringbuffer.h ../lib/portaudio-v18/portaudio.h ../lib/portaudio-v18/pa_host.h
macx:LIBS += -framework CoreAudio -framework AudioToolbox
macx:INCLUDEPATH += ../lib/portaudio-v18

# OSS Midi (Working good, Linux specific)
unix:!macx:SOURCES += midiobjectoss.cpp
unix:!macx:HEADERS += midiobjectoss.h
unix:!macx:DEFINES += __OSSMIDI__

# Windows MIDI
win32:SOURCES += midiobjectwin.cpp
win32:HEADERS += midiobjectwin.h
win32:DEFINES += __WINMIDI__

# PortMidi (Not really working, Linux ALSA, Windows and MacOS X)
#SOURCES += midiobjectportmidi.cpp
#HEADERS += midiobjectportmidi.h
#DEFINES += __PORTMIDI__
#unix:LIBS += -lportmidi -lporttime
#win32:LIBS += ../lib/pm_dll.lib

# CoreMidi (Mac OS X)
macx:SOURCES += midiobjectcoremidi.cpp
macx:HEADERS += midiobjectcoremidi.h
macx:DEFINES += __COREMIDI__
macx:LIBS    += -framework CoreMIDI -framework CoreFoundation

# ALSA PCM (Not currently working, Linux specific)
#SOURCES += playeralsa.cpp
#HEADERS += playeralsa.h
#DEFINES += __ALSA__
#unix:LIBS += -lasound

# ALSA MIDI (Not currently working, Linux specific)
#SOURCES += midiobjectalsa.cpp
#HEADERS += midiobjectalsa.h
#DEFINES  += __ALSAMIDI__

# Visuals
#SOURCES += mixxxvisual.cpp visual/visualbackplane.cpp visual/texture.cpp visual/guicontainer.cpp visual/signalvertexbuffer.cpp visual/visualbox.cpp visual/visualcontroller.cpp visual/guichannel.cpp visual/guisignal.cpp visual/light.cpp visual/material.cpp visual/picking.cpp visual/pickable.cpp visual/visualsignal.cpp visual/visualobject.cpp visual/fastvertexarray.cpp
#HEADERS += mixxxvisual.h visual/visualbackplane.h  visual/texture.h visual/guicontainer.h visual/signalvertexbuffer.h visual/visualbox.h visual/visualcontroller.h visual/guichannel.h visual/guisignal.h visual/light.h visual/material.h visual/picking.h visual/pickable.h visual/visualsignal.h visual/visualobject.h visual/fastvertexarray.h
#CONFIG += opengl
#DEFINES += __VISUALS__

# Use NVSDK when building
#DEFINES += __NVSDK__
#win32:INCLUDEPATH += c:/Progra~1/NVIDIA~1/NVSDK/OpenGL/include/glh
#win32:LIBS += c:/Progra~1/NVIDIA~1/NVSDK/OpenGL/lib/debug/glut32.lib
#unix:INCLUDEPATH +=/usr/local/nvsdk/OpenGL/include/glh
#unix:DEFINES += UNIX
#unix:LIBS += -L/usr/local/nvsdk/OpenGL/lib/ -lnv_memory

#
# End of options
#

SOURCES	+= configobject.cpp fakemonitor.cpp controlengine.cpp controlenginequeue.cpp controleventengine.cpp controleventmidi.cpp controllogpotmeter.cpp controlobject.cpp controlnull.cpp controlpotmeter.cpp controlpushbutton.cpp controlrotary.cpp controlttrotary.cpp controlbeat.cpp dlgchannel.cpp dlgplaycontrol.cpp dlgplaylist.cpp dlgmaster.cpp dlgcrossfader.cpp dlgsplit.cpp dlgpreferences.cpp dlgflanger.cpp enginebuffer.cpp enginebufferscale.cpp enginebufferscalelinear.cpp engineclipping.cpp enginefilterblock.cpp enginefilteriir.cpp engineobject.cpp enginepregain.cpp enginevolume.cpp main.cpp midiobject.cpp midiobjectnull.cpp mixxx.cpp mixxxdoc.cpp mixxxview.cpp player.cpp soundsource.cpp soundsourcemp3.cpp monitor.cpp enginechannel.cpp enginemaster.cpp wknob.cpp wbulb.cpp wplaybutton.cpp wpushbutton.cpp wwheel.cpp wslider.cpp wpflbutton.cpp wplayposslider.cpp enginedelay.cpp engineflanger.cpp enginespectralfwd.cpp enginespectralback.cpp mathstuff.cpp readerextract.cpp readerextractwave.cpp readerextractfft.cpp readerextracthfc.cpp readerextractbeat.cpp readerevent.cpp rtthread.cpp windowkaiser.cpp probabilityvector.cpp reader.cpp tracklist.cpp trackinfoobject.cpp dlgtracklist.cpp
HEADERS	+= configobject.h fakemonitor.h controlengine.h controlenginequeue.h controleventengine.h controleventmidi.h controllogpotmeter.h controlobject.h controlnull.h controlpotmeter.h controlpushbutton.h controlrotary.h controlttrotary.h controlbeat.h defs.h dlgchannel.h dlgplaycontrol.h dlgplaylist.h dlgmaster.h dlgcrossfader.h dlgsplit.h dlgpreferences.h dlgflanger.h enginebuffer.h enginebufferscale.h enginebufferscalelinear.h engineclipping.h enginefilterblock.h enginefilteriir.h engineobject.h enginepregain.h enginevolume.h midiobject.h midiobjectnull.h mixxx.h mixxxdoc.h mixxxview.h player.h soundsource.h soundsourcemp3.h monitor.h enginechannel.h enginemaster.h wknob.h wbulb.h wplaybutton.h wpushbutton.h wwheel.h wslider.h wpflbutton.h wplayposslider.h enginedelay.h engineflanger.h enginespectralfwd.h enginespectralback.h mathstuff.h readerextract.h readerextractwave.h readerextractfft.h readerextracthfc.h readerextractbeat.h readerevent.h rtthread.h windowkaiser.h probabilityvector.h reader.h  tracklist.h trackinfoobject.h dlgtracklist.h

# libsamplerate
#INCLUDEPATH += ../lib/libsamplerate
#SOURCES += enginebufferscalesrc.cpp ../lib/libsamplerate/samplerate.c ../lib/libsamplerate/src_linear.c ../lib/libsamplerate/src_sinc.c ../lib/libsamplerate/src_zoh.c
#HEADERS += enginebufferscalesrc.h ../lib/libsamplerate/samplerate.h ../lib/libsamplerate/config.h ../lib/libsamplerate/common.h ../lib/libsamplerate/float_cast.h ../lib/libsamplerate/fastest_coeffs.h ../lib/libsamplerate/high_qual_coeffs.h ../lib/libsamplerate/mid_qual_coeffs.h 

# Debug plotting through gplot API
#unix:DEFINES += __GNUPLOT__
unix:INCLUDEPATH += ../lib/gplot
unix:SOURCES += ../lib/gplot/gplot3.c
unix:HEADERS += ../lib/gplot/gplot.h

unix {
  DEFINES += __UNIX__
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  SOURCES += soundsourceaudiofile.cpp
  HEADERS += soundsourceaudiofile.h
  LIBS += -lmad #/usr/local/lib/libmad.a 
  !macx:LIBS += -laudiofile #/usr/lib/libaudiofile.a
  LIBS += -lsrfftw -lsfftw
  INCLUDEPATH += . 
    
#  Intel Compiler optimization flags
#  QMAKE_CXXFLAGS += -rcd -tpp6 -xiMK # icc pentium III
#  QMAKE_CXXFLAGS += -rcd -tpp7 -xiMKW # icc pentium IV 
#  QMAKE_CXXFLAGS += -prof_use -prof_gen # icc profiling
  !macx:CONFIG_PATH = \"/usr/share/mixxx\"
}

win32 {
  DEFINES += __WIN__
  INCLUDEPATH += ../winlib ../lib/portaudio-v18 .
  SOURCES += soundsourcesndfile.cpp
  HEADERS += soundsourcesndfile.h ../winlib/fftw.h ../winlib/rfftw.h
  LIBS += ../winlib/libmad.lib ../winlib/libsndfile.lib ../winlib/rfftw2st.lib ../winlib/fftw2st.lib
  QMAKE_CXXFLAGS += -GX
  QMAKE_LFLAGS += /NODEFAULTLIB:libcd /NODEFAULTLIB:libcmtd 
  #/NODEFAULTLIB:msvcrt.lib 
  CONFIG_PATH = \"config\"
}

macx {
  DEFINES += __MACX__
  LIBS += /usr/local/lib/libaudiofile.a -lz -framework Carbon -framework QuickTime
  CONFIG_PATH = \"./Contents/Resources/config/\" 
}

# gcc Profiling
#unix:QMAKE_CXXFLAGS_DEBUG += -pg
#unix:QMAKE_LFLAGS_DEBUG += -pg

# icc Profiling
#unix:QMAKE_CXXFLAGS_DEBUG += -qp -g
#unix:QMAKE_LFLAGS_DEBUG += -qp -g

DEFINES += CONFIG_PATH=$$CONFIG_PATH
FORMS	= dlgchanneldlg.ui dlgplaycontroldlg.ui dlgplaylistdlg.ui dlgmasterdlg.ui dlgcrossfaderdlg.ui dlgsplitdlg.ui dlgpreferencesdlg.ui dlgflangerdlg.ui dlgtracklistdlg.ui
IMAGES	= filesave.xpm
unix:TEMPLATE         = app
win32:TEMPLATE       = vcapp
CONFIG	+= qt warn_on thread debug 
DBFILE	= mixxx.db
LANGUAGE	= C++
