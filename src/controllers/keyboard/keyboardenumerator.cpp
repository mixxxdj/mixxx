#include "keyboardenumerator.h"
#include "keyboardcontroller.h"

KeyboardEnumerator::KeyboardEnumerator() {}

KeyboardEnumerator::~KeyboardEnumerator() {
    qDebug() << "Deleting Keyboard controller...";
    while (m_devices.size() > 0) {
        delete m_devices.takeLast();
    }
}

QList<Controller *> KeyboardEnumerator::queryDevices() {
    // TODO(Tomasito) Add check to see if there is indeed a keyboard connected
    m_devices.push_back(new KeyboardController());

    return m_devices;
}
