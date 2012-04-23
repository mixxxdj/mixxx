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

//Forward declaration(s)
class Controller;
class ControllerManager;

class ControllerProcessor : public QObject {
    Q_OBJECT
  public:
    ControllerProcessor(ControllerManager *pManager);
    virtual ~ControllerProcessor();
    // Starts controller polling if it hasn't already been started
    void startPolling();
    void stopPolling();

  protected:
    void timerEvent(QTimerEvent *event);

  private:
    int m_pollingTimerId;
    ControllerManager *m_pManager;
};

/** Manages enumeration/operation/deletion of hardware controllers. */
class ControllerManager : public QObject {
    Q_OBJECT
  public:
    ControllerManager(ConfigObject<ConfigValue> * pConfig);
    virtual ~ControllerManager();

    QList<Controller*> getControllers() { return m_controllers; };
    QList<Controller*> getControllerList(bool outputDevices=true, bool inputDevices=true);
    QList<QString> getPresetList(QString extension);
    //ConfigObject<ConfigValue>* getDeviceSettings() { return m_pDeviceSettings; };

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
    // This enables or disables polling as needed. To conserve CPU resources,
    // this should be called with FALSE when a controller is closed.
    void enablePolling(bool enable);

    // Writes out presets for currently connected input devices
    void slotSavePresets(bool onlyActive=false);

  private slots:
    // Open whatever controllers are selected in the preferences. This currently
    // only runs on start-up but maybe should instead be signaled by the
    // preferences dialog on apply, and only open/close changed devices
    int slotSetUpDevices();
    void slotShutdown();
    bool loadPreset(Controller* pController, const QString &filename,
                    const bool force);

  private:
    //ConfigObject<ConfigValue> *m_pDeviceSettings;
    ConfigObject<ConfigValue> *m_pConfig;
    QList<Controller*> m_controllers;
    ControllerProcessor *m_pProcessor;
    QMutex m_mControllerList;
    QList<ControllerEnumerator*> m_enumerators;
    QThread *m_pThread;
};

#endif  // CONTROLLERMANAGER_H
