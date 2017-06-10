#ifndef MIXXX_DBCONNECTION_H
#define MIXXX_DBCONNECTION_H


#include <QDir>
#include <QSqlDatabase>

#include <QtDebug>


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

    bool open();
    void close();

    QString name() const {
        return m_database.connectionName();
    }

    operator QSqlDatabase() const {
        return m_database;
    }

    friend QDebug operator<<(QDebug debug, const DbConnection& connection);

  private:
    DbConnection(const DbConnection&) = delete;
    DbConnection(const DbConnection&&) = delete;

    QSqlDatabase m_database;
};

} // namespace mixxx


#endif // MIXXX_DBCONNECTION_H
