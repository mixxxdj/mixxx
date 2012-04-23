#ifndef HIDCONTROLLERPRESET_H
#define HIDCONTROLLERPRESET_H

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"

class HidControllerPreset : public ControllerPreset {
  public:
    HidControllerPreset() {};
    virtual ~HidControllerPreset() {};

    virtual void accept(ControllerPresetVisitor* visitor) const {
        visitor->visit(this);
    }
};

#endif /* HIDCONTROLLERPRESET_H */
