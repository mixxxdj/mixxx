#ifndef MIXXX_REPOSITORY_H
#define MIXXX_REPOSITORY_H


#include <QSqlDatabase>

#include "preferences/usersettings.h"

#include "util/db/dbconnectionpool.h"


namespace mixxx {

class Repository : public QObject {
    Q_OBJECT

  public:
    static const QString kDefaultSchemaFile;

    static const int kRequiredSchemaVersion;

    static bool initDatabaseSchema(
            const QSqlDatabase& database,
            const QString& schemaFile = kDefaultSchemaFile,
            int schemaVersion = kRequiredSchemaVersion);

    explicit Repository(
            const UserSettingsPointer& pConfig);

    DbConnectionPoolPtr dbConnectionPool() const {
        return m_pDbConnectionPool;
    }

  private:
    DbConnectionPoolPtr m_pDbConnectionPool;
};

} // namespace mixxx


#endif //  MIXXX_REPOSITORY_H
