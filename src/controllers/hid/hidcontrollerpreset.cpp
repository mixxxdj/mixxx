/// HID/Bulk Controller Preset
///
/// This class represents a HID or Bulk controller preset, containing the data
/// elements that make it up.

#include "controllers/hid/hidcontrollerpreset.h"

#include "controllers/controllerpresetvisitor.h"
#include "controllers/defs_controllers.h"
#include "controllers/hid/hidcontrollerpresetfilehandler.h"

bool HidControllerPreset::savePreset(const QString& fileName) const {
    HidControllerPresetFileHandler handler;
    return handler.save(*this, fileName);
}

void HidControllerPreset::accept(ControllerPresetVisitor* visitor) {
    if (visitor) {
        visitor->visit(this);
    }
}

void HidControllerPreset::accept(ConstControllerPresetVisitor* visitor) const {
    if (visitor) {
        visitor->visit(this);
    }
}

bool HidControllerPreset::isMappable() const {
    return false;
}
