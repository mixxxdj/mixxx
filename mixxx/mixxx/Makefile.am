####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = mixxx
mixxx_SOURCES = engineclipping.cpp enginefilterlbh.cpp enginefilterrbj.cpp soundsourceheavymp3.cpp enginehmlfilter.cpp engineiirfilter.cpp controlrotary.cpp controllogpotmeter.cpp qknob.cpp soundsourcenull.cpp dlgplaylist.cpp dlgplaylistdlg.ui enginepregain.cpp midiobject.cpp playerportaudio.cpp playeralsa.cpp controlpotmeter.cpp player.cpp enginebuffer.cpp controlpushbutton.cpp engineobject.cpp controlobject.cpp soundsource.cpp dlgplaycontrol.cpp dlgmessages.cpp dlgchannel.cpp dlgplaycontroldlg.ui dlgmessagesdlg.ui dlgchanneldlg.ui mixxxview.cpp mixxxdoc.cpp mixxx.cpp main.cpp 
SOURCES = engineclipping.cpp enginefilterlbh.cpp enginefilterrbj.cpp soundsourceheavymp3.cpp enginehmlfilter.cpp engineiirfilter.cpp controlrotary.cpp controllogpotmeter.cpp qknob.cpp soundsourcenull.cpp dlgplaylist.cpp enginepregain.cpp midiobject.cpp playerportaudio.cpp playeralsa.cpp controlpotmeter.cpp player.cpp enginebuffer.cpp controlpushbutton.cpp engineobject.cpp controlobject.cpp soundsource.cpp dlgplaycontrol.cpp dlgmessages.cpp dlgchannel.cpp mixxxview.cpp mixxxdoc.cpp mixxx.cpp main.cpp 
INTERFACES = dlgplaylistdlg.ui dlgplaycontroldlg.ui dlgmessagesdlg.ui dlgchanneldlg.ui 
TRANSLATIONS = 
KDE_OPTIONS = qtonly
mixxx_LDADD   = -laudiofile -lmad -lpthread -lasound -lportaudio $(LIB_QT) $(LIBSOCKET)

SUBDIRS = docs qknob 

EXTRA_DIST = main.cpp mixxx.cpp mixxx.h mixxxdoc.cpp mixxxdoc.h mixxxview.cpp mixxxview.h lo32-app-mixxx.png lo16-app-mixxx.png filenew.xpm filesave.xpm fileopen.xpm dlgchanneldlg.ui dlgmessagesdlg.ui dlgplaycontroldlg.ui dlgchannel.cpp dlgchannel.h dlgmessages.cpp dlgmessages.h dlgplaycontrol.cpp dlgplaycontrol.h soundsource.cpp soundsource.h defs.h controlobject.cpp controlobject.h engineobject.cpp engineobject.h controlpushbutton.cpp controlpushbutton.h enginebuffer.cpp enginebuffer.h player.cpp player.h controlpotmeter.cpp controlpotmeter.h playeralsa.cpp playeralsa.h playerportaudio.cpp playerportaudio.h midiobject.h midiobject.cpp enginepregain.cpp enginepregain.h dlgplaylistdlg.ui dlgplaylist.h dlgplaylist.cpp soundsourcenull.cpp soundsourcenull.h qknob.cpp qknob.h controllogpotmeter.h controllogpotmeter.cpp controlrotary.h controlrotary.cpp engineiirfilter.h engineiirfilter.cpp enginehmlfilter.h enginehmlfilter.cpp knob000.xpm knob090.xpm knob180.xpm knob270.xpm knob045.xpm knob135.xpm knob225.xpm knob315.xpm soundsourceheavymp3.cpp soundsourceheavymp3.h enginefilterrbj.cpp enginefilterrbj.h enginefilterlbh.cpp enginefilterlbh.h engineclipping.h engineclipping.cpp 

####### kdevelop will overwrite this part!!! (end)############


# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

mixxx_METASOURCES = AUTO

# the library search path. 
mixxx_LDFLAGS = $(all_libraries) 

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(mixxx_METASOURCES)

