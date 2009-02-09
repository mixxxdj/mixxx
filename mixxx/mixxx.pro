CONFIG += ladspa
DEFINES += __PORTAUDIO__ \
    __SNDFILE__
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
DESTDIR = bin
UI_DIR = bin\ui
RCC_DIR = bin\rcc
MOC_DIR = bin\moc
OBJECTS_DIR = bin\obj
INCLUDEPATH += src \
    lib/soundtouch
HEADERS += src/analyser.h \
    src/analyserbpm.h \
    src/analyserqueue.h \
    src/analyserwaveform.h \
    src/analyserwavesummary.h \
    src/configmidi.h \
    src/configobject.h \
    src/controlbeat.h \
    src/controlevent.h \
    src/controleventengine.h \
    src/engine/engineabstractrecord.h \
    src/engine/enginebuffer.h \
    src/engine/enginebuffercue.h \
    src/engine/enginebufferscale.h \
    src/engine/enginebufferscaledummy.h \
    src/engine/enginebufferscalelinear.h \
    src/engine/enginebufferscalereal.h \
    src/engine/enginebufferscalesrc.h \
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
    src/engine/engineladspa.h \
    src/engine/enginemaster.h \
    src/engine/engineobject.h \
    src/engine/enginepregain.h \
    src/engine/engineshoutcast.h \
    src/engine/enginesidechain.h \
    src/engine/enginespectralback.h \
    src/engine/enginespectralfwd.h \
    src/engine/enginetemporal.h \
    src/engine/enginevinylsoundemu.h \
    src/engine/enginevolume.h \
    src/engine/enginevumeter.h \
    src/engine/enginexfader.h \
    src/recording/defs_recording.h \
    src/recording/enginerecord.h \
    src/recording/writeaudiofile.h \
    src/waveform/glwaveformrenderer.h \
    src/waveform/renderobject.h \
    src/waveform/waveformrenderbackground.h \
    src/waveform/waveformrenderbeat.h \
    src/waveform/waveformrenderer.h \
    src/waveform/waveformrendermark.h \
    src/waveform/waveformrendersignal.h \
    src/widget/wabstractcontrol.h \
    src/widget/wcombobox.h \
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
    src/widget/wvinylcontrolindicator.h \
    src/widget/wvisualsimple.h \
    src/widget/wvumeter.h \
    src/widget/wwaveformviewer.h \
    src/widget/wwidget.h \
    src/bpm/bpmdetect.h \
    src/bpm/bpmdetector.h \
    src/bpm/bpmreceiver.h \
    src/bpm/bpmscheme.h \
    src/bpm/peakfinder.h \
    src/bpm/wavesegmentation.h \
    src/controleventmidi.h \
    src/controllogpotmeter.h \
    src/controlnull.h \
    src/controlobject.h \
    src/controlobjectthread.h \
    src/controlobjectthreadmain.h \
    src/controlobjectthreadwidget.h \
    src/controlpotmeter.h \
    src/controlpushbutton.h \
    src/controlrotary.h \
    src/controlttrotary.h \
    src/defs.h \
    src/defs_audiofiles.h \
    src/defs_mixxxcmetrics.h \
    src/defs_promo.h \
    src/defs_urls.h \
    src/dlgabout.h \
    src/dlgbpmscheme.h \
    src/dlgbpmtap.h \
    src/dlgladspa.h \
    src/dlgprefbpm.h \
    src/dlgprefcontrols.h \
    src/dlgprefcrossfader.h \
    src/dlgprefeq.h \
    src/dlgpreferences.h \
    src/dlgprefmidi.h \
    src/dlgprefmidibindings.h \
    src/dlgprefmididevice.h \
    src/dlgprefplaylist.h \
    src/dlgprefrecord.h \
    src/dlgprefshoutcast.h \
    src/dlgprefsound.h \
    src/encodervorbis.h \
    src/fakemonitor.h \
    src/hercules.h \
    src/herculeslinux.h \
    src/imgcolor.h \
    src/imginvert.h \
    src/imgloader.h \
    src/imgsource.h \
    src/input.h \
    src/ladspaview.h \
    src/libraryscanner.h \
    src/libraryscannerdlg.h \
    src/mathstuff.h \
    src/midichanneldelegate.h \
    src/midicommand.h \
    src/mididevicehandler.h \
    src/midiinputmappingtablemodel.h \
    src/midiledhandler.h \
    src/midimapping.h \
    src/midinodelegate.h \
    src/midiobject.h \
    src/midiobjectnull.h \
    src/midiobjectoss.h \
    src/midiobjectportmidi.h \
    src/miditypedelegate.h \
    src/mixxx.h \
    src/mixxxkeyboard.h \
    src/mixxxview.h \
    src/monitor.h \
    src/mouse.h \
    src/mouselinux.h \
    src/mousewin.h \
    src/parser.h \
    src/parserm3u.h \
    src/parserpls.h \
    src/peaklist.h \
    src/playerinfo.h \
    src/powermate.h \
    src/powermatelinux.h \
    src/powermatewin.h \
    src/probabilityvector.h \
    src/proxymodel.h \
    src/reader.h \
    src/readerevent.h \
    src/readerextract.h \
    src/readerextractbeat.h \
    src/readerextractfft.h \
    src/readerextracthfc.h \
    src/readerextractwave.h \
    src/rotary.h \
    src/rtthread.h \
    src/segmentation.h \
    src/sounddevice.h \
    src/sounddeviceportaudio.h \
    src/soundmanager.h \
    src/soundsource.h \
    src/soundsourceffmpeg.h \
    src/soundsourcem4a.h \
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
    src/xmlparse.h
