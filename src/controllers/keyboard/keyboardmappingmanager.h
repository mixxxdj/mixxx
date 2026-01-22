#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QSharedPointer>

#include "preferences/configobject.h"
#include "preferences/usersettings.h"

/// Metadata for a keyboard mapping
struct KeyboardMappingInfo {
    QString filePath;
    QString name;
    QString author;
    QString description;
    QString version;
    bool isDefault;
    bool isReadOnly;

    KeyboardMappingInfo()
        : isDefault(false),
          isReadOnly(false) {
    }
};

/// Manages keyboard mappings including loading, saving, and enumeration
class KeyboardMappingManager : public QObject {
    Q_OBJECT

  public:
    explicit KeyboardMappingManager(UserSettingsPointer pConfig, QObject* parent = nullptr);
    ~KeyboardMappingManager() override;

    /// Get list of all available keyboard mappings
    QList<KeyboardMappingInfo> getAvailableMappings() const;

    /// Get info for a specific mapping by file path
    KeyboardMappingInfo getMappingInfo(const QString& filePath) const;

    /// Load a keyboard mapping from file
    QSharedPointer<ConfigObject<ConfigValueKbd>> loadMapping(const QString& filePath);

    /// Save current mapping to a custom file
    bool saveMapping(const QString& filePath,
                     const KeyboardMappingInfo& info,
                     ConfigObject<ConfigValueKbd>* pMapping);

    /// Create a new empty custom mapping
    QString createCustomMapping(const QString& name,
                                const QString& author,
                                const QString& description);

    /// Delete a custom mapping
    bool deleteMapping(const QString& filePath);
    
    /// Export mapping to JSON
    bool exportToJson(const QString& filePath,
                      const KeyboardMappingInfo& info,
                      ConfigObject<ConfigValueKbd>* pMapping);
                      
    /// Import mapping from JSON
    bool importFromJson(const QString& filePath,
                        KeyboardMappingInfo* pInfo,
                        QSharedPointer<ConfigObject<ConfigValueKbd>> pMapping);
                        
    /// Export mapping to XML
    bool exportToXml(const QString& filePath,
                     const KeyboardMappingInfo& info,
                     ConfigObject<ConfigValueKbd>* pMapping);
                     
    /// Import mapping from XML
    bool importFromXml(const QString& filePath,
                       KeyboardMappingInfo* pInfo,
                       QSharedPointer<ConfigObject<ConfigValueKbd>> pMapping);

    /// Get the default mapping path for current locale
    QString getDefaultMappingPath() const;

    /// Get the user's custom mappings directory
    QString getCustomMappingsDirectory() const;

    /// Register an external mapping file
    void addExternalMapping(const QString& filePath);

  signals:
    /// Emitted when the list of available mappings changes
    void mappingsChanged();

  private:
    /// Scan for available keyboard mappings
    void scanMappings();

    /// Parse metadata from a .kbd.cfg file
    KeyboardMappingInfo parseMetadata(const QString& filePath) const;

    /// Get locale-specific default mapping
    QString getLocaleDefaultMapping() const;

    UserSettingsPointer m_pConfig;
    QMap<QString, KeyboardMappingInfo> m_mappings;
    QStringList m_externalMappings;
    QString m_customMappingsDir;
};
