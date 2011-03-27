/*
 * Created 03/26/2011 by Tobias Rafreider
 */
#include <QMutex>
#include <QDir>
#include "recordingmanager.h"



RecordingManager::RecordingManager(ConfigObject<ConfigValue>* pConfig) :
         m_pConfig(pConfig)
{

    m_isRecording = false;

    m_recReadyCO = new ControlObject(ConfigKey("[Master]", "Record"));
    m_recReady = new ControlObjectThread(m_recReadyCO);

    QDir os_music_folder_dir(QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
    //Check if there's a folder Mixxx within the music directory
    QDir mixxxDir(os_music_folder_dir.absolutePath() +"/Mixxx");

    if(!mixxxDir.exists()){

        if(os_music_folder_dir.mkdir("Mixxx")){
            qDebug() << "Created folder 'Mixxx' within default OS Music directory";

            if(mixxxDir.mkdir("Recordings"))
                qDebug() << "Created folder 'Recordings' successfully";
            else
                qDebug() << "Could not create folder 'Recordings' within 'Mixxx'";
        }
        else{
            qDebug() << "Failed to create folder 'Mixxx'' within default OS Music directory."
                     << "Please verify that there's no file called 'Mixxx'.";
        }
    }
    else{ // the Mixxx directory already exists
        qDebug() << "Found folder 'Mixxx' within default OS music directory";
        QDir recordDir(mixxxDir.absolutePath() +"Recordings");
        if(!recordDir.exists()){
            if(mixxxDir.mkdir("Recordings"))
                qDebug() << "Created folder 'Recordings' successfully";
            else
                qDebug() << "Could not create folder 'Recordings' within 'Mixxx'";
        }
    }
    m_recordingDir = os_music_folder_dir.absolutePath() +"/Mixxx/Recordings";
}
RecordingManager::~RecordingManager()
{
    qDebug() << "Delete RecordingManager";
    delete m_recReadyCO;
    delete m_recReady;
}
//This will try to start recording
//If successfuly, slotIsRecording will be called and a signal
// isRecording will be emitted.
void RecordingManager::startRecording()
{
    QString recordPath = m_pConfig->getValueString(
            ConfigKey("[Recording]", "Path"));
    QString encodingType = m_pConfig->getValueString(
            ConfigKey("[Recording]", "Encoding"));

    //Construct the file pattern
    // dd_mm_yyyy--hours-minutes-ss   or    mm_dd_yyyy --hours-minutes:seconds
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString date_time_str = current_date_time.toString("/dd_MM_yyyy-hh'h'_mm'm'_ss's'");
    date_time_str.append(".").append(encodingType.toLower());

    QString filename (m_recordingDir);
    filename.append("/").append(date_time_str);

    m_pConfig->set(ConfigKey("[Recording]", "Path"), filename);
    m_recReady->slotSet(RECORD_READY);
    m_recordingFile = QFileInfo(filename).fileName();

}
void RecordingManager::stopRecording()
{
    qDebug() << "Recording stopped";
    m_recReady->slotSet(RECORD_OFF);
    m_recordingFile = "";

}

QString& RecordingManager::getRecordingDir(){
    return m_recordingDir;
}
void RecordingManager::slotBytesRecorded(int bytes)
{
    emit(bytesRecorded(bytes));
}
void RecordingManager::slotIsRecording(bool isRecordingActive)
{
    //qDebug() << "SlotIsRecoring " << isRecording;

    //Notify the GUI controls, see dlgrecording.cpp
    if(isRecordingActive){
        m_isRecording = true;
        emit(isRecording(true));

    }
    else{
        m_isRecording = false;
        emit(isRecording(false));
    }

}
bool RecordingManager::isRecordingActive()
{
    return m_isRecording;
}
QString& RecordingManager::getRecordingFile(){
    return m_recordingFile;
}
