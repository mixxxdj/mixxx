#include "database/mixxxdb.h"
#include "test/mixxxtest.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"

class MixxxDbTest : public MixxxTest {
  protected:
    MixxxDbTest(bool inMemoryDbConnection = false)
            : m_mixxxDb(config(), inMemoryDbConnection),
              m_dbConnectionPooler(m_mixxxDb.connectionPool()) {
    }

    const mixxx::DbConnectionPoolPtr& dbConnectionPooler() const {
        return m_dbConnectionPooler;
    }

    QSqlDatabase dbConnection() const {
        return mixxx::DbConnectionPooled(m_dbConnectionPooler);
    }

  private:
    const MixxxDb m_mixxxDb;
    const mixxx::DbConnectionPooler m_dbConnectionPooler;
};
