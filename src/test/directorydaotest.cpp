#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>
#include <QtSql>
#include <QString>
#include <QStringBuilder>
#include <QDir>
#include <QFileInfo>

#include "sources/soundsourceproxy.h"
#include "preferences/usersettings.h"
#include "library/dao/directorydao.h"
#include "library/dao/trackdao.h"

#include "test/librarytest.h"

using ::testing::ElementsAre;

namespace {

class DirectoryDAOTest : public LibraryTest {
  protected:
    void SetUp() override {
        m_supportedFileExt = "." % SoundSourceProxy::getSupportedFileExtensions().first();
    }

    void TearDown() override {
        // make sure we clean up the db
        QSqlQuery query(dbConnection());
        query.prepare("DELETE FROM " % DIRECTORYDAO_TABLE);
        query.exec();
        query.prepare("DELETE FROM library");
        query.exec();
        query.prepare("DELETE FROM track_locations");
        query.exec();
    }

    QString m_supportedFileExt;
};

TEST_F(DirectoryDAOTest, addDirTest) {
    DirectoryDAO m_DirectoryDao = collection()->getDirectoryDAO();
    // prepend dir with '/' so that QT thinks the dir starts at the root
    QString testdir(QDir::tempPath() + "/TestDir/a");
    QString testChild(QDir::tempPath() + "/TestDir/a/child");
    QString testParent(QDir::tempPath() + "/TestDir");

    //create temp dirs
    QDir(QDir::temp()).mkpath(testParent);
    QDir(QDir::temp()).mkpath(testdir);
    QDir(QDir::temp()).mkpath(testChild);

    // check if directory doa adds and thinks everything is ok
    int success = m_DirectoryDao.addDirectory(testdir);
    EXPECT_EQ(ALL_FINE, success);

    // check that we don't add the directory again
    success = m_DirectoryDao.addDirectory(testdir);
    EXPECT_EQ(ALREADY_WATCHING, success);

    // check that we don't add the directory again also if the string ends with
    // "/".
    success = m_DirectoryDao.addDirectory(testdir + "/");
    EXPECT_EQ(ALREADY_WATCHING, success);

    // check that we don't add a child directory
    success = m_DirectoryDao.addDirectory(testChild);
    EXPECT_EQ(ALREADY_WATCHING, success);

    // check that we add the parent dir
    success = m_DirectoryDao.addDirectory(testParent);
    EXPECT_EQ(ALL_FINE, success);

    QSqlQuery query(dbConnection());
    query.prepare("SELECT " % DIRECTORYDAO_DIR % " FROM " % DIRECTORYDAO_TABLE);
    success = query.exec();

    // we do not trust what directory dao thinks and better check up on it
    QStringList dirs;
    while (query.next()) {
        dirs << query.value(0).toString();
    }

    // the test db should be always empty when tests are started.
    EXPECT_EQ(1, dirs.size());
    EXPECT_QSTRING_EQ(testParent, dirs.at(0));
}

TEST_F(DirectoryDAOTest, removeDirTest) {
    DirectoryDAO m_DirectoryDao = collection()->getDirectoryDAO();
    QString testdir = getTestDataDir().path();

    // check if directory doa adds and thinks everything is ok
    m_DirectoryDao.addDirectory(testdir);

    int success = m_DirectoryDao.removeDirectory(testdir);
    EXPECT_EQ(ALL_FINE, success);

    // we do not trust what directory dao thinks and better check up on it
    QSqlQuery query(dbConnection());
    query.prepare("SELECT " % DIRECTORYDAO_DIR % " FROM " % DIRECTORYDAO_TABLE);
    success = query.exec();
    QStringList dirs;
    while (query.next()) {
        dirs << query.value(0).toString();
    }

    // the db should have now no entries left anymore
    EXPECT_EQ(0, dirs.size());
}

TEST_F(DirectoryDAOTest, getDirTest) {
    DirectoryDAO m_DirectoryDao = collection()->getDirectoryDAO();
    QString testdir = "/a/c";
    QString testdir2 = "b/d";

    m_DirectoryDao.addDirectory(testdir);
    m_DirectoryDao.addDirectory(testdir2);

    QStringList dirs = m_DirectoryDao.getDirs();

    EXPECT_EQ(2, dirs.size());
    EXPECT_QSTRING_EQ(testdir, dirs.at(0));
    EXPECT_QSTRING_EQ(testdir2, dirs.at(1));
}

TEST_F(DirectoryDAOTest, relocateDirTest) {
    DirectoryDAO &directoryDao = collection()->getDirectoryDAO();

    // use a temp dir so that we always use a real existing system path
    QString testdir(QDir::tempPath() + "/TestDir");
    QString test2(QDir::tempPath() + "/TestDir2");
    QString testnew(QDir::tempPath() + "/TestDirNew");

    directoryDao.addDirectory(testdir);
    directoryDao.addDirectory(test2);

    TrackDAO &trackDAO = collection()->getTrackDAO();
    // ok now lets create some tracks here
    trackDAO.addTracksPrepare();
    trackDAO.addTracksAddTrack(Track::newTemporary(TrackFile(testdir, "a" + m_supportedFileExt)), false);
    trackDAO.addTracksAddTrack(Track::newTemporary(TrackFile(testdir, "b" + m_supportedFileExt)), false);
    trackDAO.addTracksAddTrack(Track::newTemporary(TrackFile(test2, "c" + m_supportedFileExt)), false);
    trackDAO.addTracksAddTrack(Track::newTemporary(TrackFile(test2, "d" + m_supportedFileExt)), false);
    trackDAO.addTracksFinish(false);

    QSet<TrackId> ids = directoryDao.relocateDirectory(testdir, testnew);
    EXPECT_EQ(2, ids.size());

    QStringList dirs = directoryDao.getDirs();
    EXPECT_EQ(2, dirs.size());
    std::sort(dirs.begin(), dirs.end());
    EXPECT_THAT(dirs, ElementsAre(test2, testnew));
}

}  // namespace
