CONFIG += debug link_pkgconfig portmidi script vinylcontrol m4a
# ladspa
DEFINES += QMAKE \ # define QMAKE for not-SCons specific ifdefs like ui_scriptstudio.h
    __PORTAUDIO__ \
    __SNDFILE__ \
    SETTINGS_FILE=\\\"mixxx.cfg\\\" \
    BPMSCHEME_FILE=\\\"mixxxbpmscheme.xml\\\" \
    TRACK_FILE=\\\"mixxxtrack.xml\\\"

## Cross compile:
# Project MESSAGE: MAKEFILE_GENERATOR is [[ MINGW ]]
# DIRLIST_SEPARATOR is [[ : ]]
# DIR_SEPARATOR is [[ / ]]

## Native Windows MinGW
# Project MESSAGE: MAKEFILE_GENERATOR is [[ MINGW ]]
# DIRLIST_SEPARATOR is [[ ; ]]
# DIR_SEPARATOR is [[ \ ]]

# Linux / BSD?
# Project MESSAGE: MAKEFILE_GENERATOR is [[ UNIX ]]

DESTDIR=unknown_build
contains(MAKEFILE_GENERATOR, MINGW) {
  DIRLIST_SEPARATOR(:) {
    DESTDIR=xmingw-win32_build
  } else {
    DESTDIR=mingw-win32_build
  }
}
contains(MAKEFILE_GENERATOR, UNIX) {
    DESTDIR=$$system(uname)_build
}

# Attempt to move the build/qtcreator
# BASE_DIR=../..
# OUT_PWD=$$BASE_DIR
# The above works to load inside QtCreator, but fails to compile when make runs because files are
# trying to reference each other in ../.. rather then in the compile CWD
# Since I don't know how to fix this problem, the work around is to copy mixxx.pro back
# to Mixxx/mixxx checkout directory and load into from there using BASE_DIR=.
OUTDIRCHECK = $$basename(OUT_PWD)
contains(OUTDIRCHECK,qtcreator) {
  error("Copy mixxx.pro from Mixxx/mixxx/build/qtcreator/mixxx.pro to Mixxx/mixxx/mixxx.pro (so that /src is a subdirectory) then reopen from there.")
}

BASE_DIR=.

DESTDIR=$$BASE_DIR/$$DESTDIR

message(BASE_DIR is [[ $$BASE_DIR ]])
message(PWD is [[ $$PWD ]])
message(IN_PWD is [[ $$IN_PWD ]])
message(OUT_PWD is [[ $$OUT_PWD ]])


BUILDDIR = $$DESTDIR
UI_DIR = $$BUILDDIR/ui
RCC_DIR = $$BUILDDIR/rcc
MOC_DIR = $$BUILDDIR/moc
OBJECTS_DIR = $$BUILDDIR/obj

win32-g++ { # Bit ugly, but you can thank MS-DOS shell for f-ing up the normal way of parsing.
    QMAKE_CXXFLAGS += "\"-DSETTINGS_PATH=\\\"Local\\ Settings/Application\\ Data/Mixxx/\\\"\""
    DEFINES += __WINDOWS__
} else {
  win32 { # i586-mingw32msvc-g++ -- cross compiling
    DEFINES += "SETTINGS_PATH=\\\"Local\ Settings/Application\ Data/Mixxx/\\\""
    DEFINES += __WINDOWS__
  } else {
    DEFINES += SETTINGS_PATH=\\\".mixxx/\\\"
  }
}

TEMPLATE = app
TARGET = mixxx
QT += core \
    gui \
    sql \
    xmlpatterns \
    xml \
    network \
    svg \
    opengl \
    script \
    qt3support

CONFIG(debug) { # gdbmacros is required for inspecting Qt datatypes using gdb within QtC
    exists($$(QTDIR)/../share/qtcreator/gdbmacros/gdbmacros.cpp) {
        message(found gdbmacros.cpp relative to QTDIR)
        SOURCES += $$(QTDIR)/../share/qtcreator/gdbmacros/gdbmacros.cpp
    } else {
        exists($$(HOME)/qtcreator-1.3.0/share/qtcreator/gdbmacros/gdbmacros.cpp) {
            message(found gdbmacros.cpp relative to $$(HOME)/qtcreator-1.3.0)
            SOURCES += $$(HOME)/qtcreator-1.3.0/share/qtcreator/gdbmacros/gdbmacros.cpp
        }
    }
}

HEADERS += $$UI_DIR/ui_dlgaboutdlg.h \
    $$UI_DIR/ui_dlgmidilearning.h \
    $$UI_DIR/ui_dlgprefcontrolsdlg.h \
    $$UI_DIR/ui_dlgprefcrossfaderdlg.h \
    $$UI_DIR/ui_dlgprefeqdlg.h \
    $$UI_DIR/ui_dlgpreferencesdlg.h \
    $$UI_DIR/ui_dlgprefmidibindingsdlg.h \
    $$UI_DIR/ui_dlgprefplaylistdlg.h \
    $$UI_DIR/ui_dlgprefrecorddlg.h \
    $$UI_DIR/ui_dlgprefsounddlg.h \
    $$UI_DIR/ui_dlgprefvinyldlg.h \
    $$UI_DIR/ui_dlgprefnovinyldlg.h \
    $$UI_DIR/ui_dlgprefnomididlg.h

INCLUDEPATH += src \
    lib/replaygain \
    lib/ladspa \
    /sw/include \
    $$UI_DIR

## Generate lists of all code/form files present in a source directory for inclusion in a .pro file
# EXCLUDE_SHOUTCAST=shoutcast|encodermp3|encodervorbis
# EXCLUDE_STUDIO=/script/
# EXCLUDE_FFMPEG=ffmpeg
# EXCLUDE_TONAL=tonal
# EXCLUDE_LADSPA=ladspa
# FIND_EXCLUDE=~|unused|patch|diff|/test/|/lib/|.scon|${EXCLUDE_SHOUTCAST}|${EXCLUDE_STUDIO}|${EXCLUDE_FFMPEG}|${EXCLUDE_TONAL}|${EXCLUDE_LADSPA}
# for FILE_EXT in h cpp ui; do
#   find . | egrep -ive "${FIND_EXCLUDE}" | grep \\.${FILE_EXT} | sort -ui | sed -e "s=./=\$\$BASE_DIR/=" -e "s=\.${FILE_EXT}=\.${FILE_EXT} \\\="
# done

