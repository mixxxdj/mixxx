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
    // Parse *.kbd.xml file and load into a KeyboardControllerPreset. Returns a
    // ControllerPresetPointer pointing to this KeyboardControllerPreset
    virtual ControllerPresetPointer load(const QDomElement root, const QString deviceName) override;

    // Add <group> blocks to the given QDomDocument, containing <control /> nodes, holding info about
    // the action that will be triggered and the QKeySequence that will trigger that action
    void addControlsToDocument(const KeyboardControllerPreset& preset, QDomDocument* doc) const;
};


#endif //KEYBOARDCONTROLLERPRESETFILEHANDLER_H
