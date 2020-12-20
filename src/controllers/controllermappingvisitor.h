#pragma once

class LegacyMidiControllerMapping;
class LegacyHidControllerMapping;

class LegacyControllerMappingVisitor {
  public:
    virtual ~LegacyControllerMappingVisitor() {
    }
    virtual void visit(LegacyMidiControllerMapping* mapping) = 0;
    virtual void visit(LegacyHidControllerMapping* mapping) = 0;
};

class ConstLegacyControllerMappingVisitor {
  public:
    virtual ~ConstLegacyControllerMappingVisitor() {
    }
    virtual void visit(const LegacyMidiControllerMapping* mapping) = 0;
    virtual void visit(const LegacyHidControllerMapping* mapping) = 0;
};
