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

    /** load(QString,QString,bool)
     * Overloaded function for convenience
     * @param path The path to a controller preset XML file.
     * @param deviceName The name/id of the controller
     * @param forceLoad Forces the preset to be loaded, regardless of whether or not the controller id
     *        specified within matches the name of this Controller.
     */
    ControllerPresetPointer load(const QString path, const QString deviceName,
                                 const bool forceLoad);

    // Returns just the name of a given device (everything before the first
    // space)
    QString rootDeviceName(QString deviceName) const {
        return deviceName.left(deviceName.indexOf(" "));
    }

  protected:
    QDomElement getControllerNode(const QDomElement& root,
                                  const QString deviceName,
                                  const bool forceLoad);

    void parsePresetInfo(const QDomElement& root,
                         ControllerPreset* preset) const;

    /** addScriptFilesToPreset(QDomElement,QString,bool)
     * Loads script files specified in a QDomElement structure into the supplied
     *   ControllerPreset.
     * @param root The root node of the XML document for the preset.
     * @param deviceName The name/id of the controller
     * @param forceLoad Forces the preset to be loaded, regardless of whether or not the controller id
     *        specified within matches the name of this Controller.
     * @param preset The ControllerPreset into which the scripts should be placed.
     */
    void addScriptFilesToPreset(const QDomElement& root,
                                ControllerPreset* preset) const;

    // Creates the XML document and includes what script files are currently
    // loaded. Sub-classes need to call this before adding any other items.
    QDomDocument buildRootWithScripts(const ControllerPreset& preset,
                                              const QString deviceName) const;

    bool writeDocument(QDomDocument root, const QString fileName) const;

  private:
    // Sub-classes implement this.
    virtual ControllerPresetPointer load(const QDomElement root, const QString deviceName,
                                         const bool forceLoad) = 0;
};

#endif
