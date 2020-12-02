#pragma once

#include <QSqlDatabase>
#include <QString>
#include <QVariant>
#include <utility>

// All library-specific preferences go in the library settings table
class SettingsDAO final {
  public:
    explicit SettingsDAO(QSqlDatabase database)
            : m_database(std::move(database)) {
    }

    QString getValue(
            const QString& name,
            QString defaultValue = QString()) const;
    bool setValue(
            const QString& name,
            const QVariant& value) const;

  private:
    const QSqlDatabase m_database;
};
