#ifndef INCLUDED_OSCRECEIVER_H
#define INCLUDED_OSCRECEIVER_H

// #include "preferences/colorpalettesettings.h"

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
    QString oscGroupSB;
    QString oscKey;
    float oscValue;
    bool oscInValB;
};

class oscReceiver {
  public:
    UserSettingsPointer m_pConfig;

};

void OscReceiverMain(UserSettingsPointer m_pConfig);


#endif /* INCLUDED_OSCRECEIVER_H */
