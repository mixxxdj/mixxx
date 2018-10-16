// settingsdao.cpp
// Created 12/29/2009 by RJ Ryan (rryan@mit.edu)

#include "library/dao/settingsdao.h"

#include "util/logger.h"
#include "util/assert.h"

namespace {

mixxx::Logger kLogger("SettingsDAO");

} // anonymous namespace

SettingsDAO::SettingsDAO(const QSqlDatabase& db)
        : m_db(db) {
}

QString SettingsDAO::getValue(const QString& name, QString defaultValue) const {
    QSqlQuery query(m_db);

    if (query.prepare("SELECT value FROM settings WHERE name = :name")) {
        query.bindValue(":name", name);
        if (query.exec() && query.first()) {
            QVariant value = query.value(query.record().indexOf("value"));
            VERIFY_OR_DEBUG_ASSERT(value.isValid()) {
                kLogger.warning() << "Invalid value:" << value;
            } else {
                return value.toString();
            }
        }
    } else {
        // Prepare is expected to fail for a fresh database
        // when the schema is still empty!
        kLogger.info()
                << "Failed to prepare query:"
                << "Returning default value"
                << defaultValue
                << "for"
                << name;
    }
    return defaultValue;
}

bool SettingsDAO::setValue(const QString& name, const QVariant& value) {
    if (!value.canConvert(QMetaType::QString)) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("REPLACE INTO settings (name, value) VALUES (:name, :value)");
    query.bindValue(":name", name);
    query.bindValue(":value", value.toString());

    if (!query.exec()) {
        qDebug() << "SettingsDAO::setValue() failed" << name << ":" << value
                 << query.lastError();
        return false;
    }
    return true;
}
