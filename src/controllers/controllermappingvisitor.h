#pragma once

class LegacyMidiControllerMapping;
class LegacyHidControllerMapping;

class ConstLegacyControllerMappingVisitor {
  public:
    virtual ~ConstLegacyControllerMappingVisitor() {
    }
    virtual void visit(const LegacyMidiControllerMapping* mapping) = 0;
    virtual void visit(const LegacyHidControllerMapping* mapping) = 0;
};
