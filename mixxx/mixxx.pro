CONFIG += debug link_pkgconfig ladspa alsaseqmidi script vinylcontrol
DEFINES += QMAKE \ # define QMAKE for not-SCons specific ifdefs like ui_scriptstudio.h
    __PORTAUDIO__ \
    __SNDFILE__ \
    SETTINGS_FILE=\\\"mixxx.cfg\\\" \
    BPMSCHEME_FILE=\\\"mixxxbpmscheme.xml\\\" \
    TRACK_FILE=\\\"mixxxtrack.xml\\\"

win32-g++ {
  QMAKE_CXXFLAGS += "\"-DSETTINGS_PATH=\\\"Local\ Settings/Application\ Data/Mixxx/\\\"\"" # Bit ugly, but you can thank MS-DOS shell for f-ing up the normal way of parsing.
} else {
  win32 { # i586-mingw32msvc-g++ -- cross compiling
    DEFINES += "SETTINGS_PATH=\\\"Local\ Settings/Application\ Data/Mixxx/\\\""
  } else {
    DEFINES += SETTINGS_PATH=\\\".mixxx/\\\"
  }
}

TEMPLATE = app
TARGET = mixxx
QT += core \
    gui \
    sql \
    xml \
    network \
    svg \
    opengl \
    script \
    qt3support

unix {
  DESTDIR = bin
  win32 { # This should only happen when cross compiling...
    DESTDIR = bin-win32
  }
} else {
    DESTDIR = bin
}

BUILDDIR = $$DESTDIR
UI_DIR = $$BUILDDIR/ui
RCC_DIR = $$BUILDDIR/rcc
MOC_DIR = $$BUILDDIR/moc
OBJECTS_DIR = $$BUILDDIR/obj

CONFIG(debug) { # gdbmacros is required for inspecting Qt datatypes using gdb within QtC
    exists($$(QTDIR)/../share/qtcreator/gdbmacros/gdbmacros.cpp) {
        message(found gdbmacros.cpp relative to QTDIR)
        SOURCES += $$(QTDIR)/../share/qtcreator/gdbmacros/gdbmacros.cpp
    } else {
        exists($$(HOME)/qtcreator-1.0.0/share/qtcreator/gdbmacros/gdbmacros.cpp) {
            message(found gdbmacros.cpp relative to $$(HOME)/qtcreator-1.0.0)
            SOURCES += $$(HOME)/qtcreator-1.0.0/share/qtcreator/gdbmacros/gdbmacros.cpp
        }
    }
}

HEADERS += $$UI_DIR/ui_dlgaboutdlg.h \
    $$UI_DIR/ui_dlgbpmschemedlg.h \
    $$UI_DIR/ui_dlgbpmtapdlg.h \
    $$UI_DIR/ui_dlgmidilearning.h \
    $$UI_DIR/ui_dlgprefbpmdlg.h \
    $$UI_DIR/ui_dlgprefcontrolsdlg.h \
    $$UI_DIR/ui_dlgprefcrossfaderdlg.h \
    $$UI_DIR/ui_dlgprefeqdlg.h \
    $$UI_DIR/ui_dlgpreferencesdlg.h \
    $$UI_DIR/ui_dlgprefmidibindingsdlg.h \
#    $$UI_DIR/ui_dlgprefmididevicedlg.h \
#    $$UI_DIR/ui_dlgprefmididlg.h \
    $$UI_DIR/ui_dlgprefplaylistdlg.h \
    $$UI_DIR/ui_dlgprefrecorddlg.h \
    $$UI_DIR/ui_dlgprefshoutcastdlg.h \
    $$UI_DIR/ui_dlgprefsounddlg.h \
    $$UI_DIR/ui_dlgprefvinyldlg.h \
    $$UI_DIR/ui_dlgprefnomididlg.h

INCLUDEPATH += src \
#    lib/soundtouch \
    lib/kissfft \
	lib/ladspa \
	/sw/include \
    $$UI_DIR
