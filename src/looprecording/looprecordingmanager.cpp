// looprecordingmanager.cpp
// Created by Carl Pillot on 6/22/13.
// adapted from recordingmanager.cpp

#include <QDebug>
#include <QDir>
#include <QFile>
//#include <QMutex>
#include <sndfile.h>

#include "looprecording/looprecordingmanager.h"

#include "controlpushbutton.h"
#include "engine/enginemaster.h"
#include "engine/looprecorder/enginelooprecorder.h"
#include "engine/looprecorder/loopwriter.h"
#include "looprecording/looplayertracker.h"
#include "playerinfo.h"
#include "recording/defs_recording.h"
#include "trackinfoobject.h"

LoopRecordingManager::LoopRecordingManager(ConfigObject<ConfigValue>* pConfig,
                                           EngineMaster* pEngine)
        : m_pConfig(pConfig),
          m_loopDestination(""),
          m_loopSource("Master"),
          m_recordingDir(""),
          m_recordingTempDir(""),
          m_bRecording(false),
          m_iCurrentPlayingDeck(0),
          m_dLoopBPM(0.0),
          m_iLoopLength(0),
          m_iLoopNumber(0),
          m_iNumDecks(0),
          m_iNumSamplers(0) {

    m_pCOLoopBeats = new ControlObject(ConfigKey(LOOP_RECORDING_PREF_KEY, "loop_length"));
    m_pCOLoopPlayReady = new ControlObject(ConfigKey(LOOP_RECORDING_PREF_KEY, "play_status"));

    m_pLoopPlayReady = new ControlObjectThread(m_pCOLoopPlayReady->getKey());
    m_pNumDecks = new ControlObjectThread("[Master]","num_decks");
    m_pNumSamplers = new ControlObjectThread("[Master]","num_samplers");
    m_pRecReady = new ControlObjectThread(LOOP_RECORDING_PREF_KEY, "rec_status");
    m_pSampleRate = new ControlObjectThread("[Master]", "samplerate");

    m_pChangeExportDestination = new ControlPushButton(
                                    ConfigKey(LOOP_RECORDING_PREF_KEY,"change_export_destination"));
    m_pChangeLoopLength = new ControlPushButton(
                                    ConfigKey(LOOP_RECORDING_PREF_KEY, "change_loop_length"));
    m_pChangeLoopSource = new ControlPushButton(
                                    ConfigKey(LOOP_RECORDING_PREF_KEY, "change_loop_source"));
    m_pClearRecorder = new ControlPushButton(
                                    ConfigKey(LOOP_RECORDING_PREF_KEY, "clear_recorder"));
    m_pExportLoop = new ControlPushButton(
                                    ConfigKey(LOOP_RECORDING_PREF_KEY, "export_loop"));
    m_pTogglePlayback = new ControlPushButton(
                                    ConfigKey(LOOP_RECORDING_PREF_KEY, "toggle_playback"));
    m_pToggleLoopRecording = new ControlPushButton(
                                    ConfigKey(LOOP_RECORDING_PREF_KEY, "toggle_loop_recording"));

    connect(m_pToggleLoopRecording, SIGNAL(valueChanged(double)),
            this, SLOT(slotToggleLoopRecording(double)));
    connect(m_pTogglePlayback, SIGNAL(valueChanged(double)),
            this, SLOT(slotTogglePlayback(double)));
    connect(m_pClearRecorder, SIGNAL(valueChanged(double)),
            this, SLOT(slotToggleClear(double)));
    connect(m_pExportLoop, SIGNAL(valueChanged(double)),
            this, SLOT(slotToggleExport(double)));
    connect(m_pChangeLoopLength, SIGNAL(valueChanged(double)),
            this, SLOT(slotChangeLoopLength(double)));
    connect(m_pChangeLoopSource, SIGNAL(valueChanged(double)),
            this, SLOT(slotChangeLoopSource(double)));
    connect(m_pChangeExportDestination, SIGNAL(valueChanged(double)),
            this, SLOT(slotChangeExportDestination(double)));
    connect(m_pNumDecks, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumDecksChanged(double)));
    connect(m_pNumSamplers, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumSamplersChanged(double)));
    connect(&PlayerInfo::instance(), SIGNAL(currentPlayingDeckChanged(int)),
            this, SLOT(slotCurrentPlayingDeckChanged(int)));

    // Get the current number of decks and samplers.
    m_iNumDecks = m_pNumDecks->get();
    m_iNumSamplers = m_pNumSamplers->get();

    for (int i = 1; i <= m_iNumDecks; i++) {
        qDebug() << QString("Loop Recording Manager: Adding Rate Control [Channel%1]").arg(i);
        m_deckRateControls.append(new ControlObjectThread(
                                      ConfigKey(QString("[Channel%1]").arg(i), "rateEngine")));
    }

    m_pCOLoopBeats->set(4.0);

    if (m_iNumSamplers > 0) {
        m_loopDestination = "Sampler1";
    }

    m_dateTime = formatDateTimeForFilename(QDateTime::currentDateTime());
    m_encodingType = m_pConfig->getValueString(ConfigKey(LOOP_RECORDING_PREF_KEY, "Encoding"));

    setRecordingDir();

    m_pLoopLayerTracker = new LoopLayerTracker(m_pConfig);
    if (m_pLoopLayerTracker) {
        connect(m_pLoopLayerTracker, SIGNAL(exportLoop(QString)), this, SLOT(slotExportLoopToPlayer(QString)));
    }

    // Connect with EngineLoopRecorder and Loop Writer
    // TODO(carl): disable loop recording if fails.
    EngineLoopRecorder* pLoopRecorder = pEngine->getLoopRecorder();
    LoopWriter* pLoopWriter = pLoopRecorder->getLoopWriter();
    if (pLoopRecorder) {
        connect(this, SIGNAL(sourceChanged(QString)), pLoopRecorder, SLOT(slotSourceChanged(QString)));
        emit(sourceChanged("Master"));
    }
    if (pLoopWriter) {
        connect(pLoopWriter, SIGNAL(isRecording(bool)), this, SLOT(slotIsRecording(bool)));
        connect(pLoopWriter, SIGNAL(clearRecorder()), this, SLOT(slotClearRecorder()));
        connect(pLoopWriter, SIGNAL(loadAudio(int)), m_pLoopLayerTracker, SLOT(slotLoadToLoopDeck(int)));
        connect(this, SIGNAL(clearWriter()), pLoopWriter, SLOT(slotClearWriter()));
        connect(this, SIGNAL(startWriter(int, SNDFILE*)), pLoopWriter, SLOT(slotStartRecording(int, SNDFILE*)));
        connect(this, SIGNAL(stopWriter(bool)), pLoopWriter, SLOT(slotStopRecording(bool)));
        //connect(this, SIGNAL(fileOpen(SNDFILE*)), pLoopWriter, SLOT(slotSetFile(SNDFILE*)));
    }
    // Start thread for writing files.
    if (pLoopRecorder) {
        //qDebug() << "!~!~!~! LoopRecordingManager starting thread !~!~!~!";
        pLoopRecorder->startThread();
    }
}

