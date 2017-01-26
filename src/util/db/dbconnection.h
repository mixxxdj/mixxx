#ifndef MIXXX_DBCONNECTION_H
#define MIXXX_DBCONNECTION_H


#include <QSqlDatabase>
#include <QtDebug>


class DbConnection final {
  public:
    explicit DbConnection(
        const QString& dirPath);
    ~DbConnection();

    operator bool() const {
        return m_database.isOpen();
    }

    QSqlDatabase& database() {
        return m_database;
    }

    // SQLite3 collation function or nullptr if not supported
    static const char* const kStringCollationFunc;

    static int likeCompareLatinLow(
        QString* pattern,
        QString* string,
        QChar esc);

    friend QDebug operator<<(
        QDebug debug,
        const DbConnection& dbConnection);

  private:
    QString m_filePath;
    QSqlDatabase m_database;
};


#endif // MIXXX_DBCONNECTION_H
