/**
* @file controllerpresetfilehandler.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Mon 9 Apr 2012
* @brief Handles loading and saving of Controller presets.
*
*/
#ifndef CONTROLLERPRESETFILEHANDLER_H
#define CONTROLLERPRESETFILEHANDLER_H

#include "xmlparse.h"
#include "controllers/controllerpreset.h"

class ControllerPresetFileHandler {
  public:
    ControllerPresetFileHandler() {};
    virtual ~ControllerPresetFileHandler() {};

    ControllerPreset* load(const QString path, const QString deviceName,
                                   const bool forceLoad);

    // Returns just the name of a given device (everything before the first
    // space)
    QString rootDeviceName(QString deviceName) const {
        return deviceName.left(deviceName.indexOf(" "));
    }

  protected:
    void addScriptFilesToPreset(const QDomElement root,
                                 const QString deviceName,
                                 const bool forceLoad,
                                 ControllerPreset* preset) const;

    // Creates the XML document and includes what script files are currently
    // loaded. Sub-classes need to call this before adding any other items.
    QDomDocument buildRootWithScripts(const ControllerPreset& preset,
                                              const QString deviceName) const;

    bool writeDocument(QDomDocument root, const QString fileName) const;

  private:
    // Sub-classes implement this.
    virtual ControllerPreset* load(const QDomElement root, const QString deviceName,
                                   const bool forceLoad) = 0;
};

#endif
