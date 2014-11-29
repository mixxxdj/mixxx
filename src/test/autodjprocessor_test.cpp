#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QString>
#include <QScopedPointer>

#include "test/librarytest.h"
#include "library/autodj/autodjprocessor.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controllinpotmeter.h"
#include "playermanager.h"
#include "basetrackplayer.h"

using ::testing::_;
using ::testing::Return;

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

class FakeDeck : public BaseTrackPlayer {
  public:
    FakeDeck(const QString& group)
            : BaseTrackPlayer(NULL, group),
              playposition(ConfigKey(group, "playposition"), 0.0, 1.0, true),
              play(ConfigKey(group, "play")),
              repeat(ConfigKey(group, "repeat")) {
        play.setButtonMode(ControlPushButton::TOGGLE);
        repeat.setButtonMode(ControlPushButton::TOGGLE);
    }

    void fakeTrackLoadedEvent(TrackPointer pTrack) {
        loadedTrack = pTrack;
        emit(newTrackLoaded(pTrack));
    }

    void fakeTrackLoadFailedEvent(TrackPointer pTrack) {
        emit(loadTrackFailed(pTrack));
    }

    void fakeUnloadingTrackEvent(TrackPointer pTrack) {
        emit(unloadingTrack(pTrack));
        loadedTrack.clear();
    }

    TrackPointer getLoadedTrack() const {
        return loadedTrack;
    }

    void slotLoadTrack(TrackPointer pTrack, bool bPlay) {
        loadedTrack = pTrack;
        play.set(bPlay);
    }

    TrackPointer loadedTrack;
    ControlLinPotmeter playposition;
    ControlPushButton play;
    ControlPushButton repeat;
};

class MockPlayerManager : public PlayerManagerInterface {
  public:
    MockPlayerManager()
            : numDecks(ConfigKey("[Master]", "num_decks"), true),
              numSamplers(ConfigKey("[Master]", "num_samplers"), true),
              numPreviewDecks(ConfigKey("[Master]", "num_preview_decks"),
                              true) {
    }

    virtual ~MockPlayerManager() {
    }

    MOCK_CONST_METHOD1(getPlayer, BaseTrackPlayer*(QString));
    MOCK_CONST_METHOD1(getDeck, Deck*(unsigned int));
    MOCK_CONST_METHOD1(getPreviewDeck, PreviewDeck*(unsigned int));
    MOCK_CONST_METHOD1(getSampler, Sampler*(unsigned int));

    unsigned int numberOfDecks() const {
        return static_cast<unsigned int>(numDecks.get());
    }

    unsigned int numberOfSamplers() const {
        return static_cast<unsigned int>(numSamplers.get());
    }

    unsigned int numberOfPreviewDecks() const {
        return static_cast<unsigned int>(numPreviewDecks.get());
    }

    ControlObject numDecks;
    ControlObject numSamplers;
    ControlObject numPreviewDecks;
};

class MockAutoDJProcessor : public AutoDJProcessor {
  public:
    MockAutoDJProcessor(QObject* pParent,
                        ConfigObject<ConfigValue>* pConfig,
                        PlayerManagerInterface* pPlayerManager,
                        int iAutoDJPlaylistId,
                        TrackCollection* pCollection)
            : AutoDJProcessor(pParent, pConfig, pPlayerManager,
                              iAutoDJPlaylistId, pCollection) {
    }

    virtual ~MockAutoDJProcessor() {
    }

    MOCK_METHOD3(loadTrackToPlayer, void(TrackPointer, QString, bool));
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

        pPlayerManager.reset(new MockPlayerManager());

        // Setup 4 fake decks.
        ON_CALL(*pPlayerManager, getPlayer(QString("[Channel1]")))
                .WillByDefault(Return(&deck1));
        ON_CALL(*pPlayerManager, getPlayer(QString("[Channel2]")))
                .WillByDefault(Return(&deck2));
        ON_CALL(*pPlayerManager, getPlayer(QString("[Channel3]")))
                .WillByDefault(Return(&deck3));
        ON_CALL(*pPlayerManager, getPlayer(QString("[Channel4]")))
                .WillByDefault(Return(&deck4));
        pPlayerManager->numDecks.set(4);

        EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel1]"))).Times(1);
        EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel2]"))).Times(1);
        EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel3]"))).Times(1);
        EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel4]"))).Times(1);

        pProcessor.reset(new MockAutoDJProcessor(
                NULL, config(), pPlayerManager.data(),
                m_iAutoDJPlaylistId, collection()));
    }

    virtual ~AutoDJProcessorTest() {
    }

    void setTrackId(TrackPointer pTrack, int id) {
        pTrack->setId(id);
    }

    FakeMaster master;
    ControlObject numDecks;
    FakeDeck deck1;
    FakeDeck deck2;
    FakeDeck deck3;
    FakeDeck deck4;
    QScopedPointer<MockPlayerManager> pPlayerManager;
    int m_iAutoDJPlaylistId;
    QScopedPointer<MockAutoDJProcessor> pProcessor;
};

TEST_F(AutoDJProcessorTest, TransitionTimeLoadedFromConfig) {
    EXPECT_EQ(kDefaultTransitionTime, pProcessor->getTransitionTime());
    config()->set(ConfigKey("[Auto DJ]", "Transition"), QString("25"));
    // Creating a new MockAutoDJProcessor will get each player from player
    // manager.
    EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel1]"))).Times(1);
    EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel2]"))).Times(1);
    EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel3]"))).Times(1);
    EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel4]"))).Times(1);
    pProcessor.reset(new MockAutoDJProcessor(
            NULL, config(), pPlayerManager.data(),
            m_iAutoDJPlaylistId, collection()));
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
    // Need two tracks -- one to be loaded in the left deck and one to load in
    // the right deck.
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    // Expect that we switch into ADJ_ENABLE_P1LOADED first.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_ENABLE_P1LOADED));
    // Expect that we get a load-and-play signal for [Channel1].
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel1]"), true));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, pProcessor->getState());
    // Sets crossfader left and deck 1 playing.
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    // ADJ_ENABLE_P1LOADED logic does not set play directly. It waits for the
    // engine to load the track and set the deck playing.
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect that we transition to ADJ_IDLE.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    // Expect that we will receive a load call for [Channel2] after we get the
    // first playposition update from deck 1.
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel2]"), false));

    // Pretend a track loaded successfully and that it is now playing. This
    // triggers a call to AutoDJProcessor::playerPlayChanged and
    // AutoDJProcessor::playerPlaypositionChanged. We should switch to ADJ_IDLE
    // and queue a track to deck 2.
    deck1.play.set(1.0);
    deck1.playposition.set(100);

    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_PlayingDeck1) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    // Pretend a track is playing on deck 1.
    TrackPointer pTrack(new TrackInfoObject());
    setTrackId(pTrack, testId + 1);
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);

    // Arbitrary to check that it was unchanged.
    master.crossfader.set(0.2447);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel2]"), false));

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
    // Load track and mark it playing.
    deck2.slotLoadTrack(pTrack, true);

    // Arbitrary to check that it was unchanged.
    master.crossfader.set(0.2447);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel1]"), false));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(0.2447, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());
}
