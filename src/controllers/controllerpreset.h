#pragma once

#include <QDebug>
#include <QDir>
#include <QHash>
#include <QList>
#include <QSharedPointer>
#include <QString>

#include "defs_urls.h"

class ControllerPresetVisitor;
class ConstControllerPresetVisitor;

/// This class represents a controller preset, containing the data elements that
/// make it up.
class ControllerPreset {
  public:
    ControllerPreset()
            : m_bDirty(false) {
    }
    virtual ~ControllerPreset() {}

    struct ScriptFileInfo {
        ScriptFileInfo()
                : builtin(false) {

        }

        QString name;
        QString functionPrefix;
        QFileInfo file;
        bool builtin;
    };

    /// Adds a script file to the list of controller scripts for this preset.
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

    const QList<ScriptFileInfo>& getScriptFiles() const {
        return m_scripts;
    }

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

    virtual bool savePreset(const QString& filename) const = 0;

    virtual void accept(ControllerPresetVisitor* visitor) = 0;
    virtual void accept(ConstControllerPresetVisitor* visitor) const = 0;
    virtual bool isMappable() const = 0;

    QList<ScriptFileInfo> scripts;
    // Optional list of controller device match details
    QList< QHash<QString,QString> > m_productMatches;

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
};

typedef QSharedPointer<ControllerPreset> ControllerPresetPointer;
