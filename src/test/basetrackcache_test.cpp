#include "library/basetrackcache.h"

#include <gtest/gtest.h>

#include "library/dao/trackschema.h"
#include "test/librarytest.h"
#include "track/track.h"

namespace {

const QString kTrackLocationTest = QStringLiteral("id3-test-data/cover-test-øé~ł€˚-png.mp3");

constexpr auto kTestColor = mixxx::RgbColor(0xFF0000);

} // namespace

class BaseTrackCacheTest : public LibraryTest {
  protected:
    BaseTrackCacheTest()
            : m_pTrackCache(QSharedPointer<BaseTrackCache>::create(
                      internalCollection(),
                      LIBRARY_TABLE,
                      LIBRARYTABLE_ID,
                      QStringList{
                              LIBRARYTABLE_ID,
                              LIBRARYTABLE_COLOR,
                              LIBRARYTABLE_COVERART_COLOR},
                      QStringList{},
                      true)) {
        internalCollection()->connectTrackSource(m_pTrackCache);
        m_pTrackCache->buildIndex();
    }

    ~BaseTrackCacheTest() override {
        internalCollection()->disconnectTrackSource();
    }

    mixxx::RgbColor::optional_t colorFromCache(
            TrackId trackId, ColumnCache::Column column) const {
        return mixxx::RgbColor::fromQVariant(
                m_pTrackCache->data(trackId, m_pTrackCache->fieldIndex(column)));
    }

    const QSharedPointer<BaseTrackCache> m_pTrackCache;
};

// While a track is cached in memory, its values must take precedence
// over the index, which only gets updated after saving the track. This
// includes values that are empty, e.g. a color that has been reset.

TEST_F(BaseTrackCacheTest, resetColorOfCachedTrack) {
    const auto pTrack = getOrAddTrackByLocation(getTestDir().filePath(kTrackLocationTest));
    ASSERT_TRUE(pTrack);

    pTrack->setColor(kTestColor);
    // Saving updates the index, which then holds the color that is about
    // to become stale
    trackCollectionManager()->saveTrack(pTrack);
    ASSERT_EQ(kTestColor,
            colorFromCache(pTrack->getId(), ColumnCache::COLUMN_LIBRARYTABLE_COLOR));

    pTrack->setColor(std::nullopt);
    EXPECT_EQ(std::nullopt,
            colorFromCache(pTrack->getId(), ColumnCache::COLUMN_LIBRARYTABLE_COLOR));
}

TEST_F(BaseTrackCacheTest, resetCoverArtColorOfCachedTrack) {
    const auto pTrack = getOrAddTrackByLocation(getTestDir().filePath(kTrackLocationTest));
    ASSERT_TRUE(pTrack);

    auto coverInfo = pTrack->getCoverInfo();
    coverInfo.color = kTestColor;
    pTrack->setCoverInfo(coverInfo);
    // Saving updates the index, which then holds the color that is about
    // to become stale
    trackCollectionManager()->saveTrack(pTrack);
    ASSERT_EQ(kTestColor,
            colorFromCache(pTrack->getId(), ColumnCache::COLUMN_LIBRARYTABLE_COVERART_COLOR));

    coverInfo.color = std::nullopt;
    pTrack->setCoverInfo(coverInfo);
    EXPECT_EQ(std::nullopt,
            colorFromCache(pTrack->getId(), ColumnCache::COLUMN_LIBRARYTABLE_COVERART_COLOR));
}
