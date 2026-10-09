#include <gtest/gtest.h>

#include <QBrush>
#include <QSharedPointer>
#include <QSignalSpy>
#include <QSqlError>
#include <QSqlQuery>
#include <array>
#include <memory>

#include "library/basetrackcache.h"
#include "library/basetracktablemodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "library/keyhighlightmanager.h"
#include "library/librarytablemodel.h"
#include "library/playlisttablemodel.h"
#include "library/trackmodel.h"
#include "mixer/playerinfo.h"
#include "proto/keys.pb.h"
#include "test/keyhighlighttestcontrols.h"
#include "test/librarytest.h"
#include "track/track.h"
#include "widget/wtracktableview.h"

namespace {

using mixxx::KeyHighlightManager;
using KeyMatch = KeyHighlightManager::KeyMatch;
namespace key = mixxx::track::io::key;

// The colours a model uses until a skin sets them on its WTrackTableView.
constexpr QColor kMatchColor = WTrackTableView::kDefaultKeyHighlightMatchColor;
constexpr QColor kNeighbourColor = WTrackTableView::kDefaultKeyHighlightNeighbourColor;
constexpr QColor kShiftColor = WTrackTableView::kDefaultKeyHighlightShiftColor;
constexpr QColor kPlayedColor = WTrackTableView::kDefaultKeyHighlightPlayedColor;

const QString kTrackSourceView = QStringLiteral("keyhighlight_cache_view");

const std::array<QString, 6> kTrackFiles = {
        QStringLiteral("id3-test-data/cover-test-øé~ł€˚-png.mp3"),
        QStringLiteral("id3-test-data/cover-test-øé~ł€˚-jpg.mp3"),
        QStringLiteral("id3-test-data/cover-test-øé~ł€˚-vbr.mp3"),
        QStringLiteral("id3-test-data/cover-test-øé~ł€˚.flac"),
        QStringLiteral("id3-test-data/cover-test-øé~ł€˚.ogg"),
        QStringLiteral("id3-test-data/cover-test-øé~ł€˚.wav"),
};

/// Counts the track lookups by row. External models add the track to the
/// library on a lookup, so data() must not do any.
class CountingLibraryTableModel : public LibraryTableModel {
  public:
    /// referenceIdInModel stands in for the id doGetTrackId() returns for the
    /// reference track, as an external model's own id would.
    explicit CountingLibraryTableModel(TrackCollectionManager* pTrackCollectionManager,
            TrackId referenceIdInModel = TrackId())
            : LibraryTableModel(nullptr,
                      pTrackCollectionManager,
                      "mixxx.db.model.keyhighlighttest.library"),
              m_referenceIdInModel(referenceIdInModel) {
    }

    TrackPointer getTrack(const QModelIndex& index) const override {
        ++m_trackLookups;
        return LibraryTableModel::getTrack(index);
    }

    TrackId getTrackId(const QModelIndex& index) const override {
        ++m_trackLookups;
        return LibraryTableModel::getTrackId(index);
    }

    int trackLookups() const {
        return m_trackLookups;
    }

  private:
    TrackId doGetTrackId(const TrackPointer& pTrack) const override {
        if (!pTrack) {
            return TrackId();
        }
        return m_referenceIdInModel.isValid() ? m_referenceIdInModel : pTrack->getId();
    }

