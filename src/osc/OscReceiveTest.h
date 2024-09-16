#ifndef INCLUDED_OSCRECEIVETEST_H
#define INCLUDED_OSCRECEIVETEST_H

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
// #include <QSharedPointer>
#include <memory>

// class ConfigKey;
// UserSettingsPointer m_pConfig;

class ControlProxy;

class oscResult {
  public:
    QString oscAddress;
    QString oscGroup;
    QString oscKey;
    float oscValue;
};

// class OscReceiveTest : public QObject {
class OscReceiveTest {
    // Q_OBJECT
    OscReceiveTest(UserSettingsPointer pConfig);
    // OscReceiveTest(QObject* pParent,
    //         UserSettingsPointer pConfig);

  public:
    OscReceiveTest(){};
    //     OscReceiveTest(QObject* pParent,
    //                UserSettingsPointer pConfig);
    void OscReceiveTestMain();

    //  private slots:
    //    void ProcessMessage(const osc::ReceivedMessage& m,
    //            const IpEndpointName& remoteEndpoint);

    // int OscReceiveTestMain();
    //    void RunReceiveTest(int oscportin);

  private:
    UserSettingsPointer m_pConfig;

    //    ControlProxy* m_pCOCrossfader;
    //    ControlProxy* m_pCOCrossfaderReverse;
};

#endif /* INCLUDED_OSCSENDTESTS_H */