LoopRecordingManager::~LoopRecordingManager() {
    qDebug() << "~LoopRecordingManager";

    // Delete temporary loop recorder files.
    //slotClearRecorder();

    while (!m_deckRateControls.empty()) {
        ControlObjectThread* pControl = m_deckRateControls.takeLast();
        delete pControl;
    }

    delete m_pTogglePlayback;
    delete m_pToggleLoopRecording;
    delete m_pExportLoop;
    delete m_pClearRecorder;
    delete m_pChangeLoopSource;
    delete m_pChangeLoopLength;
    delete m_pChangeExportDestination;
    delete m_pSampleRate;
    delete m_pRecReady;
    delete m_pNumSamplers;
    delete m_pNumDecks;
    delete m_pLoopPlayReady;
    delete m_pCOLoopPlayReady;
    delete m_pCOLoopBeats;
}

// Public Slots

// Connected to EngineLoopRecorder.
void LoopRecordingManager::slotClearRecorder() {

    m_pTogglePlayback->set(0.0);
    m_pLoopLayerTracker->stop(true);
    m_pLoopLayerTracker->clear();
}

void LoopRecordingManager::slotCurrentPlayingDeckChanged(int deck) {
    //qDebug() << "LoopRecordingManager::slotCurrentPlayingDeckChanged Deck: "
    //        << deck;
    m_iCurrentPlayingDeck = deck;
}

