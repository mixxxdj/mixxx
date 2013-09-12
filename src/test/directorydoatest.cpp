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
        // make sure we clean up the db
        QSqlQuery query(m_pTrackCollection->getDatabase());
        query.prepare("DELETE FROM " % DIRECTORYDAO_TABLE);
        query.exec();

        delete m_pTrackCollection;
        delete m_pConfig;
    }

    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection *m_pTrackCollection;
};

TEST_F(DirectoryDAOTest, addDirTest) {
    DirectoryDAO m_DirectoryDao = m_pTrackCollection->getDirectoryDAO();
    QString testdir = "TestDir";

    // check if directory doa adds and thinks everything is ok
    bool success = m_DirectoryDao.addDirectory(testdir);
    EXPECT_TRUE(success);

    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare("SELECT " % DIRECTORYDAO_DIR % " FROM " % DIRECTORYDAO_TABLE);
    success = query.exec();

    // we do not trust what directory dao thinks and better check up on it
    QStringList dirs;
    while (query.next()) {
        dirs << query.value(0).toString();
    }

    // the test db should be always empty when tests are started.
    EXPECT_TRUE(dirs.size() == 1);
    if ( dirs.size() > 0) {
        EXPECT_TRUE(dirs.at(0) == testdir);
    }
}

TEST_F(DirectoryDAOTest, removeDirTest) {
    DirectoryDAO m_DirectoryDao = m_pTrackCollection->getDirectoryDAO();
    QString testdir = "TestDir";

    // check if directory doa adds and thinks everything is ok
    m_DirectoryDao.addDirectory(testdir);

    bool success = m_DirectoryDao.removeDirectory(testdir);
    EXPECT_TRUE(success);

    // we do not trust what directory dao thinks and better check up on it
    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare("SELECT " % DIRECTORYDAO_DIR % " FROM " % DIRECTORYDAO_TABLE);
    success = query.exec();
    QStringList dirs;
    while (query.next()) {
        dirs << query.value(0).toString();
    }

    // the db should have now no entries left anymore
    EXPECT_TRUE(dirs.size() == 0);
}

}  // namespace
