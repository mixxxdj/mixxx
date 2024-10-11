#pragma once

#include <QUrl>
#include <QUrlQuery>

#include "util/db/dbid.h"

class UrlHelper {
  public:
    static QUrl urlFromTemplate(const QString& urlTemplate, DbId id) {
        if (!id.isValid()) {
            return QUrl();
        }
        return QUrl(QStringLiteral("%1?id=%2").arg(urlTemplate, id.toString()));
    }

    template<typename T>
    static T idFromUrl(const QString& urlTemplate, const QUrl& url) {
        if (url.isEmpty()) {
            return T();
        }
        const QString urlString = url.adjusted(QUrl::RemoveFragment | QUrl::RemoveQuery).toString();
        if (urlString != urlTemplate) {
            return T();
        }
        const QUrlQuery query(url);
        const QString id = query.queryItemValue("id");
        if (id.isEmpty()) {
            return T();
        }
        // If id is a valid number, it will be automatically parsed while
        // coercing the QVariant to int inside the DbId constructor.
        return T(QVariant(id));
    }

    template<typename T>
    static QList<T> idsFromUrls(const QList<QUrl>& urls) {
        QList<T> ids;
        for (const QUrl& url : urls) {
            CrateId id = idFroMUrl<T>(url);
            if (id.isValid()) {
                ids.append(id);
            }
        }
        return ids;
    }
}
