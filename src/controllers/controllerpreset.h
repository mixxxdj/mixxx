/**
* @file controllerpreset.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Mon 9 Apr 2012
* @brief Controller preset
*
* This class represents a controller preset, containing the data elements that
* make it up.
*/

#ifndef CONTROLLERPRESET_H
#define CONTROLLERPRESET_H

#include <QDebug>
#include <QDir>
#include <QHash>
#include <QList>
#include <QSharedPointer>
#include <QString>

class ControllerPresetVisitor;
class ConstControllerPresetVisitor;

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

    /** addScriptFile(QString,QString)
     * Adds an entry to the list of script file names & associated list of function prefixes
     * @param filename Name of the XML file to add
     * @param functionprefix Function prefix to add
     */
    void addScriptFile(QString filename, QString functionprefix, bool builtin = false) {
        ScriptFileInfo info;
        info.name = filename;
        info.functionPrefix = functionprefix;
        // Always try to load script from the mapping's directory first
        info.file = QFileInfo(dirPath().absoluteFilePath(filename));
        info.builtin = builtin;
        m_scripts.append(info);
        setDirty(true);
    }

    QList<ScriptFileInfo> getScriptFiles(const QString& fallbackPath = QString()) const {
        if (fallbackPath.isEmpty()) {
            return m_scripts;
        }

        QDir path(fallbackPath);
        QList<ScriptFileInfo> scriptsWithFallbackPath;
        for (const ScriptFileInfo& script : m_scripts) {
            if (script.file.exists()) {
                scriptsWithFallbackPath << script;
                continue;
            }

            ScriptFileInfo info(script);
            info.file = QFileInfo(path.absoluteFilePath(info.name));
            scriptsWithFallbackPath << info;
        }
        return scriptsWithFallbackPath;
    }

    inline void setDirty(bool bDirty) {
        m_bDirty = bDirty;
    }

    inline bool isDirty() const {
        return m_bDirty;
    }

    inline void setDeviceId(const QString id) {
        m_deviceId = id;
        setDirty(true);
    }

    inline QString deviceId() const {
        return m_deviceId;
    }

    inline void setFilePath(const QString filePath) {
        m_filePath = filePath;
        setDirty(true);
    }

    inline QString filePath() const {
        return m_filePath;
    }

    inline QDir dirPath() const {
        return QFileInfo(filePath()).absoluteDir();
    }

    inline void setName(const QString name) {
        m_name = name;
        setDirty(true);
    }

    inline QString name() const {
        return m_name;
    }

    inline void setAuthor(const QString author) {
        m_author = author;
        setDirty(true);
    }

    inline QString author() const {
        return m_author;
    }

    inline void setDescription(const QString description) {
        m_description = description;
        setDirty(true);
    }

    inline QString description() const {
        return m_description;
    }

    inline void setForumLink(const QString forumlink) {
        m_forumlink = forumlink;
        setDirty(true);
    }

    inline QString forumlink() const {
        return m_forumlink;
    }

    inline void setWikiLink(const QString wikilink) {
        m_wikilink = wikilink;
        setDirty(true);
    }

    inline QString wikilink() const {
        return m_wikilink;
    }

    inline void setSchemaVersion(const QString schemaVersion) {
        m_schemaVersion = schemaVersion;
        setDirty(true);
    }

    inline QString schemaVersion() const {
        return m_schemaVersion;
    }

    inline void setMixxxVersion(const QString mixxxVersion) {
        m_mixxxVersion = mixxxVersion;
        setDirty(true);
    }

    inline QString mixxxVersion() const {
        return m_mixxxVersion;
    }

    inline void addProductMatch(QHash<QString,QString> match) {
        m_productMatches.append(match);
        setDirty(true);
    }

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
    QString m_wikilink;
    QString m_schemaVersion;
    QString m_mixxxVersion;

    QList<ScriptFileInfo> m_scripts;
};

typedef QSharedPointer<ControllerPreset> ControllerPresetPointer;

#endif
