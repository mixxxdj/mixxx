#
# Options (select one audio driver and one midi driver)
#

# PortAudio (Working good)
SOURCES += playerportaudio.cpp
HEADERS += playerportaudio.h
DEFINES += __PORTAUDIO__
# For unix, choose between static or dynamic linking of PortAudio. If you
# want dynamic use only the LIBS line, else use the two first lines.
# PortAudio files needs to be placed in ../lib for the static to work.
#unix:LIBS += -lportaudio
unix:SOURCES += ../lib/pa_lib.c ../lib/pa_convert.c ../lib/pa_unix_oss.c
unix:HEADERS += ../lib/portaudio.h ../lib/pa_host.h
win32:SOURCES += ../lib/pa_lib.c ../lib/dsound_wrapper.c ../lib/pa_dsound.c
win32:HEADERS += ../lib/portaudio.h ../lib/pa_host.h
win32:LIBS += winmm.lib /DXSDK/lib/dsound.lib

# OSS Midi (Working good, Linux specific)
SOURCES += midiobjectoss.cpp
HEADERS += midiobjectoss.h
DEFINES += __OSSMIDI__

# PortMidi (Alpha - Linux ALSA, Windows and MacOS X)
#SOURCES += midiobjectportmidi.cpp
#HEADERS += midiobjectportmidi.h
#DEFINES += __PORTMIDI__
#unix:LIBS += -lportmidi -lporttime
#macx:LIBS -= -lportmidi -lporttime
#macx:LIBS += -framework Carbon -framework CoreMIDI
#macx:SOURCES += ../../../portmidi-macosx-1.0/pmdarwin.c ../../../portmidi-macosx-1.0/pmmacosx.c ../../../portmidi-macosx-1.0/pmutil.c ../../../portmidi-macosx-1.0/portmidi.c ../../../portmidi-macosx-1.0/ptdarwin.c
#macx:HEADERS += ../../../portmidi-macosx-1.0/pminternal.h ../../../portmidi-macosx-1.0/pmmacosx.h ../../../portmidi-macosx-1.0/pmutil.h ../../../portmidi-macosx-1.0/portmidi.h ../../../portmidi-macosx-1.0/porttime.h
#win32:LIBS += ../lib/pm_dll.lib

# CoreMidi (Mac OS X)
#SOURCES += midiobjectcoremidi.cpp
#HEADERS += midiobjectcoremidi.h
#DEFINES += __COREMIDI__
#LIBS    += -framework CoreMIDI -framework CoreFoundation

# ALSA PCM (Not currently working, Linux specific)
#SOURCES += playeralsa.cpp
#HEADERS += playeralsa.h
#DEFINES += __ALSA__
#unix:LIBS += -lasound

# ALSA MIDI (Not currently working, Linux specific)
#SOURCES += midiobjectalsa.cpp
#HEADERS += midiobjectalsa.h
#DEFINES  += __ALSAMIDI__

#
# End of options
#

SOURCES	+= configobject.cpp fakemonitor.cpp controllogpotmeter.cpp controlobject.cpp controlnull.cpp controlpotmeter.cpp controlpushbutton.cpp controlrotary.cpp dlgchannel.cpp dlgplaycontrol.cpp dlgplaylist.cpp dlgmaster.cpp dlgcrossfader.cpp dlgsplit.cpp dlgpreferences.cpp dlgflanger.cpp enginebuffer.cpp engineclipping.cpp enginefilterblock.cpp enginefilteriir.cpp engineobject.cpp enginepregain.cpp enginevolume.cpp main.cpp midiobject.cpp mixxx.cpp mixxxdoc.cpp mixxxview.cpp player.cpp soundsource.cpp soundsourcemp3.cpp monitor.cpp enginechannel.cpp enginemaster.cpp wknob.cpp wbulb.cpp wplaybutton.cpp wwheel.cpp wslider.cpp wplayposslider.cpp enginedelay.cpp engineflanger.cpp
HEADERS	+= configobject.h fakemonitor.h controllogpotmeter.h controlobject.h controlnull.h controlpotmeter.h controlpushbutton.h controlrotary.h defs.h dlgchannel.h dlgplaycontrol.h dlgplaylist.h dlgmaster.h dlgcrossfader.h dlgsplit.h dlgpreferences.h dlgflanger.h enginebuffer.h engineclipping.h enginefilterblock.h enginefilteriir.h engineobject.h enginepregain.h enginevolume.h midiobject.h mixxx.h mixxxdoc.h mixxxview.h player.h soundsource.h soundsourcemp3.h monitor.h enginechannel.h enginemaster.h wknob.h wbulb.h wplaybutton.h wwheel.h wslider.h wplayposslider.h enginedelay.h engineflanger.h

unix {
  DEFINES += __UNIX__
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  SOURCES += soundsourceaudiofile.cpp
  HEADERS += soundsourceaudiofile.h
  LIBS += -lmad -lid3tag -laudiofile
  QMAKE_CXXFLAGS += -dD
  CONFIG_PATH = \"/usr/share/mixxx\"
}

win32 {
  DEFINES += __WIN__
  INCLUDEPATH += ../lib .
  SOURCES += soundsourcesndfile.cpp
  HEADERS += soundsourcesndfile.h
  LIBS += ../lib/libmad.lib ../lib/libsndfile.lib
  QMAKE_CXXFLAGS += -GX
  QMAKE_LFLAGS += /NODEFAULTLIB:libcd /NODEFAULTLIB:libcmtd /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:library
  CONFIG_PATH = \"c:\\mixxx\"
}

macx {
  DEFINES += __MACX__
}

# Profiling
QMAKE_CXXFLAGS_DEBUG += -pg
QMAKE_LFLAGS_DEBUG += -pg

DEFINES += CONFIG_PATH=$$CONFIG_PATH
FORMS	= dlgchanneldlg.ui dlgplaycontroldlg.ui dlgplaylistdlg.ui dlgmasterdlg.ui dlgcrossfaderdlg.ui dlgsplitdlg.ui dlgpreferencesdlg.ui dlgflangerdlg.ui
IMAGES	= filesave.xpm
unix:TEMPLATE         = app
win32:TEMPLATE       = vcapp
CONFIG	+= qt warn_off thread debug 
DBFILE	= mixxx.db
LANGUAGE	= C++
