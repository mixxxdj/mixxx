#ifndef SETTINGSDAO_H
#define SETTINGSDAO_H

#include <QtSql>

#define SETTINGS_TABLE "settings"

#define SETTINGSTABLE_NAME "name"
#define SETTINGSTABLE_VALUE "value"
#define SETTINGSTABLE_LOCKED "locked"
#define SETTINGSTABLE_HIDDEN "hidden"


// All library-specific preferences go in the library settings table
class SettingsDAO : public QObject {
  public:
    SettingsDAO(QSqlDatabase &db);
    virtual ~SettingsDAO();

    virtual void initialize();

    QString getValue(const QString& name, QString defaultValue = QString());
    bool setValue(const QString& name, const QVariant& value);

  private:
    QSqlDatabase m_db;
};

#endif /* SETTINGSDAO_H */
