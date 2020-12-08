#pragma once

class MidiControllerPreset;
class HidControllerPreset;

class ControllerPresetVisitor {
  public:
    virtual ~ControllerPresetVisitor() {}
    virtual void visit(MidiControllerPreset* preset) = 0;
    virtual void visit(HidControllerPreset* preset) = 0;
};

class ConstControllerPresetVisitor {
  public:
    virtual ~ConstControllerPresetVisitor() {}
    virtual void visit(const MidiControllerPreset* preset) = 0;
    virtual void visit(const HidControllerPreset* preset) = 0;
};
