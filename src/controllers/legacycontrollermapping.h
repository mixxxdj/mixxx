#pragma once

#include <util/assert.h>

#include <QDir>
#include <QFileInfo>

#include "controllers/legacycontrollersettings.h"
#include "controllers/legacycontrollersettingslayout.h"
#include "defs_urls.h"
#include "preferences/usersettings.h"

/// This class represents a controller mapping, containing the data elements that
/// make it up.
class LegacyControllerMapping {
  protected:
    LegacyControllerMapping()
            : m_bDirty(false),
              m_deviceDirection(DeviceDirection::Bidirectionnal) {
    }
    LegacyControllerMapping(const LegacyControllerMapping& other)
            : m_productMatches(other.m_productMatches),
              m_bDirty(other.m_bDirty),
              m_deviceId(other.m_deviceId),
              m_filePath(other.m_filePath),
              m_name(other.m_name),
              m_author(other.m_author),
              m_description(other.m_description),
              m_forumlink(other.m_forumlink),
              m_manualPage(other.m_manualPage),
              m_wikilink(other.m_wikilink),
              m_schemaVersion(other.m_schemaVersion),
              m_mixxxVersion(other.m_mixxxVersion),
              m_settings(other.m_settings),
              m_settingsLayout(other.m_settingsLayout.get() != nullptr
                              ? other.m_settingsLayout->clone()
                              : nullptr),
              m_scripts(other.m_scripts),
              m_deviceDirection(other.m_deviceDirection) {
    }
    LegacyControllerMapping& operator=(const LegacyControllerMapping&) = delete;
    LegacyControllerMapping(LegacyControllerMapping&&) = delete;
    LegacyControllerMapping& operator=(LegacyControllerMapping&&) = delete;
    virtual ~LegacyControllerMapping() = default;

  public:
    struct ScriptFileInfo {
        ScriptFileInfo()
                : builtin(false) {
        }

        QString name;
        QString functionPrefix;
        QFileInfo file;
        bool builtin;
    };

    // TODO (xxx): this is a temporary solution to address devices that don't
    // support and need bidirectional communication and lead to
    // polling/performance issues. The proper solution would involve refactoring
    // the bulk integration to perform a better endpoint capability discovery
    // and let Mixxx decide communication direction depending of the hardware
    // capabilities
    enum class DeviceDirection : uint8_t {
        Outgoing = 0x1,
        Incoming = 0x2,
        Bidirectionnal = 0x3
    };
    Q_DECLARE_FLAGS(DeviceDirections, DeviceDirection)

    /// Adds a script file to the list of controller scripts for this mapping.
    /// @param filename Name of the script file to add
    /// @param functionprefix The script's function prefix (or empty string)
    /// @param file A FileInfo object pointing to the script file
    /// @param builtin If this is true, the script won't be written to the XML
    void addScriptFile(const QString& name,
            const QString& functionprefix,
            const QFileInfo& file,
            bool builtin = false) {
        ScriptFileInfo info;
        info.name = name;
        info.functionPrefix = functionprefix;
        info.file = file;
        info.builtin = builtin;
        m_scripts.append(info);
        setDirty(true);
    }

    /// Adds a setting option to the list of setting option for this mapping.
    /// The option added must be a valid option.
    /// @param option The option to add
    /// @return whether or not the setting was added successfully.
    bool addSetting(std::shared_ptr<AbstractLegacyControllerSetting> option) {
        VERIFY_OR_DEBUG_ASSERT(option->valid()) {
            return false;
        }
        for (const auto& setting : std::as_const(m_settings)) {
            if (*setting == *option) {
                qWarning() << "Mapping setting duplication detected for "
                              "setting with name"
                           << option->variableName()
                           << ". Keeping the first occurrence.";
                return false;
            }
        }
        m_settings.append(option);
        return true;
    }

    /// @brief Set a setting layout as they should be perceived when edited in
    /// the preference dialog.
    /// @param layout The layout root element
    void setSettingLayout(std::unique_ptr<LegacyControllerSettingsLayoutElement>&& layout) {
        VERIFY_OR_DEBUG_ASSERT(layout.get()) {
            return;
        }
        m_settingsLayout = std::move(layout);
    }

