#pragma once

#include "util/xml.h"
#include "controllers/controllerpreset.h"

/// The ControllerPresetFileHandler is used for serializing/deserializing the
/// ControllerPreset objects to/from XML files and is also responsible
/// finding the script files that belong to a preset in the file system.
///
/// Subclasses can implement the private load function to add support for XML
/// elements that are only useful for certain mapping types.
class ControllerPresetFileHandler {
  public:
    ControllerPresetFileHandler() {};
    virtual ~ControllerPresetFileHandler() {};

    static ControllerPresetPointer loadPreset(const QFileInfo& presetFile,
            const QDir& systemPresetsPath);

    ///  Overloaded function for convenience
    ///
    /// @param path The path to a controller preset XML file.
    /// @param systemPresetsPath Fallback directory for searching script files.
    ControllerPresetPointer load(const QString& path, const QDir& systemPresetsPath);

    // Returns just the name of a given device (everything before the first
    // space)
    QString rootDeviceName(const QString& deviceName) const {
        return deviceName.left(deviceName.indexOf(" "));
    }

  protected:
    QDomElement getControllerNode(const QDomElement& root);

    void parsePresetInfo(const QDomElement& root,
                         ControllerPreset* preset) const;

    /// Adds script files from XML to the ControllerPreset.
    ///
    /// This function parses the supplied QDomElement structure, finds the
    /// matching script files inside the search paths and adds them to
    /// ControllerPreset.
    ///
    /// @param root The root node of the XML document for the preset.
    /// @param preset The ControllerPreset these scripts belong to.
    /// @param systemPresetsPath Fallback directory for searching script files.
    void addScriptFilesToPreset(const QDomElement& root,
            ControllerPreset* preset,
            const QDir& systemPresetsPath) const;

    /// Creates the XML document and includes what script files are currently
    /// loaded. Sub-classes need to call this before adding any other items.
    QDomDocument buildRootWithScripts(const ControllerPreset& preset) const;

    bool writeDocument(const QDomDocument& root, const QString& fileName) const;

  private:
    // Sub-classes implement this.
    virtual ControllerPresetPointer load(const QDomElement& root,
            const QString& filePath,
            const QDir& systemPresetPath) = 0;
};
