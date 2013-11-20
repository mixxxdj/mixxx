/**
* @file controllerpresetinfo.h
* @author Ilkka Tuohela hile@iki.fi
* @date Wed May 15 2012
* @brief Base class handling enumeration and parsing of preset info headers
*
* This class handles enumeration and parsing of controller XML description file
* <info> header tags. It can be used to match controllers automatically or to
* show details for a mapping.
*/

#ifndef CONTROLLERPRESETINFO_H
#define CONTROLLERPRESETINFO_H

#include <QtGui>
#include <QMap>
#include <QHash>

#include "configobject.h"

class PresetInfo {
  public:
    PresetInfo();
    PresetInfo(const QString path);
    virtual ~PresetInfo() {};

    inline bool isValid() const {
        return m_valid;
    }

    inline const QString getPath() const { return path; };

    inline const QString getName() const { return name; } ;
    inline const QString getDescription() const { return description; };
    inline const QString getForumLink() const { return forumlink; };
    inline const QString getWikiLink() const { return wikilink; };
    inline const QString getAuthor() const { return author; };

    inline const QList< QHash<QString,QString> > getProducts() const { return products; };

  private:
    QHash<QString,QString> parseBulkProduct(const QDomElement& element) const;
    QHash<QString,QString> parseHIDProduct(const QDomElement& element) const;
    // Note - following are just stubs, not yet implemented
    QHash<QString,QString> parseMIDIProduct(const QDomElement& element) const;
    QHash<QString,QString> parseOSCProduct(const QDomElement& element) const;

    bool m_valid;
    QString path;
    QString name;
    QString author;
    QString description;
    QString forumlink;
    QString wikilink;
    QList< QHash<QString,QString> > products;
};

class PresetInfoEnumerator {
  public:
    PresetInfoEnumerator(ConfigObject<ConfigValue> *pConfig);
    virtual ~PresetInfoEnumerator() {};

    bool isValidExtension(const QString extension);

    bool hasPresetInfo(const QString extension, const QString name);
    bool hasPresetInfo(const QString path);

    PresetInfo getPresetInfo(const QString path);

    // Return cached list of presets for this extension
    QList <PresetInfo> getPresets(const QString extension);

    // Updates presets matching given extension
    void updatePresets(const QString extension);

  protected:
    void addExtension(QString extension);
    void loadSupportedPresets();

  private:
    QList <QString> fileExtensions;
    ConfigObject<ConfigValue>* m_pConfig;

    // List of paths for controller presets
    QList <QString> controllerDirPaths;

    // Cached presets by extension. Map format is:
    // [extension,[preset_path,preset]]
    QMap <QString, QMap<QString, PresetInfo> > presetsByExtension;

};

#endif
