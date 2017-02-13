#include <gtest/gtest.h>
#include "util/db/dbconnection.h"


class SqliteLikeTest : public testing::Test {};

TEST_F(SqliteLikeTest, PatternTest) {
    QString pattern;
    QString string;
    QChar esc;

    pattern = QString::fromUtf8("%väth%");
    string = QString::fromUtf8("Sven Väth");
    esc = '\0';
    EXPECT_TRUE(DbConnection::likeCompareLatinLow(&pattern, &string, esc));

    pattern = QString::fromUtf8("%vath%");
    string = QString::fromUtf8("Sven Väth");
    esc = '\0';
    EXPECT_TRUE(DbConnection::likeCompareLatinLow(&pattern, &string, esc));

    pattern = QString::fromUtf8("%väth%");
    string = QString::fromUtf8("Sven Vath");
    esc = '\0';
    EXPECT_TRUE(DbConnection::likeCompareLatinLow(&pattern, &string, esc));

    pattern = QString::fromUtf8("%v_th%");
    string = QString::fromUtf8("Sven Väth");
    esc = '\0';
    EXPECT_TRUE(DbConnection::likeCompareLatinLow(&pattern, &string, esc));

    pattern = QString::fromUtf8("%v_th%%");
    string = QString::fromUtf8("Sven Väth");
    esc = '\0';
    EXPECT_TRUE(DbConnection::likeCompareLatinLow(&pattern, &string, esc));

    pattern = QString::fromUtf8("%v%_%th%%");
    string = QString::fromUtf8("Sven Väth");
    esc = '\0';
    EXPECT_TRUE(DbConnection::likeCompareLatinLow(&pattern, &string, esc));

    pattern = QString::fromUtf8("%v!%th%");
    string = QString::fromUtf8("Sven V%th");
    esc = '!';
    EXPECT_TRUE(DbConnection::likeCompareLatinLow(&pattern, &string, esc));

    pattern = QString::fromUtf8("%ä%");
    string = QString::fromUtf8("Tiësto");
    esc = '\0';
    EXPECT_FALSE(DbConnection::likeCompareLatinLow(&pattern, &string, esc));
}