HEADERS += \
$$BASE_DIR/src/analyser.h \
$$BASE_DIR/src/analyserqueue.h \
$$BASE_DIR/src/analyserwaveform.h \
$$BASE_DIR/src/analyserwavesummary.h \
$$BASE_DIR/src/bpm/bpmreceiver.h \
$$BASE_DIR/src/bpm/bpmscheme.h \
$$BASE_DIR/src/bpm/wavesegmentation.h \
$$BASE_DIR/src/build.h \
$$BASE_DIR/src/cachingreader.h \
$$BASE_DIR/src/configobject.h \
$$BASE_DIR/src/controlbeat.h \
$$BASE_DIR/src/controlevent.h \
$$BASE_DIR/src/controlgroupdelegate.h \
$$BASE_DIR/src/controllogpotmeter.h \
$$BASE_DIR/src/controlnull.h \
$$BASE_DIR/src/controlobject.h \
$$BASE_DIR/src/controlobjectthread.h \
$$BASE_DIR/src/controlobjectthreadmain.h \
$$BASE_DIR/src/controlobjectthreadwidget.h \
$$BASE_DIR/src/controlpotmeter.h \
$$BASE_DIR/src/controlpushbutton.h \
$$BASE_DIR/src/controlttrotary.h \
$$BASE_DIR/src/controlvaluedelegate.h \
$$BASE_DIR/src/defs_audiofiles.h \
$$BASE_DIR/src/defs.h \
$$BASE_DIR/src/defs_promo.h \
$$BASE_DIR/src/defs_urls.h \
$$BASE_DIR/src/defs_version.h \
$$BASE_DIR/src/dlgabout.h \
$$BASE_DIR/src/dlgautodj.h \
$$BASE_DIR/src/dlgladspa.h \
$$BASE_DIR/src/dlgmidilearning.h \
$$BASE_DIR/src/dlgprefcontrols.h \
$$BASE_DIR/src/dlgprefcrossfader.h \
$$BASE_DIR/src/dlgprefeq.h \
$$BASE_DIR/src/dlgpreferences.h \
$$BASE_DIR/src/dlgprefmidibindings.h \
$$BASE_DIR/src/dlgprefnomidi.h \
$$BASE_DIR/src/dlgprefplaylist.h \
$$BASE_DIR/src/dlgprefrecord.h \
$$BASE_DIR/src/dlgprefsound.h \
$$BASE_DIR/src/dlgprefvinyl.h \
$$BASE_DIR/src/dlgprepare.h \
$$BASE_DIR/src/dlgtrackinfo.h \
$$BASE_DIR/src/encoder.h \
$$BASE_DIR/src/engine/bpmcontrol.h \
$$BASE_DIR/src/engine/cuecontrol.h \
$$BASE_DIR/src/engine/engineabstractrecord.h \
$$BASE_DIR/src/engine/enginebuffer.h \
$$BASE_DIR/src/engine/enginebufferscaledummy.h \
$$BASE_DIR/src/engine/enginebufferscale.h \
$$BASE_DIR/src/engine/enginebufferscalelinear.h \
$$BASE_DIR/src/engine/enginebufferscalereal.h \
$$BASE_DIR/src/engine/enginebufferscalest.h \
$$BASE_DIR/src/engine/enginechannel.h \
$$BASE_DIR/src/engine/engineclipping.h \
$$BASE_DIR/src/engine/enginecontrol.h \
$$BASE_DIR/src/engine/enginedelay.h \
$$BASE_DIR/src/engine/enginefilterblock.h \
$$BASE_DIR/src/engine/enginefilterbutterworth8.h \
$$BASE_DIR/src/engine/enginefilter.h \
$$BASE_DIR/src/engine/enginefilteriir.h \
$$BASE_DIR/src/engine/engineflanger.h \
$$BASE_DIR/src/engine/engineladspa.h \
$$BASE_DIR/src/engine/enginemaster.h \
$$BASE_DIR/src/engine/engineobject.h \
$$BASE_DIR/src/engine/enginepregain.h \
$$BASE_DIR/src/engine/enginesidechain.h \
$$BASE_DIR/src/engine/enginespectralfwd.h \
$$BASE_DIR/src/engine/enginevinylcontrol.h \
$$BASE_DIR/src/engine/enginevinylsoundemu.h \
$$BASE_DIR/src/engine/enginevolume.h \
$$BASE_DIR/src/engine/enginevumeter.h \
$$BASE_DIR/src/engine/enginexfader.h \
$$BASE_DIR/src/engine/loopingcontrol.h \
$$BASE_DIR/src/engine/ratecontrol.h \
$$BASE_DIR/src/engine/readaheadmanager.h \
$$BASE_DIR/src/errordialog.h \
$$BASE_DIR/src/imgcolor.h \
$$BASE_DIR/src/imginvert.h \
$$BASE_DIR/src/imgloader.h \
$$BASE_DIR/src/imgsource.h \
$$BASE_DIR/src/input.h \
$$BASE_DIR/src/ladspa/ladspacontrol.h \
$$BASE_DIR/src/ladspa/ladspainstance.h \
$$BASE_DIR/src/ladspa/ladspainstancemono.h \
$$BASE_DIR/src/ladspa/ladspainstancestereo.h \
$$BASE_DIR/src/ladspa/ladspalibrary.h \
$$BASE_DIR/src/ladspa/ladspaloader.h \
$$BASE_DIR/src/ladspa/ladspaplugin.h \
$$BASE_DIR/src/ladspa/ladspapreset.h \
$$BASE_DIR/src/ladspa/ladspapresetinstance.h \
$$BASE_DIR/src/ladspa/ladspapresetknob.h \
$$BASE_DIR/src/ladspa/ladspapresetmanager.h \
$$BASE_DIR/src/ladspa/ladspapresetslot.h \
$$BASE_DIR/src/ladspaview.h \
$$BASE_DIR/src/library/abstractxmltrackmodel.h \
$$BASE_DIR/src/library/autodjfeature.h \
$$BASE_DIR/src/library/browsefeature.h \
$$BASE_DIR/src/library/browsefilter.h \
$$BASE_DIR/src/library/browsetablemodel.h \
$$BASE_DIR/src/library/cratefeature.h \
$$BASE_DIR/src/library/cratetablemodel.h \
$$BASE_DIR/src/library/dao/cratedao.h \
$$BASE_DIR/src/library/dao/cuedao.h \
$$BASE_DIR/src/library/dao/cue.h \
$$BASE_DIR/src/library/dao/dao.h \
$$BASE_DIR/src/library/dao/libraryhashdao.h \
$$BASE_DIR/src/library/dao/playlistdao.h \
$$BASE_DIR/src/library/dao/settingsdao.h \
$$BASE_DIR/src/library/dao/trackdao.h \
$$BASE_DIR/src/library/itunesfeature.h \
$$BASE_DIR/src/library/itunesplaylistmodel.h \
$$BASE_DIR/src/library/itunestrackmodel.h \
$$BASE_DIR/src/library/legacylibraryimporter.h \
$$BASE_DIR/src/library/libraryfeature.h \
$$BASE_DIR/src/library/library.h \
$$BASE_DIR/src/library/librarymidicontrol.h \
$$BASE_DIR/src/library/libraryscannerdlg.h \
$$BASE_DIR/src/library/libraryscanner.h \
$$BASE_DIR/src/library/librarytablemodel.h \
$$BASE_DIR/src/library/libraryview.h \
$$BASE_DIR/src/library/missingtablemodel.h \
$$BASE_DIR/src/library/mixxxlibraryfeature.h \
$$BASE_DIR/src/library/playlistfeature.h \
$$BASE_DIR/src/library/playlisttablemodel.h \
$$BASE_DIR/src/library/preparecratedelegate.h \
$$BASE_DIR/src/library/preparefeature.h \
$$BASE_DIR/src/library/preparelibrarytablemodel.h \
$$BASE_DIR/src/library/proxytrackmodel.h \
$$BASE_DIR/src/library/rhythmboxfeature.h \
$$BASE_DIR/src/library/rhythmboxplaylistmodel.h \
$$BASE_DIR/src/library/rhythmboxtrackmodel.h \
$$BASE_DIR/src/library/schemamanager.h \
$$BASE_DIR/src/library/searchthread.h \
$$BASE_DIR/src/library/sidebarmodel.h \
$$BASE_DIR/src/library/trackcollection.h \
$$BASE_DIR/src/library/trackmodel.h \
$$BASE_DIR/src/m4a/comment.h \
$$BASE_DIR/src/m4a/ip.h \
$$BASE_DIR/src/m4a/sf.h \
$$BASE_DIR/src/mathstuff.h \
$$BASE_DIR/src/midi/midichanneldelegate.h \
$$BASE_DIR/src/midi/mididevicedummy.h \
$$BASE_DIR/src/midi/mididevice.h \
$$BASE_DIR/src/midi/mididevicemanager.h \
$$BASE_DIR/src/midi/midideviceportmidi.h \
$$BASE_DIR/src/midi/midiinputmapping.h \
$$BASE_DIR/src/midi/midiinputmappingtablemodel.h \
$$BASE_DIR/src/midi/midiledhandler.h \
$$BASE_DIR/src/midi/midimapping.h \
$$BASE_DIR/src/midi/midimessage.h \
$$BASE_DIR/src/midi/midinodelegate.h \
$$BASE_DIR/src/midi/midioptiondelegate.h \
$$BASE_DIR/src/midi/midioutputmapping.h \
$$BASE_DIR/src/midi/midioutputmappingtablemodel.h \
$$BASE_DIR/src/midi/midiscriptengine.h \
$$BASE_DIR/src/midi/midistatusdelegate.h \
$$BASE_DIR/src/mixxxcontrol.h \
$$BASE_DIR/src/mixxxevent.h \
$$BASE_DIR/src/mixxx.h \
$$BASE_DIR/src/mixxxkeyboard.h \
$$BASE_DIR/src/mixxxview.h \
$$BASE_DIR/src/parser.h \
$$BASE_DIR/src/parserm3u.h \
$$BASE_DIR/src/parserpls.h \
$$BASE_DIR/src/peaklist.h \
$$BASE_DIR/src/player.h \
$$BASE_DIR/src/playerinfo.h \
$$BASE_DIR/src/probabilityvector.h \
$$BASE_DIR/src/recording/defs_recording.h \
$$BASE_DIR/src/recording/enginerecord.h \
$$BASE_DIR/src/recording/writeaudiofile.h \
$$BASE_DIR/src/rotary.h \
$$BASE_DIR/src/rtthread.h \
$$BASE_DIR/src/segmentation.h \
$$BASE_DIR/src/sounddevice.h \
$$BASE_DIR/src/sounddeviceportaudio.h \
$$BASE_DIR/src/soundmanager.h \
$$BASE_DIR/src/soundsource.h \
$$BASE_DIR/src/soundsourcem4a.h \
$$BASE_DIR/src/soundsourcemp3.h \
$$BASE_DIR/src/soundsourceoggvorbis.h \
$$BASE_DIR/src/soundsourceproxy.h \
$$BASE_DIR/src/soundsourcesndfile.h \
$$BASE_DIR/src/trackinfoobject.h \
$$BASE_DIR/src/transposeproxymodel.h \
$$BASE_DIR/src/upgrade.h \
$$BASE_DIR/src/vinylcontrol.h \
$$BASE_DIR/src/vinylcontrolproxy.h \
$$BASE_DIR/src/vinylcontrolscratchlib.h \
$$BASE_DIR/src/vinylcontrolsignalwidget.h \
$$BASE_DIR/src/vinylcontrolxwax.h \
$$BASE_DIR/src/waveform/glwaveformrenderer.h \
$$BASE_DIR/src/waveform/renderobject.h \
$$BASE_DIR/src/waveformviewerfactory.h \
$$BASE_DIR/src/waveform/waveformrenderbackground.h \
$$BASE_DIR/src/waveform/waveformrenderbeat.h \
$$BASE_DIR/src/waveform/waveformrenderer.h \
$$BASE_DIR/src/waveform/waveformrendermark.h \
$$BASE_DIR/src/waveform/waveformrendermarkrange.h \
$$BASE_DIR/src/waveform/waveformrendersignal.h \
$$BASE_DIR/src/waveform/waveformrendersignalpixmap.h \
$$BASE_DIR/src/widget/hexspinbox.h \
$$BASE_DIR/src/widget/wabstractcontrol.h \
$$BASE_DIR/src/widget/wbrowsetableview.h \
$$BASE_DIR/src/widget/wdisplay.h \
$$BASE_DIR/src/widget/wglwaveformviewer.h \
$$BASE_DIR/src/widget/wknob.h \
$$BASE_DIR/src/widget/wlabel.h \
$$BASE_DIR/src/widget/wlibrary.h \
$$BASE_DIR/src/widget/wlibrarysidebar.h \
$$BASE_DIR/src/widget/wlibrarytableview.h \
$$BASE_DIR/src/widget/wlibrarytextbrowser.h \
$$BASE_DIR/src/widget/wnumberbpm.h \
$$BASE_DIR/src/widget/wnumber.h \
$$BASE_DIR/src/widget/wnumberpos.h \
$$BASE_DIR/src/widget/wnumberrate.h \
$$BASE_DIR/src/widget/woverview.h \
$$BASE_DIR/src/widget/wpixmapstore.h \
$$BASE_DIR/src/widget/wpreparecratestableview.h \
$$BASE_DIR/src/widget/wpreparelibrarytableview.h \
$$BASE_DIR/src/widget/wpushbutton.h \
$$BASE_DIR/src/widget/wsearchlineedit.h \
$$BASE_DIR/src/widget/wskincolor.h \
$$BASE_DIR/src/widget/wslidercomposed.h \
$$BASE_DIR/src/widget/wslider.h \
$$BASE_DIR/src/widget/wstatuslight.h \
$$BASE_DIR/src/widget/wtracktableviewheader.h \
$$BASE_DIR/src/widget/wvisualsimple.h \
$$BASE_DIR/src/widget/wvumeter.h \
$$BASE_DIR/src/widget/wwaveformviewer.h \
$$BASE_DIR/src/widget/wwidget.h \
$$BASE_DIR/src/windowkaiser.h \
$$BASE_DIR/src/wtracktableview.h \
$$BASE_DIR/src/xmlparse.h


