#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QList>

#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "recording/defs_recording.h"
#include "encoder/encoder.h"
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
class ControlProxy;

class RecordingManager : public QObject
{
    Q_OBJECT
  public:
    RecordingManager(UserSettingsPointer pConfig, EngineMaster* pEngine);
    virtual ~RecordingManager();


    // This will try to start recording. If successful, slotIsRecording will be
    // called and a signal isRecording will be emitted.
    // The method computes the filename based on date/time information.
    void startRecording();
    void stopRecording();
    bool isRecordingActive() const;
    void setRecordingDir();
    QString& getRecordingDir();
    // Returns the currently recording file
    const QString& getRecordingFile() const;
    const QString& getRecordingLocation() const;

  signals:
    // Emits the cumulative number of bytes currently recorded.
    void bytesRecorded(int);
    void isRecording(bool);
    void durationRecorded(QString);

  public slots:
    void slotIsRecording(bool recording, bool error);
    void slotBytesRecorded(int);
    void slotDurationRecorded(quint64);
    void slotSetRecording(bool recording);

  private slots:
    void slotToggleRecording(double v);

  private:
    QString formatDateTimeForFilename(QDateTime dateTime) const;
    // slotBytesRecorded just noticed that recording must be interrupted
    // to split the file. The nth filename will follow the date/time
    // name of the first split but with a suffix.
    void splitContinueRecording();
    void warnFreespace();
    ControlProxy* m_recReady;
    ControlObject* m_recReadyCO;
    ControlPushButton* m_pToggleRecording;

    quint64 getFileSplitSize();
    unsigned int getFileSplitSeconds();
    qint64 getFreeSpace();

    UserSettingsPointer m_pConfig;
    QString m_recordingDir;
    // the base file
    QString m_recording_base_file;
    // filename without path
    QString m_recordingFile;
    // Absolute file
    QString m_recordingLocation;

    bool m_bRecording;
    bool m_dfSilence;
    qint64 m_dfCounter;

    // will be a very large number
    quint64 m_iNumberOfBytesRecorded;
    quint64 m_iNumberOfBytesRecordedSplit;
    quint64 m_split_size;
    unsigned int m_split_time;
    int m_iNumberSplits;
    unsigned int m_secondsRecorded;
    unsigned int m_secondsRecordedSplit;
    QString getRecordedDurationStr(unsigned int duration);
};

#endif // RECORDINGMANAGER_H
