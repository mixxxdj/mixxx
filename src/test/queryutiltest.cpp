#include <gtest/gtest.h>

#include "library/queryutil.h"
#include "test/mixxxdbtest.h"

class QueryUtilTest : public MixxxDbTest {};

TEST_F(QueryUtilTest, FieldEscaperEscapesQuotes) {
    FieldEscaper fieldEscaper(dbConnection());
    EXPECT_STREQ(qPrintable(QString("'foobar'")),
            qPrintable(fieldEscaper.escapeString("foobar")));
    EXPECT_STREQ(qPrintable(QString("'foobar''s'")),
            qPrintable(fieldEscaper.escapeString("foobar's")));
}