SOURCES += \
$$BASE_DIR/src/analyserqueue.cpp \
$$BASE_DIR/src/analyserwaveform.cpp \
$$BASE_DIR/src/analyserwavesummary.cpp \
$$BASE_DIR/src/bpm/bpmscheme.cpp \
$$BASE_DIR/src/bpm/wavesegmentation.cpp \
$$BASE_DIR/src/cachingreader.cpp \
$$BASE_DIR/src/configobject.cpp \
$$BASE_DIR/src/controlbeat.cpp \
$$BASE_DIR/src/controlevent.cpp \
$$BASE_DIR/src/controlgroupdelegate.cpp \
$$BASE_DIR/src/controllogpotmeter.cpp \
$$BASE_DIR/src/controlnull.cpp \
$$BASE_DIR/src/controlobject.cpp \
$$BASE_DIR/src/controlobjectthread.cpp \
$$BASE_DIR/src/controlobjectthreadmain.cpp \
$$BASE_DIR/src/controlobjectthreadwidget.cpp \
$$BASE_DIR/src/controlpotmeter.cpp \
$$BASE_DIR/src/controlpushbutton.cpp \
$$BASE_DIR/src/controlttrotary.cpp \
$$BASE_DIR/src/controlvaluedelegate.cpp \
$$BASE_DIR/src/dlgabout.cpp \
$$BASE_DIR/src/dlgautodj.cpp \
$$BASE_DIR/src/dlgladspa.cpp \
$$BASE_DIR/src/dlgmidilearning.cpp \
$$BASE_DIR/src/dlgprefcontrols.cpp \
$$BASE_DIR/src/dlgprefcrossfader.cpp \
$$BASE_DIR/src/dlgprefeq.cpp \
$$BASE_DIR/src/dlgpreferences.cpp \
$$BASE_DIR/src/dlgprefmidibindings.cpp \
$$BASE_DIR/src/dlgprefnomidi.cpp \
$$BASE_DIR/src/dlgprefplaylist.cpp \
$$BASE_DIR/src/dlgprefrecord.cpp \
$$BASE_DIR/src/dlgprefsound.cpp \
$$BASE_DIR/src/dlgprefvinyl.cpp \
$$BASE_DIR/src/dlgprepare.cpp \
$$BASE_DIR/src/dlgtrackinfo.cpp \
$$BASE_DIR/src/encoder.cpp \
$$BASE_DIR/src/engine/bpmcontrol.cpp \
$$BASE_DIR/src/engine/cuecontrol.cpp \
$$BASE_DIR/src/engine/enginebuffer.cpp \
$$BASE_DIR/src/engine/enginebufferscale.cpp \
$$BASE_DIR/src/engine/enginebufferscaledummy.cpp \
$$BASE_DIR/src/engine/enginebufferscalelinear.cpp \
$$BASE_DIR/src/engine/enginebufferscalereal.cpp \
$$BASE_DIR/src/engine/enginebufferscalest.cpp \
$$BASE_DIR/src/engine/enginechannel.cpp \
$$BASE_DIR/src/engine/engineclipping.cpp \
$$BASE_DIR/src/engine/enginecontrol.cpp \
$$BASE_DIR/src/engine/enginedelay.cpp \
$$BASE_DIR/src/engine/enginefilterblock.cpp \
$$BASE_DIR/src/engine/enginefilterbutterworth8.cpp \
$$BASE_DIR/src/engine/enginefilter.cpp \
$$BASE_DIR/src/engine/enginefilteriir.cpp \
$$BASE_DIR/src/engine/engineflanger.cpp \
$$BASE_DIR/src/engine/engineladspa.cpp \
$$BASE_DIR/src/engine/enginemaster.cpp \
$$BASE_DIR/src/engine/engineobject.cpp \
$$BASE_DIR/src/engine/enginepregain.cpp \
$$BASE_DIR/src/engine/enginesidechain.cpp \
$$BASE_DIR/src/engine/enginespectralfwd.cpp \
$$BASE_DIR/src/engine/enginevinylcontrol.cpp \
$$BASE_DIR/src/engine/enginevinylsoundemu.cpp \
$$BASE_DIR/src/engine/enginevolume.cpp \
$$BASE_DIR/src/engine/enginevumeter.cpp \
$$BASE_DIR/src/engine/enginexfader.cpp \
$$BASE_DIR/src/engine/loopingcontrol.cpp \
$$BASE_DIR/src/engine/ratecontrol.cpp \
$$BASE_DIR/src/engine/readaheadmanager.cpp \
$$BASE_DIR/src/errordialog.cpp \
$$BASE_DIR/src/imgcolor.cpp \
$$BASE_DIR/src/imginvert.cpp \
$$BASE_DIR/src/imgloader.cpp \
$$BASE_DIR/src/input.cpp \
$$BASE_DIR/src/ladspa/ladspacontrol.cpp \
$$BASE_DIR/src/ladspa/ladspainstance.cpp \
$$BASE_DIR/src/ladspa/ladspainstancemono.cpp \
$$BASE_DIR/src/ladspa/ladspainstancestereo.cpp \
$$BASE_DIR/src/ladspa/ladspalibrary.cpp \
$$BASE_DIR/src/ladspa/ladspaloader.cpp \
$$BASE_DIR/src/ladspa/ladspaplugin.cpp \
$$BASE_DIR/src/ladspa/ladspapreset.cpp \
$$BASE_DIR/src/ladspa/ladspapresetinstance.cpp \
$$BASE_DIR/src/ladspa/ladspapresetknob.cpp \
$$BASE_DIR/src/ladspa/ladspapresetmanager.cpp \
$$BASE_DIR/src/ladspa/ladspapresetslot.cpp \
$$BASE_DIR/src/ladspaview.cpp \
$$BASE_DIR/src/library/abstractxmltrackmodel.cpp \
$$BASE_DIR/src/library/autodjfeature.cpp \
$$BASE_DIR/src/library/browsefeature.cpp \
$$BASE_DIR/src/library/browsefilter.cpp \
$$BASE_DIR/src/library/browsetablemodel.cpp \
$$BASE_DIR/src/library/cratefeature.cpp \
$$BASE_DIR/src/library/cratetablemodel.cpp \
$$BASE_DIR/src/library/dao/cratedao.cpp \
$$BASE_DIR/src/library/dao/cue.cpp \
$$BASE_DIR/src/library/dao/cuedao.cpp \
$$BASE_DIR/src/library/dao/libraryhashdao.cpp \
$$BASE_DIR/src/library/dao/playlistdao.cpp \
$$BASE_DIR/src/library/dao/settingsdao.cpp \
$$BASE_DIR/src/library/dao/trackdao.cpp \
$$BASE_DIR/src/library/itunesfeature.cpp \
$$BASE_DIR/src/library/itunesplaylistmodel.cpp \
$$BASE_DIR/src/library/itunestrackmodel.cpp \
$$BASE_DIR/src/library/legacylibraryimporter.cpp \
$$BASE_DIR/src/library/library.cpp \
$$BASE_DIR/src/library/libraryfeature.cpp \
$$BASE_DIR/src/library/librarymidicontrol.cpp \
$$BASE_DIR/src/library/libraryscanner.cpp \
$$BASE_DIR/src/library/libraryscannerdlg.cpp \
$$BASE_DIR/src/library/librarytablemodel.cpp \
$$BASE_DIR/src/library/missingtablemodel.cpp \
$$BASE_DIR/src/library/mixxxlibraryfeature.cpp \
$$BASE_DIR/src/library/playlistfeature.cpp \
$$BASE_DIR/src/library/playlisttablemodel.cpp \
$$BASE_DIR/src/library/preparecratedelegate.cpp \
$$BASE_DIR/src/library/preparefeature.cpp \
$$BASE_DIR/src/library/preparelibrarytablemodel.cpp \
$$BASE_DIR/src/library/proxytrackmodel.cpp \
$$BASE_DIR/src/library/rhythmboxfeature.cpp \
$$BASE_DIR/src/library/rhythmboxplaylistmodel.cpp \
$$BASE_DIR/src/library/rhythmboxtrackmodel.cpp \
$$BASE_DIR/src/library/schemamanager.cpp \
$$BASE_DIR/src/library/searchthread.cpp \
$$BASE_DIR/src/library/sidebarmodel.cpp \
$$BASE_DIR/src/library/trackcollection.cpp \
$$BASE_DIR/src/m4a/mp4-mixxx.cpp \
$$BASE_DIR/src/main.cpp \
$$BASE_DIR/src/mathstuff.cpp \
$$BASE_DIR/src/midi/midichanneldelegate.cpp \
$$BASE_DIR/src/midi/mididevice.cpp \
$$BASE_DIR/src/midi/mididevicemanager.cpp \
$$BASE_DIR/src/midi/midideviceportmidi.cpp \
$$BASE_DIR/src/midi/midiinputmappingtablemodel.cpp \
$$BASE_DIR/src/midi/midiledhandler.cpp \
$$BASE_DIR/src/midi/midimapping.cpp \
$$BASE_DIR/src/midi/midimessage.cpp \
$$BASE_DIR/src/midi/midinodelegate.cpp \
$$BASE_DIR/src/midi/midioptiondelegate.cpp \
$$BASE_DIR/src/midi/midioutputmappingtablemodel.cpp \
$$BASE_DIR/src/midi/midiscriptengine.cpp \
$$BASE_DIR/src/midi/midistatusdelegate.cpp \
$$BASE_DIR/src/mixxxcontrol.cpp \
$$BASE_DIR/src/mixxx.cpp \
$$BASE_DIR/src/mixxxkeyboard.cpp \
$$BASE_DIR/src/mixxxview.cpp \
$$BASE_DIR/src/parser.cpp \
$$BASE_DIR/src/parserm3u.cpp \
$$BASE_DIR/src/parserpls.cpp \
$$BASE_DIR/src/peaklist.cpp \
$$BASE_DIR/src/player.cpp \
$$BASE_DIR/src/playerinfo.cpp \
$$BASE_DIR/src/probabilityvector.cpp \
$$BASE_DIR/src/recording/enginerecord.cpp \
$$BASE_DIR/src/recording/writeaudiofile.cpp \
$$BASE_DIR/src/rotary.cpp \
$$BASE_DIR/src/rtthread.cpp \
$$BASE_DIR/src/segmentation.cpp \
$$BASE_DIR/src/sounddevice.cpp \
$$BASE_DIR/src/sounddeviceportaudio.cpp \
$$BASE_DIR/src/soundmanager.cpp \
$$BASE_DIR/src/soundsource.cpp \
$$BASE_DIR/src/soundsourcem4a.cpp \
$$BASE_DIR/src/soundsourcemp3.cpp \
$$BASE_DIR/src/soundsourceoggvorbis.cpp \
$$BASE_DIR/src/soundsourceproxy.cpp \
$$BASE_DIR/src/soundsourcesndfile.cpp \
$$BASE_DIR/src/trackinfoobject.cpp \
$$BASE_DIR/src/upgrade.cpp \
$$BASE_DIR/src/vinylcontrol.cpp \
$$BASE_DIR/src/vinylcontrolproxy.cpp \
$$BASE_DIR/src/vinylcontrolscratchlib.cpp \
$$BASE_DIR/src/vinylcontrolsignalwidget.cpp \
$$BASE_DIR/src/vinylcontrolxwax.cpp \
$$BASE_DIR/src/waveform/glwaveformrenderer.cpp \
$$BASE_DIR/src/waveform/renderobject.cpp \
$$BASE_DIR/src/waveformviewerfactory.cpp \
$$BASE_DIR/src/waveform/waveformrenderbackground.cpp \
$$BASE_DIR/src/waveform/waveformrenderbeat.cpp \
$$BASE_DIR/src/waveform/waveformrenderer.cpp \
$$BASE_DIR/src/waveform/waveformrendermark.cpp \
$$BASE_DIR/src/waveform/waveformrendermarkrange.cpp \
$$BASE_DIR/src/waveform/waveformrendersignal.cpp \
$$BASE_DIR/src/waveform/waveformrendersignalpixmap.cpp \
$$BASE_DIR/src/widget/hexspinbox.cpp \
$$BASE_DIR/src/widget/wabstractcontrol.cpp \
$$BASE_DIR/src/widget/wbrowsetableview.cpp \
$$BASE_DIR/src/widget/wdisplay.cpp \
$$BASE_DIR/src/widget/wglwaveformviewer.cpp \
$$BASE_DIR/src/widget/wknob.cpp \
$$BASE_DIR/src/widget/wlabel.cpp \
$$BASE_DIR/src/widget/wlibrary.cpp \
$$BASE_DIR/src/widget/wlibrarysidebar.cpp \
$$BASE_DIR/src/widget/wlibrarytableview.cpp \
$$BASE_DIR/src/widget/wlibrarytextbrowser.cpp \
$$BASE_DIR/src/widget/wnumberbpm.cpp \
$$BASE_DIR/src/widget/wnumber.cpp \
$$BASE_DIR/src/widget/wnumberpos.cpp \
$$BASE_DIR/src/widget/wnumberrate.cpp \
$$BASE_DIR/src/widget/woverview.cpp \
$$BASE_DIR/src/widget/wpixmapstore.cpp \
$$BASE_DIR/src/widget/wpreparecratestableview.cpp \
$$BASE_DIR/src/widget/wpreparelibrarytableview.cpp \
$$BASE_DIR/src/widget/wpushbutton.cpp \
$$BASE_DIR/src/widget/wsearchlineedit.cpp \
$$BASE_DIR/src/widget/wskincolor.cpp \
$$BASE_DIR/src/widget/wslidercomposed.cpp \
$$BASE_DIR/src/widget/wslider.cpp \
$$BASE_DIR/src/widget/wstatuslight.cpp \
$$BASE_DIR/src/widget/wtracktableviewheader.cpp \
$$BASE_DIR/src/widget/wvisualsimple.cpp \
$$BASE_DIR/src/widget/wvumeter.cpp \
$$BASE_DIR/src/widget/wwaveformviewer.cpp \
$$BASE_DIR/src/widget/wwidget.cpp \
$$BASE_DIR/src/windowkaiser.cpp \
$$BASE_DIR/src/wtracktableview.cpp \
$$BASE_DIR/src/xmlparse.cpp


