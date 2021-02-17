#pragma once

#include "controllers/legacycontrollermapping.h"
#include "util/xml.h"

/// The LegacyControllerMappingFileHandler is used for serializing/deserializing the
/// LegacyControllerMapping objects to/from XML files and is also responsible
/// finding the script files that belong to a mapping in the file system.
///
/// Subclasses can implement the private load function to add support for XML
/// elements that are only useful for certain mapping types.
class LegacyControllerMappingFileHandler {
  public:
    LegacyControllerMappingFileHandler(){};
    virtual ~LegacyControllerMappingFileHandler(){};

    static LegacyControllerMappingPointer loadMapping(const QFileInfo& mappingFile,
            const QDir& systemMappingsPath);

    ///  Overloaded function for convenience
    ///
    /// @param path The path to a controller mapping XML file.
    /// @param systemMappingsPath Fallback directory for searching script files.
    LegacyControllerMappingPointer load(const QString& path, const QDir& systemMappingsPath);

    // Returns just the name of a given device (everything before the first
    // space)
    QString rootDeviceName(const QString& deviceName) const {
        return deviceName.left(deviceName.indexOf(" "));
    }

  protected:
    QDomElement getControllerNode(const QDomElement& root);

    void parseMappingInfo(const QDomElement& root,
            LegacyControllerMapping* mapping) const;

    /// Adds script files from XML to the LegacyControllerMapping.
    ///
    /// This function parses the supplied QDomElement structure, finds the
    /// matching script files inside the search paths and adds them to
    /// LegacyControllerMapping.
    ///
    /// @param root The root node of the XML document for the mapping.
    /// @param mapping The LegacyControllerMapping these scripts belong to.
    /// @param systemMappingsPath Fallback directory for searching script files.
    void addScriptFilesToMapping(const QDomElement& root,
            LegacyControllerMapping* mapping,
            const QDir& systemMappingsPath) const;

    /// Creates the XML document and includes what script files are currently
    /// loaded. Sub-classes need to call this before adding any other items.
    QDomDocument buildRootWithScripts(const LegacyControllerMapping& mapping) const;

    bool writeDocument(const QDomDocument& root, const QString& fileName) const;

  private:
    // Sub-classes implement this.
    virtual LegacyControllerMappingPointer load(const QDomElement& root,
            const QString& filePath,
            const QDir& systemMappingPath) = 0;
};
