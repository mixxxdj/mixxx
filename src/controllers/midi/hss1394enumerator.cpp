/**
* @file hss1394enumerator.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Thu 15 Mar 2012
*/

#include <hss1394/HSS1394.h>

#include "controllers/midi/hss1394controller.h"
#include "controllers/midi/hss1394enumerator.h"

Hss1394Enumerator::Hss1394Enumerator() : MidiEnumerator() {
}

Hss1394Enumerator::~Hss1394Enumerator() {
    qDebug() << "Deleting HSS1394 devices...";
    QListIterator<Controller*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        delete dev_it.next();
    }
    using namespace hss1394;
    Node::Shutdown();
}

// Enumerate the HSS1394 MIDI devices
QList<Controller*> Hss1394Enumerator::queryDevices() {
    using namespace hss1394;

    // HSS1394 does not support hotplug: if any devices were already
    // loaded, return current list
    if (m_devices.length()>0)
        return m_devices;

    hss1394::uint uNodes = Node::Instance()->GetNodeCount();

    for(hss1394::uint i=0; i<40; i++) {
        TNodeInfo tNodeInfo;
        bool bInstalled;
        if (Node::Instance()->GetNodeInfo(tNodeInfo, i, NULL, &bInstalled)) {
            QString message = QString("Node %1 (%2): Name = <%3>, GUID = %4 %5, FW[%6]")
                    .arg(QString::number(i),
                         (bInstalled)?"installed":"not installed",
                         tNodeInfo.sName.c_str(),
                         QString("%1").arg(tNodeInfo.uGUID.mu32High, 0, 16),
                         QString("%1").arg(tNodeInfo.uGUID.mu32Low, 0, 16),
                         QString("%1").arg(tNodeInfo.uProtocolVersion, 0, 16));
            qDebug() << " " << message;
            Hss1394Controller *currentDevice = new Hss1394Controller(
                tNodeInfo, i);
            m_devices.push_back(currentDevice);
        }
    }
    return m_devices;
}
