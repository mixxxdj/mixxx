// looprecordingmanager.cpp
// Created by Carl Pillot on 6/22/13.
// adapted from recordingmanager.cpp

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <sndfile.h>

#include "controlpushbutton.h"
#include "engine/enginemaster.h"
#include "engine/looprecorder/enginelooprecorder.h"
#include "engine/looprecorder/loopwriter.h"
#include "looprecording/defs_looprecording.h"
#include "looprecording/looprecordingmanager.h"
#include "trackinfoobject.h"

LoopRecordingManager::LoopRecordingManager(ConfigObject<ConfigValue>* pConfig,
                                           EngineMaster* pEngine)
        : m_pConfig(pConfig),
        m_recordingDir(""),
        m_recording_base_file(""),
        m_recordingFile(""),
        m_recordingLocation(""),
        m_isRecording(false),
        m_iLoopLength(0),
        m_iLoopNumber(0),
        m_iNumDecks(0),
        m_iNumSamplers(0) {

    m_pCOExportDestination = new ControlObject(ConfigKey(LOOP_RECORDING_PREF_KEY, "export_destination"));
    m_pCOLoopLength = new ControlObject(ConfigKey(LOOP_RECORDING_PREF_KEY, "loop_length"));
    m_pCOLoopPlayReady = new ControlObject(ConfigKey(LOOP_RECORDING_PREF_KEY, "play_status"));


    m_pLoopPlayReady = new ControlObjectThread(m_pCOLoopPlayReady->getKey());
    m_pLoopSource = new ControlObjectThread(ConfigKey(LOOP_RECORDING_PREF_KEY, "loop_source"));
    m_pMasterBPM = new ControlObjectThread("[Master]","sync_bpm");
    m_pNumDecks = new ControlObjectThread("[Master]","num_decks");
    m_pNumSamplers = new ControlObjectThread("[Master]","num_samplers");
    m_pRecReady = new ControlObjectThread(LOOP_RECORDING_PREF_KEY, "rec_status");
    m_pSampleRate = new ControlObjectThread("[Master]", "samplerate");

    m_pLoopDeck1Play = new ControlObjectThread("[LoopRecorderDeck1]","play");
    m_pLoopDeck1Stop = new ControlObjectThread("[LoopRecorderDeck1]","stop");
    m_pLoopDeck1Eject = new ControlObjectThread("[LoopRecorderDeck1]","eject");

    m_pChangeExportDestination = new ControlPushButton(ConfigKey(LOOP_RECORDING_PREF_KEY,"change_export_destination"));
    m_pChangeLoopLength = new ControlPushButton(ConfigKey(LOOP_RECORDING_PREF_KEY, "change_loop_length"));
    m_pChangeLoopSource = new ControlPushButton(ConfigKey(LOOP_RECORDING_PREF_KEY, "change_loop_source"));
    m_pClearRecorder = new ControlPushButton(ConfigKey(LOOP_RECORDING_PREF_KEY, "clear_recorder"));
    m_pExportLoop = new ControlPushButton(ConfigKey(LOOP_RECORDING_PREF_KEY, "export_loop"));
    m_pTogglePlayback = new ControlPushButton(ConfigKey(LOOP_RECORDING_PREF_KEY, "toggle_playback"));
    m_pToggleLoopRecording = new ControlPushButton(ConfigKey(LOOP_RECORDING_PREF_KEY, "toggle_loop_recording"));
    
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

    // Get the current number of decks and samplers.
    m_iNumDecks = m_pNumDecks->get();
    m_iNumSamplers = m_pNumSamplers->get();

    // Set default loop export value to Sampler1.
    m_pCOExportDestination->set(1.0);

    m_pCOLoopLength->set(4.0);

    // Set encoding format for loops to WAV.
    // TODO(carl) create prefences option to change between WAV and AIFF.
    m_pConfig->set(ConfigKey(LOOP_RECORDING_PREF_KEY, "Encoding"),QString("WAV"));

    date_time_str = formatDateTimeForFilename(QDateTime::currentDateTime());
    encodingType = m_pConfig->getValueString(ConfigKey(LOOP_RECORDING_PREF_KEY, "Encoding"));

    setRecordingDir();
            
    // Connect with EngineLoopRecorder
    EngineLoopRecorder* pLoopRecorder = pEngine->getLoopRecorder();
    LoopWriter* pLoopWriter = pLoopRecorder->getLoopWriter();
    if (pLoopWriter) {
        connect(pLoopWriter, SIGNAL(isRecording(bool)), this, SLOT(slotIsRecording(bool)));
        connect(pLoopWriter, SIGNAL(clearRecorder()), this, SLOT(slotClearRecorder()));
        connect(pLoopWriter, SIGNAL(loadAudio()), this, SLOT(slotLoadToLoopDeck()));
        connect(this, SIGNAL(clearWriter()), pLoopWriter, SLOT(slotClearWriter()));
        connect(this, SIGNAL(startWriter(int)), pLoopWriter, SLOT(slotStartRecording(int)));
        connect(this, SIGNAL(stopWriter(bool)), pLoopWriter, SLOT(slotStopRecording(bool)));
        connect(this, SIGNAL(fileOpen(SNDFILE*)), pLoopWriter, SLOT(slotSetFile(SNDFILE*)));
    }
    // Start thread for writing files.
    if (pLoopRecorder) {
        qDebug() << "!~!~!~! LoopRecordingManager starting thread !~!~!~!";
        pLoopRecorder->startThread();
    }
}

