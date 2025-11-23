#pragma once

#include <QList>
#include <QString>

class QXmlStreamAttributes;
class QFileInfo;

struct ProductInfo {
    QString protocol;
    QString vendor_id;
    QString product_id;
    QString interface_number;

    // HID-specific
    QString usage_page;
    QString usage;

    // Bulk-specific
    QString in_epaddr;
    QString out_epaddr;
};

/// Base class handling enumeration and parsing of mapping info headers
///
/// This class handles enumeration and parsing of controller XML description file
/// <info> header tags. It can be used to match controllers automatically or to
/// show details for a mapping.
class MappingInfo {
  public:
    MappingInfo() = default;
    explicit MappingInfo(const QFileInfo& fileInfo);

    inline bool isValid() const {
        return m_valid;
    }

    inline const QString getPath() const {
        return m_path;
    }
    inline const QString getDirPath() const {
        return m_dirPath;
    }
    inline const QString getName() const {
        return m_name;
    }
    inline const QString getDescription() const {
        return m_description;
    }
    inline const QString getForumLink() const {
        return m_forumlink;
    }
    inline const QString getWikiLink() const {
        return m_wikilink;
    }
    inline const QString getAuthor() const {
        return m_author;
    }

    inline const QList<ProductInfo>& getProducts() const {
        return m_products;
    }

  private:
    static ProductInfo parseBulkProduct(const QXmlStreamAttributes& xmlElementAttributes);
    static ProductInfo parseHIDProduct(const QXmlStreamAttributes& xmlElementAttributes);

    bool m_valid = false;
    QString m_path;
    QString m_dirPath;
    QString m_name;
    QString m_author;
    QString m_description;
    QString m_forumlink;
    QString m_wikilink;
    QList<ProductInfo> m_products;
};
