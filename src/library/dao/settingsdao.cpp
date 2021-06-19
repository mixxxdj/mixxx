#include "library/dao/settingsdao.h"

#include <QtSql>

#include "util/assert.h"
#include "util/logger.h"

namespace {

mixxx::Logger kLogger("SettingsDAO");

const QString kTable = QStringLiteral("settings");

const QString kColumnName = QStringLiteral("name");
const QString kColumnValue = QStringLiteral("value");
const QString kColumnLocked = QStringLiteral("locked");
const QString kColumnHidden = QStringLiteral("hidden");

} // anonymous namespace

QString SettingsDAO::getValue(const QString& name, QString defaultValue) const {
    const QString statement =
            QStringLiteral("SELECT ") +
            kColumnValue +
            QStringLiteral(" FROM ") +
            kTable +
            QStringLiteral(" WHERE ") +
            kColumnName + QStringLiteral("=:name");
    QSqlQuery query(m_database);
    if (!query.prepare(statement)) {
        // Prepare is expected to fail for a fresh database
        // when the schema is still empty!
        kLogger.info()
                << "Failed to prepare query:"
                << "Returning default value"
                << defaultValue
                << "for"
                << name;
    }
    query.bindValue(QStringLiteral(":name"), name);
    if (query.exec() && query.first()) {
        QVariant value = query.value(query.record().indexOf(kColumnValue));
        VERIFY_OR_DEBUG_ASSERT(value.isValid()) {
            kLogger.warning() << "Invalid value:" << value;
        }
        else {
            return value.toString();
        }
    }
    return defaultValue;
}

bool SettingsDAO::setValue(const QString& name, const QVariant& value) const {
    VERIFY_OR_DEBUG_ASSERT(value.canConvert(QMetaType::QString)) {
        return false;
    }

    const QString statement =
            QStringLiteral("REPLACE INTO ") +
            kTable +
            QStringLiteral(" (") +
            kColumnName +
            QChar(',') +
            kColumnValue +
            QStringLiteral(") VALUES (:name,:value)");
    QSqlQuery query(m_database);
    VERIFY_OR_DEBUG_ASSERT(query.prepare(statement)) {
        return false;
    }
    query.bindValue(QStringLiteral(":name"), name);
    query.bindValue(QStringLiteral(":value"), value.toString());
    VERIFY_OR_DEBUG_ASSERT(query.exec()) {
        kLogger.warning()
                << "Failed to set" << name << "=" << value
                << query.lastError();
        return false;
    }
    return true;
}
