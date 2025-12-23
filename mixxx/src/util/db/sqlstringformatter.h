#pragma once

#include <QSet>
#include <QStringList>
#include <QSqlDatabase>


// Utility class for formatting string values in SQL statements.
class SqlStringFormatter final {
  public:
    // Format a string for use in an SQL statement by enclosing it into
    // quote characters and escaping embedded quote characters.
    static QString format(
            const QSqlDatabase& database,
            const QString& value);

    // Format the elements of a string collection and join the resulting
    // list of strings separated by ",".
    static QString formatList(
            const QSqlDatabase& database,
            const QStringList& values);
    static QString formatList(
            const QSqlDatabase& database,
            const QSet<QString>& values);

  private:
    SqlStringFormatter() = delete; // utility class
};
