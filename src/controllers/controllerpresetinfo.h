#pragma once

#include <QString>
#include <QMap>
#include <QList>
#include <QDomElement>
#include <QFileInfo>

#include "preferences/usersettings.h"
#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetfilehandler.h"

struct ProductInfo {
    QString protocol;
    QString vendor_id;
    QString product_id;

    // HID-specific
    QString in_epaddr;
    QString out_epaddr;

    // Bulk-specific
    QString usage_page;
    QString usage;
    QString interface_number;
};

/// Base class handling enumeration and parsing of preset info headers
///
/// This class handles enumeration and parsing of controller XML description file
/// <info> header tags. It can be used to match controllers automatically or to
/// show details for a mapping.
class PresetInfo {
  public:
    PresetInfo();
    PresetInfo(const QString& path);


    inline bool isValid() const {
        return m_valid;
    }

    inline const QString getPath() const { return m_path; }
    inline const QString getDirPath() const { return m_dirPath; }
    inline const QString getName() const { return m_name; }
    inline const QString getDescription() const { return m_description; }
    inline const QString getForumLink() const { return m_forumlink; }
    inline const QString getWikiLink() const { return m_wikilink; }
    inline const QString getAuthor() const { return m_author; }

    inline const QList<ProductInfo>& getProducts() const { return m_products; }

  private:
    ProductInfo parseBulkProduct(const QDomElement& element) const;
    ProductInfo parseHIDProduct(const QDomElement& element) const;

    bool m_valid;
    QString m_path;
    QString m_dirPath;
    QString m_name;
    QString m_author;
    QString m_description;
    QString m_forumlink;
    QString m_wikilink;
    QList<ProductInfo> m_products;
};
