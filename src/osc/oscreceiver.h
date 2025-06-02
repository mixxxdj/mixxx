#ifndef INCLUDED_OSCRECEIVER_H
#define INCLUDED_OSCRECEIVER_H

#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QThread>
#include <memory>

#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "preferences/settingsmanager.h"
#include "preferences/usersettings.h"
#include "util/class.h"

class ControlProxy;

class OscResult {
  public:
    QString oscAddress;
    QString oscAddressURL;
    QString oscGroup;
    QString oscKey;
    float oscValue;
};

class OscReceiver {
  public:
    UserSettingsPointer m_pConfig;
};
void runOscReceiver(UserSettingsPointer pConfig);
void oscReceiverMain(UserSettingsPointer pConfig);
void loadOscConfiguration(UserSettingsPointer pConfig);

//  private:
QMutex m_mutex;
QReadWriteLock m_configLock;

QDateTime m_lastResponseTime;
int m_oscPortIn;

#endif /* INCLUDED_OSCRECEIVER_H */