    const QList<ScriptFileInfo>& getScriptFiles() const {
        return m_scripts;
    }

    const QList<std::shared_ptr<AbstractLegacyControllerSetting>>& getSettings() const {
        return m_settings;
    }

    bool hasDirtySettings() const {
        for (const auto& setting : m_settings) {
            if (setting->isDirty()) {
                return true;
            }
        }
        return false;
    }

    LegacyControllerSettingsLayoutElement* getSettingsLayout() {
        return m_settingsLayout.get();
    }

    void setDeviceDirection(DeviceDirections aDeviceDirection) {
        m_deviceDirection = aDeviceDirection;
    }

    DeviceDirections getDeviceDirection() const {
        return m_deviceDirection;
    }

    void setDirty(bool bDirty) {
        m_bDirty = bDirty;
    }

    bool isDirty() const {
        return m_bDirty;
    }

    void setDeviceId(const QString& id) {
        m_deviceId = id;
        setDirty(true);
    }

    QString deviceId() const {
        return m_deviceId;
    }

    void setFilePath(const QString& filePath) {
        m_filePath = filePath;
        setDirty(true);
    }

    QString filePath() const {
        return m_filePath;
    }

    QDir dirPath() const {
        return QFileInfo(filePath()).absoluteDir();
    }

    void setName(const QString& name) {
        m_name = name;
        setDirty(true);
    }

    QString name() const {
        return m_name;
    }

    void setAuthor(const QString& author) {
        m_author = author;
        setDirty(true);
    }

    inline QString author() const {
        return m_author;
    }

    inline void setDescription(const QString& description) {
        m_description = description;
        setDirty(true);
    }

    inline QString description() const {
        return m_description;
    }

    inline void setForumLink(const QString& forumlink) {
        m_forumlink = forumlink;
        setDirty(true);
    }

    inline QString forumlink() const {
        return m_forumlink;
    }

    void setManualPage(const QString& manualPage) {
        m_manualPage = manualPage;
        setDirty(true);
    }

    QString manualPage() const {
        return m_manualPage;
    }

    QString manualLink() const {
        QString page = manualPage();
        if (page.isEmpty()) {
            return {};
        }

        return MIXXX_MANUAL_CONTROLLERMANUAL_PREFIX + page + MIXXX_MANUAL_CONTROLLERMANUAL_SUFFIX;
    }

    inline void setManualLink(const QString& manuallink) {
        m_manuallink = manuallink;
        setDirty(true);
    }

    inline QString manuallink() const {
        return m_manuallink;
    }

    inline void setSchemaVersion(const QString& schemaVersion) {
        m_schemaVersion = schemaVersion;
        setDirty(true);
    }

    inline QString schemaVersion() const {
        return m_schemaVersion;
    }

    inline void setMixxxVersion(const QString& mixxxVersion) {
        m_mixxxVersion = mixxxVersion;
        setDirty(true);
    }

    inline QString mixxxVersion() const {
        return m_mixxxVersion;
    }

    inline void addProductMatch(const QHash<QString, QString>& match) {
        m_productMatches.append(match);
        setDirty(true);
    }

    virtual bool saveMapping(const QString& filename) const = 0;

    virtual bool isMappable() const = 0;

    void loadSettings(UserSettingsPointer pConfig,
            const QString& controllerName) const;
    void saveSettings(UserSettingsPointer pConfig,
            const QString& controllerName) const;
    void resetSettings();

    // Optional list of controller device match details
    QList<QHash<QString, QString>> m_productMatches;

  private:
    bool m_bDirty;

    QString m_deviceId;
    QString m_filePath;
    QString m_name;
    QString m_author;
    QString m_description;
    QString m_forumlink;
    QString m_manualPage;
    QString m_manuallink;
    QString m_schemaVersion;
    QString m_mixxxVersion;

    QList<std::shared_ptr<AbstractLegacyControllerSetting>> m_settings;
    std::unique_ptr<LegacyControllerSettingsLayoutElement> m_settingsLayout;
    QList<ScriptFileInfo> m_scripts;
    DeviceDirections m_deviceDirection;
};
