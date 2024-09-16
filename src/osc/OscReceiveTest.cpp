#include "OscReceiveTest.h"

#include <QThread>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "config.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "osc/ip/UdpSocket.h"
#include "osc/osc/OscPacketListener.h"
#include "osc/osc/OscReceivedElements.h"
#include "preferences/configobject.h"
#include "osc/osc/OscOutboundPacketStream.h"
// #include <QSharedPointer>
// #include <mutex>

#define oscClientAddress "192.168.0.125"
// #define OscPortOut 9000
// #define OscPortIn 9001
#define OUTPUT_BUFFER_SIZE 1024
#define IP_MTU_SIZE 1536

// namespace osc{

// OscReceiveTest::OscReceiveTest(QObject* pParent, UserSettingsPointer pConfig)
//         : QObject(pParent),
//           m_pConfig(pConfig) {
// }

OscReceiveTest::OscReceiveTest(UserSettingsPointer pConfig)
        : m_pConfig(pConfig) {
}

class OscReceiveTestPacketListener : public osc::OscPacketListener {
  public:
    //    UserSettingsPointer m_pConfig;
    //  private:
    UserSettingsPointer m_pConfig;

  protected:
    void ProcessMessage(const osc::ReceivedMessage& m,
            const IpEndpointName& remoteEndpoint) {
        (void)remoteEndpoint;
        //  std::mutex mmm;
        //  constexpr int max_loop = 10;
        //  std::lock_guard<std::mutex> lock(mmm);
        //  for (int i = 0; i < max_loop; i++) {
        //    QString TempOscPortIn = m_pConfig->getValueString(ConfigKey("[OSC]", "OscPortIn"));
        //    QString TempOscPortIn = std::m_pConfig->getValueString(ConfigKey("[OSC]", "OscPortIn"));
        //    QString TempOscPortOut = m_pCorenfig->getValueString(ConfigKey("[OSC]", "OscPortOut"));
        //    QString MixxxOSCStatusFilePath = m_pCoreServices->getSettings()->getSettingsPath();
        //    QString MixxxOSCStatusFilePath = m_pCoreServices->getSettings()->getSettingsPath();
        //    std::shared_ptr<mixxx::CoreServices> pCoreServices : m_pCoreServices
        //  }
        //  std::lock_guard<std::mutex> unlock(mmm);

        try {
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();

            float oscInVal;
            args >> oscInVal >> osc::EndMessage;

            oscResult oscIn;
            oscIn.oscAddress = m.AddressPattern();
            oscIn.oscGroup, oscIn.oscKey;
            oscIn.oscAddress.replace("/", "");
            oscIn.oscValue = oscInVal;
            int posDel = oscIn.oscAddress.indexOf("@", 0, Qt::CaseInsensitive);
            if (posDel > 0) {
                oscIn.oscGroup = oscIn.oscAddress.mid(0, posDel);
                oscIn.oscGroup = "[" + oscIn.oscGroup + "]";
                oscIn.oscKey = oscIn.oscAddress.mid(posDel + 1, oscIn.oscAddress.length());

                //              QString MixxxOSCStatusFileLocation = m_pConfig->getSettingsPath() + "/MixxxOSCStatus.txt";
                //                QString MixxxOSCStatusFileLocation = "/MixxxOSCStatus.txt";
                //                QFile MixxxOSCStatusFile(MixxxOSCStatusFileLocation);
                //                MixxxOSCStatusFile.open(QIODevice::ReadWrite | QIODevice::Append);
                //                QTextStream MixxxOSCStatusTxt(&MixxxOSCStatusFile);
                //                MixxxOSCStatusTxt << QString(" received message @ GROUP: %1, KEY: %2 and VALUE : %3").arg(oscIn.oscGroup).arg(oscIn.oscKey).arg(oscIn.oscValue) << "\n";
                ControlObject::getControl(oscIn.oscGroup, oscIn.oscKey)->set(oscIn.oscValue);
                //                MixxxOSCStatusFile.close();

                char buffer[IP_MTU_SIZE];
                osc::OutboundPacketStream p(buffer, IP_MTU_SIZE);
                UdpTransmitSocket transmitSocket(IpEndpointName(oscClientAddress, 9000));
                //              UdpTransmitSocket transmitSocket(IpEndpointName(oscClientAddress, OscPortOut));

                QString oscMessageHeader = "/" + oscIn.oscAddress;
                QByteArray oscMessageHeaderBa = oscMessageHeader.toLocal8Bit();
                const char* oscMessage = oscMessageHeaderBa.data();
                //              float coVal = ControlObject::getControl(oscIn.oscGroup, oscIn.oscKey)->getValue;

                p.Clear();
                p << osc::BeginBundle();
                p << osc::BeginMessage(oscMessage) << oscIn.oscValue << osc::EndMessage;
                p << osc::EndBundle;
                transmitSocket.Send(p.Data(), p.Size());
            };

        } catch (osc::Exception& e) {
            //            std::cout << "error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n";
            //            QString MixxxOSCStatusFileLocation = "/MixxxOSCStatus.txt";
            //            QFile MixxxOSCStatusFile(MixxxOSCStatusFileLocation);
            //            MixxxOSCStatusFile.open(QIODevice::ReadWrite | QIODevice::Append);
            //            QTextStream MixxxOSCStatusTxt(&MixxxOSCStatusFile);
            //            MixxxOSCStatusTxt << QString(" an error occurred parsing OscMessage from %1 : %2").arg(m.AddressPattern()).arg(osc::e.what()) << "\n";
            //            MixxxOSCStatusFile.close();
        }
    }
};

