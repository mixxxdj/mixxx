#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSet>
#include <QSignalSpy>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QTableView>
#include <QTemporaryDir>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "library/basetrackcache.h"
#include "library/columncache.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "library/librarytablemodel.h"
#include "library/mixxxlibraryfeature.h"
#include "library/playlisttablemodel.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playerinfo.h"
#include "test/librarytest.h"
#include "track/track.h"

using ::testing::UnorderedElementsAre;

namespace {

const char* const kLibraryModelNamespace = "mixxx.db.model.library";
const char* const kPlaylistModelNamespace = "mixxx.db.model.playlist";

} // namespace

// Regression tests for the full model refresh in
// BaseSqlTableModel::select(), which emits a model reset instead of
// incremental remove/insert signals. These models back both the legacy
// QWidget library (via ProxyTrackModel/QSortFilterProxyModel and
// WTrackTableView) and the QML library (via QmlLibraryTrackListModel), so the
// contract and the proxy integration are verified here.
class BaseSqlTableModelTest : public LibraryTest {
  protected:
    BaseSqlTableModelTest()
            : m_crossfader(ConfigKey("[Master]", "crossfader"), -1.0, 1.0),
              m_numDecks(ConfigKey("[App]", "num_decks")),
              m_numSamplers(ConfigKey("[App]", "num_samplers")),
              m_numPreviewDecks(ConfigKey("[App]", "num_preview_decks")) {
    }

    void SetUp() override {
        ASSERT_TRUE(m_tempDir.isValid());
        // BaseTrackTableModel requires PlayerInfo, which in turn expects the
        // mixer/app controls above to exist.
        PlayerInfo::create();
        // Reuse the production track cache so the model behaves exactly like
        // in the application (artist/title come from the track source).
        // createLibraryTrackSource() already connects the fresh cache to the
        // collection; keeping the returned shared pointer alive per test and
        // detaching it in TearDown keeps the tests hermetic (the collection
        // refuses a second connectTrackSource()).
        m_pTrackSource = MixxxLibraryFeature::createLibraryTrackSource(
                internalCollection());
    }

    void TearDown() override {
        internalCollection()->disconnectTrackSource();
        m_pTrackSource.clear();
        PlayerInfo::destroy();
    }

    TrackId addTrack(const QString& fileName,
            const QString& artist,
            const QString& title) {
        TrackPointer pTrack = getOrAddTrackByLocation(m_tempDir.filePath(fileName));
        EXPECT_TRUE(pTrack);
        if (!pTrack) {
            return TrackId();
        }
        pTrack->setArtist(artist);
        pTrack->setTitle(title);
        trackCollectionManager()->saveTrack(pTrack);
        EXPECT_TRUE(pTrack->getId().isValid());
        return pTrack->getId();
    }

    static int artistColumn(LibraryTableModel* pModel) {
        return pModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST);
    }

    static int titleColumn(LibraryTableModel* pModel) {
        return pModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE);
    }

    // Minimal QAbstractItemModel contract check that stays within the
    // assumptions of Mixxx' flat table models (no valid child indexes).
    static void expectConsistentModel(QAbstractItemModel* pModel) {
        ASSERT_GE(pModel->rowCount(), 0);
        ASSERT_GE(pModel->columnCount(), 0);
        for (int row = 0; row < pModel->rowCount(); ++row) {
            for (int column = 0; column < pModel->columnCount(); ++column) {
                const QModelIndex index = pModel->index(row, column);
                ASSERT_TRUE(index.isValid());
                EXPECT_EQ(row, index.row());
                EXPECT_EQ(column, index.column());
                EXPECT_FALSE(pModel->parent(index).isValid());
            }
        }
    }

    ControlPotmeter m_crossfader;
    ControlObject m_numDecks;
    ControlObject m_numSamplers;
    ControlObject m_numPreviewDecks;
    QTemporaryDir m_tempDir;
    // Keeps the shared track cache alive for the duration of the test (the
    // collection only holds it as a plain shared pointer copy).
    QSharedPointer<BaseTrackCache> m_pTrackSource;
};