LoopRecordingManager::~LoopRecordingManager() {
    qDebug() << "~LoopRecordingManager";
    // TODO(carl) delete temporary loop recorder files.
    delete m_pTogglePlayback;
    delete m_pToggleLoopRecording;
    delete m_pExportLoop;
    delete m_pClearRecorder;
    delete m_pChangeLoopSource;
    delete m_pChangeLoopLength;
    delete m_pChangeExportDestination;
    delete m_pLoopDeck1Eject;
    delete m_pLoopDeck1Stop;
    delete m_pLoopDeck1Play;
    delete m_pSampleRate;
    delete m_pRecReady;
    delete m_pNumSamplers;
    delete m_pNumDecks;
    delete m_pMasterBPM;
    delete m_pLoopSource;
    delete m_pLoopPlayReady;
    delete m_pCOLoopPlayReady;
    delete m_pCOLoopLength;
    delete m_pCOExportDestination;
}

// Public Slots

// Connected to EngineLoopRecorder.
void LoopRecordingManager::slotClearRecorder() {

    m_pTogglePlayback->set(0.0);
    stopLoopDeck();
    clearLoopDeck();
    
    foreach(QString location, m_filesRecorded) {
        qDebug() << "LoopRecordingManager::slotClearRecorder deleteing: " << location;
        QFile file(location);

        if (file.exists()) {
            file.remove();
        }
    }
    m_filesRecorded.clear();
}

void LoopRecordingManager::slotIsRecording(bool isRecordingActive) {

    m_isRecording = isRecordingActive;
    //emit(isLoopRecording(isRecordingActive));
}

// Connected to EngineLoopRecorder.
void LoopRecordingManager::slotLoadToLoopDeck() {
    //qDebug() << "LoopRecordingManager::loadToLoopDeck m_filesRecorded: " << m_filesRecorded;
    if (!m_filesRecorded.isEmpty()) {

        TrackPointer pTrackToPlay = TrackPointer(new TrackInfoObject(m_filesRecorded.last()), &QObject::deleteLater);
        // Signal to Player manager to load and play track.
        emit(loadToLoopDeck(pTrackToPlay, QString("[LoopRecorderDeck1]"), true));
        m_pTogglePlayback->set(1.0);
    }
}

//void LoopRecordingManager::slotCountSamplesRecorded(int samples) {
//    m_iNumSamplesRecorded += samples;
//
//    if ((m_iLoopLength > 0) && (m_iNumSamplesRecorded >= m_iLoopLength)) {
//        qDebug() << "!-!-!-! Stop recording: Loop Length: " << m_iLoopLength << " Samples: " << m_iNumSamplesRecorded;
//        stopRecording();
//    }
//}

// Private Slots

void LoopRecordingManager::slotChangeExportDestination(double v) {
    if (v > 0.) {
        //float numSamplers = m_pNumSamplers->get();
        float destination = m_pCOExportDestination->get();

        if (destination >= m_iNumSamplers) {
            m_pCOExportDestination->set(1.0);
        } else {
            m_pCOExportDestination->set(destination+1.0);
        }
    }
}

void LoopRecordingManager::slotChangeLoopLength(double v) {
    if (v > 0.) {
        float loopLength = m_pCOLoopLength->get();
        if (loopLength == 0.0f) {
            m_pCOLoopLength->set(2.0f);

        } else if (loopLength == 2.0f) {
            m_pCOLoopLength->set(4.0f);

        } else if (loopLength == 4.0f) {
            m_pCOLoopLength->set(8.0f);

        } else if (loopLength == 8.0f) {
            m_pCOLoopLength->set(16.0f);

        } else if (loopLength == 16.0f) {
            m_pCOLoopLength->set(0.0f);

        } else {
            m_pCOLoopLength->set(0.0f);
        }
    }
}

