// Created 03/26/2011 by Tobias Rafreider

#include "recording/recordingmanager.h"

#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QMutex>
#include <QStorageInfo>
#include <QtDebug>
#include <climits>

#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/enginemaster.h"
#include "engine/sidechain/enginerecord.h"
#include "engine/sidechain/enginesidechain.h"
#include "errordialoghandler.h"
#include "moc_recordingmanager.cpp"
#include "recording/defs_recording.h"

// one gibibyte
#define MIN_DISK_FREE 1024 * 1024 * 1024ll

RecordingManager::RecordingManager(UserSettingsPointer pConfig, EngineMaster* pEngine)
        : m_pConfig(pConfig),
          m_recordingDir(""),
          m_recording_base_file(""),
          m_recordingFile(""),
          m_recordingLocation(""),
          m_bRecording(false),
          m_iNumberOfBytesRecorded(0),
          m_iNumberOfBytesRecordedSplit(0),
          m_split_size(0),
          m_split_time(0),
          m_iNumberSplits(0),
          m_secondsRecorded(0),
          m_secondsRecordedSplit(0) {
    m_pToggleRecording = new ControlPushButton(ConfigKey(RECORDING_PREF_KEY, "toggle_recording"));
    connect(m_pToggleRecording,
            &ControlPushButton::valueChanged,
            this,
            &RecordingManager::slotToggleRecording);
    m_recReadyCO = new ControlObject(ConfigKey(RECORDING_PREF_KEY, "status"));
    m_recReady = new ControlProxy(m_recReadyCO->getKey(), this);

    m_split_size = getFileSplitSize();
    m_split_time = getFileSplitSeconds();


    // Register EngineRecord with the engine sidechain.
    EngineSideChain* pSidechain = pEngine->getSideChain();
    if (pSidechain) {
        EngineRecord* pEngineRecord = new EngineRecord(m_pConfig);
        connect(pEngineRecord,
                &EngineRecord::isRecording,
                this,
                &RecordingManager::slotIsRecording);
        connect(pEngineRecord,
                &EngineRecord::bytesRecorded,
                this,
                &RecordingManager::slotBytesRecorded);
        connect(pEngineRecord,
                &EngineRecord::durationRecorded,
                this,
                &RecordingManager::slotDurationRecorded);
        pSidechain->addSideChainWorker(pEngineRecord);
    }
}

RecordingManager::~RecordingManager() {
    qDebug() << "Delete RecordingManager";

    delete m_recReadyCO;
    delete m_pToggleRecording;
}

QString RecordingManager::formatDateTimeForFilename(const QDateTime& dateTime) const {
    // Use a format based on ISO 8601. Windows does not support colons in
    // filenames so we can't use them anywhere.
    QString formatted = dateTime.toString("yyyy-MM-dd_hh'h'mm'm'ss's'");
    return formatted;
}

void RecordingManager::slotSetRecording(bool recording) {
    if (recording && !isRecordingActive()) {
        startRecording();
    } else if (!recording && isRecordingActive()) {
        stopRecording();
    }
}

void RecordingManager::slotToggleRecording(double value) {
    bool toggle = static_cast<bool>(value);
    if (toggle) {
        if (isRecordingActive()) {
            stopRecording();
        } else {
            startRecording();
        }
    }
}

qint64 RecordingManager::getFreeSpace() {
    // returns the free space on the recording location in bytes
    // return -1 if the free space could not be determined
    qint64 rv = -1;
    QStorageInfo storage(getRecordingDir());
    if (storage.isValid()) {
        rv = storage.bytesAvailable();
    }
    return rv;
}

