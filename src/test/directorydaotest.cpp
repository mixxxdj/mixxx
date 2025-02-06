#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QtDebug>

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
        const auto& directoryDao = internalCollection()->getDirectoryDAO();
        const auto allDirs = directoryDao.loadAllDirectories();
        for (const auto& dir : allDirs) {
            ASSERT_EQ(DirectoryDAO::RemoveResult::Ok, directoryDao.removeDirectory(dir));
        }
        ASSERT_TRUE(directoryDao.loadAllDirectories().isEmpty());
        QSqlQuery query(dbConnection());
        ASSERT_TRUE(query.prepare("DELETE FROM library"));
        ASSERT_TRUE(query.exec());
        ASSERT_TRUE(query.prepare("DELETE FROM track_locations"));
        ASSERT_TRUE(query.exec());
    }

    static QString getSupportedFileSuffix() {
        // Return any supported suffix, it doesn't matter which
        const auto supportedFileSuffixes = SoundSourceProxy::getSupportedFileSuffixes();
        VERIFY_OR_DEBUG_ASSERT(!supportedFileSuffixes.isEmpty()) {
            return {};
        }
        return supportedFileSuffixes.constFirst();
    }
};

TEST_F(DirectoryDAOTest, add) {
    const QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    //create temp dirs
    const QString testdir = QString(tempDir.path() + "/TestDir/test");
    ASSERT_TRUE(QDir(tempDir.path()).mkpath(testdir));
    const QString testChild = QString(tempDir.path() + "/TestDir/test/child");
    ASSERT_TRUE(QDir(tempDir.path()).mkpath(testChild));
    const QString testParent = QString(tempDir.path() + "/TestDir");
    ASSERT_TRUE(QDir(tempDir.path()).mkpath(testParent));
#if !defined(__WINDOWS__)
    const QString linkdir = QString(tempDir.path() + "/testLink");
    ASSERT_TRUE(QFile::link(testdir, linkdir));
    const QString linkChild = QString(tempDir.path() + "/childLink");
    ASSERT_TRUE(QFile::link(testChild, linkChild));
    const QString linkParent = QString(tempDir.path() + "/parentLink");
    ASSERT_TRUE(QFile::link(testParent, linkParent));
#endif

    const DirectoryDAO& dao = internalCollection()->getDirectoryDAO();

    // check if directory doa adds and thinks everything is ok
    EXPECT_EQ(
            DirectoryDAO::AddResult::Ok,
            dao.addDirectory(mixxx::FileInfo(testdir)));

    // check that we don't add the directory again
    EXPECT_EQ(
            DirectoryDAO::AddResult::AlreadyWatching,
            dao.addDirectory(mixxx::FileInfo(testdir)));
#if !defined(__WINDOWS__)
    EXPECT_EQ(
            DirectoryDAO::AddResult::AlreadyWatching,
            dao.addDirectory(mixxx::FileInfo(linkdir)));
#endif

    // check that we don't add the directory again also if the string ends with
    // "/".
    EXPECT_EQ(
            DirectoryDAO::AddResult::AlreadyWatching,
            dao.addDirectory(mixxx::FileInfo(testdir)));
#if !defined(__WINDOWS__)
    EXPECT_EQ(
            DirectoryDAO::AddResult::AlreadyWatching,
            dao.addDirectory(mixxx::FileInfo(linkdir)));
#endif

    // check that we don't add a child directory
    EXPECT_EQ(
            DirectoryDAO::AddResult::AlreadyWatching,
            dao.addDirectory(mixxx::FileInfo(testChild)));
#if !defined(__WINDOWS__)
    EXPECT_EQ(
            DirectoryDAO::AddResult::AlreadyWatching,
            dao.addDirectory(mixxx::FileInfo(linkChild)));
#endif

#if !defined(__WINDOWS__)
    // Use the link to the parent directory
    const auto parentInfo = mixxx::FileInfo(linkParent);
#else
    // Use the the parent directory
    const auto parentInfo = mixxx::FileInfo(testParent);
#endif

    // check that we add the parent dir
    EXPECT_EQ(
            DirectoryDAO::AddResult::Ok,
            dao.addDirectory(parentInfo));

    // the db should now only contain the link to the parent directory
    const QList<mixxx::FileInfo> allDirs = dao.loadAllDirectories();
    ASSERT_EQ(1, allDirs.length());
    EXPECT_QSTRING_EQ(parentInfo.location(), allDirs.first().location());

#if !defined(__WINDOWS__)
    // Verify that adding the actual dir path instead of the
    // link is rejected.
    EXPECT_EQ(
            DirectoryDAO::AddResult::AlreadyWatching,
            dao.addDirectory(mixxx::FileInfo(testParent)));
#endif
}

TEST_F(DirectoryDAOTest, remove) {
    const QString testdir = getTestDataDir().path();

    const DirectoryDAO& dao = internalCollection()->getDirectoryDAO();

    EXPECT_EQ(
            DirectoryDAO::RemoveResult::NotFound,
            dao.removeDirectory(mixxx::FileInfo(testdir)));

    ASSERT_EQ(
            DirectoryDAO::AddResult::Ok,
            dao.addDirectory(mixxx::FileInfo(testdir)));

    EXPECT_EQ(
            DirectoryDAO::RemoveResult::Ok,
            dao.removeDirectory(mixxx::FileInfo(testdir)));

    // the db should have now no entries left anymore
    EXPECT_TRUE(dao.loadAllDirectories().isEmpty());

    EXPECT_EQ(
            DirectoryDAO::RemoveResult::NotFound,
            dao.removeDirectory(mixxx::FileInfo(testdir)));
}

