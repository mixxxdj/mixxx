#pragma once

#include <QDomElement>
#include <QString>
#include <memory>
#ifdef MIXXX_USE_QML
#include <QImage>
#include <QMap>
#include <bit>

#include "controllers/legacycontrollermapping.h"
#else
class LegacyControllerMapping;
#endif

class QFileInfo;
class QDir;
class LegacyControllerSettingsLayoutContainer;

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

    /// @brief Parse the setting definition block from the root node if any.
    /// @param root The root node (MixxxControllerPreset)
    /// @param mapping The mapping object to populate with the gathered data
    void parseMappingSettings(const QDomElement& root,
            LegacyControllerMapping* mapping) const;

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

  private:
    /// @brief Recursively parse setting definition and layout information
    /// within a setting node
    /// @param current The setting node (MixxxControllerPreset.settings) or any
    /// children nodes
    /// @param mapping The mapping object to populate with the gathered data
    /// @param layout The currently active layout, on which new setting item
    /// (leaf) should be attached
    void parseMappingSettingsElement(const QDomElement& current,
            LegacyControllerMapping* pMapping,
            LegacyControllerSettingsLayoutContainer* pLayout) const;

    // Sub-classes implement this.
    virtual std::shared_ptr<LegacyControllerMapping> load(const QDomElement& root,
            const QString& filePath,
            const QDir& systemMappingPath) = 0;

#ifdef MIXXX_USE_QML
  public:
    static QMap<QString, QImage::Format> kSupportedPixelFormat;
    static QMap<QString, LegacyControllerMapping::ScreenInfo::ColorEndian> kEndianFormat;
    // Maximum target frame per request for a screen controller
    static constexpr int kMaxTargetFps = 240;
    // Maximum MSAA value that can be used
    static constexpr int kMaxMsaa = 16;
    // Maximum time allowed for a screen to run a splash off animation
    static constexpr int kMaxSplashOffDuration = 3000;

    friend class ControllerRenderingEngineTest;
#endif
    friend class LegacyControllerMappingSettingsTest_parseSettingBlock_Test;
};
