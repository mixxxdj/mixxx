CONFIG += debug link_pkgconfig portmidi script vinylcontrol mad
#CONFIG += m4a hss1394 ladspa
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
    QMAKE_CXXFLAGS += "\"-DSETTINGS_PATH=\\\"Local Settings/Application Data/Mixxx/\\\"\""
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
    network \
    opengl \
    script \
    sql \
    xml

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

HEADERS += $$UI_DIR/ui_dlgpreferencesdlg.h \
    $$UI_DIR/ui_dlgprefsounddlg.h \
    $$UI_DIR/controllers/ui_dlgprefcontrollerdlg.h \
    $$UI_DIR/controllers/ui_dlgprefmappablecontrollerdlg.h \
    $$UI_DIR/controllers/ui_dlgcontrollerlearning.h \
    $$UI_DIR/controllers/ui_dlgprefnocontrollersdlg.h \
    $$UI_DIR/ui_dlgprefplaylistdlg.h \
    $$UI_DIR/ui_dlgprefcontrolsdlg.h \
    $$UI_DIR/ui_dlgprefeqdlg.h \
    $$UI_DIR/ui_dlgprefcrossfaderdlg.h \
    $$UI_DIR/ui_dlgprefeqdlg.h \
    $$UI_DIR/ui_dlgpreferencesdlg.h \
    $$UI_DIR/ui_dlgprefmidibindingsdlg.h \
    $$UI_DIR/ui_dlgprefplaylistdlg.h \
    $$UI_DIR/ui_dlgtagfetcher.h \
    $$UI_DIR/ui_dlgprefrecorddlg.h \
    $$UI_DIR/ui_dlgprefsounddlg.h \
    $$UI_DIR/ui_dlgprefvinyldlg.h \
    $$UI_DIR/ui_dlgprefnovinyldlg.h \
    $$UI_DIR/ui_dlgprefrecorddlg.h \
    $$UI_DIR/ui_dlgaboutdlg.h \
    $$UI_DIR/ui_dlgtrackinfo.h \
    $$UI_DIR/ui_dlgprepare.h \
    $$UI_DIR/ui_dlgautodj.h \
    $$UI_DIR/ui_dlgprefsounditem.h \
    $$UI_DIR/ui_dlgrecording.h \
    $$UI_DIR/ui_dlghidden.h \
    $$UI_DIR/ui_dlgmissing.h

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
$$BASE_DIR/src/analyserbeats.h \
$$BASE_DIR/src/analyserqueue.h \
$$BASE_DIR/src/analyserrg.h \
$$BASE_DIR/src/analyserwaveform.h \
$$BASE_DIR/src/audiotagger.h \
$$BASE_DIR/src/baseplayer.h \
$$BASE_DIR/src/basetrackplayer.h \
$$BASE_DIR/src/bpm/bpmscheme.h \
$$BASE_DIR/src/bpm/wavesegmentation.h \
$$BASE_DIR/src/build.h \
$$BASE_DIR/src/cachingreader.h \
$$BASE_DIR/src/circularbuffer.h \
$$BASE_DIR/src/configobject.h \
$$BASE_DIR/src/controlbeat.h \
$$BASE_DIR/src/controlevent.h \
$$BASE_DIR/src/controllers/controller.h \
$$BASE_DIR/src/controllers/controllerengine.h \
$$BASE_DIR/src/controllers/controllerenumerator.h \
$$BASE_DIR/src/controllers/controllerlearningeventfilter.h \
$$BASE_DIR/src/controllers/controllermanager.h \
$$BASE_DIR/src/controllers/controllerpreset.h \
$$BASE_DIR/src/controllers/controllerpresetfilehandler.h \
$$BASE_DIR/src/controllers/controllerpresetinfo.h \
$$BASE_DIR/src/controllers/controllerpresetvisitor.h \
$$BASE_DIR/src/controllers/defs_controllers.h \
$$BASE_DIR/src/controllers/dlgcontrollerlearning.h \
$$BASE_DIR/src/controllers/dlgprefcontroller.h \
$$BASE_DIR/src/controllers/dlgprefmappablecontroller.h \
$$BASE_DIR/src/controllers/dlgprefnocontrollers.h \
$$BASE_DIR/src/controllers/midi/midicontroller.h \
$$BASE_DIR/src/controllers/midi/midicontrollerpreset.h \
$$BASE_DIR/src/controllers/midi/midicontrollerpresetfilehandler.h \
$$BASE_DIR/src/controllers/midi/midienumerator.h \
$$BASE_DIR/src/controllers/midi/midimessage.h \
$$BASE_DIR/src/controllers/midi/midioutputhandler.h \
$$BASE_DIR/src/controllers/mixxxcontrol.h \
$$BASE_DIR/src/controllers/pitchfilter.h \
$$BASE_DIR/src/controllers/qtscript-bytearray/bytearrayclass.h \
$$BASE_DIR/src/controllers/qtscript-bytearray/bytearrayprototype.h \
$$BASE_DIR/src/controllers/softtakeover.h \
$$BASE_DIR/src/controllinpotmeter.h \
$$BASE_DIR/src/controllogpotmeter.h \
$$BASE_DIR/src/controlnull.h \
$$BASE_DIR/src/controlobject.h \
$$BASE_DIR/src/controlobjectthread.h \
$$BASE_DIR/src/controlobjectthreadmain.h \
$$BASE_DIR/src/controlobjectthreadwidget.h \
$$BASE_DIR/src/controlpotmeter.h \
$$BASE_DIR/src/controlpushbutton.h \
$$BASE_DIR/src/controlttrotary.h \
$$BASE_DIR/src/deck.h \
$$BASE_DIR/src/defs.h \
$$BASE_DIR/src/defs_promo.h \
$$BASE_DIR/src/defs_urls.h \
$$BASE_DIR/src/defs_version.h \
$$BASE_DIR/src/dlgabout.h \
$$BASE_DIR/src/dlgautodj.h \
$$BASE_DIR/src/dlghidden.h \
$$BASE_DIR/src/dlgmissing.h \
$$BASE_DIR/src/dlgprefbeats.h \
$$BASE_DIR/src/dlgprefcontrols.h \
$$BASE_DIR/src/dlgprefcrossfader.h \
$$BASE_DIR/src/dlgprefeq.h \
$$BASE_DIR/src/dlgpreferences.h \
$$BASE_DIR/src/dlgprefplaylist.h \
$$BASE_DIR/src/dlgprefrecord.h \
$$BASE_DIR/src/dlgprefreplaygain.h \
$$BASE_DIR/src/dlgprefsound.h \
$$BASE_DIR/src/dlgprefsounditem.h \
$$BASE_DIR/src/dlgprepare.h \
$$BASE_DIR/src/dlgrecording.h \
$$BASE_DIR/src/dlgtrackinfo.h \
$$BASE_DIR/src/engine/bpmcontrol.h \
$$BASE_DIR/src/engine/clockcontrol.h \
$$BASE_DIR/src/engine/cuecontrol.h \
$$BASE_DIR/src/engine/enginebuffer.h \
$$BASE_DIR/src/engine/enginebufferscale.h \
$$BASE_DIR/src/engine/enginebufferscaledummy.h \
$$BASE_DIR/src/engine/enginebufferscalelinear.h \
$$BASE_DIR/src/engine/enginebufferscalest.h \
$$BASE_DIR/src/engine/enginechannel.h \
$$BASE_DIR/src/engine/engineclipping.h \
$$BASE_DIR/src/engine/enginecontrol.h \
$$BASE_DIR/src/engine/enginedeck.h \
$$BASE_DIR/src/engine/enginedelay.h \
$$BASE_DIR/src/engine/enginefilter.h \
$$BASE_DIR/src/engine/enginefilterblock.h \
$$BASE_DIR/src/engine/enginefilterbutterworth8.h \
$$BASE_DIR/src/engine/enginefilteriir.h \
$$BASE_DIR/src/engine/engineflanger.h \
$$BASE_DIR/src/engine/enginemaster.h \
$$BASE_DIR/src/engine/enginemicrophone.h \
$$BASE_DIR/src/engine/engineobject.h \
$$BASE_DIR/src/engine/enginepassthrough.h \
$$BASE_DIR/src/engine/enginepregain.h \
$$BASE_DIR/src/engine/sidechain/enginesidechain.h \
$$BASE_DIR/src/engine/sidechain/enginerecord.h \
$$BASE_DIR/src/engine/enginevinylsoundemu.h \
$$BASE_DIR/src/engine/enginevumeter.h \
$$BASE_DIR/src/engine/engineworker.h \
$$BASE_DIR/src/engine/engineworkerscheduler.h \
$$BASE_DIR/src/engine/enginexfader.h \
$$BASE_DIR/src/engine/loopingcontrol.h \
$$BASE_DIR/src/engine/positionscratchcontroller.h \
$$BASE_DIR/src/engine/quantizecontrol.h \
$$BASE_DIR/src/engine/ratecontrol.h \
$$BASE_DIR/src/engine/readaheadmanager.h \
$$BASE_DIR/src/engine/syncworker.h \
$$BASE_DIR/src/errordialoghandler.h \
$$BASE_DIR/src/library/autodjfeature.h \
$$BASE_DIR/src/library/baseexternallibraryfeature.h \
$$BASE_DIR/src/library/baseexternalplaylistmodel.h \
$$BASE_DIR/src/library/baseexternaltrackmodel.h \
$$BASE_DIR/src/library/baseplaylistfeature.h \
$$BASE_DIR/src/library/basesqltablemodel.h \
$$BASE_DIR/src/library/basetrackcache.h \
$$BASE_DIR/src/library/browse/browsefeature.h \
$$BASE_DIR/src/library/browse/browsetablemodel.h \
$$BASE_DIR/src/library/browse/browsethread.h \
$$BASE_DIR/src/library/browse/foldertreemodel.h \
$$BASE_DIR/src/library/cratefeature.h \
$$BASE_DIR/src/library/cratetablemodel.h \
$$BASE_DIR/src/library/dao/analysisdao.h \
$$BASE_DIR/src/library/dao/cratedao.h \
$$BASE_DIR/src/library/dao/cue.h \
$$BASE_DIR/src/library/dao/cuedao.h \
$$BASE_DIR/src/library/dao/dao.h \
$$BASE_DIR/src/library/dao/libraryhashdao.h \
$$BASE_DIR/src/library/dao/playlistdao.h \
$$BASE_DIR/src/library/dao/settingsdao.h \
$$BASE_DIR/src/library/dao/trackdao.h \
$$BASE_DIR/src/library/hiddentablemodel.h \
$$BASE_DIR/src/library/itunes/itunesfeature.h \
$$BASE_DIR/src/library/legacylibraryimporter.h \
$$BASE_DIR/src/library/library.h \
$$BASE_DIR/src/library/librarycontrol.h \
$$BASE_DIR/src/library/libraryfeature.h \
$$BASE_DIR/src/library/libraryscanner.h \
$$BASE_DIR/src/library/libraryscannerdlg.h \
$$BASE_DIR/src/library/librarytablemodel.h \
$$BASE_DIR/src/library/libraryview.h \
$$BASE_DIR/src/library/missingtablemodel.h \
$$BASE_DIR/src/library/mixxxlibraryfeature.h \
$$BASE_DIR/src/library/parser.h \
$$BASE_DIR/src/library/parsercsv.h \
$$BASE_DIR/src/library/parserm3u.h \
$$BASE_DIR/src/library/parserpls.h \
$$BASE_DIR/src/library/playlistfeature.h \
$$BASE_DIR/src/library/playlisttablemodel.h \
$$BASE_DIR/src/library/preparecratedelegate.h \
$$BASE_DIR/src/library/preparefeature.h \
$$BASE_DIR/src/library/preparelibrarytablemodel.h \
$$BASE_DIR/src/library/previewbuttondelegate.h \
$$BASE_DIR/src/library/proxytrackmodel.h \
$$BASE_DIR/src/library/queryutil.h \
$$BASE_DIR/src/library/recording/recordingfeature.h \
$$BASE_DIR/src/library/rhythmbox/rhythmboxfeature.h \
$$BASE_DIR/src/library/schemamanager.h \
$$BASE_DIR/src/library/searchqueryparser.h \
$$BASE_DIR/src/library/searchthread.h \
$$BASE_DIR/src/library/setlogfeature.h \
$$BASE_DIR/src/library/sidebarmodel.h \
$$BASE_DIR/src/library/songdownloader.h \
$$BASE_DIR/src/library/stardelegate.h \
$$BASE_DIR/src/library/stareditor.h \
$$BASE_DIR/src/library/bpmdelegate.h \
$$BASE_DIR/src/library/bpmeditor.h \
$$BASE_DIR/src/library/starrating.h \
$$BASE_DIR/src/library/trackcollection.h \
$$BASE_DIR/src/library/trackmodel.h \
$$BASE_DIR/src/library/traktor/traktorfeature.h \
$$BASE_DIR/src/library/treeitem.h \
$$BASE_DIR/src/library/treeitemmodel.h \
$$BASE_DIR/src/mathstuff.h \
$$BASE_DIR/src/mixxx.h \
$$BASE_DIR/src/mixxxevent.h \
$$BASE_DIR/src/mixxxkeyboard.h \
$$BASE_DIR/src/playerinfo.h \
$$BASE_DIR/src/playermanager.h \
$$BASE_DIR/src/previewdeck.h \
$$BASE_DIR/src/recording/defs_recording.h \
$$BASE_DIR/src/recording/recordingmanager.h \
$$BASE_DIR/src/rotary.h \
$$BASE_DIR/src/sampler.h \
$$BASE_DIR/src/samplerbank.h \
$$BASE_DIR/src/sampleutil.h \
$$BASE_DIR/src/segmentation.h \
$$BASE_DIR/src/sharedglcontext.h \
$$BASE_DIR/src/singleton.h \
$$BASE_DIR/src/skin/colorschemeparser.h \
$$BASE_DIR/src/skin/imgcolor.h \
$$BASE_DIR/src/skin/imginvert.h \
$$BASE_DIR/src/skin/imgloader.h \
$$BASE_DIR/src/skin/imgsource.h \
$$BASE_DIR/src/skin/legacyskinparser.h \
$$BASE_DIR/src/skin/propertybinder.h \
$$BASE_DIR/src/skin/skinloader.h \
$$BASE_DIR/src/skin/skinparser.h \
$$BASE_DIR/src/skin/tooltips.h \
$$BASE_DIR/src/sounddevice.h \
$$BASE_DIR/src/sounddeviceportaudio.h \
$$BASE_DIR/src/soundmanager.h \
$$BASE_DIR/src/soundmanagerconfig.h \
$$BASE_DIR/src/soundmanagerutil.h \
$$BASE_DIR/src/soundsource.h \
$$BASE_DIR/src/soundsourcecoreaudio.h \
$$BASE_DIR/src/soundsourceffmpeg.h \
$$BASE_DIR/src/soundsourceflac.h \
$$BASE_DIR/src/soundsourceoggvorbis.h \
$$BASE_DIR/src/soundsourceproxy.h \
$$BASE_DIR/src/soundsourcesndfile.h \
$$BASE_DIR/src/tapfilter.h \
$$BASE_DIR/src/test/mixxxtest.h \
$$BASE_DIR/src/tonal/tonalanalyser.h \
$$BASE_DIR/src/track/beat_preferences.h \
$$BASE_DIR/src/track/beatfactory.h \
$$BASE_DIR/src/track/beatgrid.h \
$$BASE_DIR/src/track/beatmap.h \
$$BASE_DIR/src/track/beats.h \
$$BASE_DIR/src/track/beatutils.h \
$$BASE_DIR/src/trackinfoobject.h \
$$BASE_DIR/src/transposeproxymodel.h \
$$BASE_DIR/src/upgrade.h \
$$BASE_DIR/src/util.h \
$$BASE_DIR/src/util/cmdlineargs.h \
$$BASE_DIR/src/util/counter.h \
$$BASE_DIR/src/util/debug.h \
$$BASE_DIR/src/util/fifo.h \
$$BASE_DIR/src/util/lcs.h \
$$BASE_DIR/src/util/pa_memorybarrier.h \
$$BASE_DIR/src/util/pa_ringbuffer.h \
$$BASE_DIR/src/util/performancetimer.h \
$$BASE_DIR/src/util/sleepableqthread.h \
$$BASE_DIR/src/util/stat.h \
$$BASE_DIR/src/util/statsmanager.h \
$$BASE_DIR/src/util/timer.h \
$$BASE_DIR/src/util/trace.h \
$$BASE_DIR/src/waveform/renderers/glslwaveformrenderersignal.h \
$$BASE_DIR/src/waveform/renderers/glwaveformrendererfilteredsignal.h \
$$BASE_DIR/src/waveform/renderers/glwaveformrenderersimplesignal.h \
$$BASE_DIR/src/waveform/renderers/qtwaveformrendererfilteredsignal.h \
$$BASE_DIR/src/waveform/renderers/qtwaveformrenderersimplesignal.h \
$$BASE_DIR/src/waveform/renderers/waveformmark.h \
$$BASE_DIR/src/waveform/renderers/waveformmarkrange.h \
$$BASE_DIR/src/waveform/renderers/waveformmarkset.h \
$$BASE_DIR/src/waveform/renderers/waveformrenderbackground.h \
$$BASE_DIR/src/waveform/renderers/waveformrenderbeat.h \
$$BASE_DIR/src/waveform/renderers/waveformrendererabstract.h \
$$BASE_DIR/src/waveform/renderers/waveformrendererendoftrack.h \
$$BASE_DIR/src/waveform/renderers/waveformrendererfilteredsignal.h \
$$BASE_DIR/src/waveform/renderers/waveformrendererhsv.h \
$$BASE_DIR/src/waveform/renderers/waveformrendererpreroll.h \
$$BASE_DIR/src/waveform/renderers/waveformrenderersignalbase.h \
$$BASE_DIR/src/waveform/renderers/waveformrendermark.h \
$$BASE_DIR/src/waveform/renderers/waveformrendermarkrange.h \
$$BASE_DIR/src/waveform/renderers/waveformsignalcolors.h \
$$BASE_DIR/src/waveform/renderers/waveformwidgetrenderer.h \
$$BASE_DIR/src/waveform/waveform.h \
$$BASE_DIR/src/waveform/waveformfactory.h \
$$BASE_DIR/src/waveform/waveformwidgetfactory.h \
$$BASE_DIR/src/waveform/widgets/emptywaveformwidget.h \
$$BASE_DIR/src/waveform/widgets/glsimplewaveformwidget.h \
$$BASE_DIR/src/waveform/widgets/glslwaveformwidget.h \
$$BASE_DIR/src/waveform/widgets/glwaveformwidget.h \
$$BASE_DIR/src/waveform/widgets/hsvwaveformwidget.h \
$$BASE_DIR/src/waveform/widgets/qtsimplewaveformwidget.h \
$$BASE_DIR/src/waveform/widgets/qtwaveformwidget.h \
$$BASE_DIR/src/waveform/widgets/softwarewaveformwidget.h \
$$BASE_DIR/src/waveform/widgets/waveformwidgetabstract.h \
$$BASE_DIR/src/waveform/widgets/waveformwidgettype.h \
$$BASE_DIR/src/widget/hexspinbox.h \
$$BASE_DIR/src/widget/wabstractcontrol.h \
$$BASE_DIR/src/widget/wdisplay.h \
$$BASE_DIR/src/widget/wimagestore.h \
$$BASE_DIR/src/widget/wknob.h \
$$BASE_DIR/src/widget/wlabel.h \
$$BASE_DIR/src/widget/wlibrary.h \
$$BASE_DIR/src/widget/wlibrarysidebar.h \
$$BASE_DIR/src/widget/wlibrarytableview.h \
$$BASE_DIR/src/widget/wlibrarytextbrowser.h \
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
$$BASE_DIR/src/widget/wslider.h \
$$BASE_DIR/src/widget/wslidercomposed.h \
$$BASE_DIR/src/widget/wspinny.h \
$$BASE_DIR/src/widget/wstatuslight.h \
$$BASE_DIR/src/widget/wtime.h \
$$BASE_DIR/src/widget/wtrackproperty.h \
$$BASE_DIR/src/widget/wtracktableview.h \
$$BASE_DIR/src/widget/wtracktableviewheader.h \
$$BASE_DIR/src/widget/wtracktext.h \
$$BASE_DIR/src/widget/wvumeter.h \
$$BASE_DIR/src/widget/wwaveformviewer.h \
$$BASE_DIR/src/widget/wwidget.h \
$$BASE_DIR/src/widget/wwidgetgroup.h \
$$BASE_DIR/src/widget/wwidgetstack.h \
$$BASE_DIR/src/xmlparse.h

