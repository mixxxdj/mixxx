#include "library/playercoverartwatcher.h"

#include <gtest/gtest.h>

#include <QImage>

#include "library/coverart.h"

namespace {

using mixxx::PlayerCoverArtWatcher;

CoverInfo coverInfo(const QString& trackLocation, const QImage& image) {
    CoverInfoRelative cover;
    if (!image.isNull()) {
        cover.type = CoverInfo::METADATA;
        cover.setImageDigest(image);
    }
    return CoverInfo(cover, trackLocation);
}

QImage image(Qt::GlobalColor color) {
    QImage result(8, 8, QImage::Format_RGB32);
    result.fill(color);
    return result;
}

TEST(PlayerCoverArtWatcherTest, coverOfTheSameTrack) {
    EXPECT_TRUE(PlayerCoverArtWatcher::isCoverOfTrack(
            coverInfo(QStringLiteral("/a.mp3"), image(Qt::red)),
            coverInfo(QStringLiteral("/a.mp3"), image(Qt::red))));
    // A track without a cover.
    EXPECT_TRUE(PlayerCoverArtWatcher::isCoverOfTrack(
            coverInfo(QStringLiteral("/a.mp3"), QImage()),
            coverInfo(QStringLiteral("/a.mp3"), QImage())));
}

TEST(PlayerCoverArtWatcherTest, sameImageOfAnotherTrack) {
    // CoverArtCache answers with the cover info of the track that has
    // requested the image first.
    EXPECT_TRUE(PlayerCoverArtWatcher::isCoverOfTrack(
            coverInfo(QStringLiteral("/album/a.mp3"), image(Qt::red)),
            coverInfo(QStringLiteral("/album/b.mp3"), image(Qt::red))));
}

TEST(PlayerCoverArtWatcherTest, coverOfAnotherTrack) {
    EXPECT_FALSE(PlayerCoverArtWatcher::isCoverOfTrack(
            coverInfo(QStringLiteral("/a.mp3"), image(Qt::red)),
            coverInfo(QStringLiteral("/b.mp3"), image(Qt::blue))));
    EXPECT_FALSE(PlayerCoverArtWatcher::isCoverOfTrack(
            coverInfo(QStringLiteral("/a.mp3"), image(Qt::red)),
            coverInfo(QStringLiteral("/b.mp3"), QImage())));
    // Two tracks without a cover do not share an image.
    EXPECT_FALSE(PlayerCoverArtWatcher::isCoverOfTrack(
            coverInfo(QStringLiteral("/a.mp3"), QImage()),
            coverInfo(QStringLiteral("/b.mp3"), QImage())));
}

} // namespace
