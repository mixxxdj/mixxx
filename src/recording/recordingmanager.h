#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include "control/controlobject.h"
#include "preferences/usersettings.h"

class EngineMixer;
class ControlPushButton;
class ControlProxy;
class QDateTime;

/// The RecordingManager is a central class and manages
/// the recording feature of Mixxx.
///
/// There is exactly one instance of this class
///
/// All methods in this class are thread-safe
///
/// Note: The RecordingManager lives in the GUI thread
class RecordingManager : public QObject {
    Q_OBJECT
  public:
    RecordingManager(UserSettingsPointer pConfig, EngineMixer* pEngine);
    ~RecordingManager() override = default;

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
    void durationRecorded(const QString&);

  public slots:
    void slotIsRecording(bool recording, bool error);
    void slotBytesRecorded(int);
    void slotDurationRecorded(quint64);
    void slotSetRecording(bool recording);
    void slotToggleRecording(double value);

  private:
    QString formatDateTimeForFilename(const QDateTime& dateTime) const;
    // slotBytesRecorded just noticed that recording must be interrupted
    // to split the file. The nth filename will follow the date/time
    // name of the first split but with a suffix.
    void splitContinueRecording();
    void warnFreespace();
    std::unique_ptr<ControlObject> m_pCoRecStatus;
    std::unique_ptr<ControlPushButton> m_pToggleRecording;

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