void RecordingManager::startRecording() {
    QString encodingType = m_pConfig->getValueString(
            ConfigKey(RECORDING_PREF_KEY, "Encoding"));

    m_iNumberOfBytesRecordedSplit = 0;
    m_secondsRecordedSplit=0;
    m_iNumberOfBytesRecorded = 0;
    m_secondsRecorded=0;
    m_dfSilence = false;
    m_dfCounter=0;
    m_split_size = getFileSplitSize();
    m_split_time = getFileSplitSeconds();
    if (m_split_time < INT_MAX) {
        qDebug() << "Split time is:" << m_split_time;
    }
    else {
        qDebug() << "Split size is:" << m_split_size;
    }

    m_iNumberSplits = 1;
    // Append file extension.
    QString date_time_str = formatDateTimeForFilename(QDateTime::currentDateTime());
    m_recordingFile = QString("%1.%2")
            .arg(date_time_str, encodingType.toLower());

    // Storing the absolutePath of the recording file without file extension.
    m_recording_base_file = getRecordingDir();
    m_recording_base_file.append("/").append(date_time_str);
    // Appending file extension to get the filelocation.
    m_recordingLocation = m_recording_base_file + QStringLiteral(".") + encodingType.toLower();
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Path"), m_recordingLocation);
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "CuePath"), ConfigValue(m_recording_base_file + QStringLiteral(".cue")));

    m_recReady->set(RECORD_READY);
}

void RecordingManager::splitContinueRecording()
{
    ++m_iNumberSplits;
    m_secondsRecorded+=m_secondsRecordedSplit;

    m_iNumberOfBytesRecordedSplit = 0;
    m_secondsRecordedSplit=0;

    QString encodingType = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Encoding"));

    QString new_base_filename = m_recording_base_file + QStringLiteral("part") + QString::number(m_iNumberSplits);
    m_recordingLocation = new_base_filename + QStringLiteral(".") + encodingType.toLower();

    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Path"), m_recordingLocation);
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "CuePath"), ConfigValue(new_base_filename + QStringLiteral(".cue")));
    m_recordingFile = QFileInfo(m_recordingLocation).fileName();

    m_recReady->set(RECORD_SPLIT_CONTINUE);
}

void RecordingManager::stopRecording()
{
    qDebug() << "Recording stopped";
    m_recReady->set(RECORD_OFF);
    m_recordingFile = "";
    m_recordingLocation = "";
    m_iNumberOfBytesRecorded = 0;
    m_secondsRecorded = 0;
}

void RecordingManager::setRecordingDir() {
    QDir recordDir(m_pConfig->getValueString(
        ConfigKey(RECORDING_PREF_KEY, "Directory")));
    // Note: the default ConfigKey for recordDir is set in DlgPrefRecord::DlgPrefRecord.

    if (!recordDir.exists()) {
        if (recordDir.mkpath(recordDir.absolutePath())) {
            qDebug() << "Created folder" << recordDir.absolutePath() << "for recordings";
        } else {
            qDebug() << "Failed to create folder" << recordDir.absolutePath() << "for recordings";
        }
    }
    m_recordingDir = recordDir.absolutePath();
    qDebug() << "Recordings folder set to" << m_recordingDir;
}

QString& RecordingManager::getRecordingDir() {
    // Update current recording dir from preferences.
    setRecordingDir();
    return m_recordingDir;
}

// Only called when recording is active.
void RecordingManager::slotDurationRecorded(quint64 duration)
{
    if(m_secondsRecordedSplit != duration)
    {
        m_secondsRecordedSplit = duration;
        if(duration >= m_split_time)
        {
            qDebug() << "Splitting after " << duration << " seconds";
            // This will reuse the previous filename but append a suffix.
            splitContinueRecording();
        }
        emit durationRecorded(getRecordedDurationStr(m_secondsRecorded+m_secondsRecordedSplit));
    }
}
// Copy from the implementation in enginerecord.cpp
QString RecordingManager::getRecordedDurationStr(unsigned int duration) {
    return QString("%1:%2")
                 .arg(duration / 60, 2, 'f', 0, '0')   // minutes
                 .arg(duration % 60, 2, 'f', 0, '0');  // seconds
}