HEADERS += src/analyser.h \
    src/analyserbpm.h \
    src/analyserqueue.h \
    src/analyserwaveform.h \
    src/analyserwavesummary.h \
    src/configobject.h \
    src/controlbeat.h \
    src/controlevent.h \
    src/engine/engineabstractrecord.h \
    src/engine/enginebuffer.h \
    src/engine/enginebuffercue.h \
    src/engine/enginebufferscale.h \
    src/engine/enginebufferscaledummy.h \
    src/engine/enginebufferscalelinear.h \
    src/engine/enginebufferscalereal.h \
#    src/engine/enginebufferscalesrc.h \
    src/engine/enginebufferscalest.h \
    src/engine/enginechannel.h \
    src/engine/engineclipping.h \
    src/engine/enginedelay.h \
    src/engine/enginefilter.h \
    src/engine/enginefilterblock.h \
    src/engine/enginefilterbutterworth8.h \
    src/engine/enginefilteriir.h \
    src/engine/enginefilterrbj.h \
    src/engine/engineflanger.h \
    src/engine/enginemaster.h \
    src/engine/engineobject.h \
    src/engine/enginepregain.h \
    src/engine/enginesidechain.h \
#    src/engine/enginespectralback.h \
    src/engine/enginespectralfwd.h \
    src/engine/enginetemporal.h \
    src/engine/enginevinylsoundemu.h \
    src/engine/enginevolume.h \
    src/engine/enginevumeter.h \
    src/engine/enginexfader.h \
    src/waveform/glwaveformrenderer.h \
    src/waveform/renderobject.h \
    src/waveform/waveformrenderbackground.h \
    src/waveform/waveformrenderbeat.h \
    src/waveform/waveformrenderer.h \
    src/waveform/waveformrendermark.h \
    src/waveform/waveformrendersignal.h \
    src/waveform/waveformrendersignalpixmap.h \
    src/widget/wabstractcontrol.h \
#    src/widget/wcombobox.h \
    src/widget/wdisplay.h \
    src/widget/wglwaveformviewer.h \
    src/widget/wknob.h \
    src/widget/wlabel.h \
    src/widget/wnumber.h \
    src/widget/wnumberbpm.h \
    src/widget/wnumberpos.h \
    src/widget/wnumberrate.h \
    src/widget/woverview.h \
    src/widget/wpixmapstore.h \
    src/widget/wpushbutton.h \
    src/widget/wsearchlineedit.h \
    src/widget/wskincolor.h \
    src/widget/wslider.h \
    src/widget/wslidercomposed.h \
    src/widget/wstatuslight.h \
#    src/widget/wvinylcontrolindicator.h \
    src/widget/wvisualsimple.h \
    src/widget/wvumeter.h \
    src/widget/wwaveformviewer.h \
    src/widget/wwidget.h \
#    src/bpm/bpmdetect.h \
#    src/bpm/bpmdetector.h \
    src/bpm/bpmreceiver.h \
    src/bpm/bpmscheme.h \
#    src/bpm/peakfinder.h \
    src/bpm/wavesegmentation.h \
    src/controlgroupdelegate.h \
    src/controlvaluedelegate.h \
    src/controllogpotmeter.h \
    src/controlnull.h \
    src/controlobject.h \
    src/controlobjectthread.h \
    src/controlobjectthreadmain.h \
    src/controlobjectthreadwidget.h \
    src/controlpotmeter.h \
    src/controlpushbutton.h \
#    src/controlrotary.h \
    src/controlttrotary.h \
    src/defs.h \
    src/defs_audiofiles.h \
    src/defs_mixxxcmetrics.h \
    src/defs_promo.h \
    src/defs_urls.h \
    src/dlgabout.h \
    src/dlgbpmscheme.h \
    src/dlgbpmtap.h \
    src/dlgmidilearning.h \
    src/dlgprefbpm.h \
    src/dlgprefcontrols.h \
    src/dlgprefcrossfader.h \
    src/dlgprefeq.h \
    src/dlgpreferences.h \
    src/dlgprefmidibindings.h \
#    src/dlgprefmididevice.h \
    src/dlgprefnomidi.h \
    src/dlgprefplaylist.h \
    src/dlgprefsound.h \
    src/fakemonitor.h \
