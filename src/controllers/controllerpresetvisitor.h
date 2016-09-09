#ifndef CONTROLLERPRESETVISITOR_H
#define CONTROLLERPRESETVISITOR_H

class MidiControllerPreset;
class HidControllerPreset;
class KeyboardControllerPreset;

class ControllerPresetVisitor {
  public:
    virtual ~ControllerPresetVisitor() {}
    virtual void visitKeyboard(KeyboardControllerPreset* preset) = 0;
    virtual void visitMidi(MidiControllerPreset* preset) = 0;
    virtual void visitHid(HidControllerPreset* preset) = 0;
};

class ConstControllerPresetVisitor {
  public:
    virtual ~ConstControllerPresetVisitor() {}
    virtual void visitKeyboard(const KeyboardControllerPreset* preset) = 0;
    virtual void visitMidi(const MidiControllerPreset* preset) = 0;
    virtual void visitHid(const HidControllerPreset* preset) = 0;
};

#endif /* CONTROLLERPRESETVISITOR_H */
