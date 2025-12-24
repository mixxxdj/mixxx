#pragma once

#include <util/assert.h>

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QList>
#include <QString>
#include <bit>
#include <chrono>
#include <memory>
#ifdef MIXXX_USE_QML
#include <bit>
#endif

#include "controllers/legacycontrollersettings.h"
#include "controllers/legacycontrollersettingslayout.h"
#include "defs_urls.h"
#include "preferences/usersettings.h"
#include "util/assert.h"

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
#ifdef MIXXX_USE_QML
              m_modules(other.m_modules),
              m_screens(other.m_screens),
#endif
              m_scripts(other.m_scripts),
              m_deviceDirection(other.m_deviceDirection) {
    }
    LegacyControllerMapping& operator=(const LegacyControllerMapping&) = delete;
    LegacyControllerMapping(LegacyControllerMapping&&) = delete;
    LegacyControllerMapping& operator=(LegacyControllerMapping&&) = delete;
    virtual ~LegacyControllerMapping() = default;

  public:
    struct ScriptFileInfo {
        enum class Type {
            Javascript,
#ifdef MIXXX_USE_QML
            Qml,
#endif
        };

        QString name;       // Name of the script file to add.
        QString identifier; // The script's function prefix with Javascript OR
                            // the screen identifier this QML should be run for
                            // (or empty string).
        QFileInfo file;     // A FileInfo object pointing to the script file.
        Type type;          // A ScriptFileInfo::Type the specify script file type.
        bool builtin;       // If this is true, the script won't be written to the XML.
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

#ifdef MIXXX_USE_QML
    struct QMLModuleInfo {
        QMLModuleInfo(const QFileInfo& aDirinfo,
                bool isBuiltin)
                : dirinfo(aDirinfo),
                  builtin(isBuiltin) {
        }

        QFileInfo dirinfo;
        bool builtin;
    };

    struct ScreenInfo {
        // Defining a custom enum here as std::endian contains `native` which is
        // confusing and will have unpredictable behaviour depending of the
        // platform.
        enum class ColorEndian {
            Big = static_cast<int>(std::endian::big),
            Little = static_cast<int>(std::endian::little),
        };

        QString identifier; // The screen identifier.
        QSize size;         // The size of the screen.
        uint target_fps;    // The maximum FPS to render.
        uint msaa;          // The MSAA value to use for render.
        std::chrono::milliseconds
                splash_off;         // The rendering grace time given when the screen is
                                    // requested to shutdown.
        QImage::Format pixelFormat; // The pixel encoding format.
        ColorEndian endian;         // The pixel endian format.
        bool reversedColor;         // Whether or not the RGB is swapped BGR.
        bool rawData;               // Whether or not the screen is allowed to receive bare
                                    // data, not transformed.
    };
#endif

    /// Adds a script file to the list of controller scripts for this mapping.
    /// @param info The script info to add.
    virtual void addScriptFile(ScriptFileInfo info) {
        m_scripts.append(std::move(info));
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
    void setSettingLayout(std::unique_ptr<LegacyControllerSettingsLayoutElement> layout) {
        VERIFY_OR_DEBUG_ASSERT(layout) {
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

#ifdef MIXXX_USE_QML
    /// Adds a custom QML module file to the list of controller modules for this mapping.
    /// @param dirInfo A FileInfo of the directory or QML module
    /// @param builtin If this is true, the script won't be written to the XML
    virtual void addModule(const QFileInfo& dirInfo,
            bool builtin = false) {
        m_modules.append(QMLModuleInfo(
                dirInfo,
                builtin));
        setDirty(true);
    }

    const QList<QMLModuleInfo>& getModules() const {
        return m_modules;
    }

    /// @brief Adds a screen info where QML will be rendered.
    /// @param identifier The screen identifier
    /// @param size the size of the screen
    /// @param targetFps the maximum FPS to render
    /// @param msaa the MSAA value to use for render
    /// @param splashoff the rendering grace time given when the screen is requested to shutdown
    /// @param pixelFormat the pixel encoding format
    /// @param endian the pixel endian format
    /// @param reversedColor whether or not the RGB is swapped BGR
    /// @param rawData whether or not the screen is allowed to reserve bare data, not transformed
    virtual void addScreenInfo(ScreenInfo info) {
        m_screens.append(std::move(info));
        setDirty(true);
    }

    const QList<ScreenInfo>& getInfoScreens() const {
        return m_screens;
    }
#endif

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

    inline void setWikiLink(const QString& wikilink) {
        m_wikilink = wikilink;
        setDirty(true);
    }

    inline QString wikilink() const {
        return m_wikilink;
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
    QString m_wikilink;
    QString m_schemaVersion;
    QString m_mixxxVersion;

    QList<std::shared_ptr<AbstractLegacyControllerSetting>> m_settings;
    std::unique_ptr<LegacyControllerSettingsLayoutElement> m_settingsLayout;
#ifdef MIXXX_USE_QML
    QList<QMLModuleInfo> m_modules;
    QList<ScreenInfo> m_screens;
#endif
    QList<ScriptFileInfo> m_scripts;
    DeviceDirections m_deviceDirection;
};
