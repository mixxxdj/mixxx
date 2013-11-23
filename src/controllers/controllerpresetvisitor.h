#ifndef CONTROLLERPRESETVISITOR_H
#define CONTROLLERPRESETVISITOR_H

class MidiControllerPreset;
class HidControllerPreset;

class ControllerPresetVisitor {
  public:
    virtual void visit(const MidiControllerPreset* preset) = 0;
    virtual void visit(const HidControllerPreset* preset) = 0;
};

#endif /* CONTROLLERPRESETVISITOR_H */

