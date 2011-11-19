/***************************************************************************
                           hss1394enumerator.cpp
                    HSS1394 Device Enumerator Class
                    --------------------------------
    begin                : Fri Feb 26 2010
    copyright            : (C) 2010 Sean M. Pappalardo
    email                : spappalardo@mixxx.org

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <hss1394/HSS1394.h>

#include "mididevicehss1394.h"
#include "hss1394enumerator.h"


Hss1394Enumerator::Hss1394Enumerator() : MidiDeviceEnumerator() {
}

Hss1394Enumerator::~Hss1394Enumerator() {
    qDebug() << "Deleting HSS1394 devices...";
    QListIterator<MidiDevice*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        delete dev_it.next();
    }
    using namespace hss1394;
	Node::Shutdown();
}

// Enumerate the HSS1394 MIDI devices
QList<MidiDevice*> Hss1394Enumerator::queryDevices() {
    qDebug() << "Scanning HSS1394 devices:";
    using namespace hss1394;

    hss1394::uint uNodes = Node::Instance()->GetNodeCount();
    qDebug() << "   Nodes detected:" << uNodes;

    for(hss1394::uint i=0; i<40; i++) {
        TNodeInfo tNodeInfo;
        bool bInstalled;
		if (true == Node::Instance()->GetNodeInfo(tNodeInfo, i, NULL, &bInstalled)) {
            QString message = QString("Node %1 (%2): Name = <%3>, GUID = %4 %5, FW[%6]")
                .arg(i)
                .arg((bInstalled)?"installed":"not installed")
                .arg(tNodeInfo.sName.c_str())
                .arg(tNodeInfo.uGUID.mu32High, 0, 16)
                .arg(tNodeInfo.uGUID.mu32Low, 0 ,16)
                .arg(tNodeInfo.uProtocolVersion, 0, 16);
            qDebug() << " " << message;
            MidiDeviceHss1394 *currentDevice = new MidiDeviceHss1394(/*new MidiControlProcessor(NULL)*/ NULL,
                                                                          tNodeInfo,
                                                                          i);
            m_devices.push_back((MidiDevice*)currentDevice);
		}

	}

    return m_devices;
}