void RunReceiveTest(int OscPortIn) {
    //    osc::OscReceiveTestPacketListener listener;
    OscReceiveTestPacketListener listener;
    UdpListeningReceiveSocket s(
            IpEndpointName(IpEndpointName::ANY_ADDRESS, OscPortIn),
            &listener);
    //            IpEndpointName(IpEndpointName::ANY_ADDRESS, OscPortIn),
    //    QString MixxxOSCStatusFileLocation = m_pConfig->getSettingsPath() + "/MixxxOSCStatus.txt";
    //    QString MixxxOSCStatusFileLocation = "/MixxxOSCStatus.txt";
    //     QFile MixxxOSCStatusFile(MixxxOSCStatusFileLocation);
    //    MixxxOSCStatusFile.remove();
    //    MixxxOSCStatusFile.open(QIODevice::ReadWrite | QIODevice::Append);
    //    QTextStream MixxxOSCStatusTxt(&MixxxOSCStatusFile);
    //    MixxxOSCStatusTxt << QString(" listening on port : %1").arg(m_pConfig->getValue(ConfigKey("[OSC]", "OscPortIn"))) << "\n";
    //    MixxxOSCStatusTxt << QString(" listening on port : %1").arg(OscPortIn) << "\n";
    //    MixxxOSCStatusFile.close();

    s.Run();
}

//} // namespace osc

#ifndef NO_OSC_TEST_MAIN

//void OscReceiveTest::OscReceiveTestMain() {
void OscReceiveTestMain() {
    QString MixxxOSCStatusFileLocation = "/MixxxOSCStatus.txt";
    // QString MixxxOSCStatusFileLocation = std::shared_ptr<mixxx>::m_pConfig->getSettingsPath() + "/MixxxOSCStatus.txt";
    // QString MixxxOSCStatusFileLocation = m_pConfig->getSettingsPath() + "/MixxxOSCStatus.txt";
    // QString MixxxOSCStatusFileLocation = MixxxMainWindow::MixxxMainWindow::m_pCoreServices->getSettings()->getSettingsPath() + "/MixxxOSCStatus.txt";
    // QString DeckStatusFilePath = m_pConfig->getSettingsPath();
    // QString MixxxOSCStatusFilePath = MixxxMainWindow::m_pCoreServices->getSettings()->getSettingsPath() + "/MixxxOSCStatus.txt";
    QFile MixxxOSCStatusFile(MixxxOSCStatusFileLocation);
    MixxxOSCStatusFile.remove();

    MixxxOSCStatusFile.open(QIODevice::ReadWrite | QIODevice::Append);
    QTextStream MixxxOSCStatusTxt(&MixxxOSCStatusFile);
    //     QString CKOscPortOut = m_pConfig->getValue(ConfigKey("[OSC]", "OscPortOut"));
    //     MixxxOSCStatusTxt << QString(" listening on port : %1").arg(CKOscPortOut) << "\n";
    //     MixxxOSCStatusTxt << QString(" listening on port : %1").arg( m_pConfig->getValue(ConfigKey("[OSC]", "OscPortIn"))) << "\n";
    // MixxxOSCStatusTxt << QString(" listening on port : %1").arg(TempOscPortOut) << "\n";
    int TempOscPortIn = 9001;
    MixxxOSCStatusTxt << QString(" listening on port : %1").arg(TempOscPortIn) << "\n";

    //    if (m_pConfig->getValue<bool>(ConfigKey("[OSC]", "OscEnabled"))) {
    //    MixxxOSCStatusTxt << QString(" this line is in the m_pconfig if") << "\n";
    //    QString CKOscPortIn = m_pConfig->getValue(ConfigKey("[OSC]", "OscPortIn"));
    //    int CKOscPortInInt =  CKOscPortIn.toInt();

    // std::thread tosc(osc::RunReceiveTest, CKOscPortInInt);
    //    std::thread tosc(osc::RunReceiveTest, TempOscPortOutInt);

    std::thread tosc(RunReceiveTest, TempOscPortIn);
    // std::thread tosc(osc::RunReceiveTest, 9001);
    tosc.detach();
    //}

    //    MixxxOSCStatusFile.close();

    //     return 0;
}

#endif /* NO_OSC_TEST_MAIN */
