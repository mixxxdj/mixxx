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

/// The OscManager is a central class and manages
/// the osc feature of Mixxx.
///
/// There is exactly one instance of this class
///
/// All methods in this class are thread-safe
///
/// Note: The OscManager lives in the GUI thread
class OscManager : public QObject {
    Q_OBJECT
  public:
    OscManager(UserSettingsPointer pConfig, EngineMixer* pEngine);
    ~OscManager() override = default;

    // This will try to start osc. If successful, slotIsOsc will be
    // called and a signal isOsc will be emitted.
    // The method computes the filename based on date/time information.
    void startOsc();
    void stopOsc();
    bool isOscActive() const;
    void setOscDir();
    QString& getOscDir();
    // Returns the currently osc file
    const QString& getOscFile() const;
    const QString& getOscLocation() const;

  signals:
    // Emits the cumulative number of bytes currently recorded.
    void bytesOsc(int);
    void isOsc(bool);
    void durationOsc(const QString&);

  public slots:
    void slotIsOsc(bool osc, bool error);
    void slotBytesOsc(int);
    void slotDurationOsc(quint64);
    void slotSetOsc(bool osc);
    void slotToggleOsc(double value);

  private:
    QString formatDateTimeForFilename(const QDateTime& dateTime) const;
    // slotBytesOsc just noticed that osc must be interrupted
    // to split the file. The nth filename will follow the date/time
    // name of the first split but with a suffix.
    void splitContinueOsc();
    void warnFreespace();
    std::unique_ptr<ControlObject> m_pCoOscStatus;
    std::unique_ptr<ControlPushButton> m_pToggleOsc;

    quint64 getFileSplitSize();
    unsigned int getFileSplitSeconds();
    qint64 getFreeSpace();

    UserSettingsPointer m_pConfig;
    QString m_oscDir;
    // the base file
    QString m_osc_base_file;
    // filename without path
    QString m_oscFile;
    // Absolute file
    QString m_oscLocation;

    bool m_bOsc;
    bool m_dfSilence;
    qint64 m_dfCounter;

    // will be a very large number
    quint64 m_iNumberOfBytesOsc;
    quint64 m_iNumberOfBytesOscSplit;
    quint64 m_split_size;
    unsigned int m_split_time;
    int m_iNumberSplits;
    unsigned int m_secondsOsc;
    unsigned int m_secondsOscSplit;
    QString getOscDurationStr(unsigned int duration);
};
