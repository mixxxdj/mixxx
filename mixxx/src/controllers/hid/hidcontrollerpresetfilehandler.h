#ifndef HIDCONTROLLERPRESETFILEHANDLER_H
#define HIDCONTROLLERPRESETFILEHANDLER_H

#include "controllers/hid/hidcontrollerpreset.h"
#include "controllers/controllerpresetfilehandler.h"

class HidControllerPresetFileHandler : public ControllerPresetFileHandler {
  public:
    HidControllerPresetFileHandler() {};
    virtual ~HidControllerPresetFileHandler() {};

    bool save(const HidControllerPreset& preset,
              const QString deviceName, const QString fileName) const;

  private:
    virtual ControllerPreset* load(const QDomElement root,
                                   const QString deviceName,
                                   const bool forceLoad);
};

#endif /* HIDCONTROLLERPRESETFILEHANDLER_H */
