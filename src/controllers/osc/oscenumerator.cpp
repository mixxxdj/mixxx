#include "controllers/osc/oscenumerator.h"

OscEnumerator::OscEnumerator() {
    m_devices.push_back(new OscController());
}

OscEnumerator::~OscEnumerator() {
    QListIterator<Controller*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        delete dev_it.next();
    }
}

QList<Controller*> OscEnumerator::queryDevices() {
    return m_devices;
}
