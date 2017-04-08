#include <gtest/gtest.h>
#include <QtDebug>
#include <QDir>
#include <QTemporaryFile>

#include "library/queryutil.h"
#include "util/db/sqllikewildcardescaper.h"

class QueryUtilTest : public testing::Test {};

TEST_F(QueryUtilTest, FieldEscaperEscapesQuotes) {
    QTemporaryFile databaseFile("mixxxdb.sqlite");
    ASSERT_TRUE(databaseFile.open());

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("localhost");
    db.setUserName("mixxx");
    db.setPassword("mixxx");
    qDebug() << "Temp file is" << databaseFile.fileName();
    db.setDatabaseName(databaseFile.fileName());
    ASSERT_TRUE(db.open());
    FieldEscaper f(db);


    EXPECT_STREQ(qPrintable(QString("'foobar'")),
                 qPrintable(f.escapeString("foobar")));
    EXPECT_STREQ(qPrintable(QString("'foobar''s'")),
                 qPrintable(f.escapeString("foobar's")));
}

TEST_F(QueryUtilTest, FieldEscaperEscapesForLike) {
    QTemporaryFile databaseFile("mixxxdb.sqlite");
    ASSERT_TRUE(databaseFile.open());

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("localhost");
    db.setUserName("mixxx");
    db.setPassword("mixxx");
    qDebug() << "Temp file is" << databaseFile.fileName();
    db.setDatabaseName(databaseFile.fileName());
    ASSERT_TRUE(db.open());
    FieldEscaper f(db);

    EXPECT_STREQ(qPrintable(QString("xx44xx4%yy4_yy")),
                 qPrintable(SqlLikeWildcardEscaper::apply("xx4xx%yy_yy", '4')));
}