    const TrackId m_referenceIdInModel;
    mutable int m_trackLookups = 0;
};

/// Tests BaseTrackTableModel::data() for the harmonic highlighter's Key and
/// BPM cells, on a playlist model backed by a minimal track source.
///
/// Deck 0 is the highlighting reference deck. Its track is stored as G# minor
/// at 90 BPM, and pitched up to A minor at 100 BPM.
class KeyHighlightModelTest : public LibraryTest, protected KeyHighlightTestControls {
  protected:
    void SetUp() override {
        // PlayerInfo binds to the controls on construction, and the manager
        // to PlayerInfo.
        PlayerInfo::destroy();
        PlayerInfo::create();

        QList<TrackId> trackIds;
        for (const auto& trackFile : kTrackFiles) {
            const auto pTrack = getOrAddTrackByLocation(getTestDir().filePath(trackFile));
            ASSERT_TRUE(pTrack);
            trackIds.append(pTrack->getId());
        }
        m_matchId = trackIds.at(0);
        m_neutralId = trackIds.at(1);
        m_halfTempoId = trackIds.at(2);
        m_playedId = trackIds.at(3);
        m_missingId = trackIds.at(4);
        m_referenceId = trackIds.at(5);

        // Set the fields in the database directly. The track source doesn't
        // cache Track objects, so the model reads these values.
        setTrackFields(m_matchId, 101.0, key::A_MINOR, false);
        // A distant key and tempo, but a red track colour.
        setTrackFields(m_neutralId, 75.0, key::C_MINOR, false, 0xFF0000);
        setTrackFields(m_halfTempoId, 50.0, key::INVALID, false);
        setTrackFields(m_playedId, 100.0, key::A_MINOR, true);
        setTrackFields(m_missingId, 100.0, key::A_MINOR, true);
        setTrackMissing(m_missingId);
        setTrackFields(m_referenceId, 90.0, key::G_SHARP_MINOR, false);

        connectTrackSource();

        PlaylistDAO& playlistDao = internalCollection()->getPlaylistDAO();
        const int playlistId = playlistDao.createPlaylist(
                QStringLiteral("KeyHighlightModelTest"));
        ASSERT_GE(playlistId, 0);
        ASSERT_TRUE(playlistDao.appendTracksToPlaylist(trackIds, playlistId));

        setDeckKeys(0, key::G_SHARP_MINOR, key::A_MINOR);
        setDeckBpm(0, 100.0);
        m_pManager = KeyHighlightManager::createInstance();
        setHighlight(0, true);
        loadTrack(0, m_referenceId);
        ASSERT_TRUE(m_pManager->isKeyActive());
        ASSERT_TRUE(m_pManager->isBpmActive());

        // The model connects to the manager on construction.
        BaseTrackTableModel::setApplyPlayedTrackColor(true);
        m_pModel = std::make_unique<PlaylistTableModel>(nullptr,
                trackCollectionManager(),
                "mixxx.db.model.keyhighlighttest",
                /*keepHiddenTracks*/ true);
        m_pModel->selectPlaylist(playlistId);
        m_pModel->select();
        ASSERT_EQ(static_cast<int>(kTrackFiles.size()), m_pModel->rowCount());

        // Without these columns the "not tinted" expectations below would pass vacuously.
        m_keyColumn = m_pModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY);
        m_bpmColumn = m_pModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM);
        m_titleColumn = m_pModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE);
        ASSERT_GE(m_keyColumn, 0);
        ASSERT_GE(m_bpmColumn, 0);
        ASSERT_GE(m_titleColumn, 0);
        ASSERT_GE(m_pModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID), 0);
        ASSERT_GE(m_pModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED), 0);
        ASSERT_GE(m_pModel->fieldIndex(
                          ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED),
                0);
    }

    void TearDown() override {
        m_pModel.reset();
        BaseTrackTableModel::setApplyPlayedTrackColor(
                BaseTrackTableModel::kApplyPlayedTrackColorDefault);
        if (KeyHighlightManager::isCreated()) {
            KeyHighlightManager::destroy();
        }
        PlayerInfo::destroy();
    }

    void setTrackFields(TrackId trackId,
            double bpm,
            key::ChromaticKey trackKey,
            bool played,
            const QVariant& color = QVariant()) {
        QSqlQuery query(internalCollection()->database());
        query.prepare(QStringLiteral(
                "UPDATE library SET bpm=:bpm, key_id=:key_id, played=:played, "
                "color=:color WHERE id=:id"));
        query.bindValue(QStringLiteral(":bpm"), bpm);
        query.bindValue(QStringLiteral(":key_id"),
                trackKey == key::INVALID ? QVariant()
                                         : QVariant(static_cast<int>(trackKey)));
        query.bindValue(QStringLiteral(":played"), played ? 1 : 0);
        query.bindValue(QStringLiteral(":color"), color);
        query.bindValue(QStringLiteral(":id"), trackId.toVariant());
        EXPECT_TRUE(query.exec()) << query.lastError().text().toStdString();
    }

    void setTrackMissing(TrackId trackId) {
        QSqlQuery query(internalCollection()->database());
        query.prepare(QStringLiteral(
                "UPDATE track_locations SET fs_deleted=1 "
                "WHERE id=(SELECT location FROM library WHERE id=:id)"));
        query.bindValue(QStringLiteral(":id"), trackId.toVariant());
        EXPECT_TRUE(query.exec()) << query.lastError().text().toStdString();
    }

    // A non-caching subset of MixxxLibraryFeature's track source: the model
    // takes the library columns from it.
    void connectTrackSource() {
        const QStringList columns = {
                LIBRARYTABLE_ID,
                LIBRARYTABLE_PLAYED,
                LIBRARYTABLE_TIMESPLAYED,
                LIBRARYTABLE_ARTIST,
                LIBRARYTABLE_TITLE,
                LIBRARYTABLE_KEY,
                LIBRARYTABLE_KEY_ID,
                LIBRARYTABLE_BPM,
                LIBRARYTABLE_BPM_LOCK,
                LIBRARYTABLE_COLOR,
                TRACKLOCATIONSTABLE_LOCATION,
                TRACKLOCATIONSTABLE_FSDELETED,
                LIBRARYTABLE_MIXXXDELETED,
        };
        QStringList qualifiedColumns;
        for (const auto& column : columns) {
            qualifiedColumns.append(mixxx::trackschema::tableForColumn(column) +
                    QLatin1Char('.') + column);
        }
        QSqlQuery query(internalCollection()->database());
        ASSERT_TRUE(query.exec(QStringLiteral(
                "CREATE TEMPORARY VIEW %1 AS SELECT %2 FROM library "
                "INNER JOIN track_locations "
                "ON library.location = track_locations.id")
                        .arg(kTrackSourceView,
                                qualifiedColumns.join(','))))
                << query.lastError().text().toStdString();
        internalCollection()->connectTrackSource(
                QSharedPointer<BaseTrackCache>::create(internalCollection(),
                        kTrackSourceView,
                        LIBRARYTABLE_ID,
                        columns,
                        QStringList{LIBRARYTABLE_TITLE},
                        /*isCaching*/ false));
    }

    QModelIndex cell(TrackId trackId, int column) const {
        const QVector<int> rows = m_pModel->getTrackRows(trackId);
        EXPECT_EQ(1, rows.size());
        return m_pModel->index(rows.value(0, -1), column);
    }

    // The colour of a QBrush role, or an invalid QColor if there is none.
    static QColor brushColor(const QModelIndex& index, int role) {
        const QVariant value = index.data(role);
        return value.isValid() ? value.value<QBrush>().color() : QColor();
    }

    QColor background(TrackId trackId, int column) const {
        return brushColor(cell(trackId, column), Qt::BackgroundRole);
    }

    QColor foreground(TrackId trackId, int column) const {
        return cell(trackId, column).data(Qt::ForegroundRole).value<QColor>();
    }

    QColor highlightBackground(TrackId trackId, int column) const {
        return brushColor(cell(trackId, column), TrackModel::kHighlightBackgroundRole);
    }

    KeyHighlightManager* m_pManager = nullptr;
    std::unique_ptr<PlaylistTableModel> m_pModel;
    int m_keyColumn = -1;
    int m_bpmColumn = -1;
    int m_titleColumn = -1;
    TrackId m_matchId;
    TrackId m_neutralId;
    TrackId m_halfTempoId;
    TrackId m_playedId;
    TrackId m_missingId;
    TrackId m_referenceId;
};

