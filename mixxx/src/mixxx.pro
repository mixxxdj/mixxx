SOURCES	+= controllogpotmeter.cpp controlobject.cpp controlpotmeter.cpp controlpushbutton.cpp controlrotary.cpp dlgchannel.cpp dlgplaycontrol.cpp dlgplaylist.cpp enginebuffer.cpp engineclipping.cpp enginefilterlbh.cpp enginefilterrbj.cpp engineiirfilter.cpp engineobject.cpp enginepregain.cpp main.cpp midiobject.cpp mixxx.cpp mixxxdoc.cpp mixxxview.cpp player.cpp playeralsa.cpp playerportaudio.cpp qknob.cpp soundsource.cpp soundsourceheavymp3.cpp 
HEADERS	+= controllogpotmeter.h controlobject.h controlpotmeter.h controlpushbutton.h controlrotary.h defs.h dlgchannel.h dlgplaycontrol.h dlgplaylist.h enginebuffer.h engineclipping.h enginefilterlbh.h enginefilterrbj.h engineiirfilter.h engineobject.h enginepregain.h midiobject.h mixxx.h mixxxdoc.h mixxxview.h player.h playeralsa.h playerportaudio.h qknob.h soundsource.h soundsourceheavymp3.h 
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= dlgchanneldlg.ui dlgplaycontroldlg.ui dlgplaylistdlg.ui 
IMAGES	= filesave.xpm 
TEMPLATE	=app
CONFIG	+= qt warn_on thread debug
LIBS	+= -lportaudio -lmad -lasound -laudiofile
DBFILE	= mixxx.db
LANGUAGE	= C++
