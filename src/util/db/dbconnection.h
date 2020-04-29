#ifndef MIXXX_DBCONNECTION_H
#define MIXXX_DBCONNECTION_H


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

    static int likeCompareLatinLow(
        QString* pattern,
        QString* string,
        QChar esc);

    static void makeStringLatinLow(QString* string);

    struct Params {
        QString type;
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
    StringCollator m_collator;
};

} // namespace mixxx


#endif // MIXXX_DBCONNECTION_H