#    src/hercules.h \
#    src/herculeslinux.h \
    src/imgcolor.h \
    src/imginvert.h \
    src/imgloader.h \
    src/imgsource.h \
    src/input.h \
    src/libraryscanner.h \
    src/libraryscannerdlg.h \
    src/mathstuff.h \
    src/midichanneldelegate.h \
    src/mididevicehandler.h \
#    src/midiinputmapping.h \
    src/midiinputmappingtablemodel.h \
    src/midiledhandler.h \
    src/midimapping.h \
    src/midimessage.h \
    src/midinodelegate.h \
    src/midiobject.h \
    src/midiobjectnull.h \
    src/midioutputmapping.h \
	src/midioutputmappingtablemodel.h \
#    src/miditypedelegate.h \
    src/midistatusdelegate.h \
    src/mixxx.h \
    src/mixxxcontrol.h \
    src/mixxxkeyboard.h \
    src/mixxxview.h \
    src/monitor.h \
#    src/mouse.h \
#    src/mouselinux.h \
#    src/mousewin.h \
    src/parser.h \
    src/parserm3u.h \
    src/parserpls.h \
    src/peaklist.h \
    src/playerinfo.h \
#    src/powermate.h \
#    src/powermatelinux.h \
#    src/powermatewin.h \
    src/probabilityvector.h \
    src/proxymodel.h \
    src/reader.h \
    src/readerevent.h \
    src/readerextract.h \
#    src/readerextractbeat.h \
#     src/readerextractfft.h \
#     src/readerextracthfc.h \
    src/readerextractwave.h \
    src/rotary.h \
    src/rtthread.h \
    src/segmentation.h \
    src/sounddevice.h \
    src/sounddeviceportaudio.h \
    src/soundmanager.h \
    src/soundsource.h \
    src/soundsourcemp3.h \
    src/soundsourceoggvorbis.h \
    src/soundsourceproxy.h \
    src/soundsourcesndfile.h \
    src/track.h \
    src/trackcollection.h \
    src/trackimporter.h \
    src/trackinfoobject.h \
    src/trackplaylist.h \
    src/trackplaylistlist.h \
    src/waveformviewerfactory.h \
    src/windowkaiser.h \
    src/wipodtracksmodel.h \
    src/wplaylistlistmodel.h \
    src/wpromotracksmodel.h \
    src/wtracktablefilter.h \
    src/wtracktablemodel.h \
    src/wtracktableview.h \
    src/xmlparse.h \
    src/errordialog.h

SOURCES += src/analyserbpm.cpp \
    src/analyserqueue.cpp \
    src/analyserwaveform.cpp \
    src/analyserwavesummary.cpp \
    src/configobject.cpp \
    src/controlbeat.cpp \
    src/controlevent.cpp \
    src/engine/enginebuffer.cpp \
    src/engine/enginebuffercue.cpp \
    src/engine/enginebufferscale.cpp \
    src/engine/enginebufferscaledummy.cpp \
    src/engine/enginebufferscalelinear.cpp \
    src/engine/enginebufferscalereal.cpp \
#    src/engine/enginebufferscalesrc.cpp \
    src/engine/enginebufferscalest.cpp \
    src/engine/enginechannel.cpp \
    src/engine/engineclipping.cpp \
    src/engine/enginedelay.cpp \
    src/engine/enginefilter.cpp \
    src/engine/enginefilterblock.cpp \
    src/engine/enginefilterbutterworth8.cpp \
    src/engine/enginefilteriir.cpp \
    src/engine/enginefilterrbj.cpp \
    src/engine/engineflanger.cpp \
    src/engine/enginemaster.cpp \
    src/engine/engineobject.cpp \
    src/engine/enginepregain.cpp \
    src/engine/enginesidechain.cpp \
#    src/engine/enginespectralback.cpp \
    src/engine/enginespectralfwd.cpp \
    src/engine/enginetemporal.cpp \
    src/engine/enginevinylsoundemu.cpp \
    src/engine/enginevolume.cpp \
    src/engine/enginevumeter.cpp \
    src/engine/enginexfader.cpp \
    src/waveform/glwaveformrenderer.cpp \
    src/waveform/renderobject.cpp \
    src/waveform/waveformrenderbackground.cpp \
    src/waveform/waveformrenderbeat.cpp \
    src/waveform/waveformrenderer.cpp \
    src/waveform/waveformrendermark.cpp \
    src/waveform/waveformrendersignal.cpp \
    src/waveform/waveformrendersignalpixmap.cpp \
    src/widget/wabstractcontrol.cpp \
