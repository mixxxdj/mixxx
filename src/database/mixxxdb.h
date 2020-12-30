#pragma once

#include <QSqlDatabase>

#include "preferences/usersettings.h"

#include "util/db/dbconnectionpool.h"


class MixxxDb : public QObject {
    Q_OBJECT

  public:
    static const QString kDefaultSchemaFile;

    static const int kRequiredSchemaVersion;

    static bool initDatabaseSchema(
            const QSqlDatabase& database,
            int schemaVersion = kRequiredSchemaVersion,
            const QString& schemaFile = kDefaultSchemaFile);

    explicit MixxxDb(
            const UserSettingsPointer& pConfig,
            bool inMemoryConnection = false);

    mixxx::DbConnectionPoolPtr connectionPool() const {
        return m_pDbConnectionPool;
    }

  private:
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
};
