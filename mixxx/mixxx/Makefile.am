####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = mixxx
mixxx_SOURCES = soundsourcenull.cpp dlgplaylist.cpp dlgplaylistdlg.ui enginepregain.cpp engineIIRfilter.cpp midiobject.cpp playerportaudio.cpp playeralsa.cpp controlpotmeter.cpp player.cpp enginebuffer.cpp controlpushbutton.cpp engineobject.cpp controlobject.cpp soundsource.cpp dlgplaycontrol.cpp dlgmessages.cpp dlgchannel.cpp dlgplaycontroldlg.ui dlgmessagesdlg.ui dlgchanneldlg.ui mixxxview.cpp mixxxdoc.cpp mixxx.cpp main.cpp 
SOURCES = soundsourcenull.cpp dlgplaylist.cpp enginepregain.cpp engineIIRfilter.cpp midiobject.cpp playerportaudio.cpp playeralsa.cpp controlpotmeter.cpp player.cpp enginebuffer.cpp controlpushbutton.cpp engineobject.cpp controlobject.cpp soundsource.cpp dlgplaycontrol.cpp dlgmessages.cpp dlgchannel.cpp mixxxview.cpp mixxxdoc.cpp mixxx.cpp main.cpp 
INTERFACES = dlgplaylistdlg.ui dlgplaycontroldlg.ui dlgmessagesdlg.ui dlgchanneldlg.ui 
TRANSLATIONS = 
KDE_OPTIONS = qtonly
mixxx_LDADD   = -laudiofile -lmad -lpthread -lasound -lportaudio $(LIB_QT) $(LIBSOCKET)

SUBDIRS = docs 

EXTRA_DIST = main.cpp mixxx.cpp mixxx.h mixxxdoc.cpp mixxxdoc.h mixxxview.cpp mixxxview.h lo32-app-mixxx.png lo16-app-mixxx.png filenew.xpm filesave.xpm fileopen.xpm dlgchanneldlg.ui dlgmessagesdlg.ui dlgplaycontroldlg.ui dlgchannel.cpp dlgchannel.h dlgmessages.cpp dlgmessages.h dlgplaycontrol.cpp dlgplaycontrol.h soundsource.cpp soundsource.h defs.h controlobject.cpp controlobject.h engineobject.cpp engineobject.h controlpushbutton.cpp controlpushbutton.h enginebuffer.cpp enginebuffer.h player.cpp player.h controlpotmeter.cpp controlpotmeter.h playeralsa.cpp playeralsa.h playerportaudio.cpp playerportaudio.h midiobject.h midiobject.cpp engineIIRfilter.cpp engineIIRfilter.h enginepregain.cpp enginepregain.h dlgplaylistdlg.ui dlgplaylist.h dlgplaylist.cpp soundsourcenull.cpp soundsourcenull.h 

####### kdevelop will overwrite this part!!! (end)############


# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

mixxx_METASOURCES = AUTO

# the library search path. 
mixxx_LDFLAGS = $(all_libraries) 

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(mixxx_METASOURCES)

