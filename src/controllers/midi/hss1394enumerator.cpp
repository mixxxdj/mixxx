#include "controllers/midi/hss1394enumerator.h"

#include <hss1394/HSS1394.h>

#include <memory>

#include "controllers/midi/hss1394controller.h"
#include "moc_hss1394enumerator.cpp"

Hss1394Enumerator::Hss1394Enumerator()
        : MidiEnumerator() {
}

Hss1394Enumerator::~Hss1394Enumerator() {
    qDebug() << "Deleting HSS1394 devices...";
    m_devices.clear();
    using namespace hss1394;
    Node::Shutdown();
}

// Enumerate the HSS1394 MIDI devices
QList<Controller*> Hss1394Enumerator::queryDevices() {
    qDebug() << "Scanning HSS1394 devices:";
    using namespace hss1394;

    hss1394::uint uNodes = Node::Instance()->GetNodeCount();
    qDebug() << "   Nodes detected:" << uNodes;

    m_devices.clear();

    for(hss1394::uint i=0; i<40; i++) {
        TNodeInfo tNodeInfo;
        bool bInstalled;
        if (Node::Instance()->GetNodeInfo(tNodeInfo, i, nullptr, &bInstalled)) {
            qDebug() << "Node" << i << "("
                     << (bInstalled ? "installed" : "not installed")
                     << "): Name = <" << tNodeInfo.sName.c_str() << ">, GUID ="
                     << QString::number(tNodeInfo.uGUID.mu32High, 16)
                     << QString::number(tNodeInfo.uGUID.mu32Low, 16) << ", FW["
                     << QString::number(tNodeInfo.uProtocolVersion, 16) << "]";

            m_devices.push_back(std::make_unique<Hss1394Controller>(tNodeInfo, i));
        }
    }
    QList<Controller*> devices;
    devices.reserve(m_devices.size());
    for (const auto& pDevice : m_devices) {
        devices.push_back(pDevice.get());
    }
    return devices;
}