// Only called when recording is active.
void RecordingManager::slotBytesRecorded(int bytes)
{
    // auto conversion to quint64
    m_iNumberOfBytesRecorded += bytes;
    m_iNumberOfBytesRecordedSplit += bytes;

    //Split before reaching the max size. m_split_size has some headroom, as
    //seen in the constant definitions in defs_recording.h. Also, note that
    //bytes are increased in the order of 10s of KBs each call.
    if(m_iNumberOfBytesRecordedSplit >= m_split_size)
    {
        qDebug() << "Splitting after " << m_iNumberOfBytesRecorded << " bytes written";
        // This will reuse the previous filename but append a suffix.
        splitContinueRecording();
    }
    emit bytesRecorded(m_iNumberOfBytesRecorded);

    // check for free space

    // we only check every 1 MB of data to minimize syscalls
    m_dfCounter -= bytes;

    if (m_dfCounter > 0) {
        return;
    }

    qint64 dfree = getFreeSpace();
    // reset counter
    m_dfCounter = 1024 * 1024;
    if (dfree == -1) {
        qDebug() << "can't determine free space";
        return;
    }
    if (dfree > MIN_DISK_FREE) {
        m_dfSilence = false;
    } else if (m_dfSilence != true) {
        // suppress further warnings until the situation has cleared
        m_dfSilence = true;
        // we run out of diskspace and should warn the user.
        // FIXME(poelzi) temporary display a error message. Replace this with Message Infrastructure when ready
        warnFreespace();
    }
}

void RecordingManager::warnFreespace() {
    qWarning() << "RecordingManager: less than 1 GiB free space";
    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
    props->setType(DLG_WARNING);
    props->setTitle(tr("Low Disk Space Warning"));
    props->setText(tr("There is less than 1 GiB of useable space in the recording folder"));
    props->setKey("RecordingManager::warnFreespace");   // To prevent multiple windows for the same error

    props->addButton(QMessageBox::Ok);
    props->setDefaultButton(QMessageBox::Ok);
    props->setEscapeButton(QMessageBox::Ok);
    props->setModal(false);

    ErrorDialogHandler::instance()->requestErrorDialog(props);
}

void RecordingManager::slotIsRecording(bool isRecordingActive, bool error) {
    //qDebug() << "SlotIsRecording " << isRecording << error;

    // Notify the GUI controls, see dlgrecording.cpp.
    m_bRecording = isRecordingActive;
    emit isRecording(isRecordingActive);

    if (error) {
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(tr("Recording"));
        props->setText("<html>"+tr("Could not create audio file for recording!")
                       +"<p>"+tr("Ensure there is enough free disk space and you have write permission for the Recordings folder.")
                       +"<p>"+tr("You can change the location of the Recordings folder in Preferences -> Recording.")
                       +"</p></html>");
        ErrorDialogHandler::instance()->requestErrorDialog(props);
    }
}

bool RecordingManager::isRecordingActive() const {
    return m_bRecording;
}

const QString& RecordingManager::getRecordingFile() const {
    return m_recordingFile;
}

const QString& RecordingManager::getRecordingLocation() const {
    return m_recordingLocation;
}


quint64 RecordingManager::getFileSplitSize()
{
     QString fileSizeStr = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "FileSize"));
     if (fileSizeStr == SPLIT_650MB) {
         return SIZE_650MB;
     } else if (fileSizeStr == SPLIT_700MB) {
         return SIZE_700MB;
     } else if (fileSizeStr == SPLIT_1024MB) {
         return SIZE_1GB;
     } else if (fileSizeStr == SPLIT_2048MB) {
         return SIZE_2GB;
     } else if (fileSizeStr == SPLIT_4096MB) {
         return SIZE_4GB;
     } else if (fileSizeStr == SPLIT_60MIN) {
         return SIZE_4GB; //Ignore size limit. use time limit
     } else if (fileSizeStr == SPLIT_74MIN) {
         return SIZE_4GB; //Ignore size limit. use time limit
     } else if (fileSizeStr == SPLIT_80MIN) {
         return SIZE_4GB; //Ignore size limit. use time limit
     } else if (fileSizeStr == SPLIT_120MIN) {
         return SIZE_4GB; //Ignore size limit. use time limit
     } else {
         return SIZE_650MB;
     }
}
unsigned int RecordingManager::getFileSplitSeconds()
{
    QString fileSizeStr = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "FileSize"));
    if (fileSizeStr == SPLIT_60MIN) {
        return 60*60;
    } else if (fileSizeStr == SPLIT_74MIN) {
        return 74*60;
    } else if (fileSizeStr == SPLIT_80MIN) {
        return 80*60;
    } else if (fileSizeStr == SPLIT_120MIN) {
        return 120*60;
    } else { // Do not limit by time for the rest.
        return INT_MAX;
    }
}