#    src/widget/wcombobox.cpp \
    src/widget/wdisplay.cpp \
    src/widget/wglwaveformviewer.cpp \
    src/widget/wknob.cpp \
    src/widget/wlabel.cpp \
    src/widget/wnumber.cpp \
    src/widget/wnumberbpm.cpp \
    src/widget/wnumberpos.cpp \
    src/widget/wnumberrate.cpp \
    src/widget/woverview.cpp \
    src/widget/wpixmapstore.cpp \
    src/widget/wpushbutton.cpp \
    src/widget/wsearchlineedit.cpp \
    src/widget/wskincolor.cpp \
    src/widget/wslider.cpp \
    src/widget/wslidercomposed.cpp \
    src/widget/wstatuslight.cpp \
#    src/widget/wvinylcontrolindicator.cpp \
    src/widget/wvisualsimple.cpp \
    src/widget/wvumeter.cpp \
    src/widget/wwaveformviewer.cpp \
    src/widget/wwidget.cpp \
#    src/bpm/bpmdetect.cpp \
#    src/bpm/bpmdetector.cpp \
    src/bpm/bpmscheme.cpp \
#    src/bpm/peakfinder.cpp \
    src/bpm/wavesegmentation.cpp \
    src/controlgroupdelegate.cpp \
    src/controlvaluedelegate.cpp \
    src/controllogpotmeter.cpp \
    src/controlnull.cpp \
    src/controlobject.cpp \
    src/controlobjectthread.cpp \
    src/controlobjectthreadmain.cpp \
    src/controlobjectthreadwidget.cpp \
    src/controlpotmeter.cpp \
    src/controlpushbutton.cpp \
#    src/controlrotary.cpp \
    src/controlttrotary.cpp \
    src/dlgabout.cpp \
    src/dlgbpmscheme.cpp \
    src/dlgbpmtap.cpp \
    src/dlgmidilearning.cpp \
    src/dlgprefbpm.cpp \
    src/dlgprefcontrols.cpp \
    src/dlgprefcrossfader.cpp \
    src/dlgprefeq.cpp \
    src/dlgpreferences.cpp \
    src/dlgprefmidibindings.cpp \
#    src/dlgprefmididevice.cpp \
    src/dlgprefnomidi.cpp \
    src/dlgprefplaylist.cpp \
    src/dlgprefsound.cpp \
    src/fakemonitor.cpp \
#    src/hercules.cpp \
#    src/herculeslinux.cpp \
#    src/herculeslinuxlegacy.cpp \
    src/imgcolor.cpp \
    src/imginvert.cpp \
    src/imgloader.cpp \
    src/input.cpp \
    src/libraryscanner.cpp \
    src/libraryscannerdlg.cpp \
    src/mathstuff.cpp \
    src/midichanneldelegate.cpp \
    src/mididevicehandler.cpp \
#    src/midiinputmapping.cpp \
    src/midiinputmappingtablemodel.cpp \
    src/midiledhandler.cpp \
    src/midimapping.cpp \
    src/midimessage.cpp \
    src/midinodelegate.cpp \
    src/midiobject.cpp \
    src/midiobjectnull.cpp \
	src/midioutputmappingtablemodel.cpp \
#    src/miditypedelegate.cpp \
    src/midistatusdelegate.cpp \
    src/mixxx.cpp \
    src/mixxxcontrol.cpp \
    src/mixxxkeyboard.cpp \
    src/mixxxview.cpp \
    src/monitor.cpp \
#    src/mouse.cpp \
#    src/mouselinux.cpp \
#    src/mousewin.cpp \
    src/parser.cpp \
    src/parserm3u.cpp \
    src/parserpls.cpp \
    src/peaklist.cpp \
    src/playerinfo.cpp \
