#ifndef KEYBOARDENUMERATOR_H
#define KEYBOARDENUMERATOR_H

#include "controllers/controllerenumerator.h"

class KeyboardEventFilter;

class KeyboardEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    KeyboardEnumerator(KeyboardEventFilter* pKeyboard);
    virtual ~KeyboardEnumerator();
    virtual QList<Controller*> queryDevices() override;

  private:
    QList<Controller*> m_devices;
    KeyboardEventFilter* m_pKeyboard;
};


#endif // KEYBOARDENUMERATOR_H
