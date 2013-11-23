#ifndef SETTINGSDAO_H
#define SETTINGSDAO_H

#include <QtSql>

class SettingsDAO : public QObject {
  public:
    SettingsDAO(QSqlDatabase &db);
    virtual ~SettingsDAO();

    virtual void initialize();

    QString getValue(QString name, QString defaultValue = QString());
    bool setValue(QString name, QVariant value);

  private:
    QSqlDatabase m_db;
};

#endif /* SETTINGSDAO_H */