SOURCES += \
$$BASE_DIR/src/mixxxkeyboard.cpp \
$$BASE_DIR/src/configobject.cpp \
$$BASE_DIR/src/controlobjectthread.cpp \
$$BASE_DIR/src/controlobjectthreadwidget.cpp \
$$BASE_DIR/src/controlobjectthreadmain.cpp \
$$BASE_DIR/src/controlevent.cpp \
$$BASE_DIR/src/controllogpotmeter.cpp \
$$BASE_DIR/src/controlobject.cpp \
$$BASE_DIR/src/controlnull.cpp \
$$BASE_DIR/src/controlpotmeter.cpp \
$$BASE_DIR/src/controllinpotmeter.cpp \
$$BASE_DIR/src/controlpushbutton.cpp \
$$BASE_DIR/src/controlttrotary.cpp \
$$BASE_DIR/src/controlbeat.cpp \
$$BASE_DIR/src/dlgpreferences.cpp \
$$BASE_DIR/src/dlgprefsound.cpp \
$$BASE_DIR/src/dlgprefsounditem.cpp \
$$BASE_DIR/src/controllers/dlgprefcontroller.cpp \
$$BASE_DIR/src/controllers/dlgprefmappablecontroller.cpp \
$$BASE_DIR/src/controllers/dlgcontrollerlearning.cpp \
$$BASE_DIR/src/controllers/dlgprefnocontrollers.cpp \
$$BASE_DIR/src/dlgprefbeats.cpp \
$$BASE_DIR/src/dlgprefplaylist.cpp \
$$BASE_DIR/src/dlgprefcontrols.cpp \
$$BASE_DIR/src/dlgprefreplaygain.cpp \
$$BASE_DIR/src/dlgprefnovinyl.cpp \
$$BASE_DIR/src/dlgabout.cpp \
$$BASE_DIR/src/dlgprefeq.cpp \
$$BASE_DIR/src/dlgprefcrossfader.cpp \
$$BASE_DIR/src/dlgtrackinfo.cpp \
$$BASE_DIR/src/dlgprepare.cpp \
$$BASE_DIR/src/dlgautodj.cpp \
$$BASE_DIR/src/dlghidden.cpp \
$$BASE_DIR/src/dlgmissing.cpp \
$$BASE_DIR/src/engine/engineworker.cpp \
$$BASE_DIR/src/engine/engineworkerscheduler.cpp \
$$BASE_DIR/src/engine/syncworker.cpp \
$$BASE_DIR/src/engine/enginebuffer.cpp \
$$BASE_DIR/src/engine/enginebufferscale.cpp \
$$BASE_DIR/src/engine/enginebufferscaledummy.cpp \
$$BASE_DIR/src/engine/enginebufferscalelinear.cpp \
$$BASE_DIR/src/engine/engineclipping.cpp \
$$BASE_DIR/src/engine/enginefilterblock.cpp \
$$BASE_DIR/src/engine/enginefilteriir.cpp \
$$BASE_DIR/src/engine/enginefilter.cpp \
$$BASE_DIR/src/engine/engineobject.cpp \
$$BASE_DIR/src/engine/enginepregain.cpp \
$$BASE_DIR/src/engine/enginechannel.cpp \
$$BASE_DIR/src/engine/enginemaster.cpp \
$$BASE_DIR/src/engine/enginedelay.cpp \
$$BASE_DIR/src/engine/engineflanger.cpp \
$$BASE_DIR/src/engine/enginevumeter.cpp \
$$BASE_DIR/src/engine/enginevinylsoundemu.cpp \
$$BASE_DIR/src/engine/sidechain/enginesidechain.cpp \
$$BASE_DIR/src/engine/enginefilterbutterworth8.cpp \
$$BASE_DIR/src/engine/enginexfader.cpp \
$$BASE_DIR/src/engine/enginemicrophone.cpp \
$$BASE_DIR/src/engine/enginedeck.cpp \
$$BASE_DIR/src/engine/enginepassthrough.cpp \
$$BASE_DIR/src/engine/enginecontrol.cpp \
$$BASE_DIR/src/engine/ratecontrol.cpp \
$$BASE_DIR/src/engine/positionscratchcontroller.cpp \
$$BASE_DIR/src/engine/loopingcontrol.cpp \
$$BASE_DIR/src/engine/bpmcontrol.cpp \
$$BASE_DIR/src/engine/cuecontrol.cpp \
$$BASE_DIR/src/engine/quantizecontrol.cpp \
$$BASE_DIR/src/engine/clockcontrol.cpp \
$$BASE_DIR/src/engine/readaheadmanager.cpp \
$$BASE_DIR/src/cachingreader.cpp \
$$BASE_DIR/src/analyserrg.cpp \
$$BASE_DIR/src/analyserbeats.cpp \
$$BASE_DIR/src/analyserqueue.cpp \
$$BASE_DIR/src/analyserwaveform.cpp \
$$BASE_DIR/src/controllers/controller.cpp \
$$BASE_DIR/src/controllers/controllerengine.cpp \
$$BASE_DIR/src/controllers/controllerenumerator.cpp \
$$BASE_DIR/src/controllers/controllerlearningeventfilter.cpp \
$$BASE_DIR/src/controllers/controllermanager.cpp \
$$BASE_DIR/src/controllers/controllerpresetfilehandler.cpp \
$$BASE_DIR/src/controllers/controllerpresetinfo.cpp \
$$BASE_DIR/src/controllers/midi/midicontroller.cpp \
$$BASE_DIR/src/controllers/midi/midicontrollerpresetfilehandler.cpp \
$$BASE_DIR/src/controllers/midi/midienumerator.cpp \
$$BASE_DIR/src/controllers/midi/midioutputhandler.cpp \
$$BASE_DIR/src/controllers/mixxxcontrol.cpp \
$$BASE_DIR/src/controllers/qtscript-bytearray/bytearrayclass.cpp \
$$BASE_DIR/src/controllers/qtscript-bytearray/bytearrayprototype.cpp \
$$BASE_DIR/src/controllers/softtakeover.cpp \
$$BASE_DIR/src/main.cpp \
$$BASE_DIR/src/mixxx.cpp \
$$BASE_DIR/src/errordialoghandler.cpp \
$$BASE_DIR/src/upgrade.cpp \
$$BASE_DIR/src/soundsource.cpp \
$$BASE_DIR/src/soundsourceoggvorbis.cpp \
$$BASE_DIR/src/soundsourceflac.cpp \
$$BASE_DIR/src/soundsourcesndfile.cpp \
$$BASE_DIR/src/sharedglcontext.cpp \
$$BASE_DIR/src/widget/wwidget.cpp \
$$BASE_DIR/src/widget/wwidgetgroup.cpp \
$$BASE_DIR/src/widget/wwidgetstack.cpp \
$$BASE_DIR/src/widget/wlabel.cpp \
$$BASE_DIR/src/widget/wtracktext.cpp \
$$BASE_DIR/src/widget/wnumber.cpp \
$$BASE_DIR/src/widget/wnumberpos.cpp \
$$BASE_DIR/src/widget/wnumberrate.cpp \
$$BASE_DIR/src/widget/wknob.cpp \
$$BASE_DIR/src/widget/wdisplay.cpp \
$$BASE_DIR/src/widget/wvumeter.cpp \
$$BASE_DIR/src/widget/wpushbutton.cpp \
$$BASE_DIR/src/widget/wslidercomposed.cpp \
$$BASE_DIR/src/widget/wslider.cpp \
$$BASE_DIR/src/widget/wstatuslight.cpp \
$$BASE_DIR/src/widget/woverview.cpp \
$$BASE_DIR/src/widget/wspinny.cpp \
$$BASE_DIR/src/widget/wskincolor.cpp \
$$BASE_DIR/src/widget/wabstractcontrol.cpp \
$$BASE_DIR/src/widget/wsearchlineedit.cpp \
$$BASE_DIR/src/widget/wpixmapstore.cpp \
$$BASE_DIR/src/widget/wimagestore.cpp \
$$BASE_DIR/src/widget/hexspinbox.cpp \
$$BASE_DIR/src/widget/wtrackproperty.cpp \
$$BASE_DIR/src/widget/wtime.cpp \
$$BASE_DIR/src/mathstuff.cpp \
$$BASE_DIR/src/rotary.cpp \
$$BASE_DIR/src/widget/wtracktableview.cpp \
$$BASE_DIR/src/widget/wtracktableviewheader.cpp \
$$BASE_DIR/src/widget/wlibrarysidebar.cpp \
$$BASE_DIR/src/widget/wlibrary.cpp \
$$BASE_DIR/src/widget/wlibrarytableview.cpp \
$$BASE_DIR/src/widget/wpreparelibrarytableview.cpp \
$$BASE_DIR/src/widget/wpreparecratestableview.cpp \
$$BASE_DIR/src/widget/wlibrarytextbrowser.cpp \
$$BASE_DIR/src/library/preparecratedelegate.cpp \
$$BASE_DIR/src/library/trackcollection.cpp \
$$BASE_DIR/src/library/basesqltablemodel.cpp \
$$BASE_DIR/src/library/basetrackcache.cpp \
$$BASE_DIR/src/library/librarytablemodel.cpp \
$$BASE_DIR/src/library/searchqueryparser.cpp \
$$BASE_DIR/src/library/preparelibrarytablemodel.cpp \
$$BASE_DIR/src/library/missingtablemodel.cpp \
$$BASE_DIR/src/library/hiddentablemodel.cpp \
$$BASE_DIR/src/library/proxytrackmodel.cpp \
$$BASE_DIR/src/library/playlisttablemodel.cpp \
$$BASE_DIR/src/library/libraryfeature.cpp \
$$BASE_DIR/src/library/preparefeature.cpp \
$$BASE_DIR/src/library/autodjfeature.cpp \
$$BASE_DIR/src/library/mixxxlibraryfeature.cpp \
$$BASE_DIR/src/library/baseplaylistfeature.cpp \
$$BASE_DIR/src/library/playlistfeature.cpp \
$$BASE_DIR/src/library/setlogfeature.cpp \
$$BASE_DIR/src/library/browse/browsetablemodel.cpp \
$$BASE_DIR/src/library/browse/browsethread.cpp \
$$BASE_DIR/src/library/browse/browsefeature.cpp \
$$BASE_DIR/src/library/browse/foldertreemodel.cpp \
$$BASE_DIR/src/library/recording/recordingfeature.cpp \
$$BASE_DIR/src/dlgrecording.cpp \
$$BASE_DIR/src/recording/recordingmanager.cpp \
$$BASE_DIR/src/engine/sidechain/enginerecord.cpp \
$$BASE_DIR/src/library/baseexternallibraryfeature.cpp \
$$BASE_DIR/src/library/baseexternaltrackmodel.cpp \
$$BASE_DIR/src/library/baseexternalplaylistmodel.cpp \
$$BASE_DIR/src/library/rhythmbox/rhythmboxfeature.cpp \
$$BASE_DIR/src/library/itunes/itunesfeature.cpp \
$$BASE_DIR/src/library/traktor/traktorfeature.cpp \
$$BASE_DIR/src/library/cratefeature.cpp \
$$BASE_DIR/src/library/sidebarmodel.cpp \
$$BASE_DIR/src/library/libraryscanner.cpp \
$$BASE_DIR/src/library/libraryscannerdlg.cpp \
$$BASE_DIR/src/library/legacylibraryimporter.cpp \
$$BASE_DIR/src/library/library.cpp \
$$BASE_DIR/src/library/searchthread.cpp \
$$BASE_DIR/src/library/dao/cratedao.cpp \
$$BASE_DIR/src/library/cratetablemodel.cpp \
$$BASE_DIR/src/library/dao/cuedao.cpp \
$$BASE_DIR/src/library/dao/cue.cpp \
$$BASE_DIR/src/library/dao/trackdao.cpp \
$$BASE_DIR/src/library/dao/playlistdao.cpp \
$$BASE_DIR/src/library/dao/libraryhashdao.cpp \
$$BASE_DIR/src/library/dao/settingsdao.cpp \
$$BASE_DIR/src/library/dao/analysisdao.cpp \
$$BASE_DIR/src/library/librarycontrol.cpp \
$$BASE_DIR/src/library/schemamanager.cpp \
$$BASE_DIR/src/library/songdownloader.cpp \
$$BASE_DIR/src/library/starrating.cpp \
$$BASE_DIR/src/library/stardelegate.cpp \
$$BASE_DIR/src/library/stareditor.cpp \
$$BASE_DIR/src/library/bpmdelegate.cpp \
$$BASE_DIR/src/library/bpmeditor.cpp \
$$BASE_DIR/src/library/previewbuttondelegate.cpp \
$$BASE_DIR/src/audiotagger.cpp \
$$BASE_DIR/src/library/treeitemmodel.cpp \
$$BASE_DIR/src/library/treeitem.cpp \
$$BASE_DIR/src/xmlparse.cpp \
$$BASE_DIR/src/library/parser.cpp \
$$BASE_DIR/src/library/parserpls.cpp \
$$BASE_DIR/src/library/parserm3u.cpp \
$$BASE_DIR/src/library/parsercsv.cpp \
$$BASE_DIR/src/bpm/bpmscheme.cpp \
$$BASE_DIR/src/soundsourceproxy.cpp \
$$BASE_DIR/src/widget/wwaveformviewer.cpp \
$$BASE_DIR/src/waveform/waveform.cpp \
$$BASE_DIR/src/waveform/waveformfactory.cpp \
$$BASE_DIR/src/waveform/waveformwidgetfactory.cpp \
$$BASE_DIR/src/waveform/renderers/waveformwidgetrenderer.cpp \
$$BASE_DIR/src/waveform/renderers/waveformrendererabstract.cpp \
$$BASE_DIR/src/waveform/renderers/waveformrenderbackground.cpp \
$$BASE_DIR/src/waveform/renderers/waveformrendermark.cpp \
$$BASE_DIR/src/waveform/renderers/waveformrendermarkrange.cpp \
$$BASE_DIR/src/waveform/renderers/waveformrenderbeat.cpp \
$$BASE_DIR/src/waveform/renderers/waveformrendererendoftrack.cpp \
$$BASE_DIR/src/waveform/renderers/waveformrendererpreroll.cpp \
$$BASE_DIR/src/waveform/renderers/waveformrendererfilteredsignal.cpp \
$$BASE_DIR/src/waveform/renderers/waveformrendererhsv.cpp \
$$BASE_DIR/src/waveform/renderers/qtwaveformrendererfilteredsignal.cpp \
$$BASE_DIR/src/waveform/renderers/qtwaveformrenderersimplesignal.cpp \
$$BASE_DIR/src/waveform/renderers/glwaveformrendererfilteredsignal.cpp \
$$BASE_DIR/src/waveform/renderers/glwaveformrenderersimplesignal.cpp \
$$BASE_DIR/src/waveform/renderers/glslwaveformrenderersignal.cpp \
$$BASE_DIR/src/waveform/renderers/waveformsignalcolors.cpp \
$$BASE_DIR/src/waveform/renderers/waveformrenderersignalbase.cpp \
$$BASE_DIR/src/waveform/renderers/waveformmark.cpp \
$$BASE_DIR/src/waveform/renderers/waveformmarkset.cpp \
$$BASE_DIR/src/waveform/renderers/waveformmarkrange.cpp \
$$BASE_DIR/src/waveform/widgets/waveformwidgetabstract.cpp \
$$BASE_DIR/src/waveform/widgets/emptywaveformwidget.cpp \
$$BASE_DIR/src/waveform/widgets/softwarewaveformwidget.cpp \
$$BASE_DIR/src/waveform/widgets/hsvwaveformwidget.cpp \
$$BASE_DIR/src/waveform/widgets/qtwaveformwidget.cpp \
$$BASE_DIR/src/waveform/widgets/qtsimplewaveformwidget.cpp \
$$BASE_DIR/src/waveform/widgets/glwaveformwidget.cpp \
$$BASE_DIR/src/waveform/widgets/glsimplewaveformwidget.cpp \
$$BASE_DIR/src/waveform/widgets/glslwaveformwidget.cpp \
$$BASE_DIR/src/skin/imginvert.cpp \
$$BASE_DIR/src/skin/imgloader.cpp \
$$BASE_DIR/src/skin/imgcolor.cpp \
$$BASE_DIR/src/skin/skinloader.cpp \
$$BASE_DIR/src/skin/legacyskinparser.cpp \
$$BASE_DIR/src/skin/colorschemeparser.cpp \
$$BASE_DIR/src/skin/propertybinder.cpp \
$$BASE_DIR/src/skin/tooltips.cpp \
$$BASE_DIR/src/sampleutil.cpp \
$$BASE_DIR/src/trackinfoobject.cpp \
$$BASE_DIR/src/track/beatgrid.cpp \
$$BASE_DIR/src/track/beatmap.cpp \
$$BASE_DIR/src/track/beatfactory.cpp \
$$BASE_DIR/src/track/beatutils.cpp \
$$BASE_DIR/src/baseplayer.cpp \
$$BASE_DIR/src/basetrackplayer.cpp \
$$BASE_DIR/src/deck.cpp \
$$BASE_DIR/src/sampler.cpp \
$$BASE_DIR/src/previewdeck.cpp \
$$BASE_DIR/src/playermanager.cpp \
$$BASE_DIR/src/samplerbank.cpp \
$$BASE_DIR/src/sounddevice.cpp \
$$BASE_DIR/src/sounddeviceportaudio.cpp \
$$BASE_DIR/src/soundmanager.cpp \
$$BASE_DIR/src/soundmanagerconfig.cpp \
$$BASE_DIR/src/soundmanagerutil.cpp \
$$BASE_DIR/src/dlgprefrecord.cpp \
$$BASE_DIR/src/playerinfo.cpp \
$$BASE_DIR/src/encoder/encoder.cpp \
$$BASE_DIR/src/encoder/encodermp3.cpp \
$$BASE_DIR/src/encoder/encodervorbis.cpp \
$$BASE_DIR/src/segmentation.cpp \
$$BASE_DIR/src/tapfilter.cpp \
$$BASE_DIR/src/util/pa_ringbuffer.c \
$$BASE_DIR/src/util/sleepableqthread.cpp \
$$BASE_DIR/src/util/statsmanager.cpp \
$$BASE_DIR/src/util/stat.cpp \
$$BASE_DIR/src/util/timer.cpp \
$$BASE_DIR/src/util/performancetimer.cpp


