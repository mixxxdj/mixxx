#ifndef INCLUDED_OSCRECEIVER_H
#define INCLUDED_OSCRECEIVER_H

#include <QDataStream>
#include <QFile>
#include <QThread>
#include <memory>

#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "preferences/settingsmanager.h"
#include "preferences/usersettings.h"
#include "util/class.h"

class ControlProxy;

class oscResult {
  public:
    QString oscAddress;
    QString oscGroup;
    QString oscKey;
    float oscValue;
};

class oscReceiver {
  public:
    UserSettingsPointer m_pConfig;
};

//    void sendOscSyncTriggers(UserSettingsPointer m_pConfig);
//    void processOscMessage(UserSettingsPointer m_pConfig, float oscInVal,
//    const QString& oscInAddress) { void
//    OscGetParameterRequest(UserSettingsPointer m_pConfig, const QString&
//    oscGroup, const QString& oscKey, float oscValue); void
//    OscGetValueRequest(UserSettingsPointer m_pConfig, const QString& oscGroup,
//    const QString& oscKey, float oscValue); void
//    OscSetRequest(UserSettingsPointer m_pConfig, const QString& oscGroup,
//    const QString& oscKey, float oscValue);
void RunOscReceiver(int OscPortIn, UserSettingsPointer m_pConfig);
void OscReceiverMain(UserSettingsPointer m_pConfig);

#endif /* INCLUDED_OSCRECEIVER_H */
