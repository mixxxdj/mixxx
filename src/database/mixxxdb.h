#ifndef MIXXXDB_H
#define MIXXXDB_H


#include <QSqlDatabase>

#include "preferences/usersettings.h"

#include "util/db/dbconnection.h"


class MixxxDB : public QObject {
    Q_OBJECT

  public:
    static const QString kDefaultSchemaFile;
    static const int kRequiredSchemaVersion;

    explicit MixxxDB(
            UserSettingsPointer pConfig);

    bool initDatabaseSchema(
            const QString& schemaFile = kDefaultSchemaFile,
            int schemaVersion = kRequiredSchemaVersion);

    QSqlDatabase database() const {
        return m_dbConnection.database();
    }

  private:
    DbConnection m_dbConnection;
};


#endif //  MIXXXDB_H