SOURCES += src/analyserbpm.cpp \
    src/analyserqueue.cpp \
    src/analyserwaveform.cpp \
    src/analyserwavesummary.cpp \
    src/configmidi.cpp \
    src/configobject.cpp \
    src/controlbeat.cpp \
    src/controlevent.cpp \
    src/controleventengine.cpp \
    src/engine/enginebuffer.cpp \
    src/engine/enginebuffercue.cpp \
    src/engine/enginebufferscale.cpp \
    src/engine/enginebufferscaledummy.cpp \
    src/engine/enginebufferscalelinear.cpp \
    src/engine/enginebufferscalereal.cpp \
    src/engine/enginebufferscalesrc.cpp \
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
    src/engine/engineladspa.cpp \
    src/engine/enginemaster.cpp \
    src/engine/engineobject.cpp \
    src/engine/enginepregain.cpp \
    src/engine/engineshoutcast.cpp \
    src/engine/enginesidechain.cpp \
    src/engine/enginespectralback.cpp \
    src/engine/enginespectralfwd.cpp \
    src/engine/enginetemporal.cpp \
    src/engine/enginevinylsoundemu.cpp \
    src/engine/enginevolume.cpp \
    src/engine/enginevumeter.cpp \
    src/engine/enginexfader.cpp \
    src/recording/enginerecord.cpp \
    src/recording/writeaudiofile.cpp \
    src/waveform/glwaveformrenderer.cpp \
    src/waveform/renderobject.cpp \
    src/waveform/waveformrenderbackground.cpp \
    src/waveform/waveformrenderbeat.cpp \
    src/waveform/waveformrenderer.cpp \
    src/waveform/waveformrendermark.cpp \
    src/waveform/waveformrendersignal.cpp \
    src/widget/wabstractcontrol.cpp \
    src/widget/wcombobox.cpp \
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
    src/widget/wvinylcontrolindicator.cpp \
    src/widget/wvisualsimple.cpp \
    src/widget/wvumeter.cpp \
    src/widget/wwaveformviewer.cpp \
    src/widget/wwidget.cpp \
    src/bpm/bpmdetect.cpp \
    src/bpm/bpmdetector.cpp \
    src/bpm/bpmscheme.cpp \
    src/bpm/peakfinder.cpp \
    src/bpm/wavesegmentation.cpp \
    src/controleventmidi.cpp \
    src/controllogpotmeter.cpp \
    src/controlnull.cpp \
    src/controlobject.cpp \
    src/controlobjectthread.cpp \
    src/controlobjectthreadmain.cpp \
    src/controlobjectthreadwidget.cpp \
    src/controlpotmeter.cpp \
    src/controlpushbutton.cpp \
    src/controlrotary.cpp \
    src/controlttrotary.cpp \
    src/dlgabout.cpp \
    src/dlgbpmscheme.cpp \
    src/dlgbpmtap.cpp \
    src/dlgladspa.cpp \
    src/dlgprefbpm.cpp \
    src/dlgprefcontrols.cpp \
    src/dlgprefcrossfader.cpp \
    src/dlgprefeq.cpp \
    src/dlgpreferences.cpp \
    src/dlgprefmidi.cpp \
    src/dlgprefmidibindings.cpp \
    src/dlgprefmididevice.cpp \
    src/dlgprefplaylist.cpp \
    src/dlgprefrecord.cpp \
    src/dlgprefshoutcast.cpp \
    src/dlgprefsound.cpp \
    src/encodervorbis.cpp \
    src/fakemonitor.cpp \
    src/hercules.cpp \
    src/herculeslinux.cpp \
    src/herculeslinuxlegacy.cpp \
    src/imgcolor.cpp \
    src/imginvert.cpp \
    src/imgloader.cpp \
    src/input.cpp \
    src/ladspaview.cpp \
    src/libraryscanner.cpp \
    src/libraryscannerdlg.cpp \
    src/mathstuff.cpp \
    src/midichanneldelegate.cpp \
    src/midicommand.cpp \
    src/mididevicehandler.cpp \
    src/midiinputmappingtablemodel.cpp \
    src/midiledhandler.cpp \
    src/midimapping.cpp \
    src/midinodelegate.cpp \
    src/midiobject.cpp \
    src/midiobjectalsaseq.cpp \
    src/midiobjectcoremidi.cpp \
    src/midiobjectnull.cpp \
    src/midiobjectoss.cpp \
    src/midiobjectportmidi.cpp \
    src/midiobjectwin.cpp \
    src/miditypedelegate.cpp \
    src/mixxx.cpp \
    src/mixxxkeyboard.cpp \
    src/mixxxview.cpp \
    src/monitor.cpp \
    src/mouse.cpp \
    src/mouselinux.cpp \
    src/mousewin.cpp \
    src/parser.cpp \
    src/parserm3u.cpp \
    src/parserpls.cpp \
    src/peaklist.cpp \
    src/playerinfo.cpp \
    src/powermate.cpp \
    src/powermatelinux.cpp \
    src/powermatewin.cpp \
    src/probabilityvector.cpp \
    src/proxymodel.cpp \
    src/reader.cpp \
    src/readerevent.cpp \
    src/readerextract.cpp \
    src/readerextractbeat.cpp \
    src/readerextractfft.cpp \
    src/readerextracthfc.cpp \
    src/readerextractwave.cpp \
    src/rotary.cpp \
    src/rtthread.cpp \
    src/segmentation.cpp \
    src/sounddevice.cpp \
    src/sounddeviceportaudio.cpp \
    src/soundmanager.cpp \
    src/soundsource.cpp \
    src/soundsourceffmpeg.cpp \
    src/soundsourcem4a.cpp \
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
    src/main.cpp