void LoopRecordingManager::slotChangeLoopSource(double v) {
    // Available sources: Master out, PFL out, microphone, passthrough1, passthrough2,
    // all main decks, all samplers.
    // Sources are defined in defs_looprecording.h

    if (v > 0.) {
        //float numDecks = m_pNumDecks->get();
        //float numSamplers = m_pNumSamplers->get();
        float source = m_pLoopSource->get();

        if (source < INPUT_PT2) {
            m_pLoopSource->slotSet(source+1.0);
        } else if (source >= INPUT_PT2 && source < INPUT_DECK_BASE){
            // Set to first deck
            m_pLoopSource->slotSet(INPUT_DECK_BASE+1.0);
        } else if (source > INPUT_DECK_BASE && source < INPUT_DECK_BASE+m_iNumDecks) {
            m_pLoopSource->slotSet(source+1.0);
        } else if (m_iNumSamplers > 0.0 && source >= INPUT_DECK_BASE+m_iNumDecks && source < INPUT_SAMPLER_BASE) {
            m_pLoopSource->slotSet(INPUT_SAMPLER_BASE+1.0);
        } else if (source > INPUT_SAMPLER_BASE && source < INPUT_SAMPLER_BASE+m_iNumSamplers) {
            m_pLoopSource->slotSet(source+1.0);
        } else {
            m_pLoopSource->slotSet(INPUT_MASTER);
        }
    }
}

void LoopRecordingManager::slotNumDecksChanged(double v) {
    m_iNumDecks = (int) v;
}

void LoopRecordingManager::slotNumSamplersChanged(double v) {
    m_iNumSamplers = (int) v;
    //qDebug() << "!!!!LoopRecordingManager::slotNumSamplersChanged num: " << m_iNumSamplers;
}

void LoopRecordingManager::slotToggleClear(double v) {
    //qDebug() << "LoopRecordingManager::slotClearRecorder v: " << v;
    if (v > 0.) {
        emit(clearWriter());
        m_pToggleLoopRecording->set(0.);
    }
}

void LoopRecordingManager::slotToggleExport(double v) {
    //qDebug() << "LoopRecordingManager::slotToggleExport v: " << v;
    if (v > 0.) {
        QString dest_str = QString::number((int)m_pCOExportDestination->get());
        exportLoopToPlayer(QString("[Sampler%1]").arg(dest_str));
    }
}

void LoopRecordingManager::slotToggleLoopRecording(double v) {
    if (v > 0.) {
        //qDebug() << "LoopRecordingManager::slotToggleLoopRecording BPM: " << m_pMasterBPM->get();
        if (m_isRecording) {
            stopRecording();
        } else {
            startRecording();
        }
    }
}

void LoopRecordingManager::slotTogglePlayback(double v) {
    qDebug() << "Toggle Playback: " << v;
    if (v > 0.0) {
        playLoopDeck();
    } else {
        stopLoopDeck();
    }
}

void LoopRecordingManager::clearLoopDeck() {
    m_pLoopDeck1Eject->slotSet(1.0);
    m_pLoopDeck1Eject->slotSet(0.0);
}

void LoopRecordingManager::exportLoopToPlayer(QString group) {
    //qDebug() << "LoopRecordingManager::exportLoopToPlayer m_filesRecorded: " << m_filesRecorded;

    // TODO(carl) handle multi-layered loops.
    setRecordingDir();
    QString dir = m_recordingDir;
    QString encodingType = m_pConfig->getValueString(ConfigKey(LOOP_RECORDING_PREF_KEY, "Encoding"));
    //Append file extension
    QString cur_date_time_str = formatDateTimeForFilename(QDateTime::currentDateTime());

    QString newFileLocation = QString("%1%2_%3.%4")
        .arg(dir,"loop",cur_date_time_str, encodingType.toLower());

    if (!m_filesRecorded.isEmpty()) {

        if (saveLoop(newFileLocation)) {
            emit(exportToPlayer(newFileLocation, group));
        } else {
            qDebug () << "LoopRecordingManager::exportLoopToPlayer Error Saving File: " << newFileLocation;
        }
    }
}

QString LoopRecordingManager::formatDateTimeForFilename(QDateTime dateTime) const {
    // Use a format based on ISO 8601. Windows does not support colons in
    // filenames so we can't use them anywhere.
    QString formatted = dateTime.toString("yyyy-MM-dd_hh'h'mm'm'ss's'");
    return formatted;
}

