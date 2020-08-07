#pragma once
/// @file hidcontrollerpreset.h
/// @brief HID/Bulk Controller Preset

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/hid/hidcontrollerpresetfilehandler.h"

/// This class represents a HID or Bulk controller preset, containing the data
/// elements that make it up.
class HidControllerPreset : public ControllerPreset {
  public:
    HidControllerPreset() {
    }
    ~HidControllerPreset() override {
    }

    bool savePreset(const QString& fileName) const override;

    void accept(ControllerPresetVisitor* visitor) override;
    void accept(ConstControllerPresetVisitor* visitor) const override;
    bool isMappable() const override;
};
