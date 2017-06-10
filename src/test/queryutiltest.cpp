#include <gtest/gtest.h>

#include "test/mixxxtest.h"

#include "repository/repository.h"

#include "library/queryutil.h"
#include "util/db/sqllikewildcardescaper.h"

class QueryUtilTest : public MixxxTest {
  protected:
    QueryUtilTest()
          : m_repository(config()),
            m_dbConnectionScope(m_repository.dbConnectionPool()) {
        // This test only needs a connection to an empty database
        // without any particular schema. No need to initialize the
        // database schema.
    }

    mixxx::Repository m_repository;
    const mixxx::DbConnectionPool::ThreadLocalScope m_dbConnectionScope;
};

TEST_F(QueryUtilTest, FieldEscaperEscapesQuotes) {
    FieldEscaper fieldEscaper(m_dbConnectionScope);
    EXPECT_STREQ(qPrintable(QString("'foobar'")),
                 qPrintable(fieldEscaper.escapeString("foobar")));
    EXPECT_STREQ(qPrintable(QString("'foobar''s'")),
                 qPrintable(fieldEscaper.escapeString("foobar's")));
}

TEST_F(QueryUtilTest, SqlLikeWildcardEscaperEscapesForLike) {
    EXPECT_STREQ(qPrintable(QString("xx44xx4%yy4_yy")),
                 qPrintable(SqlLikeWildcardEscaper::apply("xx4xx%yy_yy", '4')));
}