unsigned int LoopRecordingManager::getLoopLength() {
    double bpm = m_pMasterBPM->get();
    if (bpm == 0.) {
        return 0;
    }

    // loop length in samples = x beats * y sec/beat * z sample rate * 2 channels
    double loopLength = m_pCOLoopLength->get();
    double sampleRate = m_pSampleRate->get();
    //float secondsPerBeat = 60.0f/bpm;

    unsigned int length = (unsigned int)((loopLength * 60. * sampleRate * 2.)/bpm);

    qDebug() << "!!!!!!!LoopRecordingManager::getloopLength sampleRate: " << sampleRate
             << " loopLength: " << loopLength << " bpm: " << bpm
             << " length: " << length;

    return length;
}

SNDFILE* LoopRecordingManager::openSndFile(QString filePath) {
    qDebug() << "LoopRecordingManager::openSndFile path: " << filePath;
    unsigned long samplerate = m_pSampleRate->get();

    // set sfInfo
    SF_INFO sfInfo;
    sfInfo.samplerate = samplerate;
    sfInfo.channels = 2;

    sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    SNDFILE* pSndFile = sf_open(filePath.toLocal8Bit(), SFM_WRITE, &sfInfo);
    if (pSndFile) {
        sf_command(pSndFile, SFC_SET_NORM_FLOAT, NULL, SF_FALSE) ;
    }
    // TODO(carl) graceful error handling. 
//    // check if file is really open
//    if (!isFileOpen()) {
//        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
//        props->setType(DLG_WARNING);
//        props->setTitle(tr("Loop Recording"));
//        props->setText(tr("<html>Could not create audio file for loop recording!<p><br>Maybe you do not have enough free disk space or file permissions.</html>"));
//        ErrorDialogHandler::instance()->requestErrorDialog(props);
//        return false;
//    }
    return pSndFile;
}

void LoopRecordingManager::playLoopDeck() {
    m_pLoopDeck1Play->slotSet(1.0);
}

bool LoopRecordingManager::saveLoop(QString newFileLocation) {

    // Right now just save the last recording created.
    // In the future this will be more complex.
    if (!m_filesRecorded.isEmpty()) {

        QString oldFileLocation = m_filesRecorded.last();
        QFile file(oldFileLocation);

        if (file.exists()) {
            return file.copy(newFileLocation);
        }
    }

    return false;
}

void LoopRecordingManager::setRecordingDir() {
    QDir recordDir(m_pConfig->getValueString(
                    ConfigKey("[Recording]", "Directory")).append(LOOP_RECORDING_DIR));
    // Note: the default ConfigKey for recordDir is set in DlgPrefRecord::DlgPrefRecord
    
    if (!recordDir.exists()) {
        if (recordDir.mkpath(recordDir.absolutePath())) {
            qDebug() << "Created folder" << recordDir.absolutePath() << "for recordings";
        } else {
            qDebug() << "Failed to create folder" << recordDir.absolutePath() << "for recordings";
        }
    }
    m_recordingDir = recordDir.absolutePath();
    qDebug() << "Loop Recordings folder set to" << m_recordingDir;
}

void LoopRecordingManager::startRecording() {
    qDebug() << "LoopRecordingManager startRecording";

    m_iLoopLength = getLoopLength();
    emit(startWriter(m_iLoopLength));
    //m_isRecording = true;

    QString number_str = QString::number(m_iLoopNumber++);

    // TODO(carl) do we really need this?
    //m_recordingFile = QString("%1_%2.%3")
    //.arg("loop",date_time_str, encodingType.toLower());

    // Storing the absolutePath of the recording file without file extension
    m_recording_base_file = QString("%1/%2_%3_%4").arg(m_recordingDir,"loop",number_str,date_time_str);
    //m_recording_base_file.append("/loop_" + m_iLoopNumber + "_" + date_time_str);
    // appending file extension to get the filelocation
    m_recordingLocation = m_recording_base_file + "."+ encodingType.toLower();

    SNDFILE* pSndFile = openSndFile(m_recordingLocation);
    if (pSndFile != NULL) {
        emit fileOpen(pSndFile);
        // add to file registry
        m_filesRecorded << m_recordingLocation;
    } else {
        // error message and stop recording.
    }
}

void LoopRecordingManager::stopRecording()
{
    //qDebug() << "LoopRecordingManager::stopRecording NumSamples: " << m_iNumSamplesRecorded;
    qDebug() << "LoopRecordingManager::stopRecording";
    emit(stopWriter(true));
    //m_iNumSamplesRecorded = 0;
    m_recordingFile = "";
    m_recordingLocation = "";
}

void LoopRecordingManager::stopLoopDeck() {
    //m_pLoopDeck1Stop->slotSet(1.0);
    //m_pLoopDeck1Stop->slotSet(0.0);
    m_pLoopDeck1Play->slotSet(0.0);
}