# Soundtouch
INCLUDEPATH += $$BASE_DIR/lib/soundtouch-1.6.0
SOURCES += $$BASE_DIR/src/engine/enginebufferscalest.cpp \
    $$BASE_DIR/lib/soundtouch-1.6.0/SoundTouch.cpp \
    $$BASE_DIR/lib/soundtouch-1.6.0/TDStretch.cpp \
    $$BASE_DIR/lib/soundtouch-1.6.0/RateTransposer.cpp \
    $$BASE_DIR/lib/soundtouch-1.6.0/AAFilter.cpp \
    $$BASE_DIR/lib/soundtouch-1.6.0/FIFOSampleBuffer.cpp \
    $$BASE_DIR/lib/soundtouch-1.6.0/FIRFilter.cpp \
    $$BASE_DIR/lib/soundtouch-1.6.0/PeakFinder.cpp \
    $$BASE_DIR/lib/soundtouch-1.6.0/BPMDetect.cpp \
    $$BASE_DIR/lib/soundtouch-1.6.0/cpu_detect_x86_gcc.cpp

# Fidlib
SOURCES += $$BASE_DIR/lib/fidlib-0.9.10/fidlib.c
win32-g++ {
    DEFINES += T_MINGW
}
!win32-g++ {
    DEFINES += T_LINUX
}