void LoopRecordingManager::slotExportLoopToPlayer(QString filePath) {
    QString group = QString("[%1]").arg(m_loopDestination);
    emit(exportToPlayer(filePath, group));
    slotClearRecorder();
}

void LoopRecordingManager::slotIsRecording(bool isRecordingActive) {
    m_bRecording = isRecordingActive;
}

// Private Slots

void LoopRecordingManager::slotChangeExportDestination(double v) {
    if (v <= 0.) {
        return;
    }

    if (m_iNumSamplers <= 0) {
        m_loopDestination = "";
        emit destinationChanged(m_loopDestination);
        return;
    }

    int samplerNum = m_loopDestination.right(1).toInt();

    if (samplerNum >= m_iNumSamplers) {
        m_loopDestination = "Sampler1";
    } else {
        m_loopDestination = QString("Sampler%1").arg(++samplerNum);
    }
    emit destinationChanged(m_loopDestination);
}

void LoopRecordingManager::slotChangeLoopLength(double v) {
    if (v <= 0.0) {
        return;
    }

    float loopLength = m_pCOLoopBeats->get();

    if (loopLength == 0) {
        m_pCOLoopBeats->set(2);
    } else if (loopLength == 16) {
        m_pCOLoopBeats->set(0);
    } else {
        m_pCOLoopBeats->set(loopLength * 2);
    }
}

void LoopRecordingManager::slotChangeLoopSource(double v) {
    if (v <= 0.0) {
        return;
    }

    // Available sources: None (Loop Recorder is off), Master out, PFL out,
    // microphone, passthrough1, passthrough2,
    // all main decks, all samplers.
    if (m_loopSource == "Master") {
        m_loopSource = "Headphones";
    } else if (m_loopSource == "Headphones") {
        m_loopSource = "Microphone";
    } else if (m_loopSource == "Microphone") {
        m_loopSource = "Channel1";
    } else if (m_loopSource.startsWith("Channel")) {
        int deckNum = m_loopSource.right(1).toInt();

        qDebug() << "Channel: " << deckNum;

        if (deckNum > 0 && deckNum < m_iNumDecks) {
            m_loopSource = QString("Channel%1").arg(++deckNum);
        } else if (deckNum >= m_iNumDecks && m_iNumSamplers > 0) {
            m_loopSource = "Sampler1";
        } else {
            m_loopSource = "Master";
        }
    } else if (m_loopSource.startsWith("Sampler")) {
        int samplerNum = m_loopSource.right(1).toInt();
        if (samplerNum > 0 && samplerNum < m_iNumSamplers) {
            m_loopSource = QString("Sampler%1").arg(++samplerNum);
        } else {
            m_loopSource = "Master";
        }
    } else {
        m_loopSource = "Master";
    }

    emit(sourceChanged(m_loopSource));
}

void LoopRecordingManager::slotNumDecksChanged(double v) {
    m_iNumDecks = (int) v;
    int iNumRateControls = m_deckRateControls.size();

    if(m_iNumDecks > iNumRateControls) {
        for (int i = iNumRateControls+1; i <= m_iNumDecks; i++) {
            qDebug() << QString("Loop Recording Manager: Adding Rate Control [Channel%1]").arg(i);
            m_deckRateControls.append(new ControlObjectThread(
                                          ConfigKey(QString("[Channel%1]").arg(i), "rateEngine")));
        }
    }
}

