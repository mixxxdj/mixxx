#
# Options (comment out what's not wanted)
#

# ALSA PCM
#SOURCES += playeralsa.cpp
#HEADERS += playeralsa.h
#DEFINES += __ALSA__
#unix:LIBS += -lasound

# ALSA MIDI
#SOURCES += midiobjectalsa.cpp
#HEADERS += midiobjectalsa.h
#DEFINES  += __ALSAMIDI__

# PortAudio
SOURCES += playerportaudio.cpp
HEADERS += playerportaudio.h
DEFINES += __PORTAUDIO__
unix:LIBS += -lportaudio
win32:LIBS += ../lib/PAstaticDSD.lib dsound.lib

# PortMidi
SOURCES += midiobjectportmidi.cpp
HEADERS += midiobjectportmidi.h
DEFINES += __PORTMIDI__
unix:LIBS += -lportmidi -lporttime
win32:LIBS += ../lib/portmidi.lib ../lib/porttime.lib 

# OSS Midi
#SOURCES += midiobjectoss.cpp
#HEADERS += midiobjectoss.h
#DEFINES += __OSSMIDI__

#
# End of options
#

SOURCES	+= configobject.cpp fakemonitor.cpp controllogpotmeter.cpp controlobject.cpp controlnull.cpp controlpotmeter.cpp controlpushbutton.cpp controlrotary.cpp dlgchannel.cpp dlgplaycontrol.cpp dlgplaylist.cpp dlgmaster.cpp dlgcrossfader.cpp dlgsplit.cpp dlgpreferences.cpp enginebuffer.cpp engineclipping.cpp enginefilterblock.cpp enginefilterrbj.cpp enginefilteriir.cpp engineobject.cpp enginepregain.cpp enginevolume.cpp main.cpp midiobject.cpp mixxx.cpp mixxxdoc.cpp mixxxview.cpp player.cpp soundsource.cpp soundsourcemp3.cpp soundsourceaflibfile.cpp monitor.cpp enginechannel.cpp enginemaster.cpp wknob.cpp wbulb.cpp wplaybutton.cpp wwheel.cpp wslider.cpp wplayposslider.cpp
HEADERS	+= configobject.h fakemonitor.h controllogpotmeter.h controlobject.h controlnull.h controlpotmeter.h controlpushbutton.h controlrotary.h defs.h dlgchannel.h dlgplaycontrol.h dlgplaylist.h dlgmaster.h dlgcrossfader.h dlgsplit.h dlgpreferences.h enginebuffer.h engineclipping.h enginefilterblock.h enginefilterrbj.h enginefilteriir.h engineobject.h enginepregain.h enginevolume.h midiobject.h mixxx.h mixxxdoc.h mixxxview.h player.h soundsource.h soundsourcemp3.h soundsourceaflibfile.h monitor.h enginechannel.h enginemaster.h wknob.h wbulb.h wplaybutton.h wwheel.h wslider.h wplayposslider.h

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  LIBS	+= -lmad -laudiofile -lid3tag
  QMAKE_CXXFLAGS += -O
}

win32 {
  INCLUDEPATH += ../portmidi/
  INCLUDEPATH += ../mad-0.14.2b/libid3tag
  INCLUDEPATH += ../mad-0.14.2b
  INCLUDEPATH += ../portaudio/pa_common
  INCLUDEPATH += .
  LIBS += ../lib/libmad.lib ../lib/libid3tag.lib ../lib/libz.lib
  QMAKE_CXXFLAGS += -GX
  QMAKE_LFLAGS += /NODEFAULTLIB:libcd /NODEFAULTLIB:libcmtd /NODEFAULTLIB:msvcrt.lib
}

# Profiling
#QMAKE_CXXFLAGS_DEBUG += -pg
#QMAKE_LFLAGS_DEBUG += -pg

FORMS	= dlgchanneldlg.ui dlgplaycontroldlg.ui dlgplaylistdlg.ui dlgmasterdlg.ui dlgcrossfaderdlg.ui dlgsplitdlg.ui dlgpreferencesdlg.ui
IMAGES	= filesave.xpm
TEMPLATE	=app
CONFIG	+= qt warn_on thread release
DBFILE	= mixxx.db
LANGUAGE	= C++
