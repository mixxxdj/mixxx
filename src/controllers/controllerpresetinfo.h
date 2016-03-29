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

#include <QString>
#include <QMap>
#include <QList>
#include <QHash>
#include <QDomElement>

#include "preferences/usersettings.h"
#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetfilehandler.h"

class PresetInfo {
  public:
    PresetInfo();
    PresetInfo(const QString path);
    virtual ~PresetInfo() {};

    inline bool isValid() const {
        return m_valid;
    }

    inline const QString getPath() const { return m_path; };

    inline const QString getName() const { return m_name; } ;
    inline const QString getDescription() const { return m_description; };
    inline const QString getForumLink() const { return m_forumlink; };
    inline const QString getWikiLink() const { return m_wikilink; };
    inline const QString getAuthor() const { return m_author; };

    inline const QList<QHash<QString,QString> > getProducts() const { return m_products; };

  private:
    QHash<QString,QString> parseBulkProduct(const QDomElement& element) const;
    QHash<QString,QString> parseHIDProduct(const QDomElement& element) const;
    // Note - following are just stubs, not yet implemented
    QHash<QString,QString> parseMIDIProduct(const QDomElement& element) const;
    QHash<QString,QString> parseOSCProduct(const QDomElement& element) const;

    bool m_valid;
    QString m_path;
    QString m_name;
    QString m_author;
    QString m_description;
    QString m_forumlink;
    QString m_wikilink;
    QList<QHash<QString,QString> > m_products;
};

#endif
