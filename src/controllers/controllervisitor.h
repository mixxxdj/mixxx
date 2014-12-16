#ifndef CONTROLLERVISITOR_H
#define CONTROLLERVISITOR_H

class MidiController;
class HidController;
class BulkController;

class ControllerVisitor {
  public:
    virtual void visit(MidiController* controller) = 0;
    virtual void visit(HidController* controller) = 0;
    virtual void visit(BulkController* controller) = 0;
};

#endif /* CONTROLLERVISITOR_H */