#    src/powermate.cpp \
#    src/powermatelinux.cpp \
#    src/powermatewin.cpp \
    src/probabilityvector.cpp \
    src/proxymodel.cpp \
    src/reader.cpp \
    src/readerevent.cpp \
    src/readerextract.cpp \
#    src/readerextractbeat.cpp \
#    src/readerextractfft.cpp \
#    src/readerextracthfc.cpp \
    src/readerextractwave.cpp \
    src/rotary.cpp \
    src/rtthread.cpp \
    src/segmentation.cpp \
    src/sounddevice.cpp \
    src/sounddeviceportaudio.cpp \
    src/soundmanager.cpp \
    src/soundsource.cpp \
    src/soundsourcemp3.cpp \
    src/soundsourceoggvorbis.cpp \
    src/soundsourceproxy.cpp \
    src/soundsourcesndfile.cpp \
    src/track.cpp \
    src/trackcollection.cpp \
    src/trackimporter.cpp \
    src/trackinfoobject.cpp \
    src/trackplaylist.cpp \
    src/trackplaylistlist.cpp \
    src/waveformviewerfactory.cpp \
    src/windowkaiser.cpp \
    src/wipodtracksmodel.cpp \
    src/wplaylistlistmodel.cpp \
    src/wpromotracksmodel.cpp \
    src/wtracktablefilter.cpp \
    src/wtracktablemodel.cpp \
    src/wtracktableview.cpp \
    src/xmlparse.cpp \
    src/main.cpp \
    src/errordialog.cpp

# Soundtouch
INCLUDEPATH += lib/soundtouch-1.4.1
SOURCES += lib/soundtouch-1.4.1/SoundTouch.cpp \
    lib/soundtouch-1.4.1/TDStretch.cpp \
    lib/soundtouch-1.4.1/RateTransposer.cpp \
    lib/soundtouch-1.4.1/AAFilter.cpp \
    lib/soundtouch-1.4.1/FIFOSampleBuffer.cpp \
    lib/soundtouch-1.4.1/FIRFilter.cpp \
    lib/soundtouch-1.4.1/PeakFinder.cpp \
    lib/soundtouch-1.4.1/BPMDetect.cpp \
    lib/soundtouch-1.4.1/cpu_detect_x86_gcc.cpp

# Fidlib
SOURCES += lib/fidlib-0.9.9/fidlib.c
win32-g++ {
    DEFINES += T_MINGW
}
!win32-g++ {
    DEFINES += T_LINUX
}

# kissfft
SOURCES += lib/kissfft/kiss_fft.c

FORMS += src/dlgaboutdlg.ui \
    src/dlgbpmschemedlg.ui \
    src/dlgbpmtapdlg.ui \
    src/dlgmidilearning.ui \
    src/dlgprefbpmdlg.ui \
    src/dlgprefcontrolsdlg.ui \
    src/dlgprefcrossfaderdlg.ui \
    src/dlgprefeqdlg.ui \
    src/dlgpreferencesdlg.ui \
    src/dlgprefmidibindingsdlg.ui \
#    src/dlgprefmididevicedlg.ui \
#    src/dlgprefmididlg.ui \
    src/dlgprefplaylistdlg.ui \
    src/dlgprefsounddlg.ui \
    src/dlgprefvinyldlg.ui \
    src/dlgprefnomididlg.ui

RESOURCES += res/mixxx.qrc
HEADERS += src/recording/defs_recording.h \
    src/recording/enginerecord.h \
    src/recording/writeaudiofile.h \
    src/dlgprefrecord.h
SOURCES += src/recording/enginerecord.cpp \
    src/recording/writeaudiofile.cpp \
    src/dlgprefrecord.cpp
