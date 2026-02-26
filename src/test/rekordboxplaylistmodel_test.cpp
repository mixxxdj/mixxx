#include <gtest/gtest.h>

#include <QSqlQuery>

#include "library/basetrackcache.h"
#include "library/columncache.h"
#include "library/coverart.h"
#include "library/dao/trackschema.h"
#include "library/rekordbox/rekordboxfeature.h"
#include "library/trackmodel.h"
#include "test/librarytest.h"

namespace {

const QString kLibraryTable = QStringLiteral("rekordbox_library");
const QString kLibraryView = QStringLiteral("rekordbox_library_view");
const QString kPlaylistsTable = QStringLiteral("rekordbox_playlists");
const QString kPlaylistTracksTable = QStringLiteral("rekordbox_playlist_tracks");

void createRekordboxTables(QSqlDatabase& db) {
    QSqlQuery q(db);
    ASSERT_TRUE(q.exec(
            "CREATE TABLE " + kLibraryTable +
            " (id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  rb_id INTEGER, artist TEXT, title TEXT, album TEXT, year INTEGER,"
            "  genre TEXT, tracknumber TEXT, location TEXT UNIQUE, comment TEXT,"
            "  duration INTEGER, bitrate TEXT, bpm FLOAT, key TEXT, key_id INTEGER,"
            "  tuning_frequency_hz REAL, rating INTEGER, analyze_path TEXT UNIQUE,"
            "  device TEXT, color INTEGER,"
            "  coverart_type INTEGER, coverart_source INTEGER, coverart_location TEXT,"
            "  coverart_color INTEGER, coverart_digest BLOB, coverart_hash INTEGER)"));
    ASSERT_TRUE(q.exec(
            "CREATE TEMPORARY VIEW IF NOT EXISTS " + kLibraryView +
            " AS SELECT *, coverart_digest AS coverart FROM " + kLibraryTable));
    ASSERT_TRUE(q.exec(
            "CREATE TABLE " + kPlaylistsTable +
            " (id INTEGER PRIMARY KEY, name TEXT UNIQUE)"));
    ASSERT_TRUE(q.exec(
            "CREATE TABLE " + kPlaylistTracksTable +
            " (id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  playlist_id INTEGER REFERENCES " + kPlaylistsTable + "(id),"
            "  track_id INTEGER REFERENCES " + kLibraryTable + "(id),"
            "  position INTEGER)"));
}

QSharedPointer<BaseTrackCache> makeTrackSource(TrackCollection* pCollection) {
    const QStringList columns = {
            LIBRARYTABLE_ID,
            LIBRARYTABLE_ARTIST,
            LIBRARYTABLE_TITLE,
            TRACKLOCATIONSTABLE_LOCATION,
            LIBRARYTABLE_COVERART_SOURCE,
            LIBRARYTABLE_COVERART_TYPE,
            LIBRARYTABLE_COVERART_LOCATION,
            LIBRARYTABLE_COVERART_COLOR,
            LIBRARYTABLE_COVERART_DIGEST,
            LIBRARYTABLE_COVERART_HASH,
            LIBRARYTABLE_COVERART,
    };
    return QSharedPointer<BaseTrackCache>::create(
            pCollection,
            kLibraryView,
            LIBRARYTABLE_ID,
            columns,
            QStringList{LIBRARYTABLE_ARTIST, LIBRARYTABLE_TITLE},
            false);
}

} // namespace

class RekordboxPlaylistModelTest : public LibraryTest {
  protected:
    void SetUp() override {
        auto db = dbConnection();
        createRekordboxTables(db);
        m_pTrackSource = makeTrackSource(internalCollection());
        m_pModel = std::make_unique<RekordboxPlaylistModel>(
                nullptr, trackCollectionManager(), m_pTrackSource);
    }

    QSharedPointer<BaseTrackCache> m_pTrackSource;
    std::unique_ptr<RekordboxPlaylistModel> m_pModel;
};

TEST_F(RekordboxPlaylistModelTest, LibraryViewProvidesCoverartAlias) {
    auto db = dbConnection();
    QSqlQuery q(db);

    const QByteArray kDigest = QByteArray::fromHex("deadbeef");
    ASSERT_TRUE(q.prepare(
            "INSERT INTO " + kLibraryTable +
            " (location, coverart_digest) VALUES (:loc, :digest)"));
    q.bindValue(":loc", "/tmp/test.mp3");
    q.bindValue(":digest", kDigest);
    ASSERT_TRUE(q.exec());

    ASSERT_TRUE(q.exec("SELECT coverart, coverart_digest FROM " + kLibraryView));
    ASSERT_TRUE(q.next());
    EXPECT_EQ(kDigest, q.value(0).toByteArray());
    EXPECT_EQ(kDigest, q.value(1).toByteArray());
    EXPECT_EQ(q.value(0).toByteArray(), q.value(1).toByteArray());
}

TEST_F(RekordboxPlaylistModelTest, GetCapabilitiesIncludesEditMetadata) {
    const TrackModel::Capabilities caps = m_pModel->getCapabilities();
    EXPECT_TRUE(caps.testFlag(TrackModel::Capability::EditMetadata));
}

TEST_F(RekordboxPlaylistModelTest, GetCapabilitiesPreservesBaseCapabilities) {
    const TrackModel::Capabilities caps = m_pModel->getCapabilities();
    EXPECT_TRUE(caps.testFlag(TrackModel::Capability::AddToTrackSet));
    EXPECT_TRUE(caps.testFlag(TrackModel::Capability::AddToAutoDJ));
    EXPECT_TRUE(caps.testFlag(TrackModel::Capability::LoadToDeck));
    EXPECT_TRUE(caps.testFlag(TrackModel::Capability::Sorting));
}

TEST_F(RekordboxPlaylistModelTest, CoverartColumnHasValidFieldIndex) {
    auto db = dbConnection();
    QSqlQuery q(db);

    ASSERT_TRUE(q.exec(
            "INSERT INTO " + kPlaylistsTable + " (name) VALUES ('TestPlaylist')"));

    m_pModel->setPlaylist("TestPlaylist");

    const int coverartFieldIndex =
            m_pModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART);
    EXPECT_GE(coverartFieldIndex, 0)
            << "COLUMN_LIBRARYTABLE_COVERART must map to a valid column index "
               "so that CoverArtDelegate is created for the thumbnail column";
}
