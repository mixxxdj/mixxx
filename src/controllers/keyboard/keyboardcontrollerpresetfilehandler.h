#ifndef KEYBOARDCONTROLLERPRESETFILEHANDLER_H
#define KEYBOARDCONTROLLERPRESETFILEHANDLER_H

#include "controllers/controllerpresetfilehandler.h"
#include "controllers/keyboard/keyboardcontrollerpreset.h"

class KeyboardControllerPresetFileHandler : public ControllerPresetFileHandler {
public:
    KeyboardControllerPresetFileHandler();
    virtual ~KeyboardControllerPresetFileHandler();

    bool save(const KeyboardControllerPreset& preset,
              const QString deviceName, const QString fileName) const;

private:
    virtual ControllerPresetPointer load(const QDomElement root, const QString deviceName);
};


#endif //KEYBOARDCONTROLLERPRESETFILEHANDLER_H