FORMS += src/dlgaboutdlg.ui \
    src/dlgbpmschemedlg.ui \
    src/dlgbpmtapdlg.ui \
    src/dlgprefbpmdlg.ui \
    src/dlgprefcontrolsdlg.ui \
    src/dlgprefcrossfaderdlg.ui \
    src/dlgprefeqdlg.ui \
    src/dlgpreferencesdlg.ui \
    src/dlgprefmidibindingsdlg.ui \
    src/dlgprefmididevicedlg.ui \
    src/dlgprefmididlg.ui \
    src/dlgprefplaylistdlg.ui \
    src/dlgprefrecorddlg.ui \
    src/dlgprefshoutcastdlg.ui \
    src/dlgprefsounddlg.ui \
    src/dlgprefvinyldlg.ui
RESOURCES += res/mixxx.qrc
unix { 
    DEFINES += BPMSCHEME_FILE=".mixxxbpmscheme.xm" \
        SETTINGS_FILE=".mixxx.cfg" \
        TRACK_FILE=".mixxxtrack.xml"
    !macx { 
        DEFINES += __LINUX__ \
            __ALSASEQMIDI__ \
            TEMPORAL \
            __UNIX__
        HEADERS += src/midiobjectalsaseq.h
        SOURCES += src/midiobjectalsaseq.cpp
        LIBS += -lasound \
            `pkg-config --libs portaudio-2.0` \
            `pkg-config --libs jack`
        CCFLAGS += `pkg-config --cflags portaudio-2.0`
    }
    CXXFLAGS += -DX -D__UNIX__ -D__LINUX__ -DBPMSCHEME_FILE=\\".mixxxbpmscheme.xml\\" -DSETTINGS_FILE=\\".mixxx.cfg\\" -DTRACK_FILE=\\".mixxxtrack.xml\\ \
        ...
}
macx { 
    DEFINES += __COREMIDI__
    LIBS += -framework CoreMIDI \
        -framework CoreFoundation \
        -framework CoreAudio \
        -framework AudioToolbox \
        -framework AudioUnit \
        -lportaudio
    HEADERS += src/midiobjectcoremidi.h
    SOURCES += src/midiobjectcoremidi.cpp
}
win32 { 
    DEFINES += __WINMIDI__
    HEADERS += src/midiobjectwin.h
    SOURCES += src/midiobjectwin.cpp
    LIBS += WinMM \
        ogg_static \
        vorbis_static \
        vorbisfile_static \
        imm32 \
        wsock32 \
        delayimp \
        winspool \
        shell32
    CXXFLAGS += -DWIN32
    CCFLAGS += -DWIN32
}
CONFIG(ladspa) { 
    DEFINES += __LADSPA__
    HEADERS += src/ladspa/ladspacontrol.h \
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
    SOURCES += src/ladspa/ladspacontrol.cpp \
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
    INCLUDEPATH += 
}
CONFIG(script) { 
    DEFINES += __MIDISCRIPT__
    HEADERS += src/script/interp.h \
        src/script/lua/luainterface.h \
        src/script/lua/luarecorder.h \
        src/script/macro.h \
        src/script/macrolist.h \
        src/script/macrolistitem.h \
        src/script/midiscriptengine.h \
        src/script/numbercontrolevent.h \
        src/script/numberrecorder.h \
        src/script/playinterface.h \
        src/script/python/pythoninterface.h \
        src/script/qtscriptinterface.h \
        src/script/recorder.h \
        src/script/scriptcontrolevent.h \
        src/script/scriptcontrolqueue.h \
        src/script/scriptengine.h \
        src/script/scriptrecorder.h \
        src/script/scriptstudio.h \
        src/script/scripttest.h \
        src/script/sdatetime.h \
        src/script/signalrecorder.h \
        src/script/trackcontrolevent.h \
        src/script/trackrecorder.h
    SOURCES += src/script/lua/luainterface.cpp \
        src/script/lua/luarecorder.cpp \
        src/script/lua/tolua.cpp \
        src/script/macro.cpp \
        src/script/macrolist.cpp \
        src/script/macrolistitem.cpp \
        src/script/midiscriptengine.cpp \
        src/script/numbercontrolevent.cpp \
        src/script/numberrecorder.cpp \
        src/script/playinterface.cpp \
        src/script/python/pythoninterface.cpp \
        src/script/qtscriptinterface.cpp \
        src/script/recorder.cpp \
        src/script/scriptcontrolevent.cpp \
        src/script/scriptcontrolqueue.cpp \
        src/script/scriptengine.cpp \
        src/script/scriptrecorder.cpp \
        src/script/scriptstudio.cpp \
        src/script/scripttest.cpp \
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
    HEADERS += src/m4a/comment.h \
        src/m4a/ip.h \
        src/m4a/sf.h
    SOURCES += src/m4a/mp4-mixxx.cpp
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
        src/engine/enginevinylcontrol.h \
        src/dlgprefvinyl.h
    SOURCES += src/vinylcontrol.cpp \
        src/vinylcontrolproxy.cpp \
        src/vinylcontrolscratchlib.cpp \
        src/vinylcontrolsignalwidget.cpp \
        src/vinylcontrolxwax.cpp \
        ../../lib/scratchlib/DAnalyse.cpp \
        src/engine/enginevinylcontrol.cpp \
        src/dlgprefvinyl.cpp
    CPPPATH += ../../lib/scratchlib \
        ../../lib/xwax
    win32:SOURCES += ../../lib/xwax/timecoder_win32.c
    !win32:SOURCES += ../../lib/xwax/timecoder.c
}
CONFIG(cmetrics):DEFINES += __C_METRICS__ \
    client='MIXXX' \
    server='metrics.mixxx.org'
!CONFIG(hifieq):CXXFLAGS += -D__LOFI__ \
    -D__NO_INTTYPES__
