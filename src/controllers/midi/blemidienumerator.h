#pragma once

#ifdef Q_OS_ANDROID

#include <QList>
#include <QMutex>
#include <QObject>
#include <QTimer>

#include "controllers/midi/midienumerator.h"
#include "preferences/usersettings.h"

class Controller;

class BleMidiEnumerator : public MidiEnumerator {
    Q_OBJECT

  public:
    explicit BleMidiEnumerator(UserSettingsPointer pConfig);
    ~BleMidiEnumerator() override;

    QList<Controller*> queryDevices() override;
    void startScan();
    bool isConnected() const;
    void rescan();

  private slots:
    void slotOnScanTimeout();

  private:
    UserSettingsPointer m_pConfig;
    QList<Controller*> m_devices;
    QTimer* m_pScanTimer;
    bool m_scanning;
    mutable QMutex m_mutex;
};

#endif // Q_OS_ANDROID