void LoopRecordingManager::slotNumSamplersChanged(double v) {
    m_iNumSamplers = (int) v;
    //qDebug() << "!!!!LoopRecordingManager::slotNumSamplersChanged num: " << m_iNumSamplers;
}


// TODO: review this method.
void LoopRecordingManager::slotToggleClear(double v) {
    //qDebug() << "LoopRecordingManager::slotClearRecorder v: " << v;
    if (v <= 0.) {
        return;
    }
    emit(clearWriter());
    m_pToggleLoopRecording->set(0.);
    m_dLoopBPM = 0.0;
}

void LoopRecordingManager::slotToggleExport(double v) {
    //qDebug() << "LoopRecordingManager::slotToggleExport v: " << v;
    if (v <= 0.) {
        return;
    }

    setRecordingDir();
    QString currentDateTime = formatDateTimeForFilename(QDateTime::currentDateTime());

    QString newFilePath = QString("%1/%2_%3.%4")
    .arg(m_recordingDir, "loop", currentDateTime, m_encodingType.toLower());

    m_pLoopLayerTracker->finalizeLoop(newFilePath, m_dLoopBPM);
}

void LoopRecordingManager::slotToggleLoopRecording(double v) {
    //qDebug() << "LoopRecordingManager::slotToggleLoopRecorder";

    if (v <= 0.) {
        return;
    }
    if (m_bRecording) {
        stopRecording();
    } else {
        startRecording();
    }
}

void LoopRecordingManager::slotTogglePlayback(double v) {
    qDebug() << "Toggle Playback: " << v;
    if (v > 0.0) {
        m_pLoopLayerTracker->play();
    } else {
        m_pLoopLayerTracker->stop(false);
    }
}

QString LoopRecordingManager::formatDateTimeForFilename(QDateTime dateTime) const {
    // Use a format based on ISO 8601. Windows does not support colons in
    // filenames so we can't use them anywhere.
    QString formatted = dateTime.toString("yyMMdd_hhmmss");
    return formatted;
}

double LoopRecordingManager::getCurrentBPM() {
    // Note: This doesn't account for any changes to the BPM using the rate slider.
    // When master_sync is merged this will become trivial, because we can get the
    // bpm from the sync_bpm control object.

    TrackPointer pTrack = PlayerInfo::instance().getCurrentPlayingTrack();

    if (pTrack == NULL) {
        qDebug() << "!~!~!~!~! LoopRecordingManager::getCurrentBPM() Current Track NULL";
        return 0.0;
    }

    double baseTrackBPM = pTrack->getBpm();

    if ((m_iCurrentPlayingDeck >= 0) && (m_iCurrentPlayingDeck < m_deckRateControls.size())) {

        double rate = m_deckRateControls[m_iCurrentPlayingDeck]->get();
        //qDebug() << "!~!~!~!~! LoopRecordingManager::getCurrentBPM() Base BPM: " << baseTrackBPM
        //<< " Rate: " << rate;

        // TODO(Carl): scaling only works for 44.1 kHz sample rate
        // Rate is incorrect for sample rates higher than 44.1.
        // This will be fixed for free by master_sync
        return baseTrackBPM * rate;

    } else {
        //qDebug() << "!~!~!~!~! LoopRecordingManager::getCurrentBPM() 0";
        return 0;
    }
}

int LoopRecordingManager::getLoopLength() {
    double bpm = getCurrentBPM();
    if (bpm == 0.) {
        return 0;
    }

    // loop length in samples = x beats * y sample rate * 2 channels * 60 sec/min / z bpm
    double loopBeats = m_pCOLoopBeats->get();
    double sampleRate = m_pSampleRate->get();
    
    int length = (int)((loopBeats * sampleRate * 2.0 * 60.0)/bpm);

    if (!even(length)) {
        length--;
    }

    //qDebug() << "!!!!!!!LoopRecordingManager::getloopLength sampleRate: " << sampleRate
    //         << " loopLength: " << loopLength << " bpm: " << bpm
    //         << " length: " << length;

    return length;
}

