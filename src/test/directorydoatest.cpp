#include <gtest/gtest.h>

#include <QtDebug>
#include <QtSql>
#include <QString>
#include <QStringBuilder>
#include <QDir>

#include "configobject.h"
#include "library/dao/directorydao.h"
#include "library/trackcollection.h"

namespace {

class DirectoryDAOTest : public testing::Test {
  protected:
    virtual void SetUp() {
        m_pConfig = new ConfigObject<ConfigValue>(QDir::currentPath().append("/src/test/test_data/test.cfg"));
        m_pTrackCollection = new TrackCollection(m_pConfig);
    }

    virtual void TearDown() {
        delete m_pTrackCollection;
        delete m_pConfig;
    }

    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection *m_pTrackCollection;
};

TEST_F(DirectoryDAOTest, addDirTest) {
    DirectoryDAO m_DirectoryDao = m_pTrackCollection->getDirectoryDAO();
    QString testdir = "TestDir";

    bool success = m_DirectoryDao.addDirectory(testdir);
    EXPECT_TRUE(success);

    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare("SELECT " % DIRECTORYDAO_DIR % " FROM " % DIRECTORYDAO_TABLE);

    success = query.exec();
    EXPECT_TRUE(success);

    QStringList dirs;
    while (query.next()) {
        dirs << query.value(0).toString();
    }

    EXPECT_TRUE(dirs.size() == 1);
    if ( dirs.size() > 0) {
        EXPECT_TRUE(dirs.at(0) == testdir);
    }

    query.prepare("DELETE FROM " % DIRECTORYDAO_TABLE  % " WHERE "
                   % DIRECTORYDAO_DIR % "= :dir");
    query.bindValue(":dir", testdir);
    success = query.exec();
    EXPECT_TRUE(success);
}

}  // namespace
