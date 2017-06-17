#include <gtest/gtest.h>

#include "test/mixxxtest.h"

#include "database/mixxxdb.h"

#include "library/queryutil.h"
#include "util/db/sqllikewildcardescaper.h"
#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"

class QueryUtilTest : public MixxxTest {
  protected:
    QueryUtilTest()
          : m_mixxxDb(config()),
            m_dbConnectionPooler(m_mixxxDb.connectionPool()),
            m_dbConnection(mixxx::DbConnectionPooled(m_mixxxDb.connectionPool())) {
        // This test only needs a connection to an empty database
        // without any particular schema. No need to initialize the
        // database schema.
    }

  private:
    const MixxxDb m_mixxxDb;
    const mixxx::DbConnectionPooler m_dbConnectionPooler;

  protected:
    QSqlDatabase m_dbConnection;
};

TEST_F(QueryUtilTest, FieldEscaperEscapesQuotes) {
    FieldEscaper fieldEscaper(m_dbConnection);
    EXPECT_STREQ(qPrintable(QString("'foobar'")),
                 qPrintable(fieldEscaper.escapeString("foobar")));
    EXPECT_STREQ(qPrintable(QString("'foobar''s'")),
                 qPrintable(fieldEscaper.escapeString("foobar's")));
}

TEST_F(QueryUtilTest, SqlLikeWildcardEscaperEscapesForLike) {
    EXPECT_STREQ(qPrintable(QString("xx44xx4%yy4_yy")),
                 qPrintable(SqlLikeWildcardEscaper::apply("xx4xx%yy_yy", '4')));
}
