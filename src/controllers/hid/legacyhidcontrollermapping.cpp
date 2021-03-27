/// HID/Bulk Controller Mapping
///
/// This class represents a HID or Bulk controller mapping, containing the data
/// elements that make it up.

#include "controllers/hid/legacyhidcontrollermapping.h"

#include "controllers/defs_controllers.h"
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"

bool LegacyHidControllerMapping::saveMapping(const QString& fileName) const {
    LegacyHidControllerMappingFileHandler handler;
    return handler.save(*this, fileName);
}

bool LegacyHidControllerMapping::isMappable() const {
    return false;
}