# Soundtouch
INCLUDEPATH += $$BASE_DIR/lib/soundtouch-1.4.1
SOURCES += $$BASE_DIR/lib/soundtouch-1.4.1/SoundTouch.cpp \
    $$BASE_DIR/lib/soundtouch-1.4.1/TDStretch.cpp \
    $$BASE_DIR/lib/soundtouch-1.4.1/RateTransposer.cpp \
    $$BASE_DIR/lib/soundtouch-1.4.1/AAFilter.cpp \
    $$BASE_DIR/lib/soundtouch-1.4.1/FIFOSampleBuffer.cpp \
    $$BASE_DIR/lib/soundtouch-1.4.1/FIRFilter.cpp \
    $$BASE_DIR/lib/soundtouch-1.4.1/PeakFinder.cpp \
    $$BASE_DIR/lib/soundtouch-1.4.1/BPMDetect.cpp \
    $$BASE_DIR/lib/soundtouch-1.4.1/cpu_detect_x86_gcc.cpp

# Fidlib
SOURCES += $$BASE_DIR/lib/fidlib-0.9.10/fidlib.c
win32-g++ {
    DEFINES += T_MINGW
}
!win32-g++ {
    DEFINES += T_LINUX
}

# ReplayGain

SOURCES += $$BASE_DIR/lib/replaygain/replaygain_analysis.c

