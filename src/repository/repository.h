#ifndef MIXXX_REPOSITORY_H
#define MIXXX_REPOSITORY_H


#include <QSqlDatabase>

#include "preferences/usersettings.h"

#include "util/db/dbconnection.h"


namespace mixxx {

class Repository : public QObject {
    Q_OBJECT

  public:
    static const QString kDefaultSchemaFile;
    static const int kRequiredSchemaVersion;

    explicit Repository(
            const UserSettingsPointer& pConfig);

    bool initDatabaseSchema(
            const QString& schemaFile = kDefaultSchemaFile,
            int schemaVersion = kRequiredSchemaVersion);

    QSqlDatabase database() const {
        return m_dbConnection.database();
    }

  private:
    DbConnection m_dbConnection;
};

} // namespace mixxx


#endif //  MIXXX_REPOSITORY_H
