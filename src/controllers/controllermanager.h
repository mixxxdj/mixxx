#pragma once

#include <QMutex>
#include <QSharedPointer>
#include <QTimer>
#include <memory>

#include "controllers/controllerenumerator.h"
#include "preferences/usersettings.h"
#include "util/duration.h"

// Forward declaration(s)
class Controller;
class ControllerLearningEventFilter;
class MappingInfoEnumerator;
class LegacyControllerMapping;
class ControllerEnumerator;

/// Function to sort controllers by name
bool controllerCompare(const Controller& lhs, const Controller& rhs);

/// Manages enumeration/operation/deletion of hardware controllers.
class ControllerManager : public QObject {
    Q_OBJECT
  public:
    ControllerManager(UserSettingsPointer pConfig);
    ~ControllerManager() override;

    QList<Controller*> getControllerList(bool outputDevices=true, bool inputDevices=true);
    ControllerLearningEventFilter* getControllerLearningEventFilter() const;
    QSharedPointer<MappingInfoEnumerator> getMainThreadUserMappingEnumerator() {
        return m_pMainThreadUserMappingEnumerator;
    }
    QSharedPointer<MappingInfoEnumerator> getMainThreadSystemMappingEnumerator() {
        return m_pMainThreadSystemMappingEnumerator;
    }
    QString getConfiguredMappingFileForDevice(const QString& name);

    /// Prevent other parts of Mixxx from having to manually connect to our slots
    void setUpDevices() { emit requestSetUpDevices(); };

    static QList<QString> getMappingPaths(UserSettingsPointer pConfig);

  signals:
    void devicesChanged();
    void requestSetUpDevices();
    void requestShutdown();
    void requestInitialize();
    void mappingApplied(bool applied);

  public slots:
    void updateControllerList();

    void slotApplyMapping(Controller* pController,
            std::shared_ptr<LegacyControllerMapping> pMapping,
            bool bEnabled);
    void openController(Controller* pController);
    void closeController(Controller* pController);

  private slots:
    /// Perform initialization that should be delayed until the ControllerManager
    /// thread is started.
    void slotInitialize();
    /// Open whatever controllers are selected in the preferences. This currently
    /// only runs on start-up but maybe should instead be signaled by the
    /// preferences dialog on apply, and only open/close changed devices
    void slotSetUpDevices();
    void slotShutdown();
    /// Calls poll() on all devices that have isPolling() true.
    void pollDevices();
    void startPolling();
    void stopPolling();
    void pollIfAnyControllersOpen();

  private:
    UserSettingsPointer m_pConfig;
    std::unique_ptr<ControllerLearningEventFilter> m_pControllerLearningEventFilter;
    QTimer m_pollTimer;
    mutable QMutex m_mutex;
    std::vector<std::unique_ptr<ControllerEnumerator>> m_enumerators;
    // the Controller* don't live longer than the enumerators, don't reorder
    // m_controllers above m_enumerators or else you'll get a use-after-free
    QList<Controller*> m_controllers;
    std::unique_ptr<QThread> m_pThread;
    QSharedPointer<MappingInfoEnumerator> m_pMainThreadUserMappingEnumerator;
    QSharedPointer<MappingInfoEnumerator> m_pMainThreadSystemMappingEnumerator;
    bool m_skipPoll;
};
