#include "keyboardenumerator.h"
#include "keyboardcontroller.h"

class KeyboardEventFilter;

KeyboardEnumerator::KeyboardEnumerator(KeyboardEventFilter* pKeyboard) : m_pKeyboard(pKeyboard) {}

KeyboardEnumerator::~KeyboardEnumerator() {
    qDebug() << "Deleting Keyboard controller...";
    while (m_devices.size() > 0) {
        delete m_devices.takeLast();
    }
}

QList<Controller *> KeyboardEnumerator::queryDevices() {
    // TODO(Tomasito) Add check to see if there is indeed a keyboard connected
    m_devices.push_back(new KeyboardController(m_pKeyboard));

    return m_devices;
}
