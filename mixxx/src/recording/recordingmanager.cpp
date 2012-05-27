/*
 * Created 03/26/2011 by Tobias Rafreider
 */
#include <QMutex>
#include <QDir>
#include <QDebug>
#include "recordingmanager.h"
#include "recording/defs_recording.h"
#include "controlpushbutton.h"

#define CD_650

RecordingManager::RecordingManager(ConfigObject<ConfigValue>* pConfig) :
        m_pConfig(pConfig),
        m_recordingDir(""),
        m_recording_base_file(""),
        m_recordingFile(""),
        m_recordingLocation(""),
        m_isRecording(false),
        m_iNumberOfBytesRecored(0),
        m_split_size(0),
        m_iNumberSplits(0) {
    m_pToggleRecording = new ControlPushButton(ConfigKey(RECORDING_PREF_KEY, "toggle_recording"));
    connect(m_pToggleRecording, SIGNAL(valueChanged(double)),
            this, SLOT(slotToggleRecording(double)));
    m_recReadyCO = new ControlObject(ConfigKey(RECORDING_PREF_KEY, "status"));
    m_recReady = new ControlObjectThread(m_recReadyCO);

    m_split_size = getFileSplitSize();
}

RecordingManager::~RecordingManager()
{
    qDebug() << "Delete RecordingManager";
    delete m_recReadyCO;
    delete m_recReady;
}

QString RecordingManager::formatDateTimeForFilename(QDateTime dateTime) const {
    // Use a format based on ISO 8601
    QString formatted = dateTime.toString("yyyy-MM-dd_hh'h':mm'm':ss's'");
#ifdef __WINDOWS__
    // Windows does not support colons in filenames.
    formatted = formatted.replace(":", "");
#endif
    return formatted;
}

void RecordingManager::slotToggleRecording(double v) {
    if (v > 0) {
        if (isRecordingActive()) {
            stopRecording();
        } else {
            startRecording();
        }
    }
}

void RecordingManager::startRecording(bool generateFileName) {
    m_iNumberOfBytesRecored = 0;
    m_split_size = getFileSplitSize();
    qDebug() << "Split size is:" << m_split_size;
    QString encodingType = m_pConfig->getValueString(
            ConfigKey("[Recording]", "Encoding"));

    if(generateFileName) {
        m_iNumberSplits = 1;
        //Append file extension
        QString date_time_str = formatDateTimeForFilename(QDateTime::currentDateTime());
        m_recordingFile = QString("%1.%2")
                .arg(date_time_str, encodingType.toLower());

        // Storing the absolutePath of the recording file without file extension
        m_recording_base_file = getRecordingDir();
        m_recording_base_file.append("/").append(date_time_str);
        //appending file extension to get the filelocation
        m_recordingLocation = m_recording_base_file + "."+ encodingType.toLower();
        m_pConfig->set(ConfigKey("[Recording]", "Path"), m_recordingLocation);
        m_pConfig->set(ConfigKey("[Recording]", "CuePath"), m_recording_base_file +".cue");
    } else {
        //This is only executed if filesplit occurs
        ++m_iNumberSplits;
        QString new_base_filename = m_recording_base_file +"part"+QString::number(m_iNumberSplits);
        m_recordingLocation = new_base_filename + "." +encodingType.toLower();

        m_pConfig->set(ConfigKey("[Recording]", "Path"), m_recordingLocation);
        m_pConfig->set(ConfigKey("[Recording]", "CuePath"), new_base_filename +".cue");
        m_recordingFile = QFileInfo(m_recordingLocation).fileName();
    }
    m_recReady->slotSet(RECORD_READY);
}

void RecordingManager::stopRecording()
{
    qDebug() << "Recording stopped";
    m_recReady->slotSet(RECORD_OFF);
    m_recordingFile = "";
    m_recordingLocation = "";
    m_iNumberOfBytesRecored = 0;
}

void RecordingManager::setRecordingDir() {
    QString userRecordings = m_pConfig->getValueString(
        ConfigKey("[Recording]", "Directory"));

    QDir recordDir;
    if (userRecordings == "") {
        qDebug() << "Recording directory was not set";
        // Create recordings directory under music library path by default
        QDir musicDir(m_pConfig->getValueString(ConfigKey("[Playlist]", "Directory")));
        userRecordings = musicDir.absolutePath()+"/Mixxx/Recordings";
        recordDir.setPath(userRecordings);

        // Save default preference value
        m_pConfig->set(ConfigKey("[Recording]", "Directory"),
                       recordDir.absolutePath());
    } else {
        recordDir.setPath(userRecordings);
    }

    if (!recordDir.exists()) {
        if (recordDir.mkpath(recordDir.absolutePath())) {
            qDebug() << "Created folder" << userRecordings << "for recordings";
        } else {
            qDebug() << "Failed to create folder" << userRecordings << "for recordings";
        }
    }
    m_recordingDir = recordDir.absolutePath();
    qDebug() << "Recordings folder set to" << m_recordingDir;
}

QString& RecordingManager::getRecordingDir() {
    // Update current recording dir from preferences
    setRecordingDir();
    return m_recordingDir;
}

//Only called when recording is active
void RecordingManager::slotBytesRecorded(int bytes)
{
    //auto conversion to long
    m_iNumberOfBytesRecored += bytes;
    if(m_iNumberOfBytesRecored >= m_split_size)
    {
        //stop and start recording
        stopRecording();
        //Dont generate a new filename
        //This will reuse the previous filename but appends a suffix
        startRecording(false);
    }
    emit(bytesRecorded(m_iNumberOfBytesRecored));
}

void RecordingManager::slotIsRecording(bool isRecordingActive)
{
    //qDebug() << "SlotIsRecording " << isRecording;

    //Notify the GUI controls, see dlgrecording.cpp
    m_isRecording = isRecordingActive;
    emit(isRecording(isRecordingActive));
}

bool RecordingManager::isRecordingActive() {
    return m_isRecording;
}

QString& RecordingManager::getRecordingFile() {
    return m_recordingFile;
}

QString& RecordingManager::getRecordingLocation() {
    return m_recordingLocation;
}

long RecordingManager::getFileSplitSize()
{
     QString fileSizeStr = m_pConfig->getValueString(ConfigKey("[Recording]","FileSize"));
     if(fileSizeStr == SPLIT_650MB)
         return SIZE_650MB;
     else if(fileSizeStr == SPLIT_700MB)
         return SIZE_700MB;
     else if(fileSizeStr == SPLIT_1024MB)
         return SIZE_1GB;
     else if(fileSizeStr == SPLIT_2048MB)
         return SIZE_2GB;
     else if(fileSizeStr == SPLIT_4096MB)
         return SIZE_4GB;
     else
         return SIZE_650MB;
}

