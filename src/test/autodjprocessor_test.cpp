#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QString>
#include <QScopedPointer>

#include "test/librarytest.h"
#include "library/autodj/autodjprocessor.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controllinpotmeter.h"
#include "playerinfo.h"

using ::testing::_;

static int kDefaultTransitionTime = 10;
const QString kTrackLocationTest(QDir::currentPath() %
                                 "/src/test/id3-test-data/cover-test.mp3");

class FakeMaster {
  public:
    FakeMaster()
            : crossfader(ConfigKey("[Master]", "crossfader"), -1.0, 1.0),
              crossfaderReverse(ConfigKey("[Mixer Profile]", "xFaderReverse")) {
        crossfaderReverse.setButtonMode(ControlPushButton::TOGGLE);
    }

    ControlPotmeter crossfader;
    ControlPushButton crossfaderReverse;
};

class FakeDeck {
  public:
    FakeDeck(const QString& group)
            : playposition(ConfigKey(group, "playposition"), 0.0, 1.0, true),
              play(ConfigKey(group, "play")),
              repeat(ConfigKey(group, "repeat")) {
        play.setButtonMode(ControlPushButton::TOGGLE);
        repeat.setButtonMode(ControlPushButton::TOGGLE);
    }

    ControlLinPotmeter playposition;
    ControlPushButton play;
    ControlPushButton repeat;
};

class MockAutoDJProcessor : public AutoDJProcessor {
  public:
    MockAutoDJProcessor(QObject* pParent,
                        ConfigObject<ConfigValue>* pConfig,
                        int iAutoDJPlaylistId,
                        TrackCollection* pCollection)
            : AutoDJProcessor(pParent, pConfig, iAutoDJPlaylistId, pCollection) {
    }

    MOCK_METHOD1(loadTrack, void(TrackPointer));
    MOCK_METHOD1(transitionTimeChanged, void(int));
    MOCK_METHOD1(autoDJStateChanged, void(AutoDJProcessor::AutoDJState));
};

class AutoDJProcessorTest : public LibraryTest {
  protected:
    AutoDJProcessorTest()
            :  deck1("[Channel1]"),
               deck2("[Channel2]"),
               deck3("[Channel3]"),
               deck4("[Channel4]") {
        PlaylistDAO& playlistDao = collection()->getPlaylistDAO();
        m_iAutoDJPlaylistId = playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
        // If the AutoDJ playlist does not exist yet then create it.
        if (m_iAutoDJPlaylistId < 0) {
            m_iAutoDJPlaylistId = playlistDao.createPlaylist(
                    AUTODJ_TABLE, PlaylistDAO::PLHT_AUTO_DJ);
        }

        pProcessor.reset(new MockAutoDJProcessor(NULL, config(), m_iAutoDJPlaylistId,
                                                 collection()));
    }

    virtual ~AutoDJProcessorTest() {
    }

    void setTrackId(TrackPointer pTrack, int id) {
        pTrack->setId(id);
    }

    FakeMaster master;
    FakeDeck deck1;
    FakeDeck deck2;
    FakeDeck deck3;
    FakeDeck deck4;
    int m_iAutoDJPlaylistId;
    QScopedPointer<MockAutoDJProcessor> pProcessor;
};

TEST_F(AutoDJProcessorTest, TransitionTimeLoadedFromConfig) {
    EXPECT_EQ(kDefaultTransitionTime, pProcessor->getTransitionTime());
    config()->set(ConfigKey("[Auto DJ]", "Transition"), QString("25"));
    pProcessor.reset(new MockAutoDJProcessor(NULL, config(), m_iAutoDJPlaylistId,
                                             collection()));
    EXPECT_EQ(25, pProcessor->getTransitionTime());
}

TEST_F(AutoDJProcessorTest, DecksPlayingWarning) {
    deck1.play.set(1);
    deck2.play.set(1);
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_BOTH_DECKS_PLAYING, err);
}

TEST_F(AutoDJProcessorTest, QueueEmpty) {
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_QUEUE_EMPTY, err);
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_DecksStopped) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, transitionTimeChanged(kDefaultTransitionTime));
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_ENABLE_P1LOADED));
    EXPECT_CALL(*pProcessor, loadTrack(_));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, pProcessor->getState());
    // Sets crossfader left and deck 1 playing.
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_PlayingDeck1) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    // Pretend a track is playing on deck 1.
    TrackPointer pTrack(new TrackInfoObject());
    setTrackId(pTrack, testId + 1);
    PlayerInfo::instance().setTrackInfo("[Channel1]", pTrack);
    deck1.play.set(1);

    // Arbitrary to check that it was unchanged.
    master.crossfader.set(0.2447);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, transitionTimeChanged(kDefaultTransitionTime));
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrack(_));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(0.2447, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_PlayingDeck2) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    // Pretend a track is playing on deck 2.
    TrackPointer pTrack(new TrackInfoObject());
    setTrackId(pTrack, testId + 1);
    PlayerInfo::instance().setTrackInfo("[Channel2]", pTrack);
    deck2.play.set(1);

    // Arbitrary to check that it was unchanged.
    master.crossfader.set(0.2447);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, transitionTimeChanged(kDefaultTransitionTime));
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrack(_));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(0.2447, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());
}
