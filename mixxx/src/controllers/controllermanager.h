/**
  * @file controllermanager.h
  * @author Sean Pappalardo spappalardo@mixxx.org
  * @date Sat Apr 30 2011
  * @brief Manages creation/enumeration/deletion of hardware controllers.
  */

#ifndef CONTROLLERMANAGER_H
#define CONTROLLERMANAGER_H

#include "configobject.h"
#include "controllers/controllerenumerator.h"
#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetinfo.h"

//Forward declaration(s)
class Controller;
class ControllerLearningEventFilter;

// Function to sort controllers by name
bool controllerCompare(Controller *a,Controller *b);

/** Manages enumeration/operation/deletion of hardware controllers. */
class ControllerManager : public QObject {
    Q_OBJECT
  public:
    ControllerManager(ConfigObject<ConfigValue> * pConfig);
    virtual ~ControllerManager();

    QList<Controller*> getControllers() const;
    QList<Controller*> getControllerList(bool outputDevices=true, bool inputDevices=true);
    ControllerLearningEventFilter* getControllerLearningEventFilter() const;
    PresetInfoEnumerator* getPresetInfoManager();

    // Prevent other parts of Mixxx from having to manually connect to our slots
    void setUpDevices() { emit(requestSetUpDevices()); };
    void savePresets(bool onlyActive=false) { emit(requestSave(onlyActive)); };
    void shutdown() { emit(requestShutdown()); };

  signals:
    void devicesChanged();
    void requestSetUpDevices();
    void requestShutdown();
    void requestSave(bool onlyActive);

  public slots:
    void updateControllerList();

    void openController(Controller* pController);
    void closeController(Controller* pController);

    // Writes out presets for currently connected input devices
    void slotSavePresets(bool onlyActive=false);

  private slots:
    // Open whatever controllers are selected in the preferences. This currently
    // only runs on start-up but maybe should instead be signaled by the
    // preferences dialog on apply, and only open/close changed devices
    int slotSetUpDevices();
    void slotShutdown();
    bool loadPreset(Controller* pController,
                    ControllerPresetPointer preset);
    bool loadPreset(Controller* pController, const QString &filename,
                    const bool force);
    // Calls poll() on all devices that have isPolling() true.
    void pollDevices();
    void startPolling();
    void stopPolling();
    void maybeStartOrStopPolling();

    static QString presetFilenameFromName(QString name) {
        return name.replace(" ", "_");
    }

  private:
    ConfigObject<ConfigValue> *m_pConfig;
    ControllerLearningEventFilter* m_pControllerLearningEventFilter;
    QTimer m_pollTimer;
    mutable QMutex m_mutex;
    QList<ControllerEnumerator*> m_enumerators;
    QList<Controller*> m_controllers;
    QThread* m_pThread;
    PresetInfoEnumerator* m_pPresetInfoManager;
};

#endif  // CONTROLLERMANAGER_H
