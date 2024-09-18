#ifndef INCLUDED_OSCRECEIVER_H
#define INCLUDED_OSCRECEIVER_H

// #include "preferences/colorpalettesettings.h"

#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "preferences/settingsmanager.h"
#include "preferences/usersettings.h"
#include "util/class.h"

// #include "osc/OscReceivedElements.h"
// #include "ip/UdpSocket.h"
// #include "osc/OscPacketListener.h"

#include <QDataStream>
#include <QFile>
#include <QThread>
#include <memory>

class ControlProxy;

class oscResult {
  public:
    QString oscAddress;
    QString oscGroup;
    QString oscKey;
    float oscValue;
};

//class OscReceiverMain : public QObject {
//    Q_OBJECT
//  private:
//    UserSettingsPointer m_pConfig;
//};

class oscReceiver {
  public:
    UserSettingsPointer m_pConfig;

};

//oscReceiver::oscReceiver (UserSettingsPointer *_apasspointer)
//{
//    apasspointer = _apasspointer;
//}

void OscReceiverMain(UserSettingsPointer m_pConfig);

//void OscReceiverMain();

#endif /* INCLUDED_OSCRECEIVER_H */
