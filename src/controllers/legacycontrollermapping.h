#pragma once

#include <QDebug>
#include <QDir>
#include <QHash>
#include <QImage>
#include <QList>
#include <QOpenGLContext>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <bit>
#include <memory>

#include "defs_urls.h"
#include "util/assert.h"

/// This class represents a controller mapping, containing the data elements that
/// make it up.
class LegacyControllerMapping {
  public:
    LegacyControllerMapping()
            : m_bDirty(false) {
    }
    virtual ~LegacyControllerMapping() = default;

    virtual std::shared_ptr<LegacyControllerMapping> clone() const = 0;

    struct ScriptFileInfo {
        enum Type {
            JAVASCRIPT,
#ifdef MIXXX_USE_QML
            QML,
#endif
        };

        ScriptFileInfo()
                : builtin(false) {
        }

        QString name;
        QString identifier;
        QFileInfo file;
        Type type;
        bool builtin;
    };

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
        ScreenInfo(const QString& aIdentifier,
                const QSize& aSize,
                uint aTargetFps,
                uint aSplashOff,
                QImage::Format aPixelFormat,
                std::endian anEndian,
                bool isReversedColor,
                bool isRawData)
                : identifier(aIdentifier),
                  size(aSize),
                  target_fps(aTargetFps),
                  splash_off(aSplashOff),
                  pixelFormat(aPixelFormat),
                  endian(anEndian),
                  reversedColor(isReversedColor),
                  rawData(isRawData) {
        }

        QString identifier;
        QSize size;
        uint target_fps;
        uint splash_off;
        QImage::Format pixelFormat;
        std::endian endian;
        bool reversedColor;
        bool rawData;
    };
#endif

    /// Adds a script file to the list of controller scripts for this mapping.
    /// @param filename Name of the script file to add
    /// @param identifier The script's function prefix with Javascript OR the
    /// screen identifier this QML should be run for (or empty string)
    /// @param file A FileInfo object pointing to the script file
    /// @param type A ScriptFileInfo::Type the specify script file type
    /// @param builtin If this is true, the script won't be written to the XML
    virtual void addScriptFile(const QString& name,
            const QString& identifier,
            const QFileInfo& file,
            ScriptFileInfo::Type type = ScriptFileInfo::Type::JAVASCRIPT,
            bool builtin = false) {
        ScriptFileInfo info;
        info.name = name;
        info.identifier = identifier;
        info.file = file;
        info.type = type;
        info.builtin = builtin;
        m_scripts.append(info);
        setDirty(true);
    }

    const QList<ScriptFileInfo>& getScriptFiles() const {
        return m_scripts;
    }

#ifdef MIXXX_USE_QML
    /// Adds a custom QML module file to the list of controller modules for this mapping.
    /// @param dirinfo A FileInfo of the directory or QML module
    /// @param builtin If this is true, the script won't be written to the XML
    virtual void addLibraryDirectory(const QFileInfo& dirinfo,
            bool builtin = false) {
        m_modules.append(QMLModuleInfo(
                dirinfo,
                builtin));
        setDirty(true);
    }

    const QList<QMLModuleInfo>& getLibraryDirectories() const {
        return m_modules;
    }

    /// @brief Adds a screen info where QML will be rendered.
    /// @param identifier The screen identifier
    /// @param size the size of the screen
    /// @param targetFps the maximum FPS to render
    /// @param splashoff the rendering grace time given when the screen is requested to shutdown
    /// @param pixelFormat the pixel encoding format
    /// @param endian the pixel endian format
    /// @param reversedColor whether or not the RGB is swapped BGR
    /// @param rawData whether or not the screen is allowed to reserve bare data, not transformed
    virtual void addScreenInfo(const QString& identifier,
            const QSize& size,
            uint targetFps = 30,
            uint splashoff = 50,
            QImage::Format pixelFormat = QImage::Format_RGB32,
            std::endian endian = std::endian::little,
            bool reversedColor = false,
            bool rawData = false) {
        m_screens.append(ScreenInfo(identifier,
                size,
                targetFps,
                splashoff,
                pixelFormat,
                endian,
                reversedColor,
                rawData));
        setDirty(true);
    }

    const QList<ScreenInfo>& getInfoScreens() const {
        return m_screens;
    }
#endif

    inline void setDirty(bool bDirty) {
        m_bDirty = bDirty;
    }

    inline bool isDirty() const {
        return m_bDirty;
    }

    inline void setDeviceId(const QString& id) {
        m_deviceId = id;
        setDirty(true);
    }

    inline QString deviceId() const {
        return m_deviceId;
    }

    inline void setFilePath(const QString& filePath) {
        m_filePath = filePath;
        setDirty(true);
    }

    inline QString filePath() const {
        return m_filePath;
    }

    inline QDir dirPath() const {
        return QFileInfo(filePath()).absoluteDir();
    }

    inline void setName(const QString& name) {
        m_name = name;
        setDirty(true);
    }

    inline QString name() const {
        return m_name;
    }

    inline void setAuthor(const QString& author) {
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

    QList<ScriptFileInfo> m_scripts;
#ifdef MIXXX_USE_QML
    QList<QMLModuleInfo> m_modules;
    QList<ScreenInfo> m_screens;
#endif
};
