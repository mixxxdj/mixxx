#pragma once

#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"
#include "controllers/legacycontrollermapping.h"

/// This class represents a HID or Bulk controller mapping, containing the data
/// elements that make it up.
class LegacyHidControllerMapping : public LegacyControllerMapping {
  public:
    LegacyHidControllerMapping() {
    }
    ~LegacyHidControllerMapping() override {
    }

    std::shared_ptr<LegacyControllerMapping> clone() const override {
        return std::make_shared<LegacyHidControllerMapping>(*this);
    }

    bool saveMapping(const QString& fileName) const override;

    bool isMappable() const override;
};