FORMS += src/dlgprefrecorddlg.ui
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
        CONFIG(alsaseqmidi) {
            DEFINES += __ALSASEQMIDI__
            HEADERS += src/midiobjectalsaseq.h
            SOURCES += src/midiobjectalsaseq.cpp
        }
        CONFIG(portmidi) {
            DEFINES += __PORTMIDI__
            HEADERS += src/midiobjectportmidi.h
            SOURCES += src/midiobjectportmidi.cpp
        }
        CONFIG(ossmidi) {
            DEFINES += __OSSMIDI__
            HEADERS += src/midiobjectoss.h
            SOURCES += src/midiobjectoss.cpp
        }
        LIBS += -lasound
        PKGCONFIG += portaudio-2.0 \
            jack \
            id3tag \
            mad \
            vorbisfile \
            sndfile
    }
}
macx { 
    DEFINES += __COREMIDI__
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
    HEADERS += src/midiobjectcoremidi.h
    SOURCES += src/midiobjectcoremidi.cpp
}
win32 { 
    DEFINES += __WINMIDI__
    HEADERS += src/midiobjectwin.h
    SOURCES += src/midiobjectwin.cpp
    LIBS += ../mixxx-winlib/libsndfile/mingw-bin/libsndfile-1.dll \
#        ../mixxx-winlib/sndfile.dll \
#        ../mixxx-winlib/portaudio.dll \
        ../mixxx-winlib/portaudio-snapshot/mingw-bin/libportaudio-2.dll \
        ../mixxx-winlib/libmad.a \
        ../mixxx-winlib/libid3tag.a \
        ../mixxx-winlib/vorbisfile.dll \
        ../mixxx-winlib/vorbis.dll \
        ../mixxx-winlib/libfftw3-3.dll \
        ../mixxx-winlib/ogg.dll \
        -lwinmm
    INCLUDEPATH += ../mixxx-winlib
}
CONFIG(ladspa) { 
    DEFINES += __LADSPA__
    HEADERS += src/engine/engineladspa.h \
        src/dlgladspa.h \
        src/ladspaview.h \
        src/ladspa/ladspacontrol.h \
        src/ladspa/ladspainstance.h \
        src/ladspa/ladspainstancemono.h \
        src/ladspa/ladspainstancestereo.h \
        src/ladspa/ladspalibrary.h \
        src/ladspa/ladspaloader.h \
        src/ladspa/ladspaplugin.h \
        src/ladspa/ladspapreset.h \
        src/ladspa/ladspapresetinstance.h \
        src/ladspa/ladspapresetknob.h \
        src/ladspa/ladspapresetmanager.h \
        src/ladspa/ladspapresetslot.h
    SOURCES += src/engine/engineladspa.cpp \
        src/dlgladspa.cpp \
        src/ladspaview.cpp \
        src/ladspa/ladspacontrol.cpp \
        src/ladspa/ladspainstance.cpp \
        src/ladspa/ladspainstancemono.cpp \
        src/ladspa/ladspainstancestereo.cpp \
        src/ladspa/ladspalibrary.cpp \
        src/ladspa/ladspaloader.cpp \
        src/ladspa/ladspaplugin.cpp \
        src/ladspa/ladspapreset.cpp \
        src/ladspa/ladspapresetinstance.cpp \
        src/ladspa/ladspapresetknob.cpp \
        src/ladspa/ladspapresetmanager.cpp \
        src/ladspa/ladspapresetslot.cpp
    win32{
        INCLUDEPATH += lib\ladspa
    }
}
CONFIG(script) { 
    DEFINES += __MIDISCRIPT__
    HEADERS += src/script/interp.h \
        src/script/macro.h \
        src/script/macrolist.h \
        src/script/macrolistitem.h \
        src/script/midiscriptengine.h \
        src/script/numbercontrolevent.h \
        src/script/numberrecorder.h \
        src/script/playinterface.h \
        src/script/qtscriptinterface.h \
        src/script/recorder.h \
        src/script/scriptcontrolevent.h \
        src/script/scriptcontrolqueue.h \
        src/script/scriptengine.h \
        src/script/scriptrecorder.h \
        src/script/scriptstudio.h \
#        src/script/scripttest.h \
        src/script/sdatetime.h \
        src/script/signalrecorder.h \
        src/script/trackcontrolevent.h \
        src/script/trackrecorder.h \
        $$UI_DIR/ui_scriptstudio.h
    SOURCES += src/script/macro.cpp \
        src/script/macrolist.cpp \
        src/script/macrolistitem.cpp \
        src/script/midiscriptengine.cpp \
        src/script/numbercontrolevent.cpp \
        src/script/numberrecorder.cpp \
        src/script/playinterface.cpp \
        src/script/qtscriptinterface.cpp \
        src/script/recorder.cpp \
        src/script/scriptcontrolevent.cpp \
        src/script/scriptcontrolqueue.cpp \
        src/script/scriptengine.cpp \
        src/script/scriptrecorder.cpp \
        src/script/scriptstudio.cpp \
#        src/script/scripttest.cpp \
        src/script/sdatetime.cpp \
        src/script/signalrecorder.cpp \
        src/script/trackcontrolevent.cpp \
        src/script/trackrecorder.cpp
    FORMS += src/script/scriptstudio.ui
}
CONFIG(tonal) { 
    DEFINES += 
    HEADERS += src/tonal/ChordCorrelator.hxx \
        src/tonal/ChordExtractor.hxx \
        src/tonal/ChordSegmentator.hxx \
        src/tonal/CircularPeakPicking.hxx \
        src/tonal/CircularPeakTunner.hxx \
        src/tonal/CircularPeaksToPCP.hxx \
        src/tonal/ConstantQFolder.hxx \
        src/tonal/ConstantQTransform.hxx \
        src/tonal/DiscontinuousSegmentation.hxx \
        src/tonal/FourierTransform.hxx \
        src/tonal/InstantTunningEstimator.hxx \
        src/tonal/PCPSmother.hxx \
        src/tonal/Segmentation.hxx \
        src/tonal/SemitoneCenterFinder.hxx \
        src/tonal/TonalAnalysis.hxx \
        src/tonal/tonalanalyser.h
    SOURCES += src/tonal/ConstantQFolder.cxx \
        src/tonal/ConstantQTransform.cxx \
        src/tonal/FourierTransform.cxx \
        src/tonal/Segmentation.cxx \
        src/tonal/TonalAnalysis.cxx \
        src/tonal/tonalanalyser.cpp
}
CONFIG(m4a) { 
    DEFINES += __M4A__
    HEADERS += src/soundsourcem4a.h \
        src/m4a/comment.h \
        src/m4a/ip.h \
        src/m4a/sf.h
    SOURCES += src/soundsourcem4a.cpp \
        src/m4a/mp4-mixxx.cpp
    LIBS += libmp4v2 \
        libfaad
}
CONFIG(vinylcontrol) { 
    DEFINES += __VINYLCONTROL__
    HEADERS += src/vinylcontrol.h \
        src/vinylcontrolproxy.h \
        src/vinylcontrolscratchlib.h \
        src/vinylcontrolsignalwidget.h \
        src/vinylcontrolxwax.h \
        lib/scratchlib/DAnalyse.h \
        lib/xwax/timecoder.h \
        src/engine/enginevinylcontrol.h \
        src/dlgprefvinyl.h
    SOURCES += src/vinylcontrol.cpp \
        src/vinylcontrolproxy.cpp \
        src/vinylcontrolscratchlib.cpp \
        src/vinylcontrolsignalwidget.cpp \
        src/vinylcontrolxwax.cpp \
        lib/scratchlib/DAnalyse.cpp \
        src/engine/enginevinylcontrol.cpp \
        src/dlgprefvinyl.cpp

    INCLUDEPATH += lib/scratchlib \
        lib/xwax
    win32:SOURCES += lib/xwax/timecoder_win32.c
    !win32:SOURCES += lib/xwax/timecoder.c
}
CONFIG(cmetrics):DEFINES += __C_METRICS__ \
    client='MIXXX' \
    server='metrics.mixxx.org'
