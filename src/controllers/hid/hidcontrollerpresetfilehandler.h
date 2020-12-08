#pragma once

#include "controllers/hid/hidcontrollerpreset.h"
#include "controllers/controllerpresetfilehandler.h"

class HidControllerPresetFileHandler : public ControllerPresetFileHandler {
  public:
    HidControllerPresetFileHandler() {};
    virtual ~HidControllerPresetFileHandler() {};

    bool save(const HidControllerPreset& preset, const QString& fileName) const;

  private:
    virtual ControllerPresetPointer load(const QDomElement& root,
            const QString& filePath,
            const QDir& systemPresetsPath);
};
