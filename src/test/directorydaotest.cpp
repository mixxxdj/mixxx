#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QtDebug>
#include <QtSql>

#include "library/dao/directorydao.h"
#include "library/dao/trackdao.h"
#include "preferences/usersettings.h"
#include "sources/soundsourceproxy.h"
#include "test/librarytest.h"
#include "track/track.h"

using ::testing::UnorderedElementsAre;

class DirectoryDAOTest : public LibraryTest {
  protected:
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

    static QString getSupportedFileExt() {
        const auto defaultFileExt = QStringLiteral("mp3");
        if (SoundSourceProxy::isFileExtensionSupported(defaultFileExt)) {
            return defaultFileExt;
        } else {
            return SoundSourceProxy::getSupportedFileExtensions().constFirst();
        }
    }
};

TEST_F(DirectoryDAOTest, addDirTest) {
    DirectoryDAO m_DirectoryDao = internalCollection()->getDirectoryDAO();
    // prepend dir with '/' so that QT thinks the dir starts at the root
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    QString testdir = QString(tempDir.path() + "/TestDir/a");
    QString testChild = QString(tempDir.path() + "/TestDir/a/child");
    QString testParent = QString(tempDir.path() + "/TestDir");

    //create temp dirs
    ASSERT_TRUE(QDir(tempDir.path()).mkpath(testParent));
    ASSERT_TRUE(QDir(tempDir.path()).mkpath(testdir));
    ASSERT_TRUE(QDir(tempDir.path()).mkpath(testChild));

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
    ASSERT_TRUE(success);

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
    DirectoryDAO m_DirectoryDao = internalCollection()->getDirectoryDAO();
    QString testdir = getTestDataDir().path();

    // check if directory doa adds and thinks everything is ok
    m_DirectoryDao.addDirectory(testdir);

    int success = m_DirectoryDao.removeDirectory(testdir);
    EXPECT_EQ(ALL_FINE, success);

    // we do not trust what directory dao thinks and better check up on it
    QSqlQuery query(dbConnection());
    query.prepare("SELECT " % DIRECTORYDAO_DIR % " FROM " % DIRECTORYDAO_TABLE);
    success = query.exec();
    ASSERT_TRUE(success);

    QStringList dirs;
    while (query.next()) {
        dirs << query.value(0).toString();
    }

    // the db should have now no entries left anymore
    EXPECT_EQ(0, dirs.size());
}

TEST_F(DirectoryDAOTest, getDirTest) {
    DirectoryDAO m_DirectoryDao = internalCollection()->getDirectoryDAO();
    QString testdir = "/a/c";
    QString testdir2 = "b/d";

    m_DirectoryDao.addDirectory(testdir);
    m_DirectoryDao.addDirectory(testdir2);

    QStringList dirs = m_DirectoryDao.getDirs();

    EXPECT_EQ(2, dirs.size());
    EXPECT_QSTRING_EQ(testdir, dirs.at(0));
    EXPECT_QSTRING_EQ(testdir2, dirs.at(1));
}

TEST_F(DirectoryDAOTest, relocateDirectory) {
    // Test with 2 independent root directories for relocation
    const QTemporaryDir tempDir1;
    ASSERT_TRUE(tempDir1.isValid());
    const QTemporaryDir tempDir2;
    ASSERT_TRUE(tempDir2.isValid());

    // Create temp dirs (with LIKE/GLOB wildcards and quotes in name)
#if defined(__WINDOWS__)
    // Exclude reserved characters from path names: ", *, ?
    // https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file#naming-conventions
    const QString oldDir = tempDir1.filePath("Test'_%Dir");
    const QString newDir = tempDir2.filePath("TestDir'_%New");
    const QString otherDir = tempDir2.filePath("TestDir'_%Other");
#else
    const QString oldDir = tempDir1.filePath("Test'\"_*%?Dir");
    const QString newDir = tempDir2.filePath("TestDir'\"_*%?New");
    const QString otherDir = tempDir2.filePath("TestDir'\"_*%?Other");
#endif
    ASSERT_TRUE(QDir(tempDir1.path()).mkpath(oldDir));
    ASSERT_TRUE(QDir(tempDir2.path()).mkpath(newDir));
    ASSERT_TRUE(QDir(tempDir2.path()).mkpath(otherDir));

    const DirectoryDAO& dao = internalCollection()->getDirectoryDAO();

    ASSERT_EQ(ALL_FINE, dao.addDirectory(oldDir));
    ASSERT_EQ(ALL_FINE, dao.addDirectory(otherDir));

    const QStringList oldDirs = dao.getDirs();
    ASSERT_EQ(2, oldDirs.size());
    ASSERT_THAT(oldDirs, UnorderedElementsAre(oldDir, otherDir));

    // Add tracks that should be relocated
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        TrackFile(oldDir, "a." + getSupportedFileExt())),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        TrackFile(oldDir, "b." + getSupportedFileExt())),
                                false)
                        .isValid());

    // Add tracks that should be unaffected by the relocation
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        TrackFile(newDir, "c." + getSupportedFileExt())),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        TrackFile(otherDir, "d." + getSupportedFileExt())),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        TrackFile(oldDir + "." + getSupportedFileExt())),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        TrackFile(newDir + "." + getSupportedFileExt())),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        TrackFile(otherDir + "." + getSupportedFileExt())),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        TrackFile(oldDir.toLower(), "a." + getSupportedFileExt())),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        TrackFile(oldDir.toUpper(), "b." + getSupportedFileExt())),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        TrackFile(newDir.toLower(), "c." + getSupportedFileExt())),
                                false)
                        .isValid());
    ASSERT_TRUE(
            internalCollection()
                    ->addTrack(
                            Track::newTemporary(
                                    TrackFile(otherDir.toUpper(), "d." + getSupportedFileExt())),
                            false)
                    .isValid());

    QList<RelocatedTrack> relocatedTracks =
            dao.relocateDirectory(oldDir, newDir);
    EXPECT_EQ(2, relocatedTracks.size());

    const QStringList newDirs = dao.getDirs();
    EXPECT_EQ(2, newDirs.size());
    EXPECT_THAT(newDirs, UnorderedElementsAre(newDir, otherDir));
}