TEST_F(KeyHighlightModelTest, MatchesAreTinted) {
    for (const int column : {m_keyColumn, m_bpmColumn}) {
        EXPECT_EQ(kMatchColor, background(m_matchId, column)) << column;
        EXPECT_EQ(kMatchColor, highlightBackground(m_matchId, column)) << column;
        // Light text on the dark green.
        EXPECT_EQ(QColor(Qt::white), foreground(m_matchId, column)) << column;
    }
    // Half tempo gets the light green, with dark text.
    EXPECT_EQ(kNeighbourColor, background(m_halfTempoId, m_bpmColumn));
    EXPECT_EQ(kNeighbourColor, highlightBackground(m_halfTempoId, m_bpmColumn));
    EXPECT_EQ(QColor(Qt::black), foreground(m_halfTempoId, m_bpmColumn));
}

TEST_F(KeyHighlightModelTest, NeutralCellsHideTrackColor) {
    ASSERT_EQ(KeyMatch::None, m_pManager->keyMatch(key::C_MINOR));
    ASSERT_EQ(KeyHighlightManager::BpmMatch::None, m_pManager->bpmMatch(75.0));
    // No match: the Key and BPM cells stay neutral instead of showing the red
    // track colour, and keep the normal text colour.
    for (const int column : {m_keyColumn, m_bpmColumn}) {
        EXPECT_FALSE(background(m_neutralId, column).isValid()) << column;
        EXPECT_FALSE(highlightBackground(m_neutralId, column).isValid()) << column;
        EXPECT_FALSE(cell(m_neutralId, column).data(Qt::ForegroundRole).isValid())
                << column;
    }
    // Other cells still show the track colour.
    EXPECT_EQ(QColor(0xFF, 0x00, 0x00).rgb(),
            background(m_neutralId, m_titleColumn).rgb());
    // A track without a key is never tinted in the Key cell.
    EXPECT_FALSE(background(m_halfTempoId, m_keyColumn).isValid());
}

