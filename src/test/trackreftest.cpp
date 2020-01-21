#include <gtest/gtest.h>

#include <QDir>
#include <QTemporaryFile>
#include <QtDebug>

#include "track/trackref.h"

namespace {

class TrackRefTest : public testing::Test {
  protected:

    // NOTE(uklotzde): Explicitly initialize all const members, even
    // if only the default constructor is used. This workaround was
    // needed for LLVM version 7.0.2 (clang-700.1.81).
    TrackRefTest()
      : m_tempFile("TrackRefTest.tmp"),
        m_tempFileDir(QDir::tempPath()),
        m_tempFileName(m_tempFile.fileName()),
        m_tempFileInfo(m_tempFileDir, m_tempFileName),
        m_validTrackId(123),
        m_invalidTrackId() {
    }

    virtual void SetUp() {
        ASSERT_TRUE(m_validTrackId.isValid());
        ASSERT_FALSE(m_invalidTrackId.isValid());
    }

    virtual void TearDown() {
    }

    static void verifyFileInfo(const TrackRef& actual, const TrackFile& trackFile) {
        EXPECT_TRUE(actual.hasLocation());
        EXPECT_EQ(trackFile.location(), actual.getLocation());
        EXPECT_TRUE(actual.hasCanonicalLocation());
        EXPECT_EQ(trackFile.canonicalLocation(), actual.getCanonicalLocation());
    }

    const QTemporaryFile m_tempFile;
    const QDir m_tempFileDir;
    const QString m_tempFileName;
    const TrackFile m_tempFileInfo;
    const TrackId m_validTrackId;
    const TrackId m_invalidTrackId;
};

TEST_F(TrackRefTest, DefaultConstructor) {
    TrackRef actual;

    EXPECT_FALSE(actual.hasLocation());
    EXPECT_FALSE(actual.hasCanonicalLocation());
    EXPECT_FALSE(actual.hasId());
}

TEST_F(TrackRefTest, FromFileInfoWithId) {
    const TrackRef actual(
            TrackRef::fromFileInfo(m_tempFileInfo, m_validTrackId));

    verifyFileInfo(actual, m_tempFileInfo);
    EXPECT_TRUE(actual.hasId());
    EXPECT_EQ(m_validTrackId, actual.getId());
}

TEST_F(TrackRefTest, FromFileInfoWithoutId) {
    const TrackRef actual(
            TrackRef::fromFileInfo(m_tempFileInfo));

    verifyFileInfo(actual, m_tempFileInfo);
    EXPECT_FALSE(actual.hasId());
    EXPECT_EQ(m_invalidTrackId, actual.getId());
}

TEST_F(TrackRefTest, CopyAndSetId) {
    const TrackRef withoutId(
            TrackRef::fromFileInfo(m_tempFileInfo));

    const TrackRef actual(withoutId, m_validTrackId);

    verifyFileInfo(actual, m_tempFileInfo);
    EXPECT_TRUE(actual.hasId());
    EXPECT_EQ(m_validTrackId, actual.getId());
}

}  // namespace
