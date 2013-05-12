#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H

#include <QDesktopServices>
#include <QDateTime>
#include <QObject>


#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "controlobjectthreadmain.h"
#include "recording/defs_recording.h"

/*
 * The RecordingManager is a central class and manages
 * the recoring feature of Mixxx.
 *
 * There is exactly one instance of this class
 *
 * All methods in this class are thread-safe
 *
 * Note: The RecordingManager lives in the GUI thread
 */

class EngineMaster;
class ControlPushButton;
class ControlObjectThreadMain;

class RecordingManager : public QObject
{
    Q_OBJECT
  public:
    RecordingManager(ConfigObject<ConfigValue>* pConfig, EngineMaster* pEngine);
    virtual ~RecordingManager();


    // This will try to start recording If successfuly, slotIsRecording will be
    // called and a signal isRecording will be emitted.  Parameter semantic: If
    // true the method computes the filename based on date/time information this
    // is the default behaviour If false, slotBytesRecorded just noticed that
    // recording must be interrupted to split the file The nth filename will
    // follow the date/time name of the first split but with a suffix
    void startRecording(bool generateFileName=true);
    void stopRecording();
    bool isRecordingActive();
    void setRecordingDir();
    QString& getRecordingDir();
    // Returns the currently recording file
    QString& getRecordingFile();
    QString& getRecordingLocation();

  signals:
    //emits the commulated number of bytes being recorded
    void bytesRecorded(long);
    void isRecording(bool);

  public slots:
    void slotIsRecording(bool);
    void slotBytesRecorded(int);

  private slots:
    void slotSetRecording(bool recording);
    void slotToggleRecording(double v);

  private:
    QString formatDateTimeForFilename(QDateTime dateTime) const;
    ControlObjectThread* m_recReady;
    ControlObject* m_recReadyCO;
    ControlPushButton* m_pToggleRecording;

    long getFileSplitSize();

    ConfigObject<ConfigValue>* m_pConfig;
    QString m_recordingDir;
    //the base file
    QString m_recording_base_file;
    //filename without path
    QString m_recordingFile;
    //Absolute file
    QString m_recordingLocation;

    bool m_isRecording;
    //will be a very large number
    quint64 m_iNumberOfBytesRecored;
    quint64 m_split_size;
    int m_iNumberSplits;
};

#endif // RECORDINGMANAGER_H