// Pins the contract of MixxxLibraryFeature::createLibraryTrackSource(): the
// temporary 'library_cache_view' is queryable with the production column set,
// and the connected source is what the library models read artist/title from.
TEST_F(BaseSqlTableModelTest, CreateLibraryTrackSourceBuildsCacheView) {
    ASSERT_TRUE(m_pTrackSource);

    QSqlQuery query(internalCollection()->database());
    ASSERT_TRUE(query.exec(QStringLiteral(
            "SELECT %1, %2, %3, %4, %5 FROM library_cache_view LIMIT 0")
                    .arg(LIBRARYTABLE_ID,
                            LIBRARYTABLE_ARTIST,
                            LIBRARYTABLE_TITLE,
                            LIBRARYTABLE_ALBUM,
                            TRACKLOCATIONSTABLE_LOCATION)));

    // The connected track source feeds the models: data not present in the
    // SQL result (artist/title via the track source) resolves correctly.
    LibraryTableModel model(nullptr, trackCollectionManager(), kLibraryModelNamespace);
    addTrack("a.mp3", "Artist A", "Title A");
    model.select();
    ASSERT_EQ(1, model.rowCount());
    const int artistCol = artistColumn(&model);
    const int titleCol = titleColumn(&model);
    ASSERT_GE(artistCol, 0);
    ASSERT_GE(titleCol, 0);
    EXPECT_QSTRING_EQ("Artist A", model.data(model.index(0, artistCol)).toString());
    EXPECT_QSTRING_EQ("Title A", model.data(model.index(0, titleCol)).toString());
}

TEST_F(BaseSqlTableModelTest, SelectEmitsModelResetInsteadOfRowChanges) {
    LibraryTableModel model(nullptr, trackCollectionManager(), kLibraryModelNamespace);

    addTrack("a.mp3", "Artist A", "Title A");
    addTrack("b.mp3", "Artist B", "Title B");

    QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
    QSignalSpy insertedSpy(&model, &QAbstractItemModel::rowsInserted);
    QSignalSpy removedSpy(&model, &QAbstractItemModel::rowsRemoved);

    model.select();

    EXPECT_EQ(2, model.rowCount());
    EXPECT_EQ(1, resetSpy.count());
    EXPECT_EQ(0, insertedSpy.count());
    EXPECT_EQ(0, removedSpy.count());
}

TEST_F(BaseSqlTableModelTest, SelectKeepsRowsAndData) {
    LibraryTableModel model(nullptr, trackCollectionManager(), kLibraryModelNamespace);

    addTrack("a.mp3", "Artist A", "Title A");
    addTrack("b.mp3", "Artist B", "Title B");
    model.select();
    ASSERT_EQ(2, model.rowCount());
    expectConsistentModel(&model);

    const int artistCol = artistColumn(&model);
    const int titleCol = titleColumn(&model);
    ASSERT_GE(artistCol, 0);
    ASSERT_GE(titleCol, 0);

    QSet<QString> artists;
    for (int row = 0; row < model.rowCount(); ++row) {
        artists.insert(model.data(model.index(row, artistCol)).toString());
        EXPECT_FALSE(model.data(model.index(row, titleCol)).toString().isEmpty());
    }
    EXPECT_THAT(artists, UnorderedElementsAre(QString("Artist A"), QString("Artist B")));
}

TEST_F(BaseSqlTableModelTest, SearchFiltersAndEmitsModelReset) {
    LibraryTableModel model(nullptr, trackCollectionManager(), kLibraryModelNamespace);
    addTrack("a.mp3", "Artist A", "Alpha Song");
    addTrack("b.mp3", "Artist B", "Beta Song");
    model.select();
    ASSERT_EQ(2, model.rowCount());

    QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
    model.search("Alpha");

    EXPECT_EQ(1, resetSpy.count());
    ASSERT_EQ(1, model.rowCount());
    expectConsistentModel(&model);
    const int titleCol = titleColumn(&model);
    ASSERT_GE(titleCol, 0);
    EXPECT_QSTRING_EQ("Alpha Song", model.data(model.index(0, titleCol)).toString());
}

