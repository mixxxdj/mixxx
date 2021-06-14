#include "util/fileinfo.h"

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QtDebug>

namespace mixxx {

class FileInfoTest : public testing::Test {
  protected:
    const QTemporaryDir m_tempDir;

    const QString m_relativePath;
    const QString m_absolutePath;

    const QString m_relativePathMissing;
    const QString m_absolutePathMissing;

    FileInfoTest()
            : m_relativePath(QStringLiteral("relative")),
              m_absolutePath(m_tempDir.filePath(m_relativePath)),
              m_relativePathMissing(QStringLiteral("missing")),
              m_absolutePathMissing(m_tempDir.filePath(m_relativePathMissing)) {
    }

    void SetUp() override {
        ASSERT_TRUE(m_tempDir.isValid());
        ASSERT_TRUE(QDir(m_tempDir.path()).mkpath(m_relativePath));
        ASSERT_TRUE(FileInfo(m_absolutePath).exists());
        ASSERT_FALSE(FileInfo(m_absolutePathMissing).exists());
    }
};

TEST_F(FileInfoTest, emptyPathIsRelative) {
    EXPECT_FALSE(FileInfo().isAbsolute());
    EXPECT_TRUE(FileInfo().isRelative());
}

TEST_F(FileInfoTest, nonEmptyPathIsEitherAbsoluteOrRelative) {
    EXPECT_TRUE(FileInfo(m_absolutePath).isAbsolute());
    EXPECT_FALSE(FileInfo(m_absolutePath).isRelative());
    EXPECT_TRUE(FileInfo(m_absolutePathMissing).isAbsolute());
    EXPECT_FALSE(FileInfo(m_absolutePathMissing).isRelative());
    EXPECT_FALSE(FileInfo(m_relativePath).isAbsolute());
    EXPECT_TRUE(FileInfo(m_relativePath).isRelative());
    EXPECT_FALSE(FileInfo(m_relativePathMissing).isAbsolute());
    EXPECT_TRUE(FileInfo(m_relativePathMissing).isRelative());
}

TEST_F(FileInfoTest, hasLocation) {
    EXPECT_FALSE(FileInfo().hasLocation());
    EXPECT_TRUE(FileInfo(m_absolutePath).hasLocation());
    EXPECT_TRUE(FileInfo(m_absolutePath).hasLocation());
    EXPECT_FALSE(FileInfo(m_relativePath).hasLocation());
    EXPECT_TRUE(FileInfo(m_absolutePathMissing).hasLocation());
    EXPECT_FALSE(FileInfo(m_relativePathMissing).hasLocation());
}

TEST_F(FileInfoTest, freshCanonicalFileInfo) {
    FileInfo fileInfo(m_absolutePathMissing);
    // This test assumes that caching is enabled resulting
    // in expected inconsistencies until refreshed.
    ASSERT_TRUE(fileInfo.m_fileInfo.caching());

    ASSERT_TRUE(fileInfo.canonicalLocation().isEmpty());
    ASSERT_TRUE(fileInfo.resolveCanonicalLocation().isEmpty());

    // Restore the missing file
    QFile file(m_absolutePathMissing);
    ASSERT_FALSE(fileInfo.checkFileExists());
    ASSERT_TRUE(file.open(QIODevice::ReadWrite | QIODevice::NewOnly));
    ASSERT_TRUE(fileInfo.checkFileExists());

    // The cached canonical location should still be invalid
    EXPECT_TRUE(fileInfo.canonicalLocation().isEmpty());
    // The refreshed canonical location should be valid
    EXPECT_FALSE(fileInfo.resolveCanonicalLocation().isEmpty());
    // The cached canonical location should have been updated
    EXPECT_FALSE(fileInfo.canonicalLocation().isEmpty());

    // Remove the file
    ASSERT_TRUE(file.remove());
    ASSERT_FALSE(fileInfo.checkFileExists());
    ASSERT_TRUE(FileInfo(m_absolutePathMissing).canonicalLocation().isEmpty());

    // Note: Qt (5.14.x) doesn't seem to invalidate the canonical location
    // after it has been set once, even when refreshing the QFileInfo. This
    // is what the remaining part of the test validates, although Mixxx does
    // NOT rely on this behavior! Just to get notified when this behavior
    // changes for some future Qt version and for ensuring that the behavior
    // is identical on all platforms.

    // The cached canonical location should still be valid
    EXPECT_FALSE(fileInfo.canonicalLocation().isEmpty());
    // The canonical location should not be refreshed again, i.e. it remains
    // valid after the file has disappeared
    EXPECT_FALSE(fileInfo.resolveCanonicalLocation().isEmpty());
}

} // namespace mixxx
