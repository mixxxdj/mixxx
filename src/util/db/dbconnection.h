#pragma once

#include <QSqlDatabase>
#include <QtDebug>

#include "util/string.h"

namespace mixxx {

class DbConnection final {
  public:
    // Order string fields lexicographically with a
    // custom collation function if available (SQLite3).
    // Otherwise the query is returned unmodified.
    static QString collateLexicographically(
            const QString& orderByQuery);

    // Wraps the given column into the `mixxx_hue()` SQL function call, which
    // maps a stored color code to a hue based sort key. Use this to order
    // color columns by hue instead of by their raw code, e.g.
    //     ORDER BY mixxx_hue(color) ASC
    // The returned expression is only valid if the function is available,
    // i.e. for SQLite3 connections (see initDatabase()).
    //
    // The resulting order is consistent with
    // mixxx::RgbColor::sortKey(), which is used for sorting rows in C++.
    static QString hueSortKey(const QString& column);

    static int likeCompareLatinLow(
        QString* pattern,
        QString* string,
        QChar esc);

    static void makeStringLatinLow(QString* string);

    struct Params {
        QString type;
        QString connectOptions;
        QString hostName;
        QString filePath;
        QString userName;
        QString password;
    };

    // All constructors are reserved for DbConnectionPool!!
    DbConnection(
            const Params& params,
            const QString& connectionName);
    DbConnection(
            const DbConnection& prototype,
            const QString& connectionName);
    ~DbConnection();

    QString name() const {
        return m_sqlDatabase.connectionName();
    }

    bool open();
    void close();

    bool isOpen() const {
        return m_sqlDatabase.isOpen();
    }

    operator QSqlDatabase() const {
        return m_sqlDatabase;
    }

    friend QDebug operator<<(QDebug debug, const DbConnection& connection);

  private:
    DbConnection(const DbConnection&) = delete;
    DbConnection(const DbConnection&&) = delete;

    QSqlDatabase m_sqlDatabase;
    mixxx::StringCollator m_collator;
};

} // namespace mixxx
