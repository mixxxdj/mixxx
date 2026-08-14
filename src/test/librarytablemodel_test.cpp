#include "library/librarytablemodel.h"

#include <gtest/gtest.h>

#include <memory>

#include "control/controlobject.h"
#include "library/columncache.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "test/librarytest.h"
#include "track/track.h"

namespace {

const QString kTrackLocationTest = QStringLiteral("id3-test-data/cover-test-png.mp3");

} // namespace

class LibraryTableModelTest : public LibraryTest {
  protected:
    void SetUp() override {
        m_pCrossfader = std::make_unique<ControlObject>(ConfigKey("[Master]", "crossfader"));
        m_pNumDecks = std::make_unique<ControlObject>(
                ConfigKey("[App]", "num_decks"),
                true);
        m_pNumSamplers = std::make_unique<ControlObject>(
                ConfigKey("[App]", "num_samplers"),
                true);
        m_pNumPreviewDecks = std::make_unique<ControlObject>(
                ConfigKey("[App]", "num_preview_decks"),
                true);
        PlayerInfo::create();
        m_pLibraryTableModel = std::make_unique<LibraryTableModel>(
                nullptr,
                trackCollectionManager(),
                "LibraryTableModelTest");
    }

    void TearDown() override {
        m_pLibraryTableModel.reset();
        PlayerInfo::destroy();
        m_pNumPreviewDecks.reset();
        m_pNumSamplers.reset();
        m_pNumDecks.reset();
        m_pCrossfader.reset();
    }

    TrackPointer addTrackToModel(const QString& trackLocation) {
        const auto pTrack = getOrAddTrackByLocation(getTestDir().filePath(trackLocation));
        EXPECT_NE(nullptr, pTrack);
        if (pTrack) {
            m_pLibraryTableModel->select();
        }
        return pTrack;
    }

    QString loadedDeckDisplayText(const TrackPointer& pTrack) const {
        EXPECT_NE(nullptr, pTrack);
        if (!pTrack) {
            return QString();
        }

        const TrackId trackId = pTrack->getId();
        EXPECT_TRUE(trackId.isValid());
        if (!trackId.isValid()) {
            return QString();
        }

        const auto rows = m_pLibraryTableModel->getTrackRows(trackId);
        EXPECT_EQ(1, rows.size());
        if (rows.size() != 1) {
            return QString();
        }

        const int loadedDeckColumn =
                m_pLibraryTableModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_LOADED_DECK);
        EXPECT_GE(loadedDeckColumn, 0);
        if (loadedDeckColumn < 0) {
            return QString();
        }

        return m_pLibraryTableModel
                ->data(m_pLibraryTableModel->index(rows.first(), loadedDeckColumn), Qt::DisplayRole)
                .toString();
    }

    std::unique_ptr<ControlObject> m_pCrossfader;
    std::unique_ptr<ControlObject> m_pNumDecks;
    std::unique_ptr<ControlObject> m_pNumSamplers;
    std::unique_ptr<ControlObject> m_pNumPreviewDecks;
    std::unique_ptr<LibraryTableModel> m_pLibraryTableModel;
};

TEST_F(LibraryTableModelTest, LoadedDeckColumnDisplay) {
    const int loadedDeckColumn =
            m_pLibraryTableModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_LOADED_DECK);
    EXPECT_GE(loadedDeckColumn, 0);

    const auto pTrack = addTrackToModel(kTrackLocationTest);
    ASSERT_NE(nullptr, pTrack);

    EXPECT_QSTRING_EQ(QString(), loadedDeckDisplayText(pTrack));

    PlayerInfo::instance().setTrackInfo(PlayerManager::groupForDeck(0), pTrack);
    EXPECT_QSTRING_EQ(QStringLiteral("1"), loadedDeckDisplayText(pTrack));

    PlayerInfo::instance().setTrackInfo(PlayerManager::groupForDeck(2), pTrack);
    EXPECT_QSTRING_EQ(QStringLiteral("1 3"), loadedDeckDisplayText(pTrack));
}
