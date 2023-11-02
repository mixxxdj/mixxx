#pragma once

#include <QDomElement>
#include <QString>
#include <memory>

class QFileInfo;
class QDir;
class LegacyControllerMapping;
#ifdef MIXXX_USE_QML
#include <QImage>
#include <QMap>
#include <bit>
#endif

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

    static std::shared_ptr<LegacyControllerMapping> loadMapping(const QFileInfo& mappingFile,
            const QDir& systemMappingsPath);

    ///  Overloaded function for convenience
    ///
    /// @param path The path to a controller mapping XML file.
    /// @param systemMappingsPath Fallback directory for searching script files.
    std::shared_ptr<LegacyControllerMapping> load(
            const QString& path, const QDir& systemMappingsPath);

    // Returns just the name of a given device (everything before the first
    // space)
    QString rootDeviceName(const QString& deviceName) const {
        return deviceName.left(deviceName.indexOf(" "));
    }

  protected:
    QDomElement getControllerNode(const QDomElement& root);

    void parseMappingInfo(const QDomElement& root,
            std::shared_ptr<LegacyControllerMapping> mapping) const;

    /// Adds script files and QML scenes from XML to the LegacyControllerMapping.
    ///
    /// This function parses the supplied QDomElement structure, finds the
    /// matching script files and QML scenes inside the search paths and adds them to
    /// LegacyControllerMapping.
    ///
    /// @param root The root node of the XML document for the mapping.
    /// @param mapping The LegacyControllerMapping these scripts belong to.
    /// @param systemMappingsPath Fallback directory for searching script files.
    void addScriptFilesToMapping(const QDomElement& root,
            std::shared_ptr<LegacyControllerMapping> mapping,
            const QDir& systemMappingsPath) const;

    /// Creates the XML document and includes what script files are currently
    /// loaded. Sub-classes need to call this before adding any other items.
    QDomDocument buildRootWithScripts(const LegacyControllerMapping& mapping) const;

    bool writeDocument(const QDomDocument& root, const QString& fileName) const;

    /// Find script file in the mapping or system path.
    ///
    /// @param mapping The controller mapping the script belongs to.
    /// @param filename The script filename.
    /// @param systemMappingsPath The system mappings path to use as fallback.
    /// @return Returns a QFileInfo object. If the script was not found in either
    /// of the search directories, the QFileInfo object might point to a
    /// non-existing file.
    static QFileInfo findScriptFile(std::shared_ptr<LegacyControllerMapping> mapping,
            const QString& filename,
            const QDir& systemMappingsPath);

#ifdef MIXXX_USE_QML
    /// Find a module directory (QML) in the mapping or system path.
    ///
    /// @param mapping The controller mapping the module directory belongs to.
    /// @param dirname The module directory name.
    /// @param systemMappingsPath The system mappings path to use as fallback.
    /// @return Returns a QFileInfo object. If the script was not found in either
    /// of the search directories, the QFileInfo object might point to a
    /// non-existing file.
    static QFileInfo findLibraryPath(std::shared_ptr<LegacyControllerMapping> mapping,
            const QString& dirname,
            const QDir& systemMappingsPath);
#endif

  private:
    // Sub-classes implement this.
    virtual std::shared_ptr<LegacyControllerMapping> load(const QDomElement& root,
            const QString& filePath,
            const QDir& systemMappingPath) = 0;

#ifdef MIXXX_USE_QML
    static QMap<QString, QImage::Format> kSupportedPixelFormat;
    static QMap<QString, std::endian> kEndianFormat;

    friend class ControllerRenderingEngineTest;
#endif
};

#ifdef MIXXX_USE_QML
// Maximum target frame per request for a a screen controller
#define MAX_TARGET_FPS 240

// Maximum time allowed for a screen to run a splash off animation
#define MAX_SPLASHOFF_DURATION 3000
#endif
