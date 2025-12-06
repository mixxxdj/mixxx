#pragma once

#include <QUrl>
#include <QVersionNumber>

class QXmlStreamAttributes;
class QFileInfo;

struct ProductInfo {
    QString friendlyName;
    QUrl visualUrl;

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

bool operator==(const ProductInfo& a, const ProductInfo& b);

size_t qHash(const ProductInfo& product);

QDebug operator<<(QDebug dbg, const ProductInfo& product);

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

    inline const QString& getPath() const {
        return m_path;
    }
    inline const QString& getDirPath() const {
        return m_dirPath;
    }
    inline const QString& getName() const {
        return m_name;
    }
    inline const QVersionNumber& getMixxxVersion() const {
        return m_mixxxVersion;
    }
    inline const QString& getDescription() const {
        return m_description;
    }
    inline const QString& getForumLink() const {
        return m_forumlink;
    }
    inline const QString& getWikiLink() const {
        return m_wikilink;
    }
    inline const QString& getAuthor() const {
        return m_author;
    }
    inline bool hasSettings() const {
        return m_hasSettings;
    }
    inline bool hasScreens() const {
        return m_hasScreens;
    }

    inline const QList<ProductInfo>& getProducts() const {
        return m_products;
    }

    bool operator==(const MappingInfo& b) const {
        return m_valid == b.m_valid &&
                m_path == b.m_path &&
                m_dirPath == b.m_dirPath &&
                m_name == b.m_name &&
                m_author == b.m_author &&
                m_description == b.m_description &&
                m_forumlink == b.m_forumlink &&
                m_wikilink == b.m_wikilink &&
                m_products == b.m_products;
    }

  private:
    static ProductInfo parseBulkProduct(const QXmlStreamAttributes& xmlElementAttributes);
    static ProductInfo parseHIDProduct(const QXmlStreamAttributes& xmlElementAttributes);

    bool m_valid = false;
    bool m_hasSettings;
    bool m_hasScreens;
    QString m_path;
    QString m_dirPath;
    QVersionNumber m_mixxxVersion;
    QString m_name;
    QString m_author;
    QString m_description;
    QString m_forumlink;
    QString m_wikilink;
    QList<ProductInfo> m_products;
};