TEST_F(KeyHighlightModelTest, PlayedBeatsMatch) {
    for (const int column : {m_keyColumn, m_bpmColumn}) {
        EXPECT_EQ(kPlayedColor, background(m_playedId, column)) << column;
        EXPECT_EQ(kPlayedColor, highlightBackground(m_playedId, column)) << column;
        EXPECT_EQ(QColor(Qt::white), foreground(m_playedId, column)) << column;
    }
}

TEST_F(KeyHighlightModelTest, MissingBeatsPlayed) {
    // A missing file isn't tinted at all and keeps the "missing" text colour.
    for (const int column : {m_keyColumn, m_bpmColumn}) {
        EXPECT_FALSE(background(m_missingId, column).isValid()) << column;
        EXPECT_FALSE(highlightBackground(m_missingId, column).isValid()) << column;
        EXPECT_EQ(QColor(WTrackTableView::kDefaultTrackMissingColor),
                foreground(m_missingId, column))
                << column;
    }
}

TEST_F(KeyHighlightModelTest, PitchedReferenceTrack) {
    // Guard the premise: by its stored key and BPM, the reference track
    // wouldn't match its own playing key and tempo.
    ASSERT_EQ(KeyMatch::ShiftUp, m_pManager->keyMatch(key::G_SHARP_MINOR));
    ASSERT_EQ(KeyHighlightManager::BpmMatch::None, m_pManager->bpmMatch(90.0));

    // Both cells match the playing deck, without a shift hint.
    EXPECT_EQ(kMatchColor, background(m_referenceId, m_keyColumn));
    EXPECT_EQ(static_cast<int>(KeyMatch::Perfect),
            cell(m_referenceId, m_keyColumn).data(TrackModel::kKeyMatchRole).toInt());
    EXPECT_EQ(kMatchColor, background(m_referenceId, m_bpmColumn));

    // The cell shows the stored key, and the playing key separately.
    const QModelIndex keyCell = cell(m_referenceId, m_keyColumn);
    EXPECT_EQ(KeyUtils::keyToString(key::G_SHARP_MINOR), keyCell.data().toString());
    EXPECT_EQ(KeyUtils::keyToString(key::A_MINOR),
            keyCell.data(TrackModel::kPlayingKeyRole).toString());
    EXPECT_FALSE(cell(m_matchId, m_keyColumn).data(TrackModel::kPlayingKeyRole).isValid());

    // Once ejected, the track is classified by its stored key and BPM.
    ejectTrack(0);
    EXPECT_EQ(kShiftColor, background(m_referenceId, m_keyColumn));
    EXPECT_FALSE(background(m_referenceId, m_bpmColumn).isValid());
    EXPECT_FALSE(keyCell.data(TrackModel::kPlayingKeyRole).isValid());
}

