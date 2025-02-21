#pragma once

#include <lo/lo.h>

#include <QThread>
#include <memory>

#include "control/controlproxy.h"
#include "preferences/settingsmanager.h"
#include "preferences/usersettings.h"
#include "util/class.h"

namespace {
const bool sDebug = true;
} // namespace

class ControlProxy;

class OscResult {
  public:
    QString oscAddress;
    QString oscAddressURL;
    QString oscGroup;
    QString oscKey;
    float oscValue;
};

class OscReceiver : public QObject {
    Q_OBJECT // Ensure Qt meta-object system recognizes this class

  public:
    explicit OscReceiver(UserSettingsPointer pConfig, QObject* parent = nullptr);
    ~OscReceiver() override;

    // void startOscReceiver(int oscPortin);
    int startOscReceiver(int oscPortin);
    void stop();
    void oscReceiverMain(UserSettingsPointer pConfig);
    void determineOscAction(OscResult& oscIn);
    void doGetP(OscResult& oscIn);
    void doGetV(OscResult& oscIn);
    void doGetT(OscResult& oscIn);
    void doSet(OscResult& oscIn, float value);
    void sendOscSyncTriggers();

  private:
    bool m_stopFlag = false;
    UserSettingsPointer m_pConfig;
    QThread* m_pThread = nullptr;
    QMutex m_mutex;
    QReadWriteLock m_configLock;

  signals:
    void oscMessageReceived(OscResult oscIn);
};

// Static callback functions
static void errorCallback(int num, const char* msg, const char* where);
static int quit_handler(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message data,
        void* user_data);
static int messageCallback(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message data,
        void* user_data);
