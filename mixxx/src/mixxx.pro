#
# Options (comment out what's not wanted)
#

# ALSA
SOURCES += playeralsa.cpp
DEFINES += __ALSA__
LIBS += -lasound

# PortAudio
#SOURCES += playerportaudio.cpp
#DEFINES += __PORTAUDIO__
#LIBS += -lportaudio

# PortMidi
DEFINES += __PORTMIDI__
LIBS += -Llibportmidi.a

#
# End of options
#

SOURCES	+= controllogpotmeter.cpp controlobject.cpp controlpotmeter.cpp controlpushbutton.cpp controlrotary.cpp dlgchannel.cpp dlgplaycontrol.cpp dlgplaylist.cpp enginebuffer.cpp engineclipping.cpp enginefilterlbh.cpp enginefilterrbj.cpp engineiirfilter.cpp engineobject.cpp enginepregain.cpp main.cpp midiobject.cpp mixxx.cpp mixxxdoc.cpp mixxxview.cpp player.cpp qknob.cpp soundsource.cpp soundsourceheavymp3.cpp 
HEADERS	+= controllogpotmeter.h controlobject.h controlpotmeter.h controlpushbutton.h controlrotary.h defs.h dlgchannel.h dlgplaycontrol.h dlgplaylist.h enginebuffer.h engineclipping.h enginefilterlbh.h enginefilterrbj.h engineiirfilter.h engineobject.h enginepregain.h midiobject.h mixxx.h mixxxdoc.h mixxxview.h player.h playeralsa.h playerportaudio.h qknob.h soundsource.h soundsourceheavymp3.h 
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  LIBS	+= -lmad -laudiofile
}
win32 {
  INCLUDEPATH += ../portmidi/
  INCLUDEPATH += ../mad-0.14.2b
  INCLUDEPATH += ../portaudio/pa_common
  INCLUDEPATH += .
  LIBS += winmm.lib ../lib/portmidi.lib ../lib/PAstaticDSD.lib dsound.lib ../lib/porttime.lib ../lib/libmad.lib
}
FORMS	= dlgchanneldlg.ui dlgplaycontroldlg.ui dlgplaylistdlg.ui 
IMAGES	= filesave.xpm 
TEMPLATE = app
CONFIG	+= qt warn_on thread debug
DBFILE	= mixxx.db
LANGUAGE	= C++
