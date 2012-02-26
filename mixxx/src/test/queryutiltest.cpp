#include <gtest/gtest.h>
#include <QtDebug>
#include <QDir>
#include <QTemporaryFile>

#include "library/queryutil.h"

class QueryUtilTest : public testing::Test {
  protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(QueryUtilTest, FieldEscaperEscapesQuotes) {
    QTemporaryFile databaseFile("mixxxdb.sqlite");
    Q_ASSERT(databaseFile.open());

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("localhost");
    db.setUserName("mixxx");
    db.setPassword("mixxx");
    qDebug() << "Temp file is" << databaseFile.fileName();
    db.setDatabaseName(databaseFile.fileName());
    Q_ASSERT(db.open());
    FieldEscaper f(db);


    EXPECT_STREQ(qPrintable(QString("'foobar'")),
                 qPrintable(f.escapeString("foobar")));
    EXPECT_STREQ(qPrintable(QString("'foobar''s'")),
                 qPrintable(f.escapeString("foobar's")));
}
