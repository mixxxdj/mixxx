#ifndef HIDCONTROLLERPRESETFILEHANDLER_H
#define HIDCONTROLLERPRESETFILEHANDLER_H

#include <QDir>
#include <QDomElement>
#include <QString>

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetfilehandler.h"
#include "controllers/hid/hidcontrollerpreset.h"

class HidControllerPreset;

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

#endif /* HIDCONTROLLERPRESETFILEHANDLER_H */
