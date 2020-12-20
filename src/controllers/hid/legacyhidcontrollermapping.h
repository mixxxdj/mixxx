#pragma once

#include "controllers/legacycontrollermapping.h"
#include "controllers/controllermappingvisitor.h"
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"

/// This class represents a HID or Bulk controller mapping, containing the data
/// elements that make it up.
class LegacyHidControllerMapping : public LegacyControllerMapping {
  public:
    LegacyHidControllerMapping() {
    }
    ~LegacyHidControllerMapping() override {
    }

    bool saveMapping(const QString& fileName) const override;

    void accept(LegacyControllerMappingVisitor* visitor) override;
    void accept(ConstLegacyControllerMappingVisitor* visitor) const override;
    bool isMappable() const override;
};
