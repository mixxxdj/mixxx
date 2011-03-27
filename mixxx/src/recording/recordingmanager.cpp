/*
 * Created 03/26/2011 by Tobias Rafreider
 */
#include <QMutex>
#include <QDir>
#include "recordingmanager.h"
#include "recording/defs_recording.h"

#define CD_650

RecordingManager::RecordingManager(ConfigObject<ConfigValue>* pConfig) :
         m_pConfig(pConfig)
{

    m_isRecording = false;
    m_iNumberOfBytesRecored = 0;
    m_iNumberSplits = 0;
    m_recording_path = "";

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
    m_split_size = getFileSplitSize();
}
RecordingManager::~RecordingManager()
{
    qDebug() << "Delete RecordingManager";
    delete m_recReadyCO;
    delete m_recReady;
}
/*
* This will try to start recording
* If successfuly, slotIsRecording will be called and a signal
* isRecording will be emitted.
* Parameter semantic: If true the method
* computes the filename based on date/time information
* this is the default behaviour
* If false, slotBytesRecorded just noticed
* that recording must be interrupted to split the file
* The nth filename will follow the date/time name
* of the first split but with a suffix
*/
void RecordingManager::startRecording(bool generateFileName)
{
    m_iNumberOfBytesRecored = 0;
    m_split_size = getFileSplitSize();
    QString encodingType = m_pConfig->getValueString(
            ConfigKey("[Recording]", "Encoding"));

    if(generateFileName){
        m_iNumberSplits = 1;


        //Construct the file pattern
        // dd_mm_yyyy--hours-minutes-ss   or    mm_dd_yyyy --hours-minutes:seconds
        QDateTime current_date_time = QDateTime::currentDateTime();
        QString date_time_str = current_date_time.toString("dd_MM_yyyy-hh'h'_mm'm'_ss's'");

        QString filename (m_recordingDir);
        filename.append("/").append(date_time_str);
         //Storing the absolutePath of the recording file
        m_recording_path = filename;
        m_pConfig->set(ConfigKey("[Recording]", "Path"), m_recording_path + "."+ encodingType.toLower());
        m_pConfig->set(ConfigKey("[Recording]", "CuePath"), m_recording_path +".cue");
        m_recordingFile = QFileInfo(filename).fileName();
    }
    else{ //This is only executed if filesplit occurs
        ++m_iNumberSplits;
        QString filename = m_recording_path +"part"+QString::number(m_iNumberSplits);
        m_pConfig->set(ConfigKey("[Recording]", "Path"), filename + "."+ encodingType.toLower());
        m_pConfig->set(ConfigKey("[Recording]", "CuePath"), filename +".cue");
        m_recordingFile = QFileInfo(filename).fileName();
    }
    m_recReady->slotSet(RECORD_READY);
}
void RecordingManager::stopRecording()
{
    qDebug() << "Recording stopped";
    m_recReady->slotSet(RECORD_OFF);
    m_recordingFile = "";
    m_iNumberOfBytesRecored = 0;
}

QString& RecordingManager::getRecordingDir(){
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
//returns the name of the file
QString& RecordingManager::getRecordingFile(){
    return m_recordingFile;
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