TEST_F(KeyHighlightModelTest, UnpitchedReferenceTrackHasNoPlayingKey) {
    setDeckKey(0, key::G_SHARP_MINOR);
    const QModelIndex keyCell = cell(m_referenceId, m_keyColumn);
    EXPECT_EQ(kMatchColor, brushColor(keyCell, Qt::BackgroundRole));
    EXPECT_FALSE(keyCell.data(TrackModel::kPlayingKeyRole).isValid());
}

TEST_F(KeyHighlightModelTest, DataDoesNotLookUpTracks) {
    CountingLibraryTableModel model(trackCollectionManager());
    model.select();
    // The library view leaves out the missing track.
    ASSERT_EQ(static_cast<int>(kTrackFiles.size()) - 1, model.rowCount());
    const QList<int> roles = {Qt::DisplayRole,
            Qt::EditRole,
            Qt::ToolTipRole,
            Qt::DecorationRole,
            Qt::TextAlignmentRole,
            Qt::CheckStateRole,
            Qt::BackgroundRole,
            Qt::ForegroundRole,
            TrackModel::kDataExportRole,
            TrackModel::kTuningFrequencyRole,
            TrackModel::kKeyMatchRole,
            TrackModel::kHighlightBackgroundRole,
            TrackModel::kPlayingKeyRole};
    const auto readAllRoles = [&model, &roles]() {
        for (int row = 0; row < model.rowCount(); ++row) {
            for (int column = 0; column < model.columnCount(); ++column) {
                const QModelIndex index = model.index(row, column);
                for (const int role : roles) {
                    index.data(role);
                }
            }
        }
    };
    readAllRoles();
    setHighlight(0, false);
    readAllRoles();
    EXPECT_EQ(0, model.trackLookups());
}

TEST_F(KeyHighlightModelTest, ReferenceRowInModelIdSpace) {
    // Like an external model, whose ids aren't library ids, the model maps the
    // reference track to the neutral track's row.
    CountingLibraryTableModel model(trackCollectionManager(), m_neutralId);
    model.select();
    const int keyColumn = model.fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY);
    ASSERT_GE(keyColumn, 0);
    const auto keyCell = [&model, keyColumn](TrackId trackId) {
        const QVector<int> rows = model.getTrackRows(trackId);
        EXPECT_EQ(1, rows.size());
        return model.index(rows.value(0, -1), keyColumn);
    };

    EXPECT_EQ(static_cast<int>(KeyMatch::Perfect),
            keyCell(m_neutralId).data(TrackModel::kKeyMatchRole).toInt());
    EXPECT_EQ(KeyUtils::keyToString(key::A_MINOR),
            keyCell(m_neutralId).data(TrackModel::kPlayingKeyRole).toString());
    // The library id's row is classified by its stored key.
    EXPECT_EQ(static_cast<int>(KeyMatch::ShiftUp),
            keyCell(m_referenceId).data(TrackModel::kKeyMatchRole).toInt());
    EXPECT_FALSE(keyCell(m_referenceId).data(TrackModel::kPlayingKeyRole).isValid());
}

TEST_F(KeyHighlightModelTest, BpmChangeRepaintsBpmColumnOnly) {
    QSignalSpy spy(m_pModel.get(), &QAbstractItemModel::dataChanged);
    // 101 BPM is outside 85 +/- 6 %.
    setDeckBpm(0, 85.0);
    ASSERT_EQ(1, spy.count());
    const auto topLeft = spy.at(0).at(0).value<QModelIndex>();
    const auto bottomRight = spy.at(0).at(1).value<QModelIndex>();
    EXPECT_EQ(m_bpmColumn, topLeft.column());
    EXPECT_EQ(m_bpmColumn, bottomRight.column());
    EXPECT_EQ(0, topLeft.row());
    EXPECT_EQ(m_pModel->rowCount() - 1, bottomRight.row());

    EXPECT_FALSE(background(m_matchId, m_bpmColumn).isValid());
    // The key cell and the reference track's BPM cell don't change.
    EXPECT_EQ(kMatchColor, background(m_matchId, m_keyColumn));
    EXPECT_EQ(kMatchColor, background(m_referenceId, m_bpmColumn));
}

} // namespace
