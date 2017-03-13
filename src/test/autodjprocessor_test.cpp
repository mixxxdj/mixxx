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
        // The real EngineBuffer ejects the track first which clears a variety
        // of controls (play, track_samples, track_samplerate, playposition,
        // etc.).
        fakeUnloadingTrackEvent(pTrack);
        emit(loadTrackFailed(pTrack));
    }

    void fakeUnloadingTrackEvent(TrackPointer pTrack) {
        play.set(0.0);
        emit(unloadingTrack(pTrack));
        loadedTrack.clear();
    }

    TrackPointer getLoadedTrack() const {
        return loadedTrack;
    }

    // This method emulates requesting a track load to a player and emits no
    // signals. Normally, the reader thread attempts to load the file and emits
    // a success or failure signal. To simulate a load success, call
    // fakeTrackLoadedEvent. To simulate a failure, call
    // fakeTrackLoadFailedEvent.
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

    // Load the track and mark it playing (as the loadTrackToPlayer signal would
    // have connected to this eventually).
    TrackPointer pTrack(new TrackInfoObject());
    setTrackId(pTrack, testId);
    deck1.slotLoadTrack(pTrack, true);

    // Signal that the request to load pTrack succeeded.
    deck1.fakeTrackLoadedEvent(pTrack);

    // Pretend the engine moved forward on the deck.
    deck1.playposition.set(0.1);

    // By now we will have transitioned to idle and requested a load to deck 2.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_DecksStopped_TrackLoadFails) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // Need three tracks -- one to be loaded in the left deck (failing to load),
    // one to load in the left deck (succeeding) and one to load in the right
    // deck (succeeding).
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    // Expect that we switch into ADJ_ENABLE_P1LOADED first.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_ENABLE_P1LOADED));
    // Expect that we get a load-and-play signal for [Channel1].
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel1]"), true));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, pProcessor->getState());
    // Sets crossfader left.
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    // ADJ_ENABLE_P1LOADED logic does not set play directly. It waits for the
    // engine to load the track and set the deck playing.
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Once the track load fails, we should get another load-and-play request
    // for Deck 1.
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel1]"), true));

    // Load the track and mark it playing (as the loadTrackToPlayer signal would
    // have connected to this eventually).
    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    setTrackId(pTrack, testId);
    deck1.slotLoadTrack(pTrack, true);

    // Signal that the request to load pTrack failed.
    deck1.fakeTrackLoadFailedEvent(pTrack);

    // Check that we are still in ADJ_ENABLE_P1LOADED mode since we haven't
    // received a playposition update from a successfully loaded track yet.
    EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, pProcessor->getState());

    // Check the crossfader is the same.
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());

    // The deck1 play state was cleared by the engine after the failure to
    // load. Check that the AutoDJProcessor didn't set either deck to playing.
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Now pretend that the follow-up load request succeeded.
    deck1.slotLoadTrack(pTrack, true);
    deck1.fakeTrackLoadedEvent(pTrack);

    // Expect that we will receive a load call for [Channel2] after we get the
    // first playposition update from deck 1.
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel2]"), false));

    // Expect that we will switch into ADJ_IDLE.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));

    // Pretend the engine moved forward on the deck. This will request a track
    // load on deck 2 and put us in IDLE mode.
    deck1.playposition.set(0.1);

    // By now we will have transitioned to idle and requested a load to deck 2.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_DecksStopped_TrackLoadFailsRightDeck) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // Need three tracks -- one to be loaded in the left deck (succeeding), one
    // to load in the righ deck (failing) and one to load in the right deck
    // (succeeding).
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    // Expect that we switch into ADJ_ENABLE_P1LOADED first.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_ENABLE_P1LOADED));
    // Expect that we get a load-and-play signal for [Channel1].
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel1]"), true));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, pProcessor->getState());
    // Sets crossfader left.
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    // ADJ_ENABLE_P1LOADED logic does not set play directly. It waits for the
    // engine to load the track and set the deck playing.
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Load the track and mark it playing (as the loadTrackToPlayer signal would
    // have connected to this eventually).
    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    setTrackId(pTrack, testId);
    deck1.slotLoadTrack(pTrack, true);

    // Signal that the request to load pTrack to deck1 succeeded.
    deck1.fakeTrackLoadedEvent(pTrack);

    // Expect that we will switch into ADJ_IDLE.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));

    // Expect that we will receive a load call for [Channel2] after we get the
    // first playposition update from deck 1.
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel2]"), false));

    // Pretend the engine moved forward on the deck. This will request a track
    // load on deck 2 and put us in IDLE mode.
    deck1.playposition.set(0.1);

    // Check that we are now in ADJ_IDLE mode.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Expect that we will receive another load call for [Channel2] after we get
    // the track load failed signal.
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel2]"), false));

    // Now pretend that the deck2 load request failed.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadFailedEvent(pTrack);

    // Check that we are still in ADJ_IDLE mode.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Pretend the deck2 load request succeeded.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // Check that we are still in ADJ_IDLE mode and the left deck is playing.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_PlayingDeck1) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    // Pretend a track is playing on deck 1.
    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    setTrackId(pTrack, testId + 1);
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

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

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(0.2447, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_PlayingDeck1_TrackLoadFailed) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    // Pretend a track is playing on deck 1.
    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    setTrackId(pTrack, testId + 1);
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    // Arbitrary to check that it was unchanged.
    master.crossfader.set(0.2447);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track will fail to load and the second will succeed.
    pAutoDJTableModel->appendTrack(testId);
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

    // After the load failed signal we will receive another track load signal
    // for deck 2.
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel2]"), false));

    // Pretend the track load fails.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadFailedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(0.2447, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(0.2447, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_PlayingDeck2) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    // Pretend a track is playing on deck 2.
    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    setTrackId(pTrack, testId + 1);
    // Load track and mark it playing.
    deck2.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck2.fakeTrackLoadedEvent(pTrack);

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

    // Pretend the track load succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(0.2447, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_PlayingDeck2_TrackLoadFailed) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    // Pretend a track is playing on deck 2.
    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    setTrackId(pTrack, testId + 1);
    // Load track and mark it playing.
    deck2.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck2.fakeTrackLoadedEvent(pTrack);

    // Arbitrary to check that it was unchanged.
    master.crossfader.set(0.2447);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track will fail to load and the second will succeed.
    pAutoDJTableModel->appendTrack(testId);
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

    // After the load failed signal we will receive another track load signal
    // for deck 1.
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel1]"), false));

    // Pretend the track load fails.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadFailedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(0.2447, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(0.2447, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, FadeToDeck1_LoadOnDeck2_TrackLoadSuccess) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    // Crossfader starts on the right.
    master.crossfader.set(1.0);
    // Pretend a track is playing on deck 2.
    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    setTrackId(pTrack, testId + 1);
    // Load track and mark it playing.
    deck2.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck2.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track is loaded into deck 1 and the second track is loaded into
    // deck 2 afer we fade to deck 1.
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel1]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck1.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Expect that we will transition into P2FADING mode.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_P2FADING));

    // Pretend the track is over (should trigger an instant-fade).
    deck2.playposition.set(1.0);

    EXPECT_EQ(AutoDJProcessor::ADJ_P2FADING, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect that we will transition into IDLE mode and receive a track-load
    // request for deck 2 after deck 1 starts playing.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel2]"), false));

    // Now start deck1 playing.
    deck1.playposition.set(0.1);

    // Check we are in IDLE mode, the crossfader is fully left, and deck 1 is
    // playing.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend the track load request succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, FadeToDeck1_LoadOnDeck2_TrackLoadFailed) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    // Crossfader starts on the right.
    master.crossfader.set(1.0);
    // Pretend a track is playing on deck 2.
    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    setTrackId(pTrack, testId + 1);
    // Load track and mark it playing.
    deck2.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck2.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track is loaded into deck 1, the second track is loaded into
    // deck 2 (fails) afer we fade to deck 1, and the third is loaded into deck
    // 2 (succeeds).
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel1]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck1.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Expect that we will transition into P2FADING mode.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_P2FADING));

    // Pretend the track is over (should trigger an instant-fade).
    deck2.playposition.set(1.0);

    EXPECT_EQ(AutoDJProcessor::ADJ_P2FADING, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect that we will transition into IDLE mode and receive a track-load
    // request for deck 2 after deck 1 starts playing.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel2]"), false));

    // Now start deck1 playing.
    deck1.playposition.set(0.1);

    // Check we are in IDLE mode, the crossfader is fully left, and deck 1 is
    // playing.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect we will receive another track load request for deck 2 after we
    // fail the first request.
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel2]"), false));

    // Pretend the track load request fails.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadFailedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend the second track load request succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, FadeToDeck2_LoadOnDeck1_TrackLoadSuccess) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    // Crossfader starts on the left.
    master.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    setTrackId(pTrack, testId + 1);
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track is loaded into deck 2 and the second track is loaded into
    // deck 1 afer we fade to deck 2.
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect that we will transition into P1FADING mode.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_P1FADING));

    // Pretend the track is over (should trigger an instant-fade).
    deck1.playposition.set(1.0);

    EXPECT_EQ(AutoDJProcessor::ADJ_P1FADING, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Expect that we will transition into IDLE mode and receive a track-load
    // request for deck 1 after deck 2 starts playing.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel1]"), false));

    // Now start deck2 playing.
    deck2.playposition.set(0.1);

    // Check we are in IDLE mode, the crossfader is fully right, and deck 2 is
    // playing.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Pretend the track load request succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, FadeToDeck2_LoadOnDeck1_TrackLoadFailed) {
    int testId = collection()->getTrackDAO().addTrack(kTrackLocationTest, false);
    ASSERT_LT(0, testId);

    // Crossfader starts on the left.
    master.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    setTrackId(pTrack, testId + 1);
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track is loaded into deck 2, the second track is loaded into
    // deck 1 (fails) afer we fade to deck 2, and the third is loaded into deck
    // 1 (succeeds).
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect that we will transition into P1FADING mode.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_P1FADING));

    // Pretend the track is over (should trigger an instant-fade).
    deck1.playposition.set(1.0);

    EXPECT_EQ(AutoDJProcessor::ADJ_P1FADING, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Expect that we will transition into IDLE mode and receive a track-load
    // request for deck 1 after deck 2 starts playing.
    EXPECT_CALL(*pProcessor, autoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel1]"), false));

    // Now start deck2 playing.
    deck2.playposition.set(0.1);

    // Check we are in IDLE mode, the crossfader is fully right, and deck 2 is
    // playing.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Expect we will receive another track load request for deck 1 after we
    // fail the first request.
    EXPECT_CALL(*pProcessor, loadTrackToPlayer(_, QString("[Channel1]"), false));

    // Pretend the track load request fails.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadFailedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Pretend the track load request succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, master.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());
}
