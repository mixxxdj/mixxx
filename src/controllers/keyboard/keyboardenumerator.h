#ifndef KEYBOARDENUMERATOR_H
#define KEYBOARDENUMERATOR_H

#include "controllers/controllerenumerator.h"

class KeyboardEnumerator : public ControllerEnumerator {
    Q_OBJECT

public:
    KeyboardEnumerator();
    virtual ~KeyboardEnumerator();

    virtual QList<Controller*> queryDevices();

private:
    QList<Controller*> m_devices;
};


#endif // KEYBOARDENUMERATOR_H
