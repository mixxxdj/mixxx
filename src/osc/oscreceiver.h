#pragma once

#include <lo/lo.h>

#include <QDateTime>
#include <QMutex>
#include <QReadWriteLock>
#include <QString>
#include <QThread>
#include <atomic>
#include <memory>

#include "control/controlproxy.h"
#include "osc/oscfunctions.h"
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
    Q_OBJECT

  public:
    explicit OscReceiver(UserSettingsPointer pConfig);
    ~OscReceiver() override;

    int startOscReceiver(int oscPortin);
    void stop();
    void determineOscAction(OscResult& oscIn);
    void doGetP(OscResult& oscIn);
    void doGetV(OscResult& oscIn);
    void doGetT(OscResult& oscIn);
    void doSet(OscResult& oscIn, float value);
    void sendOscSyncTriggers();
    void checkResponsiveness();
    void loadOscConfiguration(UserSettingsPointer pConfig);

  private:
    UserSettingsPointer m_pConfig;
    OscFunctions m_oscFunctions;
    bool m_stopFlag = false;
    QMutex m_mutex;
    QReadWriteLock m_configLock;

    QDateTime m_lastResponseTime;
    int m_oscPortIn;

    void restartOscReceiver(int oscPortin);

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