FORMS += \
$$BASE_DIR/src/dlgaboutdlg.ui \
$$BASE_DIR/src/dlgautodj.ui \
$$BASE_DIR/src/dlgmidilearning.ui \
$$BASE_DIR/src/dlgprefcontrolsdlg.ui \
$$BASE_DIR/src/dlgprefcrossfaderdlg.ui \
$$BASE_DIR/src/dlgprefeqdlg.ui \
$$BASE_DIR/src/dlgpreferencesdlg.ui \
$$BASE_DIR/src/dlgprefmidibindingsdlg.ui \
$$BASE_DIR/src/dlgprefnomididlg.ui \
$$BASE_DIR/src/dlgprefplaylistdlg.ui \
$$BASE_DIR/src/dlgprefrecorddlg.ui \
$$BASE_DIR/src/dlgprefsounddlg.ui \
$$BASE_DIR/src/dlgprefvinyldlg.ui \
$$BASE_DIR/src/dlgprepare.ui \
$$BASE_DIR/src/dlgtrackinfo.ui \
$$BASE_DIR/src/script/scriptstudio.ui


RESOURCES += $$BASE_DIR/src/../res/mixxx.qrc

FORMS += $$BASE_DIR/src/dlgprefrecorddlg.ui
!win32:unix {
    !macx {
        DEFINES += __LINUX__ \
            TEMPORAL \
            __UNIX__ \

        # if PREFIX is defined by the user, we use it! ( 19/12/2003, J_Zar)
        isEmpty( PREFIX ) {
            PREFIX = /usr/local
        }
        UNIX_SHARE_PATH = $${PREFIX}/share/mixxx
        DEFINES += UNIX_SHARE_PATH=\\\"$$UNIX_SHARE_PATH\\\"
        CONFIG(portmidi) {
            DEFINES += __PORTMIDI__
        }
        LIBS += -lasound -lportmidi -lporttime
        PKGCONFIG += portaudio-2.0 \
            jack \
            id3tag \
            mad \
            vorbisfile \
            sndfile
    }
}
macx {
    # Needed for portmidi on OSX?  Probably not...
    LIBS += -framework CoreMIDI \
        -framework CoreFoundation \
        -framework CoreAudio \
        -framework AudioToolbox \
        -framework AudioUnit \
        -L/sw/lib \
        -lportaudio \
        -lmad \
        -lsndfile \
        -logg \
        -lvorbis \
        -lvorbisfile \
        -lfftw3 \
        -lid3tag
}
win32 {
    DEFINES += __WINMIDI__
    HEADERS += $$BASE_DIR/src/midiobjectwin.h
    SOURCES += $$BASE_DIR/src/midiobjectwin.cpp
    LIBS += $$BASE_DIR/../mixxx-winlib/libsndfile-1.dll \
        $$BASE_DIR/../mixxx-winlib/portaudio_x86.dll \
        $$BASE_DIR/../mixxx-winlib/libmad.a \ # libmad-0.15.1b
        $$BASE_DIR/../mixxx-winlib/libid3tag.a \ # libid3tag-0.15.1b
        $$BASE_DIR/../mixxx-winlib/libvorbisfile.dll \
        $$BASE_DIR/../mixxx-winlib/libvorbis.dll \
#        $$BASE_DIR/../mixxx-winlib/libfftw3-3.dll \
        $$BASE_DIR/../mixxx-winlib/libogg.dll \
        -lwinmm
    INCLUDEPATH += $$BASE_DIR/../mixxx-winlib
}
CONFIG(ladspa) {
    DEFINES += __LADSPA__
    HEADERS += $$BASE_DIR/src/engine/engineladspa.h \
        $$BASE_DIR/src/dlgladspa.h \
        $$BASE_DIR/src/ladspaview.h \
        $$BASE_DIR/src/ladspa/ladspacontrol.h \
        $$BASE_DIR/src/ladspa/ladspainstance.h \
        $$BASE_DIR/src/ladspa/ladspainstancemono.h \
        $$BASE_DIR/src/ladspa/ladspainstancestereo.h \
        $$BASE_DIR/src/ladspa/ladspalibrary.h \
        $$BASE_DIR/src/ladspa/ladspaloader.h \
        $$BASE_DIR/src/ladspa/ladspaplugin.h \
        $$BASE_DIR/src/ladspa/ladspapreset.h \
        $$BASE_DIR/src/ladspa/ladspapresetinstance.h \
        $$BASE_DIR/src/ladspa/ladspapresetknob.h \
        $$BASE_DIR/src/ladspa/ladspapresetmanager.h \
        $$BASE_DIR/src/ladspa/ladspapresetslot.h
    SOURCES += $$BASE_DIR/src/engine/engineladspa.cpp \
        $$BASE_DIR/src/dlgladspa.cpp \
        $$BASE_DIR/src/ladspaview.cpp \
        $$BASE_DIR/src/ladspa/ladspacontrol.cpp \
        $$BASE_DIR/src/ladspa/ladspainstance.cpp \
        $$BASE_DIR/src/ladspa/ladspainstancemono.cpp \
        $$BASE_DIR/src/ladspa/ladspainstancestereo.cpp \
        $$BASE_DIR/src/ladspa/ladspalibrary.cpp \
        $$BASE_DIR/src/ladspa/ladspaloader.cpp \
        $$BASE_DIR/src/ladspa/ladspaplugin.cpp \
        $$BASE_DIR/src/ladspa/ladspapreset.cpp \
        $$BASE_DIR/src/ladspa/ladspapresetinstance.cpp \
        $$BASE_DIR/src/ladspa/ladspapresetknob.cpp \
        $$BASE_DIR/src/ladspa/ladspapresetmanager.cpp \
        $$BASE_DIR/src/ladspa/ladspapresetslot.cpp
    win32{
        INCLUDEPATH += lib\ladspa
    }
}
CONFIG(script) {
    DEFINES += __MIDISCRIPT__
}