!CONFIG(hifieq):CXXFLAGS += -D__LOFI__ \
    -D__NO_INTTYPES__
CONFIG(shoutcast) {
    DEFINES += __SHOUTCAST__
    HEADERS += src/dlgprefshoutcast.h \
        src/encodervorbis.h \
        src/engine/engineshoutcast.h
    SOURCES += src/dlgprefshoutcast.cpp \
        src/encodervorbis.cpp \
        src/engine/engineshoutcast.cpp
    LIBS += shout \
        vorbisenc
    FORMS += src/dlgprefshoutcastdlg.ui
}

# CONFIG(record) {
#    DEFINES += __RECORD__
#    HEADERS += src/recording/defs_recording.h \
#        src/recording/enginerecord.h \
#        src/recording/writeaudiofile.h \
#        src/dlgprefrecord.h
#    SOURCES += src/recording/enginerecord.cpp \
#        src/recording/writeaudiofile.cpp \
#        src/dlgprefrecord.cpp
#    LIBS +=
#    FORMS += src/dlgprefrecorddlg.ui
#}

CONFIG(ffmpeg) {
    DEFINES += __FFMPEGFILE__
    HEADERS += src/soundsourceffmpeg.h
    SOURCES += src/soundsourceffmpeg.cpp
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
    !exists($$DESTDIR):system( mkdir $$DESTDIR )
    # MinGW run-time
    DLLs += $$(QTDIR)/../mingw/bin/mingwm10.dll
    # Qt4 libraries
    debug {
        DLLs += $$(QTDIR)/lib/Qt3Supportd4.dll \
            $$(QTDIR)/lib/QtCored4.dll \
            $$(QTDIR)/lib/QtGuid4.dll \
            $$(QTDIR)/lib/QtNetworkd4.dll \
            $$(QTDIR)/lib/QtSqld4.dll \
            $$(QTDIR)/lib/QtXmld4.dll \
            $$(QTDIR)/lib/QtOpenGLd4.dll \
            $$(QTDIR)/lib/QtScriptd4.dll
    } else {
        DLLs += $$(QTDIR)/lib/Qt3Support4.dll \
            $$(QTDIR)/lib/QtCore4.dll \
            $$(QTDIR)/lib/QtGui4.dll \
            $$(QTDIR)/lib/QtNetwork4.dll \
            $$(QTDIR)/lib/QtSql4.dll \
            $$(QTDIR)/lib/QtXml4.dll \
            $$(QTDIR)/lib/QtOpenGL4.dll \
            $$(QTDIR)/lib/QtScript4.dll
    }
    # mixxx-winlibs DLLs
    DLLs += ../mixxx-winlib/ogg.dll \
#        ../mixxx-winlib/portaudio.dll \
        ../mixxx-winlib/portaudio-snapshot/mingw-bin/libportaudio-2.dll \
        ../mixxx-winlib/libsndfile/mingw-bin/libsndfile-1.dll \
#        ../mixxx-winlib/sndfile.dll \
        ../mixxx-winlib/vorbis.dll \
        ../mixxx-winlib/vorbisfile.dll

    # check if DLL exists at target, if not copy it there
    for(DLL, DLLs):!exists( $$DESTDIR/$$basename(DLL) ) {
        message( copying "$$replace(DLL, /,$$DIR_SEPARATOR)" -> "$$DESTDIR" ... )
        system( $$QMAKE_COPY "$$replace(DLL, /,$$DIR_SEPARATOR)" "$$DESTDIR" )
    }
    # create DESTDIR\testrun-mixxx.cmd to run mixxx using the workspace resource files.
    win32-g++:system( echo $$TARGET --resourcePath $$replace(PWD, /,$${DIR_SEPERATOR})$${DIR_SEPERATOR}res>$${PWD}$${DIR_SEPERATOR}$$replace(DESTDIR, /,$${DIR_SEPERATOR})$${DIR_SEPERATOR}testrun-$${TARGET}.cmd )
}

# .mixxx_flags.svn -- Do this near the end so we capture all additions to the DEFINES variable
message( Generating .mixxx_flags.svn with contents: $${LITERAL_HASH}define BUILD_FLAGS '"'$$replace(DEFINES,__,)'"' )
system( echo $${LITERAL_HASH}define BUILD_FLAGS '"'$$replace(DEFINES,__,)'"'>.mixxx_flags.svn )

# .mixxx_version.svn
BUILD_REV = $$system( svnversion )
isEmpty( BUILD_REV ):BUILD_REV = Killroy was here
BUILD_REV += - built via qmake/Qt Creator
message( Generating .mixxx_version.svn with contents: $${LITERAL_HASH}define BUILD_REV '"'$$BUILD_REV'"' )
system( echo $${LITERAL_HASH}define BUILD_REV '"'$$BUILD_REV'"'>.mixxx_version.svn )
