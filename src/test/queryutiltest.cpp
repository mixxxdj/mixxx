#include <gtest/gtest.h>

#include "test/librarytest.h"

#include "library/queryutil.h"
#include "util/db/sqllikewildcardescaper.h"

class QueryUtilTest : public LibraryTest {
  protected:
    QueryUtilTest()
        : m_fieldEscaper(collection()->database()) {
    }

    FieldEscaper m_fieldEscaper;
};

TEST_F(QueryUtilTest, FieldEscaperEscapesQuotes) {
    EXPECT_STREQ(qPrintable(QString("'foobar'")),
                 qPrintable(m_fieldEscaper.escapeString("foobar")));
    EXPECT_STREQ(qPrintable(QString("'foobar''s'")),
                 qPrintable(m_fieldEscaper.escapeString("foobar's")));
}

TEST_F(QueryUtilTest, FieldEscaperEscapesForLike) {
    EXPECT_STREQ(qPrintable(QString("xx44xx4%yy4_yy")),
                 qPrintable(SqlLikeWildcardEscaper::apply("xx4xx%yy_yy", '4')));
}