CONFIG(Vamp) {
DEFINES += __VAMP__
INCLUDEPATH += $$BASE_DIR/lib/vamp
HEADERS += $$BASE_DIR/src/vamp/vampanalyser.h \
    $$BASE_DIR/src/analyservamptest.h \
    $$BASE_DIR/src/analyservampkeytest.h \
    $$BASE_DIR/lib/vamp/vamp/vamp.h \
    $$BASE_DIR/lib/vamp/vamp-hostsdk/hostguard.h \
SOURCES += $$BASE_DIR/src/vamp/vampanalyser.cpp \
    $$BASE_DIR/src/analyservamptest.cpp \
    $$BASE_DIR/src/analyservampkeytest.cpp \
    $$BASE_DIR/lib/vamp/src/vamp-hostsdk/PluginBufferingAdapter.cpp \
	$$BASE_DIR/lib/vamp/src/vamp-hostsdk/PluginChannelAdapter.cpp \
	$$BASE_DIR/lib/vamp/src/vamp-hostsdk/PluginHostAdapter.cpp \
	$$BASE_DIR/lib/vamp/src/vamp-hostsdk/PluginInputDomainAdapter.cpp \
	$$BASE_DIR/lib/vamp/src/vamp-hostsdk/PluginLoader.cpp \
	$$BASE_DIR/lib/vamp/src/vamp-hostsdk/PluginSummarisingAdapter.cpp \
	$$BASE_DIR/lib/vamp/src/vamp-hostsdk/PluginWrapper.cpp \
	$$BASE_DIR/lib/vamp/src/vamp-hostsdk/RealTime.cpp \
	$$BASE_DIR/lib/vamp/src/vamp-sdk/PluginAdapter.cpp \
	$$BASE_DIR/lib/vamp/src/vamp-sdk/RealTime.cpp
}

