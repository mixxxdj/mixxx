#include <gtest/gtest.h>

#include "library/queryutil.h"
#include "test/mixxxdbtest.h"
#include "util/db/sqllikewildcardescaper.h"

class QueryUtilTest : public MixxxDbTest {};

TEST_F(QueryUtilTest, FieldEscaperEscapesQuotes) {
    FieldEscaper fieldEscaper(dbConnection());
    EXPECT_STREQ(qPrintable(QString("'foobar'")),
            qPrintable(fieldEscaper.escapeString("foobar")));
    EXPECT_STREQ(qPrintable(QString("'foobar''s'")),
            qPrintable(fieldEscaper.escapeString("foobar's")));
}

TEST_F(QueryUtilTest, SqlLikeWildcardEscaperEscapesForLike) {
    EXPECT_STREQ(qPrintable(QString("xx44xx4%yy4_yy")),
            qPrintable(SqlLikeWildcardEscaper::apply("xx4xx%yy_yy", '4')));
}
