#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H

#include <QDateTime>
#include <QObject>
#include <QString>

#include "preferences/usersettings.h"
#include "controlobject.h"
#include "recording/defs_recording.h"

//
// The RecordingManager is a central class and manages
// the recording feature of Mixxx.
//
// There is exactly one instance of this class
//
// All methods in this class are thread-safe
//
// Note: The RecordingManager lives in the GUI thread
//

class EngineMaster;
class ControlPushButton;
class ControlObjectSlave;

class RecordingManager : public QObject
{
    Q_OBJECT
  public:
    RecordingManager(UserSettingsPointer pConfig, EngineMaster* pEngine);
    virtual ~RecordingManager();


    // This will try to start recording. If successful, slotIsRecording will be
    // called and a signal isRecording will be emitted.
    // Parameter semantic: If true, the method computes the filename based on
    // date/time information. This is the default behavior. If false,
    // slotBytesRecorded just noticed that recording must be interrupted
    // to split the file. The nth filename will follow the date/time
    // name of the first split but with a suffix.
    void startRecording(bool generateFileName=true);
    void stopRecording();
    bool isRecordingActive();
    void setRecordingDir();
    QString& getRecordingDir();
    // Returns the currently recording file
    QString& getRecordingFile();
    QString& getRecordingLocation();

  signals:
    // Emits the cumulative number of bytes currently recorded.
    void bytesRecorded(long);
    void isRecording(bool);
    void durationRecorded(QString);

  public slots:
    void slotIsRecording(bool);
    void slotBytesRecorded(int);
    void slotDurationRecorded(QString);

  private slots:
    void slotSetRecording(bool recording);
    void slotToggleRecording(double v);

  private:
    QString formatDateTimeForFilename(QDateTime dateTime) const;
    ControlObjectSlave* m_recReady;
    ControlObject* m_recReadyCO;
    ControlPushButton* m_pToggleRecording;

    long getFileSplitSize();

    UserSettingsPointer m_pConfig;
    QString m_recordingDir;
    // the base file
    QString m_recording_base_file;
    // filename without path
    QString m_recordingFile;
    // Absolute file
    QString m_recordingLocation;

    bool m_bRecording;
    // will be a very large number
    quint64 m_iNumberOfBytesRecorded;
    quint64 m_split_size;
    int m_iNumberSplits;
    QString m_durationRecorded;
};

#endif // RECORDINGMANAGER_H