CONFIG(tonal) {
#    DEFINES +=
    HEADERS += $$BASE_DIR/src/tonal/ChordCorrelator.hxx \
        $$BASE_DIR/src/tonal/ChordExtractor.hxx \
        $$BASE_DIR/src/tonal/ChordSegmentator.hxx \
        $$BASE_DIR/src/tonal/CircularPeakPicking.hxx \
        $$BASE_DIR/src/tonal/CircularPeakTunner.hxx \
        $$BASE_DIR/src/tonal/CircularPeaksToPCP.hxx \
        $$BASE_DIR/src/tonal/ConstantQFolder.hxx \
        $$BASE_DIR/src/tonal/ConstantQTransform.hxx \
        $$BASE_DIR/src/tonal/DiscontinuousSegmentation.hxx \
        $$BASE_DIR/src/tonal/FourierTransform.hxx \
        $$BASE_DIR/src/tonal/InstantTunningEstimator.hxx \
        $$BASE_DIR/src/tonal/PCPSmother.hxx \
        $$BASE_DIR/src/tonal/Segmentation.hxx \
        $$BASE_DIR/src/tonal/SemitoneCenterFinder.hxx \
        $$BASE_DIR/src/tonal/TonalAnalysis.hxx \
        $$BASE_DIR/src/tonal/tonalanalyser.h
    SOURCES += $$BASE_DIR/src/tonal/ConstantQFolder.cxx \
        $$BASE_DIR/src/tonal/ConstantQTransform.cxx \
        $$BASE_DIR/src/tonal/FourierTransform.cxx \
        $$BASE_DIR/src/tonal/Segmentation.cxx \
        $$BASE_DIR/src/tonal/TonalAnalysis.cxx \
        $$BASE_DIR/src/tonal/tonalanalyser.cpp
}
CONFIG(m4a) {
    DEFINES += __M4A__
    DEFINES += __MP4V2__ __M4AHACK__
    win32{
        INCLUDEPATH += $$BASE_DIR/../mixxx-winlib/mp4v2/include \
            $$BASE_DIR/../mixxx-winlib/faad2/include
        HEADERS += $$BASE_DIR/../mixxx-winlib/mp4v2/include/mp4.h \
            $$BASE_DIR/../mixxx-winlib/mp4v2/include/mpeg4ip.h \
            $$BASE_DIR/../mixxx-winlib/mp4v2/include/mpeg4ip_version.h \
            $$BASE_DIR/../mixxx-winlib/mp4v2/include/mpeg4ip_win32.h
        LIBS += $$BASE_DIR/../mixxx-winlib/mp4v2/mingw-bin/libmp4v2-0.dll \
            $$BASE_DIR/../mixxx-winlib/libfaad2.dll
    } else {
        LIBS += -lmp4v2 \
            -lfaad
    }
}
CONFIG(vinylcontrol) {
    DEFINES += __VINYLCONTROL__
    HEADERS += \
        $$BASE_DIR/lib/scratchlib/DAnalyse.h \
        $$BASE_DIR/lib/xwax/timecoder.h
    SOURCES += \
        $$BASE_DIR/lib/scratchlib/DAnalyse.cpp

    INCLUDEPATH += $$BASE_DIR/lib/scratchlib \
        $$BASE_DIR/lib/xwax
    win32:SOURCES += $$BASE_DIR/lib/xwax/timecoder_win32.c
    !win32:SOURCES += $$BASE_DIR/lib/xwax/timecoder.c
}
!CONFIG(hifieq):CXXFLAGS += -D__LOFI__ \
    -D__NO_INTTYPES__
CONFIG(shoutcast) {
    DEFINES += __SHOUTCAST__
    HEADERS += $$BASE_DIR/src/dlgprefshoutcast.h \
        $$BASE_DIR/src/encodervorbis.h \
        $$BASE_DIR/src/engine/engineshoutcast.h
    SOURCES += $$BASE_DIR/src/dlgprefshoutcast.cpp \
        $$BASE_DIR/src/encodervorbis.cpp \
        $$BASE_DIR/src/engine/engineshoutcast.cpp
    LIBS += shout \
        vorbisenc
    FORMS += $$BASE_DIR/src/dlgprefshoutcastdlg.ui
}

