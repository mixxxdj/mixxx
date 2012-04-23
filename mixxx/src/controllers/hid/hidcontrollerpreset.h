#ifndef HIDCONTROLLERPRESET_H
#define HIDCONTROLLERPRESET_H

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"

class HidControllerPreset : public ControllerPreset {
  public:
    HidControllerPreset() {}
    virtual ~HidControllerPreset() {}

    virtual void accept(ControllerPresetVisitor* visitor) const {
        if (visitor) {
            visitor->visit(this);
        }
    }
};

#endif /* HIDCONTROLLERPRESET_H */