TEST_F(DirectoryDAOTest, loadAll) {
    const QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    //create temp dirs
    const QString testdir = tempDir.path() + "/a/c";
    ASSERT_TRUE(QDir(tempDir.path()).mkpath(testdir));
    const QString testdir2 = tempDir.path() + "b/d";
    ASSERT_TRUE(QDir(tempDir.path()).mkpath(testdir2));

    const DirectoryDAO dao = internalCollection()->getDirectoryDAO();

    ASSERT_EQ(DirectoryDAO::AddResult::Ok, dao.addDirectory(mixxx::FileInfo(testdir)));
    ASSERT_EQ(DirectoryDAO::AddResult::Ok, dao.addDirectory(mixxx::FileInfo(testdir2)));

    const QList<mixxx::FileInfo> allDirs = dao.loadAllDirectories();
    EXPECT_EQ(2, allDirs.size());
    EXPECT_THAT(allDirs, UnorderedElementsAre(mixxx::FileInfo(testdir), mixxx::FileInfo(testdir2)));
}

TEST_F(DirectoryDAOTest, relocateDirectory) {
    // Test with 2 independent root directories for relocation
    const QTemporaryDir tempDir1;
    ASSERT_TRUE(tempDir1.isValid());
    const QTemporaryDir tempDir2;
    ASSERT_TRUE(tempDir2.isValid());

    // Create temp dirs (with LIKE/GLOB wildcards, Unicode characters, and quotes in name).
    // The different directories only differ by the character 'e' with decorations, namely
    // 'é' and 'ë'.
#if defined(__WINDOWS__)
    // Exclude reserved characters from path names: ", *, ?
    // https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file#naming-conventions
    const QString oldDirPath = tempDir1.filePath("Test'_%Dir");
    const QString newDirPath = tempDir2.filePath("Tést'_%Dir");
    const QString otherDirPath = tempDir2.filePath("Tëst'_%Dir");
#else
    const QString oldDirPath = tempDir1.filePath("Test'\"_*%?Dir");
    const QString newDirPath = tempDir2.filePath("Tést'\"_*%?Dir");
    const QString otherDirPath = tempDir2.filePath("Tëst'\"_*%?Dir");
#endif
    ASSERT_TRUE(QDir{}.mkpath(oldDirPath));
    ASSERT_TRUE(QDir{}.mkpath(newDirPath));
    ASSERT_TRUE(QDir{}.mkpath(otherDirPath));

    const auto oldDirInfo = mixxx::FileInfo(oldDirPath);
    const auto newDirInfo = mixxx::FileInfo(newDirPath);
    const auto otherDirInfo = mixxx::FileInfo(otherDirPath);

    const DirectoryDAO& dao = internalCollection()->getDirectoryDAO();

    ASSERT_EQ(DirectoryDAO::AddResult::Ok, dao.addDirectory(oldDirInfo));
    ASSERT_EQ(DirectoryDAO::AddResult::Ok, dao.addDirectory(otherDirInfo));

    const QList<mixxx::FileInfo> oldDirs = dao.loadAllDirectories();
    ASSERT_EQ(2, oldDirs.size());
    ASSERT_THAT(oldDirs, UnorderedElementsAre(oldDirInfo, otherDirInfo));

    // Add tracks that should be relocated
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        oldDirPath, "a." + getSupportedFileSuffix()),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        oldDirPath, "b." + getSupportedFileSuffix()),
                                false)
                        .isValid());

    // Add tracks that should be unaffected by the relocation
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        newDirPath, "c." + getSupportedFileSuffix()),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        otherDirPath, "d." + getSupportedFileSuffix()),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        oldDirPath + "." + getSupportedFileSuffix()),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        newDirPath + "." + getSupportedFileSuffix()),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        otherDirPath + "." + getSupportedFileSuffix()),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        oldDirPath.toLower(), "a." + getSupportedFileSuffix()),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        oldDirPath.toUpper(), "b." + getSupportedFileSuffix()),
                                false)
                        .isValid());
    ASSERT_TRUE(internalCollection()
                        ->addTrack(
                                Track::newTemporary(
                                        newDirPath.toLower(), "c." + getSupportedFileSuffix()),
                                false)
                        .isValid());
    ASSERT_TRUE(
            internalCollection()
                    ->addTrack(
                            Track::newTemporary(
                                    otherDirPath.toUpper(), "d." + getSupportedFileSuffix()),
                            false)
                    .isValid());

    QList<RelocatedTrack> relocatedTracks =
            dao.relocateDirectory(oldDirPath, newDirPath).second;
    EXPECT_EQ(2, relocatedTracks.size());

    const QList<mixxx::FileInfo> newDirs = dao.loadAllDirectories();
    EXPECT_EQ(2, newDirs.size());
    EXPECT_THAT(newDirs, UnorderedElementsAre(newDirInfo, otherDirInfo));
}