# CONFIG(record) {
#    DEFINES += __RECORD__
#    HEADERS += $$BASE_DIR/src/recording/defs_recording.h \
#        $$BASE_DIR/src/recording/enginerecord.h \
#        $$BASE_DIR/src/recording/writeaudiofile.h \
#        $$BASE_DIR/src/dlgprefrecord.h
#    SOURCES += $$BASE_DIR/src/recording/enginerecord.cpp \
#        $$BASE_DIR/src/recording/writeaudiofile.cpp \
#        $$BASE_DIR/src/dlgprefrecord.cpp
#    LIBS +=
#    FORMS += $$BASE_DIR/src/dlgprefrecorddlg.ui
#}

CONFIG(ffmpeg) {
    DEFINES += __FFMPEGFILE__
    PKGCONFIG += libavcodec  \
            libavformat
    LIBS += -lavcodec \
        -lavformat \
        -lz \
        -la52 \
        -ldts \
        -lgsm \
        -ldc1394_control \
        -ldl \
        -lvorbisenc \
        -lraw1394 \
        -lavutil \
        -lvorbis \
        -lm \
        -logg
}

# Copy Windows dependencies to DESTDIR.
win32 {
    !exists($$DESTDIR):system( mkdir \"$$replace(DESTDIR, /,$$DIR_SEPARATOR)\" )
    # MinGW run-time
    DLLs += $$(QTDIR)/../mingw/bin/mingwm10.dll $$(QTDIR)/../mingw/bin/libexpat-1.dll
    CONFIG(m4a): DLLs += $$BASE_DIR/../mixxx-winlib/mp4v2/mingw-bin/libmp4v2-0.dll \
        $$BASE_DIR/../mixxx-winlib/libfaad2.dll
    # Qt4 libraries
    debug {
        DLLs += $$(QTDIR)/bin/Qt3Supportd4.dll \
            $$(QTDIR)/bin/QtCored4.dll \
            $$(QTDIR)/bin/QtGuid4.dll \
            $$(QTDIR)/bin/QtNetworkd4.dll \
            $$(QTDIR)/bin/QtSqld4.dll \
            $$(QTDIR)/bin/QtXmld4.dll \
            $$(QTDIR)/bin/QtOpenGLd4.dll \
            $$(QTDIR)/bin/QtScriptd4.dll
        # include GNU Debugger in debug distros
        DLLs += $$(QTDIR)/../mingw/bin/gdb.exe
    } else {
        DLLs += $$(QTDIR)/bin/Qt3Support4.dll \
            $$(QTDIR)/bin/QtCore4.dll \
            $$(QTDIR)/bin/QtGui4.dll \
            $$(QTDIR)/bin/QtNetwork4.dll \
            $$(QTDIR)/bin/QtSql4.dll \
            $$(QTDIR)/bin/QtXml4.dll \
            $$(QTDIR)/bin/QtOpenGL4.dll \
            $$(QTDIR)/bin/QtScript4.dll
    }
    # mixxx-winlibs DLLs
    DLLs += $$BASE_DIR/../mixxx-winlib/libogg.dll \
        $$BASE_DIR/../mixxx-winlib/portaudio_x86.dll \
#        $$BASE_DIR/../mixxx-winlib/portaudio.dll \
        $$BASE_DIR/../mixxx-winlib/libsndfile-1.dll \
#        $$BASE_DIR/../mixxx-winlib/sndfile.dll \
        $$BASE_DIR/../mixxx-winlib/libvorbis.dll \
        $$BASE_DIR/../mixxx-winlib/libvorbisfile.dll

    # check if DLL exists at target, if not copy it there
    for(DLL, DLLs):!exists( $$DESTDIR/$$basename(DLL) ) {
        message( copying \"$$replace(DLL, /,$$DIR_SEPARATOR)\" -> \"$$DESTDIR\" ... )
        system( $$QMAKE_COPY \"$$replace(DLL, /,$$DIR_SEPARATOR)\" \"$$DESTDIR\" )
    }
    # create DESTDIR\testrun-mixxx.cmd to run mixxx using the workspace resource files.
    message ( Creating testrun-mixxx.cmd at \"$${PWD}$${DIR_SEPARATOR}$$replace(DESTDIR, /,$${DIR_SEPARATOR})$${DIR_SEPARATOR}testrun-$${TARGET}.cmd\" )
    system( echo $$TARGET --resourcePath \"$$replace(PWD, /,$${DIR_SEPARATOR})$${DIR_SEPARATOR}res\">\"$${PWD}$${DIR_SEPARATOR}$$replace(DESTDIR, /,$${DIR_SEPARATOR})$${DIR_SEPARATOR}testrun-$${TARGET}.cmd\" )
}

# Get info from BZR about the current branch
BZR_REVNO = $$system( bzr revno )
BZR_INFO = $$system( bzr info )
for(BZR_INFO_BITS, BZR_INFO) {
	BZR_BRANCH_URL = $${BZR_INFO_BITS}
}
BZR_BRANCH_NAME = $$dirname(BZR_BRANCH_URL)
BZR_BRANCH_NAME = $$basename(BZR_BRANCH_NAME)
message(BRANCH_NAME is $$BZR_BRANCH_NAME)
message(REVISION is $$BZR_REVNO)
message(BRANCH_URL is $$BZR_BRANCH_URL)

win32 {
    # Makefile target to build an NSIS Installer...
    # TODO: either fix this to work in a cross-compile or make a seperate cross-compile NSIS target
    # CMD Usage: C:/Qt/QtCreator/mingw/bin/mingw32-make -f Makefile.Debug nsis
    # SH Usage: make -f Makefile.Debug nsis
    nsis.target = nsis
    exists($$BUILDDIR/gdb.exe):INCLUDE_GDB = -DINCLUDE_GDB
    nsis.commands = \"$$(PROGRAMFILES)\NSIS\makensis.exe\" -NOCD -DGCC -DBINDIR=\"$$BUILDDIR\" -DBUILD_REV=\"$$BZR_BRANCH_NAME-$$BZR_REVNO\" $$INCLUDE_GDB build\\\\nsis\\\\Mixxx.nsi
    # nsis.depends =
    QMAKE_EXTRA_UNIX_TARGETS += nsis
}

# build.h
BUILD_REV = $${BZR_BRANCH_NAME} : $${BZR_REVNO}
isEmpty( BUILD_REV ):BUILD_REV = Killroy was here
BUILD_REV += - built via qmake/Qt Creator
message( Generating src$${DIR_SEPARATOR}build.h with contents: $${LITERAL_HASH}define BUILD_REV '"'$$BUILD_REV'"' )
system( echo $${LITERAL_HASH}define BUILD_REV '"'$$BUILD_REV'"'>src$${DIR_SEPARATOR}build.h )
system( echo $${LITERAL_HASH}define BUILD_FLAGS '"'$$replace(DEFINES,__,)'"'>>src$${DIR_SEPARATOR}build.h )