# ReplayGain

SOURCES += $$BASE_DIR/lib/replaygain/replaygain.cpp

FORMS += \
$$BASE_DIR/src/controllers/dlgcontrollerlearning.ui \
$$BASE_DIR/src/controllers/dlgprefcontrollerdlg.ui \
$$BASE_DIR/src/controllers/dlgprefmappablecontrollerdlg.ui \
$$BASE_DIR/src/controllers/dlgprefnocontrollersdlg.ui \
$$BASE_DIR/src/dlgaboutdlg.ui \
$$BASE_DIR/src/dlgautodj.ui \
$$BASE_DIR/src/dlghidden.ui \
$$BASE_DIR/src/dlgmissing.ui \
$$BASE_DIR/src/dlgplugindownloader.ui \
$$BASE_DIR/src/dlgprefbeatsdlg.ui \
$$BASE_DIR/src/dlgprefcontrolsdlg.ui \
$$BASE_DIR/src/dlgprefcrossfaderdlg.ui \
$$BASE_DIR/src/dlgprefeqdlg.ui \
$$BASE_DIR/src/dlgpreferencesdlg.ui \
$$BASE_DIR/src/dlgprefnovinyldlg.ui \
$$BASE_DIR/src/dlgprefplaylistdlg.ui \
$$BASE_DIR/src/dlgprefreplaygaindlg.ui \
$$BASE_DIR/src/dlgprefsounddlg.ui \
$$BASE_DIR/src/dlgprefsounditem.ui \
$$BASE_DIR/src/dlgprefvinyldlg.ui \
$$BASE_DIR/src/dlgprepare.ui \
$$BASE_DIR/src/dlgrecording.ui \
$$BASE_DIR/src/dlgtrackinfo.ui


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
    CONFIG(portmidi) {
        LIBS += -lportmidi_s
    }
    LIBS += \
        -L$$BASE_DIR/../mixxx-mingw/lib -lFLAC -logg -lvorbis \
        -lvorbisenc -lvorbisfile
    CONFIG(mad) {
        LIBS += -lmad -lid3tag
    }
    LIBS += \
        -lz -lprotobuf-lite -lsndfile \
        -lportaudio.dll -ltag.dll -lwinmm -lws2_32 -lmingw32
    INCLUDEPATH += $$BASE_DIR/../mixxx-mingw/include
}

