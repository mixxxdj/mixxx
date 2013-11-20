// settingsdao.cpp
// Created 12/29/2009 by RJ Ryan (rryan@mit.edu)

#include "library/dao/settingsdao.h"

SettingsDAO::SettingsDAO(QSqlDatabase& db)
        : m_db(db) {
}

SettingsDAO::~SettingsDAO() {

}

void SettingsDAO::initialize() {
}

QString SettingsDAO::getValue(QString name, QString defaultValue) {
    QSqlQuery query(m_db);

    query.prepare("SELECT value FROM settings WHERE name = :name");
    query.bindValue(":name", name);

    QString value = defaultValue;
    if (query.exec() && query.first()) {
        value = query.value(query.record().indexOf("value")).toString();
    }
    return value;
}

bool SettingsDAO::setValue(QString name, QVariant value) {
    if (!qVariantCanConvert<QString>(value)) {
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
