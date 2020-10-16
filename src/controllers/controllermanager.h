/**
  * @file controllermanager.h
  * @author Sean Pappalardo spappalardo@mixxx.org
  * @date Sat Apr 30 2011
  * @brief Manages creation/enumeration/deletion of hardware controllers.
  */

#ifndef CONTROLLERMANAGER_H
#define CONTROLLERMANAGER_H

#include <QSharedPointer>

#include "controllers/controllerenumerator.h"
#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetinfo.h"
#include "controllers/controllerpresetinfoenumerator.h"
#include "preferences/usersettings.h"

//Forward declaration(s)
class Controller;
class ControllerLearningEventFilter;

// Function to sort controllers by name
bool controllerCompare(Controller *a, Controller *b);

/** Manages enumeration/operation/deletion of hardware controllers. */
class ControllerManager : public QObject {
    Q_OBJECT
  public:
    ControllerManager(UserSettingsPointer pConfig);
    virtual ~ControllerManager();

    static const mixxx::Duration kPollInterval;

    QList<Controller*> getControllers() const;
    QList<Controller*> getControllerList(bool outputDevices=true, bool inputDevices=true);
    ControllerLearningEventFilter* getControllerLearningEventFilter() const;
    QSharedPointer<PresetInfoEnumerator> getMainThreadUserPresetEnumerator() {
        return m_pMainThreadUserPresetEnumerator;
    }
    QSharedPointer<PresetInfoEnumerator> getMainThreadSystemPresetEnumerator() {
        return m_pMainThreadSystemPresetEnumerator;
    }
    QString getConfiguredPresetFileForDevice(QString name);

    // Prevent other parts of Mixxx from having to manually connect to our slots
    void setUpDevices() { emit requestSetUpDevices(); };

    static QList<QString> getPresetPaths(UserSettingsPointer pConfig);

  signals:
    void devicesChanged();
    void requestSetUpDevices();
    void requestShutdown();
    void requestInitialize();

  public slots:
    void updateControllerList();

    void slotApplyPreset(Controller* pController, ControllerPresetPointer pPreset, bool bEnabled);
    void openController(Controller* pController);
    void closeController(Controller* pController);

  private slots:
    // Perform initialization that should be delayed until the ControllerManager
    // thread is started.
    void slotInitialize();
    // Open whatever controllers are selected in the preferences. This currently
    // only runs on start-up but maybe should instead be signaled by the
    // preferences dialog on apply, and only open/close changed devices
    void slotSetUpDevices();
    void slotShutdown();
    // Calls poll() on all devices that have isPolling() true.
    void pollDevices();
    void startPolling();
    void stopPolling();
    void maybeStartOrStopPolling();

  private:
    UserSettingsPointer m_pConfig;
    ControllerLearningEventFilter* m_pControllerLearningEventFilter;
    QTimer m_pollTimer;
    mutable QMutex m_mutex;
    QList<ControllerEnumerator*> m_enumerators;
    QList<Controller*> m_controllers;
    QThread* m_pThread;
    QSharedPointer<PresetInfoEnumerator> m_pMainThreadUserPresetEnumerator;
    QSharedPointer<PresetInfoEnumerator> m_pMainThreadSystemPresetEnumerator;
    bool m_skipPoll;
};

#endif  // CONTROLLERMANAGER_H