CONFIG(portmidi) {
    DEFINES += __PORTMIDI__
    HEADERS += \
        $$BASE_DIR/src/controllers/midi/portmidicontroller.h \
        $$BASE_DIR/src/controllers/midi/portmidienumerator.h
    SOURCES += \
        $$BASE_DIR/src/controllers/midi/portmidienumerator.cpp \
        $$BASE_DIR/src/controllers/midi/portmidicontroller.cpp
}

CONFIG(ladspa) {
    DEFINES += __LADSPA__
    HEADERS += $$BASE_DIR/src/engine/engineladspa.h \
        $$BASE_DIR/src/dlgladspa.h \
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
        $$BASE_DIR/src/ladspaview.h
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
INCLUDEPATH += $$BASE_DIR/lib/vamp-2.3
HEADERS +=
    $$BASE_DIR/src/vamp/vamppluginloader.h \
    $$BASE_DIR/src/dlgprefbeats.h \
    $$BASE_DIR/src/vamp/vampanalyser.h \
    $$UI_DIR/ui_dlgprefbeatsdlg.h \
    $$BASE_DIR/src/analyservamptest.h \
    $$BASE_DIR/src/analyservampkeytest.h \
    $$BASE_DIR/lib/vamp/vamp/vamp.h \
    $$BASE_DIR/lib/vamp/vamp-hostsdk/hostguard.h
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
FORMS += \
    $$BASE_DIR/src/dlgprefbeatsdlg.ui
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

CONFIG(mad) {
    DEFINES += __MAD__
    HEADERS += \
        $$BASE_DIR/src/soundsourcemp3.h
    SOURCES += \
        $$BASE_DIR/src/soundsourcemp3.cpp
}
CONFIG(vinylcontrol) {
    DEFINES += __VINYLCONTROL__
    HEADERS += \
        $$BASE_DIR/lib/xwax/timecoder.h \
        $$BASE_DIR/src/dlgprefnovinyl.h \
        $$BASE_DIR/src/dlgprefvinyl.h \
        $$BASE_DIR/src/engine/enginevinylsoundemu.h \
        $$BASE_DIR/src/engine/vinylcontrolcontrol.h \
        $$BASE_DIR/src/vinylcontrol/steadypitch.h \
        $$BASE_DIR/src/vinylcontrol/vinylcontrol.h \
        $$BASE_DIR/src/vinylcontrol/vinylcontrolmanager.h \
        $$BASE_DIR/src/vinylcontrol/vinylcontrolproxy.h \
        $$BASE_DIR/src/vinylcontrol/vinylcontrolsignalwidget.h \
        $$BASE_DIR/src/vinylcontrol/vinylcontrolxwax.h
    SOURCES += \
        $$BASE_DIR/src/vinylcontrol/vinylcontrol.cpp \
        $$BASE_DIR/src/vinylcontrol/vinylcontrolproxy.cpp \
        $$BASE_DIR/src/vinylcontrol/vinylcontrolxwax.cpp \
        $$BASE_DIR/src/dlgprefvinyl.cpp \
        $$BASE_DIR/src/vinylcontrol/vinylcontrolsignalwidget.cpp \
        $$BASE_DIR/src/vinylcontrol/vinylcontrolmanager.cpp \
        $$BASE_DIR/src/vinylcontrol/steadypitch.cpp \
        $$BASE_DIR/src/engine/vinylcontrolcontrol.cpp

    INCLUDEPATH += $$BASE_DIR/lib/xwax
    win32:SOURCES += $$BASE_DIR/lib/xwax/timecoder_win32.cpp
    win32:SOURCES += $$BASE_DIR/lib/xwax/lut.cpp
    !win32:SOURCES += $$BASE_DIR/lib/xwax/timecoder.c
    !win32:SOURCES += $$BASE_DIR/lib/xwax/lut.c
}
!CONFIG(hifieq):CXXFLAGS += -D__LOFI__ \
    -D__NO_INTTYPES__
CONFIG(shoutcast) {
    DEFINES += __SHOUTCAST__
    HEADERS += $$BASE_DIR/src/dlgprefshoutcast.h \
        $$BASE_DIR/src/recording/encodermp3.h \
        $$BASE_DIR/src/recording/encodervorbis.h \
        $$BASE_DIR/src/engine/engineshoutcast.h \
        $$BASE_DIR/src/shoutcast/defs_shoutcast.h
    SOURCES += $$BASE_DIR/src/dlgprefshoutcast.cpp \
        $$BASE_DIR/src/recording/encodervorbis.cpp \
        $$BASE_DIR/src/engine/engineshoutcast.cpp
    LIBS += shout \
        vorbisenc
    FORMS += $$BASE_DIR/src/dlgprefshoutcastdlg.ui
}

# CONFIG(record) {
#    DEFINES += __RECORD__
#    HEADERS += $$BASE_DIR/src/recording/defs_recording.h \
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

CONFIG(hid) {
    HIDAPI_INTERNAL_PATH = $$BASE_DIR/../mixxx-winlib/lib/hidapi-0.8.0-pre
    DEFINES += __HID__
    SOURCES += \
        $$BASE_DIR/src/controllers/hid/hidcontroller.cpp \
        $$BASE_DIR/src/controllers/hid/hidenumerator.cpp \
        $$BASE_DIR/src/controllers/hid/hidcontrollerpresetfilehandler.cpp
    HEADERS += \
        $$BASE_DIR/src/controllers/hid/hidblacklist.h \
        $$BASE_DIR/src/controllers/hid/hidcontroller.h \
        $$BASE_DIR/src/controllers/hid/hidcontrollerpreset.h \
        $$BASE_DIR/src/controllers/hid/hidcontrollerpresetfilehandler.h \
        $$BASE_DIR/src/controllers/hid/hidenumerator.h
    win32 {
        SOURCES += $$HIDAPI_INTERNAL_PATH/windows/hid.c
    } macx {
        SOURCES += $$HIDAPI_INTERNAL_PATH/mac/hid.c
    } else {
        SOURCES += $$HIDAPI_INTERNAL_PATH/linux/hid-libusb.c
    }
}

CONFIG(bulk) {
    DEFINES += __BULK__
    SOURCES += \
        $$BASE_DIR/src/controllers/bulk/bulkcontroller.cpp \
        $$BASE_DIR/src/controllers/bulk/bulkenumerator.cpp
    HEADERS += \
        $$BASE_DIR/src/controllers/bulk/bulkcontroller.h \
        $$BASE_DIR/src/controllers/bulk/bulkenumerator.h \
        $$BASE_DIR/src/controllers/bulk/bulksupported.h
    !CONFIG(hid) {
        SOURCES += \
        $$BASE_DIR/src/controllers/hid/hidcontrollerpresetfilehandler.cpp
    }
}

CONFIG(PromoTracks) {
    DEFINES += __PROMO__
    HEADERS += \
        $$BASE_DIR/src/library/promotracksfeature.h \
        $$BASE_DIR/src/library/bundledsongswebview.h \
        $$BASE_DIR/src/library/featuredartistswebview.h
    SOURCES += \
        $$BASE_DIR/src/library/promotracksfeature.cpp \
        $$BASE_DIR/src/library/bundledsongswebview.cpp \
        $$BASE_DIR/src/library/featuredartistswebview.cpp
}

CONFIG(hss1394) {
    HEADERS += \
        $$BASE_DIR/src/controllers/midi/hss1394controller.h \
        $$BASE_DIR/src/controllers/midi/hss1394enumerator.h
}

# Copy Windows dependencies to DESTDIR.
win32 {
    !exists($$DESTDIR):system( mkdir \"$$replace(DESTDIR, /,$$DIR_SEPARATOR)\" )
    # MinGW run-time
    DLLs += $$(QTDIR)/../mingw/bin/mingwm10.dll \
        $$(QTDIR)/../mingw/bin/libstdc++-6.dll \
        $$(QTDIR)/../mingw/bin/libexpat-1.dll
    CONFIG(m4a): DLLs += $$BASE_DIR/../mixxx-winlib/mp4v2/mingw-bin/libmp4v2-0.dll \
        $$BASE_DIR/../mixxx-winlib/libfaad2.dll
    # Qt4 libraries
    debug {
        DLLs += $$(QTDIR)/bin/QtCored4.dll \
            $$(QTDIR)/bin/QtGuid4.dll \
            $$(QTDIR)/bin/QtNetworkd4.dll \
            $$(QTDIR)/bin/QtSqld4.dll \
            $$(QTDIR)/bin/QtXmld4.dll \
            $$(QTDIR)/bin/QtOpenGLd4.dll \
            $$(QTDIR)/bin/QtScriptd4.dll
        # include GNU Debugger in debug distros
        DLLs += $$(QTDIR)/../mingw/bin/gdb.exe
    } else {
        DLLs += $$(QTDIR)/bin/QtCore4.dll \
            $$(QTDIR)/bin/QtGui4.dll \
            $$(QTDIR)/bin/QtNetwork4.dll \
            $$(QTDIR)/bin/QtSql4.dll \
            $$(QTDIR)/bin/QtXml4.dll \
            $$(QTDIR)/bin/QtOpenGL4.dll \
            $$(QTDIR)/bin/QtScript4.dll
    }
    # mixxx-mingw DLLs
    DLLs += \
        $$BASE_DIR/../mixxx-mingw/lib/libogg-0.dll \
        $$BASE_DIR/../mixxx-mingw/lib/libportaudio-2.dll \
        $$BASE_DIR/../mixxx-mingw/lib/libprotobuf-lite-8.dll \
        $$BASE_DIR/../mixxx-mingw/lib/libsndfile-1.dll \
        $$BASE_DIR/../mixxx-mingw/lib/libtag.dll \
        $$BASE_DIR/../mixxx-mingw/lib/libvorbis-0.dll \
        $$BASE_DIR/../mixxx-mingw/lib/libvorbisfile-3.dll

    # check if DLL exists at target, if not copy it there
    for(DLL, DLLs):!exists( $$DESTDIR/$$basename(DLL) ) {
        message( copying \"$$replace(DLL, /,$$DIR_SEPARATOR)\" -> \"$$DESTDIR\" ... )
        system( $$QMAKE_COPY \"$$replace(DLL, /,$$DIR_SEPARATOR)\" \"$$DESTDIR\" )
    }
    # create DESTDIR\testrun-mixxx.cmd to run mixxx using the workspace resource files.
    message ( Creating testrun-mixxx.cmd at \"$${PWD}$${DIR_SEPARATOR}$$replace(DESTDIR, /,$${DIR_SEPARATOR})$${DIR_SEPARATOR}testrun-$${TARGET}.cmd\" )
    system( echo $$TARGET --resourcePath \"$$replace(PWD, /,$${DIR_SEPARATOR})$${DIR_SEPARATOR}res\">\"$${PWD}$${DIR_SEPARATOR}$$replace(DESTDIR, /,$${DIR_SEPARATOR})$${DIR_SEPARATOR}testrun-$${TARGET}.cmd\" )
}

exists( .bzr ) {
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
    VCS_BRANCH_NAME = $${BZR_BRANCH_NAME}
    VCS_REVNO = $${BZR_REVNO}
}

exists ( .git ) {
    # Get info from git about the current branch
    GIT_REVNO = $$system( git log --pretty=oneline --first-parent | wc -l )
    GIT_BRANCH_NAME = $$system( git branch | grep \* | sed -e "s/\* //;" )
    message(BRANCH_NAME is $$GIT_BRANCH_NAME)
    message(REVISION is $$GIT_REVNO)
    VCS_BRANCH_NAME = $${GIT_BRANCH_NAME}
    VCS_REVNO = $${GIT_REVNO}
}

win32 {
    # Makefile target to build an NSIS Installer...
    # TODO: either fix this to work in a cross-compile or make a seperate cross-compile NSIS target
    # CMD Usage: C:/Qt/QtCreator/mingw/bin/mingw32-make -f Makefile.Debug nsis
    # SH Usage: make -f Makefile.Debug nsis
    nsis.target = nsis
    exists($$BUILDDIR/gdb.exe):INCLUDE_GDB = -DINCLUDE_GDB
    nsis.commands = \"$$(PROGRAMFILES)\\\\NSIS\\\\makensis.exe\" -NOCD -DGCC -DBINDIR=\"$$BUILDDIR\" -DBUILD_REV=\"$$VCS_BRANCH_NAME-$$VCS_REVNO\" $$INCLUDE_GDB build\\\\nsis\\\\Mixxx.nsi
    # nsis.depends =
    QMAKE_EXTRA_TARGETS += nsis
}

# build.h
BUILD_REV = $${VCS_BRANCH_NAME} : $${VCS_REVNO}
isEmpty( BUILD_REV ):BUILD_REV = Killroy was here
BUILD_REV += - built via qmake/Qt Creator
message( Generating src$${DIR_SEPARATOR}build.h with contents: $${LITERAL_HASH}define BUILD_REV '"'$$BUILD_REV'"' )
system( echo $${LITERAL_HASH}define BUILD_REV '"'$$BUILD_REV'"'>src$${DIR_SEPARATOR}build.h )
system( echo $${LITERAL_HASH}define BUILD_FLAGS '"'$$replace(DEFINES,__,)'"'>>src$${DIR_SEPARATOR}build.h )

PROTOS += \
    src/proto/waveform.proto \
    src/proto/skin.proto \
    src/proto/beats.proto
include(protobuf.pri)
