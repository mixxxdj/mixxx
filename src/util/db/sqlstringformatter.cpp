#include "util/db/sqlstringformatter.h"

#include <QSqlDriver>
#include <QSqlField>

#include "util/assert.h"


QString SqlStringFormatter::format(
        const QSqlDatabase& database,
        const QString& value) {
    QSqlDriver* pDriver = database.driver();
    DEBUG_ASSERT_AND_HANDLE(pDriver != nullptr) {
        return value; // unformatted
    }
    QSqlField stringField(QString(), QVariant::String);
    stringField.setValue(value);
    return pDriver->formatValue(stringField);
}

QString SqlStringFormatter::formatList(
        const QSqlDatabase& database,
        const QStringList& values) {
    QString result;
    QString previousString;
    for (const QString& value: values) {
        if (!previousString.isEmpty()) {
            result.append(',');
        }
        previousString = format(database, value);
        result += previousString;
    }
    return result;
}

QString SqlStringFormatter::formatList(
        const QSqlDatabase& database,
        const QSet<QString>& values) {
    QString result;
    QString previousString;
    for (const QString& value: values) {
        if (!previousString.isEmpty()) {
            result.append(',');
        }
        previousString = format(database, value);
        result += previousString;
    }
    return result;
}
