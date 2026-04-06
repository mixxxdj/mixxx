#include "library/dao/tracklinkdao.h"

#include <gtest/gtest.h>

#include <QTemporaryDir>

#include "library/trackcollection.h"
#include "test/librarytest.h"
#include "track/track.h"

class TrackLinkDaoTest : public LibraryTest {
  protected:
    TrackLinkDao& dao() {
        return internalCollection()->getTrackLinkDAO();
    }

    // Add a temporary track file and return its library TrackId.
    TrackId addTrack(const QString& filename) {
        const QString path = m_tempDir.filePath(filename);
        // Create the file so the library accepts it.
        QFile f(path);
        f.open(QIODevice::WriteOnly);
        f.close();
        const auto pTrack = getOrAddTrackByLocation(path);
        EXPECT_TRUE(pTrack != nullptr);
        if (!pTrack) {
            return TrackId{};
        }
        return pTrack->getId();
    }

    QTemporaryDir m_tempDir;
};

TEST_F(TrackLinkDaoTest, LinkAndGetLinks) {
    TrackId trackId = addTrack(QStringLiteral("stem_track.stem.mp4"));
    TrackId targetId = addTrack(QStringLiteral("source_track.flac"));
    ASSERT_TRUE(trackId.isValid());
    ASSERT_TRUE(targetId.isValid());

    const double offset = 10.5;
    const TrackLink::Type type = TrackLink::Type::Stem;

    EXPECT_TRUE(dao().linkTracks(trackId, targetId, offset, type));

    QList<TrackLink> links = dao().getLinksForTrack(trackId);
    ASSERT_EQ(links.size(), 1);
    EXPECT_EQ(links[0].trackId, trackId);
    EXPECT_EQ(links[0].targetTrackId, targetId);
    EXPECT_DOUBLE_EQ(links[0].offsetMs, offset);
    EXPECT_EQ(links[0].type, type);

    QList<TrackLink> targetLinks = dao().getLinksToTarget(targetId);
    ASSERT_EQ(targetLinks.size(), 1);
    EXPECT_EQ(targetLinks[0].trackId, trackId);
}

TEST_F(TrackLinkDaoTest, UpdateOffset) {
    TrackId trackId = addTrack(QStringLiteral("stem_update.stem.mp4"));
    TrackId targetId = addTrack(QStringLiteral("source_update.flac"));
    ASSERT_TRUE(trackId.isValid());
    ASSERT_TRUE(targetId.isValid());

    const TrackLink::Type type = TrackLink::Type::Stem;

    ASSERT_TRUE(dao().linkTracks(trackId, targetId, 10.0, type));
    EXPECT_TRUE(dao().updateOffset(trackId, targetId, type, 20.0));

    QList<TrackLink> links = dao().getLinksForTrack(trackId);
    ASSERT_EQ(links.size(), 1);
    EXPECT_DOUBLE_EQ(links[0].offsetMs, 20.0);
}

TEST_F(TrackLinkDaoTest, Unlink) {
    TrackId trackId = addTrack(QStringLiteral("stem_unlink.stem.mp4"));
    TrackId targetId = addTrack(QStringLiteral("source_unlink.flac"));
    ASSERT_TRUE(trackId.isValid());
    ASSERT_TRUE(targetId.isValid());

    const TrackLink::Type type = TrackLink::Type::Stem;

    ASSERT_TRUE(dao().linkTracks(trackId, targetId, 10.0, type));
    EXPECT_TRUE(dao().unlinkTracks(trackId, targetId, type));

    QList<TrackLink> links = dao().getLinksForTrack(trackId);
    EXPECT_EQ(links.size(), 0);
}

TEST_F(TrackLinkDaoTest, DuplicateLinkUpsert) {
    TrackId trackId = addTrack(QStringLiteral("stem_upsert.stem.mp4"));
    TrackId targetId = addTrack(QStringLiteral("source_upsert.flac"));
    ASSERT_TRUE(trackId.isValid());
    ASSERT_TRUE(targetId.isValid());

    const TrackLink::Type type = TrackLink::Type::Stem;

    EXPECT_TRUE(dao().linkTracks(trackId, targetId, 10.0, type));
    // Upsert: linking again should update the offset.
    EXPECT_TRUE(dao().linkTracks(trackId, targetId, 30.0, type));

    QList<TrackLink> links = dao().getLinksForTrack(trackId);
    ASSERT_EQ(links.size(), 1);
    EXPECT_DOUBLE_EQ(links[0].offsetMs, 30.0);
}