TEST_F(BaseSqlTableModelTest, SortOrdersAndEmitsModelReset) {
    LibraryTableModel model(nullptr, trackCollectionManager(), kLibraryModelNamespace);
    addTrack("a.mp3", "Zeta", "Title");
    addTrack("b.mp3", "Alpha", "Title");
    model.select();
    ASSERT_EQ(2, model.rowCount());

    const int artistCol = artistColumn(&model);
    ASSERT_GE(artistCol, 0);

    QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
    model.sort(artistCol, Qt::AscendingOrder);

    EXPECT_EQ(1, resetSpy.count());
    ASSERT_EQ(2, model.rowCount());
    expectConsistentModel(&model);
    EXPECT_QSTRING_EQ("Alpha", model.data(model.index(0, artistCol)).toString());
    EXPECT_QSTRING_EQ("Zeta", model.data(model.index(1, artistCol)).toString());
}

TEST_F(BaseSqlTableModelTest, SortFilterProxyModelReflectsSourceSelect) {
    LibraryTableModel model(nullptr, trackCollectionManager(), kLibraryModelNamespace);
    addTrack("a.mp3", "Artist A", "Title A");
    addTrack("b.mp3", "Artist B", "Title B");
    model.select();

    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    ASSERT_EQ(2, proxy.rowCount());
    EXPECT_TRUE(proxy.mapToSource(proxy.index(0, 0)).isValid());

    addTrack("c.mp3", "Artist C", "Title C");
    model.select();

    EXPECT_EQ(3, model.rowCount());
    EXPECT_EQ(3, proxy.rowCount());
    for (int row = 0; row < proxy.rowCount(); ++row) {
        EXPECT_TRUE(proxy.mapToSource(proxy.index(row, 0)).isValid());
    }
}

TEST_F(BaseSqlTableModelTest, ProxyTrackModelReflectsSourceSelect) {
    LibraryTableModel model(nullptr, trackCollectionManager(), kLibraryModelNamespace);
    addTrack("a.mp3", "Artist A", "Title A");
    addTrack("b.mp3", "Artist B", "Title B");
    model.select();

    ProxyTrackModel proxy(&model, true);
    ASSERT_EQ(2, proxy.rowCount());

    addTrack("c.mp3", "Artist C", "Title C");
    model.select();

    EXPECT_EQ(3, model.rowCount());
    EXPECT_EQ(3, proxy.rowCount());
    EXPECT_TRUE(proxy.mapToSource(proxy.index(0, 0)).isValid());
}

TEST_F(BaseSqlTableModelTest, LegacyViewSurvivesSourceReset) {
    LibraryTableModel model(nullptr, trackCollectionManager(), kLibraryModelNamespace);
    addTrack("a.mp3", "Artist A", "Title A");
    addTrack("b.mp3", "Artist B", "Title B");
    model.select();

    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    QTableView view;
    view.setModel(&proxy);
    view.selectRow(0);
    ASSERT_TRUE(view.selectionModel()->hasSelection());

    addTrack("c.mp3", "Artist C", "Title C");
    model.select();

    EXPECT_EQ(3, view.model()->rowCount());
    // The view must remain usable after the source reset.
    view.selectRow(0);
    EXPECT_TRUE(view.selectionModel()->hasSelection());
    EXPECT_TRUE(view.currentIndex().isValid());
}

TEST_F(BaseSqlTableModelTest, PlaylistSelectEmitsModelReset) {
    const int playlistId = internalCollection()->getPlaylistDAO().createPlaylist(
            "TestPlaylist", PlaylistDAO::PLHT_NOT_HIDDEN);
    ASSERT_GE(playlistId, 0);

    const TrackId trackA = addTrack("a.mp3", "Artist A", "Title A");
    const TrackId trackB = addTrack("b.mp3", "Artist B", "Title B");

    PlaylistTableModel model(nullptr, trackCollectionManager(), kPlaylistModelNamespace);
    model.selectPlaylist(playlistId);
    ASSERT_TRUE(model.appendTrack(trackA));
    ASSERT_TRUE(model.appendTrack(trackB));
    model.select();
    ASSERT_EQ(2, model.rowCount());
    expectConsistentModel(&model);

    QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
    QSignalSpy insertedSpy(&model, &QAbstractItemModel::rowsInserted);
    QSignalSpy removedSpy(&model, &QAbstractItemModel::rowsRemoved);

    model.select();

    EXPECT_EQ(2, model.rowCount());
    EXPECT_EQ(1, resetSpy.count());
    EXPECT_EQ(0, insertedSpy.count());
    EXPECT_EQ(0, removedSpy.count());
}