SNDFILE* LoopRecordingManager::openSndFile(QString filePath) {
    qDebug() << "LoopRecordingManager::openSndFile path: " << filePath;

    // TODO(carl) create file path here?
//    QDir recordDir(filePath);
//    if (!recordDir.exists()) {
//        if (recordDir.mkpath(recordDir.absolutePath())) {
//            qDebug() << "Created folder" << recordDir.absolutePath() << "for loop file";
//        } else {
//            qDebug() << "Failed to create folder" << recordDir.absolutePath() << "for recording";
//            return NULL;
//        }
//    }

    unsigned long samplerate = m_pSampleRate->get();

    // set sfInfo
    SF_INFO sfInfo;
    sfInfo.samplerate = samplerate;
    sfInfo.channels = 2;

    if (m_encodingType == ENCODING_WAVE) {
        sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    } else {
        sfInfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
    }

    SNDFILE* pSndFile = sf_open(filePath.toLocal8Bit(), SFM_WRITE, &sfInfo);
    if (pSndFile) {
        sf_command(pSndFile, SFC_SET_NORM_FLOAT, NULL, SF_FALSE) ;
    }
    return pSndFile;
}

void LoopRecordingManager::setRecordingDir() {

    m_recordingDir = m_pConfig->getValueString(ConfigKey("[Recording]", "Directory"));
    m_recordingTempDir = m_recordingDir;
    m_recordingTempDir.append(LOOP_TEMP_DIR);

    //QDir recordDir(recordDirConfig);
    QDir loopTempDir(m_recordingTempDir);
    // Note: the default ConfigKey for recordDir is set in DlgPrefRecord::DlgPrefRecord

    if (!loopTempDir.exists()) {
        if (loopTempDir.mkpath(loopTempDir.absolutePath())) {
            qDebug() << "Created folder" << loopTempDir.absolutePath() << "for loop recording";
        } else {
            qDebug() << "Failed to create folder" << loopTempDir.absolutePath() << "for recording";
            m_recordingDir = "";
            m_recordingTempDir = "";
            return;
            // TODO(carl): prevent recording from happening at all?
        }
    }
    //m_recordingDir = recordDir.absolutePath();
    //m_recordingTempDir = loopTempDir.absolutePath();
    qDebug() << "Loop recording temp directory set to" << m_recordingTempDir;
}

void LoopRecordingManager::startRecording() {
    //qDebug() << "LoopRecordingManager startRecording";

    // update encoding type
    m_encodingType = m_pConfig->getValueString(ConfigKey(LOOP_RECORDING_PREF_KEY, "Encoding"));

    // TODO(carl): make sure the bpm is only set on first layer when recording multiple layers.
    m_dLoopBPM = getCurrentBPM();
    m_iLoopLength = getLoopLength();

    QString numberStr = QString::number(m_iLoopNumber++);
    setRecordingDir();
    QString recordingLocation = QString("%1/%2-%3_%4.%5").arg(
            m_recordingTempDir, "loop", numberStr, m_dateTime, m_encodingType.toLower());

    SNDFILE* pSndFile = openSndFile(recordingLocation);
    if (pSndFile != NULL) {
        emit(startWriter(m_iLoopLength, pSndFile));
        //emit fileOpen(pSndFile);
        // add to loop tracker
        m_pLoopLayerTracker->addLoopLayer(recordingLocation, m_iLoopLength);
    } else {
        // TODO(carl): error message and stop recording.
    }
}

void LoopRecordingManager::stopRecording() {
    //qDebug() << "LoopRecordingManager::stopRecording";
    emit(stopWriter(true));
}
