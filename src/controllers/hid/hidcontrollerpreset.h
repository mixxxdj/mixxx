#pragma once
/**
 * @file hidcontrollerpreset.h
 * @brief HID/Bulk Controller Preset
 *
 * This class represents a HID or Bulk controller preset, containing the data
 * elements that make it up.
 */

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/hid/hidcontrollerpresetfilehandler.h"

class HidControllerPreset : public ControllerPreset {
  public:
    HidControllerPreset() {}
    virtual ~HidControllerPreset() {}

    bool savePreset(const QString& fileName) const override;

    virtual void accept(ControllerPresetVisitor* visitor);
    virtual void accept(ConstControllerPresetVisitor* visitor) const;
    virtual bool isMappable() const;
};
