#include "library/autodj/autodjprocessor.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QScopedPointer>
#include <QString>

#include "control/controllinpotmeter.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "engine/engine.h"
#include "library/dao/trackschema.h"
#include "library/playlisttablemodel.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "sources/soundsourceproxy.h"
#include "test/librarytest.h"
#include "track/track.h"

using ::testing::_;
using ::testing::Return;

namespace {
const int kDefaultTransitionTime = 10;
const mixxx::audio::ChannelCount kChannelCount = mixxx::kEngineChannelOutputCount;
const QString kTrackLocationTest = QStringLiteral("id3-test-data/cover-test-png.mp3");
const QString kAppGroup = QStringLiteral("[App]");
} // namespace

class FakeMixer {
  public:
    FakeMixer()
            : crossfader(ConfigKey("[Master]", "crossfader"), -1.0, 1.0),
              crossfaderReverse(ConfigKey("[Mixer Profile]", "xFaderReverse")) {
        crossfaderReverse.setButtonMode(mixxx::control::ButtonMode::Toggle);
    }

    ControlPotmeter crossfader;
    ControlPushButton crossfaderReverse;
};

class FakeDeck : public BaseTrackPlayer {
  public:
    FakeDeck(const QString& group, EngineChannel::ChannelOrientation orient)
            : BaseTrackPlayer(NULL, group),
              trackSamples(ConfigKey(group, "track_samples")),
              samplerate(ConfigKey(group, "track_samplerate")),
              rateratio(ConfigKey(group, "rate_ratio"), true, false, false, 1.0),
              playposition(ConfigKey(group, "playposition"), 0.0, 1.0, 0, 0, true),
              play(ConfigKey(group, "play")),
              repeat(ConfigKey(group, "repeat")),
              introStartPos(ConfigKey(group, "intro_start_position")),
              introEndPos(ConfigKey(group, "intro_end_position")),
              outroStartPos(ConfigKey(group, "outro_start_position")),
              outroEndPos(ConfigKey(group, "outro_end_position")),
              orientation(ConfigKey(group, "orientation")) {
        play.setButtonMode(mixxx::control::ButtonMode::Toggle);
        repeat.setButtonMode(mixxx::control::ButtonMode::Toggle);
        outroStartPos.set(Cue::kNoPosition);
        outroEndPos.set(Cue::kNoPosition);
        orientation.set(orient);
    }

    void fakeTrackLoadedEvent(TrackPointer pTrack) {
        loadedTrack = pTrack;
        trackSamples.set(pTrack->getDuration() * pTrack->getSampleRate() * 2);
        samplerate.set(pTrack->getSampleRate());
        emit newTrackLoaded(pTrack);
    }

    void fakeTrackLoadFailedEvent(TrackPointer pTrack) {
        // The real EngineBuffer ejects the track first which clears a variety
        // of controls (play, track_samples, track_samplerate, playposition,
        // etc.).
        fakeUnloadingTrackEvent(pTrack);
    }

    void fakeUnloadingTrackEvent(TrackPointer pTrack) {
        play.set(0.0);
        emit loadingTrack(TrackPointer(), pTrack);
        loadedTrack.reset();
        emit playerEmpty();
    }

    TrackPointer getLoadedTrack() const override {
        return loadedTrack;
    }

    void setupEqControls() override{};

    // This method emulates requesting a track load to a player and emits no
    // signals. Normally, the reader thread attempts to load the file and emits
    // a success or failure signal. To simulate a load success, call
    // fakeTrackLoadedEvent. To simulate a failure, call
    // fakeTrackLoadFailedEvent.
    void slotLoadTrack(TrackPointer pTrack, bool bPlay) override {
        loadedTrack = pTrack;
        samplerate.set(pTrack->getSampleRate());
        play.set(bPlay);
    }

    void slotEjectTrack(double val) override {
        if (val > 0) {
            loadedTrack = nullptr;
        }
    }

    MOCK_METHOD1(slotCloneFromGroup, void(const QString& group));
    MOCK_METHOD0(slotCloneDeck, void());

    TrackPointer loadedTrack;
    ControlObject trackSamples;
    ControlObject samplerate;
    ControlObject rateratio;
    ControlLinPotmeter playposition;
    ControlPushButton play;
    ControlPushButton repeat;
    ControlObject introStartPos;
    ControlObject introEndPos;
    ControlObject outroStartPos;
    ControlObject outroEndPos;
    ControlObject orientation;
};

class MockPlayerManager : public PlayerManagerInterface {
  public:
    MockPlayerManager()
            : numDecks(ConfigKey(kAppGroup, QStringLiteral("num_decks")), true),
              numSamplers(ConfigKey(kAppGroup, QStringLiteral("num_samplers")), true),
              numPreviewDecks(ConfigKey(kAppGroup, QStringLiteral("num_preview_decks")),
                      true) {
    }

    virtual ~MockPlayerManager() {
    }

    MOCK_CONST_METHOD1(getPlayer, BaseTrackPlayer*(const QString&));
    MOCK_CONST_METHOD1(getPlayer, BaseTrackPlayer*(const ChannelHandle&));
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
                        UserSettingsPointer pConfig,
                        PlayerManagerInterface* pPlayerManager,
                        TrackCollectionManager* pTrackCollectionManager,
                        int iAutoDJPlaylistId)
            : AutoDJProcessor(pParent, pConfig, pPlayerManager,
                              pTrackCollectionManager, iAutoDJPlaylistId) {
    }

    virtual ~MockAutoDJProcessor() {
    }

    MOCK_METHOD3(emitLoadTrackToPlayer, void(TrackPointer, const QString&, bool));
    MOCK_METHOD1(emitAutoDJStateChanged, void(AutoDJProcessor::AutoDJState));
};

class AutoDJProcessorTest : public LibraryTest {
  protected:
    TrackPointer newTestTrack(TrackId trackId = {}) const {
        TrackPointer pTrack(
                Track::newDummy(getTestDir().filePath(kTrackLocationTest), trackId));
        EXPECT_EQ(
                SoundSourceProxy::UpdateTrackFromSourceResult::MetadataImportedAndUpdated,
                SoundSourceProxy(pTrack).updateTrackFromSource(
                        SoundSourceProxy::UpdateTrackFromSourceMode::Once,
                        SyncTrackMetadataParams{}));
        return pTrack;
    }

    AutoDJProcessorTest()
            : deck1("[Channel1]", EngineChannel::LEFT),
              deck2("[Channel2]", EngineChannel::RIGHT),
              deck3("[Channel3]", EngineChannel::LEFT),
              deck4("[Channel4]", EngineChannel::RIGHT) {
        qRegisterMetaType<TrackPointer>("TrackPointer");

        PlaylistDAO& playlistDao = internalCollection()->getPlaylistDAO();
        m_iAutoDJPlaylistId = playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
        // If the AutoDJ playlist does not exist yet then create it.
        if (m_iAutoDJPlaylistId < 0) {
            m_iAutoDJPlaylistId = playlistDao.createPlaylist(
                    AUTODJ_TABLE, PlaylistDAO::PLHT_AUTO_DJ);
        }

        pPlayerManager.reset(new MockPlayerManager());
        PlayerInfo::create();

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

        pProcessor.reset(new MockAutoDJProcessor(nullptr,
                config(),
                pPlayerManager.data(),
                trackCollectionManager(),
                m_iAutoDJPlaylistId));
    }

    virtual ~AutoDJProcessorTest() {
        PlayerInfo::destroy();
    }

    TrackId addTrackToCollection(const QString& trackLocation) {
        TrackPointer pTrack =
                getOrAddTrackByLocation(getTestDir().filePath(trackLocation));
        return pTrack ? pTrack->getId() : TrackId();
    }

    FakeMixer mixer;
    FakeDeck deck1;
    FakeDeck deck2;
    FakeDeck deck3;
    FakeDeck deck4;
    QScopedPointer<MockPlayerManager> pPlayerManager;
    int m_iAutoDJPlaylistId;
    QScopedPointer<MockAutoDJProcessor> pProcessor;
};

TEST_F(AutoDJProcessorTest, FullIntroOutro_LongerIntro) {
    pProcessor->setTransitionMode(AutoDJProcessor::TransitionMode::FullIntroOutro);

    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the left.
    mixer.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Pretend that track is 1 minute and 40 seconds long.
    pTrack->setDuration(100);
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_EQ(AutoDJProcessor::ADJ_DISABLED, pProcessor->getState());
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);

    // Set intro + outro cues. Outro is 10 seconds long; intro is 30 seconds.
    const double kSamplesPerSecond = kChannelCount * pTrack->getSampleRate();
    deck1.outroStartPos.set(60 * kSamplesPerSecond);
    deck1.outroEndPos.set(70 * kSamplesPerSecond);
    deck2.introStartPos.set(10 * kSamplesPerSecond);
    deck2.introEndPos.set(40 * kSamplesPerSecond);

    // AutoDJProcessor calculates the transition when the newTrackLoaded signal
    // is emitted.
    deck2.fakeTrackLoadedEvent(pTrack);
    // The incoming track should seek to the intro start
    EXPECT_DOUBLE_EQ(0.1, deck2.playposition.get());

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Seek the outgoing track to where outro start cue is placed. It should
    // start fading.
    deck1.playposition.set(0.6);
    EXPECT_EQ(AutoDJProcessor::ADJ_LEFT_FADING, pProcessor->getState());

    EXPECT_DOUBLE_EQ(0.1, deck2.playposition.get());

    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    deck1.playposition.set(0.7);
    deck1.play.set(0.0);

    // Expect that we will transition into IDLE mode and request a track load on deck1.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    // Advance track to the point where crossfading should be over (intro end)
    deck2.playposition.set(0.4);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
}

TEST_F(AutoDJProcessorTest, FullIntroOutro_LongerOutro) {
    pProcessor->setTransitionMode(AutoDJProcessor::TransitionMode::FullIntroOutro);

    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the left.
    mixer.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Pretend that track is 1 minute and 40 seconds long.
    pTrack->setDuration(100);
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_EQ(AutoDJProcessor::ADJ_DISABLED, pProcessor->getState());
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);

    // Set intro + outro cues. Outro is 20 seconds long; intro is 10 seconds.
    const double kSamplesPerSecond = kChannelCount * pTrack->getSampleRate();
    deck1.outroStartPos.set(70 * kSamplesPerSecond);
    deck1.outroEndPos.set(90 * kSamplesPerSecond);
    deck2.introStartPos.set(10 * kSamplesPerSecond);
    deck2.introEndPos.set(20 * kSamplesPerSecond);

    // AutoDJProcessor calculates the transition when the newTrackLoaded signal
    // is emitted.
    deck2.fakeTrackLoadedEvent(pTrack);

    // The incoming track should be at intro start
    EXPECT_DOUBLE_EQ(0.1, deck2.playposition.get());

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Seek the outgoing track to where outro start cue is placed. It should not
    // be fading yet.
    deck1.playposition.set(0.7);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Seek the outgoing track to where the transition should start
    // outro end (90 s) - intro length (10 s)
    deck1.playposition.set(0.8);
    EXPECT_EQ(AutoDJProcessor::ADJ_LEFT_FADING, pProcessor->getState());

    EXPECT_DOUBLE_EQ(0.1, deck2.playposition.get());

    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    deck1.playposition.set(0.9);
    deck1.play.set(0.0);

    // Expect that we will transition into IDLE mode and request a track load on deck1.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    // Advance track to the point where crossfading should be over.
    deck2.playposition.set(0.2);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
}

TEST_F(AutoDJProcessorTest, FadeAtOutroStart_LongerIntro) {
    pProcessor->setTransitionMode(AutoDJProcessor::TransitionMode::FadeAtOutroStart);

    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the left.
    mixer.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Pretend that track is 1 minute and 40 seconds long.
    pTrack->setDuration(100);
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_EQ(AutoDJProcessor::ADJ_DISABLED, pProcessor->getState());
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);

    // Set intro + outro cues. Outro is 10 seconds long; intro is 20 seconds.
    const double kSamplesPerSecond = kChannelCount * pTrack->getSampleRate();
    deck1.outroStartPos.set(80 * kSamplesPerSecond);
    deck1.outroEndPos.set(90 * kSamplesPerSecond);
    deck2.introStartPos.set(10 * kSamplesPerSecond);
    deck2.introEndPos.set(30 * kSamplesPerSecond);

    // AutoDJProcessor calculates the transition when the newTrackLoaded signal
    // is emitted.
    deck2.fakeTrackLoadedEvent(pTrack);

    // The incoming track should be at intro start
    EXPECT_DOUBLE_EQ(0.1, deck2.playposition.get());

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Seek the outgoing track to where outro start cue is placed. It should
    // start fading.
    deck1.playposition.set(0.8);
    EXPECT_EQ(AutoDJProcessor::ADJ_LEFT_FADING, pProcessor->getState());

    EXPECT_DOUBLE_EQ(0.1, deck2.playposition.get());

    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Seek the outgoing track to where fading should end
    deck1.playposition.set(0.90);
    deck1.play.set(0.0);

    // Expect that we will transition into IDLE mode and request a track load on deck1.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    // Advance track to the point where crossfading should be over.
    deck2.playposition.set(0.3);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
}

TEST_F(AutoDJProcessorTest, FadeAtOutroStart_LongerOutro) {
    pProcessor->setTransitionMode(AutoDJProcessor::TransitionMode::FadeAtOutroStart);

    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the left.
    mixer.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Pretend that track is 1 minute and 40 seconds long.
    pTrack->setDuration(100);
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_EQ(AutoDJProcessor::ADJ_DISABLED, pProcessor->getState());
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);

    // Set intro + outro cues. Outro is 20 seconds long; intro is 10 seconds.
    const double kSamplesPerSecond = kChannelCount * pTrack->getSampleRate();
    deck1.outroStartPos.set(60 * kSamplesPerSecond);
    deck1.outroEndPos.set(80 * kSamplesPerSecond);
    deck2.introStartPos.set(10 * kSamplesPerSecond);
    deck2.introEndPos.set(20 * kSamplesPerSecond);

    // AutoDJProcessor calculates the transition when the newTrackLoaded signal
    // is emitted.
    deck2.fakeTrackLoadedEvent(pTrack);

    // The incoming track should be at intro start
    EXPECT_DOUBLE_EQ(0.1, deck2.playposition.get());

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Seek the outgoing track to where outro start cue is placed. It should
    // start fading.
    deck1.playposition.set(0.6);
    EXPECT_EQ(AutoDJProcessor::ADJ_LEFT_FADING, pProcessor->getState());

    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Seek the outgoing track to where fading should end. The rest of the outro
    // should be cut off.
    deck1.playposition.set(0.7);
    deck1.play.set(0.0);

    // Expect that we will transition into IDLE mode and request a track load on deck1.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    // Advance track to the point where crossfading should be over.
    deck2.playposition.set(0.2);

    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
}

TEST_F(AutoDJProcessorTest, TransitionTimeLoadedFromConfig) {
    EXPECT_EQ(kDefaultTransitionTime, pProcessor->getTransitionTime());
    config()->set(ConfigKey("[Auto DJ]", "Transition"), QString("25"));
    // Creating a new MockAutoDJProcessor will get each player from player
    // manager.
    EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel1]"))).Times(1);
    EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel2]"))).Times(1);
    EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel3]"))).Times(1);
    EXPECT_CALL(*pPlayerManager, getPlayer(QString("[Channel4]"))).Times(1);

    // We need to call reset *before* constructing a new MockAutoDJProcessor,
    // because otherwise the new object will try to create COs that already
    // exist because they were created by the previous instance.
    pProcessor.reset();
    pProcessor.reset(new MockAutoDJProcessor(nullptr,
            config(),
            pPlayerManager.data(),
            trackCollectionManager(),
            m_iAutoDJPlaylistId));
    EXPECT_EQ(25, pProcessor->getTransitionTime());
}

TEST_F(AutoDJProcessorTest, DecksPlayingWarning) {
    deck1.play.set(1);
    deck2.play.set(1);
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_BOTH_DECKS_PLAYING, err);
}

TEST_F(AutoDJProcessorTest, Decks34PlayingWarning) {
    deck3.play.set(1);
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_DECKS_3_4_PLAYING, err);

    deck3.play.set(0);
    deck4.play.set(1);
    err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_DECKS_3_4_PLAYING, err);
}

TEST_F(AutoDJProcessorTest, QueueEmpty) {
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_QUEUE_EMPTY, err);
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_DecksStopped) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // Need two tracks -- one to be loaded in the left deck and one to load in
    // the right deck.
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    // Expect that we switch into ADJ_ENABLE_P1LOADED first.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_ENABLE_P1LOADED));
    // Expect that we get a load-and-play signal for [Channel1].
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), true));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, pProcessor->getState());
    // Sets crossfader left and deck 1 playing.
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    // ADJ_ENABLE_P1LOADED logic does not set play directly. It waits for the
    // engine to load the track and set the deck playing.
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect that we transition to ADJ_IDLE.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    // Expect that we will receive a load call for [Channel2] after we get the
    // first playposition update from deck 1.
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Pretend a track loaded successfully and that it is now playing. This
    // triggers a call to AutoDJProcessor::playerPlayChanged and
    // AutoDJProcessor::playerPlaypositionChanged. We should switch to ADJ_IDLE
    // and queue a track to deck 2.

    // Load the track and mark it playing (as the loadTrackToPlayer signal would
    // have connected to this eventually).
    TrackPointer pTrack = trackCollectionManager()->getTrackById(testId);
    deck1.slotLoadTrack(pTrack, true);

    // Signal that the request to load pTrack succeeded.
    deck1.fakeTrackLoadedEvent(pTrack);

    // Pretend the engine moved forward on the deck.
    deck1.playposition.set(0.1);

    // By now we will have transitioned to idle and requested a load to deck 2.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_DecksStopped_TrackLoadFails) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // Need three tracks -- one to be loaded in the left deck (failing to load),
    // one to load in the left deck (succeeding) and one to load in the right
    // deck (succeeding).
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    // Expect that we switch into ADJ_ENABLE_P1LOADED first.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_ENABLE_P1LOADED));
    // Expect that we get a load-and-play signal for [Channel1].
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), true));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, pProcessor->getState());
    // Sets crossfader left.
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    // ADJ_ENABLE_P1LOADED logic does not set play directly. It waits for the
    // engine to load the track and set the deck playing.
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Once the track load fails, we should get another load-and-play request
    // for Deck 1.
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), true));

    // Load the track and mark it playing (as the loadTrackToPlayer signal would
    // have connected to this eventually).
    TrackPointer pTrack(newTestTrack(testId));
    deck1.slotLoadTrack(pTrack, true);

    // Signal that the request to load pTrack failed.
    deck1.fakeTrackLoadFailedEvent(pTrack);

    // Check that we are still in ADJ_ENABLE_P1LOADED mode since we haven't
    // received a playposition update from a successfully loaded track yet.
    EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, pProcessor->getState());

    // Check the crossfader is the same.
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());

    // The deck1 play state was cleared by the engine after the failure to
    // load. Check that the AutoDJProcessor didn't set either deck to playing.
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Now pretend that the follow-up load request succeeded.
    deck1.slotLoadTrack(pTrack, true);
    deck1.fakeTrackLoadedEvent(pTrack);

    // Expect that we will receive a load call for [Channel2] after we get the
    // first playposition update from deck 1.
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Expect that we will switch into ADJ_IDLE.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));

    // Pretend the engine moved forward on the deck. This will request a track
    // load on deck 2 and put us in IDLE mode.
    deck1.playposition.set(0.1);

    // By now we will have transitioned to idle and requested a load to deck 2.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_DecksStopped_TrackLoadFailsRightDeck) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // Need three tracks -- one to be loaded in the left deck (succeeding), one
    // to load in the right deck (failing) and one to load in the right deck
    // (succeeding).
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    // Expect that we switch into ADJ_ENABLE_P1LOADED first.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_ENABLE_P1LOADED));
    // Expect that we get a load-and-play signal for [Channel1].
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), true));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, pProcessor->getState());
    // Sets crossfader left.
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    // ADJ_ENABLE_P1LOADED logic does not set play directly. It waits for the
    // engine to load the track and set the deck playing.
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Load the track and mark it playing (as the loadTrackToPlayer signal would
    // have connected to this eventually).
    TrackPointer pTrack(newTestTrack(testId));
    deck1.slotLoadTrack(pTrack, true);

    // Signal that the request to load pTrack to deck1 succeeded.
    deck1.fakeTrackLoadedEvent(pTrack);

    // Expect that we will switch into ADJ_IDLE.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));

    // Expect that we will receive a load call for [Channel2] after we get the
    // first playposition update from deck 1.
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Pretend the engine moved forward on the deck. This will request a track
    // load on deck 2 and put us in IDLE mode.
    deck1.playposition.set(0.1);

    // Check that we are now in ADJ_IDLE mode.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Expect that we will receive another load call for [Channel2] after we get
    // the track load failed signal.
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

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
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_PlayingDeck1) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    // Arbitrary to check that it was unchanged.
    mixer.crossfader.set(0.2447);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    EXPECT_DOUBLE_EQ(-1, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_PlayingDeck1_TrackLoadFailed) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    // Arbitrary to check that it was unchanged.
    mixer.crossfader.set(0.2447);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track will fail to load and the second will succeed.
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(-1, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // After the load failed signal we will receive another track load signal
    // for deck 2.
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Pretend the track load fails.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadFailedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_PlayingDeck2) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Pretend a track is playing on deck 2.
    TrackPointer pTrack = newTestTrack();
    // Load track and mark it playing.
    deck2.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck2.fakeTrackLoadedEvent(pTrack);

    // Arbitrary to check that it was unchanged.
    mixer.crossfader.set(0.2447);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, EnabledSuccess_PlayingDeck2_TrackLoadFailed) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Pretend a track is playing on deck 2.
    TrackPointer pTrack = newTestTrack();
    // Load track and mark it playing.
    deck2.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck2.fakeTrackLoadedEvent(pTrack);

    // Arbitrary to check that it was unchanged.
    mixer.crossfader.set(0.2447);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track will fail to load and the second will succeed.
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(1, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // After the load failed signal we will receive another track load signal
    // for deck 1.
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    // Pretend the track load fails.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadFailedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, EnabledDisabledSuccess) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, pProcessor->getState());

    err = pProcessor->toggleAutoDJ(false);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_DISABLED, pProcessor->getState());
}

TEST_F(AutoDJProcessorTest, FadeToDeck1_LoadOnDeck2_TrackLoadSuccess) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the right.
    mixer.crossfader.set(1.0);
    // Pretend a track is playing on deck 2.
    TrackPointer pTrack = newTestTrack();
    // Load track and mark it playing.
    deck2.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck2.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track is loaded into deck 1 and the second track is loaded into
    // deck 2 after we fade to deck 1.
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck1.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Expect that we will transition into RIGHT_FADING mode.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_RIGHT_FADING));

    // Pretend the track is over (should trigger an instant-fade).
    deck2.playposition.set(1.0);

    EXPECT_EQ(AutoDJProcessor::ADJ_RIGHT_FADING, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    // Deck is still playing, because the crossfader is processed in the next audio
    // callback.
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Fake a final callback, normally in this case the engine
    // stops the deck
    deck2.playposition.set(9.9999);
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect that we will transition into IDLE mode and receive a track-load
    // request for deck 2 after deck 1 starts playing.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Now start deck1 playing.
    deck1.playposition.set(0.1);

    // Check we are in IDLE mode, the crossfader is fully left, and deck 1 is
    // playing.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend the track load request succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, FadeToDeck1_LoadOnDeck2_TrackLoadFailed) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the right.
    mixer.crossfader.set(1.0);
    // Pretend a track is playing on deck 2.
    TrackPointer pTrack = newTestTrack();
    // Load track and mark it playing.
    deck2.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck2.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track is loaded into deck 1, the second track is loaded into
    // deck 2 (fails) after we fade to deck 1, and the third is loaded into deck
    // 2 (succeeds).
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck1.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Expect that we will transition into RIGHT_FADING mode.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_RIGHT_FADING));

    // Pretend the track is over (should trigger an instant-fade).
    deck2.playposition.set(1.0);

    EXPECT_EQ(AutoDJProcessor::ADJ_RIGHT_FADING, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    // Deck is still playing, because the crossfader is processed in the next audio
    // callback.
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Fake a final callback, normally in this case the engine
    // stops the deck
    deck2.playposition.set(9.9999);
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect that we will transition into IDLE mode and receive a track-load
    // request for deck 2 after deck 1 starts playing.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Now start deck1 playing.
    deck1.playposition.set(0.1);

    // Check we are in IDLE mode, the crossfader is fully left, and deck 1 is
    // playing.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect we will receive another track load request for deck 2 after we
    // fail the first request.
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Pretend the track load request fails.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadFailedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend the second track load request succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, FadeToDeck2_LoadOnDeck1_TrackLoadSuccess) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the left.
    mixer.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track is loaded into deck 2 and the second track is loaded into
    // deck 1 after we fade to deck 2.
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect that we will transition into LEFT_FADING mode.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_LEFT_FADING));

    // Pretend the track is over (should trigger an instant-fade).
    deck1.playposition.set(1.0);

    EXPECT_EQ(AutoDJProcessor::ADJ_LEFT_FADING, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    // Deck is still playing, because the crossfader is processed in the next audio
    // callback.
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Fake a final callback, normally in this case the engine
    // stops the deck
    deck1.playposition.set(9.9999);
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());

    // Expect that we will transition into IDLE mode and receive a track-load
    // request for deck 1 after deck 2 starts playing.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    // Now start deck2 playing.
    deck2.playposition.set(0.1);

    // Check we are in IDLE mode, the crossfader is fully right, and deck 2 is
    // playing.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Pretend the track load request succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, FadeToDeck2_LoadOnDeck1_TrackLoadFailed) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the left.
    mixer.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // The first track is loaded into deck 2, the second track is loaded into
    // deck 1 (fails) after we fade to deck 2, and the third is loaded into deck
    // 1 (succeeds).
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // No change to the crossfader or play states.
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect that we will transition into LEFT_FADING mode.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_LEFT_FADING));

    // Pretend the track is over (should trigger an instant-fade).
    deck1.playposition.set(1.0);

    EXPECT_EQ(AutoDJProcessor::ADJ_LEFT_FADING, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    // Deck is still playing, because the crossfader is processed in the next audio
    // callback.
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Fake a final callback, normally in this case the engine
    // stops the deck
    deck1.playposition.set(9.9999);
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());

    // Expect that we will transition into IDLE mode and receive a track-load
    // request for deck 1 after deck 2 starts playing.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    // Now start deck2 playing.
    deck2.playposition.set(0.1);

    // Check we are in IDLE mode, the crossfader is fully right, and deck 2 is
    // playing.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Expect we will receive another track load request for deck 1 after we
    // fail the first request.
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    // Pretend the track load request fails.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadFailedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Pretend the track load request succeeds.
    deck1.slotLoadTrack(pTrack, false);
    deck1.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader, or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());
}


TEST_F(AutoDJProcessorTest, FadeToDeck2_Long_Transition) {
    pProcessor->setTransitionMode(AutoDJProcessor::TransitionMode::FixedFullTrack);

    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the left.
    mixer.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // Set a long transition time
    // The test tracks are only 30s
    pProcessor->setTransitionTime(60);

    // We expect that fading starts at the middle of the track
    // And Auto-DJ should keep running

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Seek track to 45 % it should not fade
    deck1.playposition.set(0.45);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());

    // Expect that we will transition into LEFT_FADING mode.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_LEFT_FADING));

    // Seek track to 55 % it should fade
    deck1.playposition.set(0.55);
    EXPECT_EQ(AutoDJProcessor::ADJ_LEFT_FADING, pProcessor->getState());

    EXPECT_LT(-1.0, mixer.crossfader.get());

    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Now start deck2 playing.
    deck2.playposition.set(0.1);

    // Seek track to End
    deck1.playposition.set(1.0);
    EXPECT_EQ(AutoDJProcessor::ADJ_LEFT_FADING, pProcessor->getState());

    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
    // Deck is still playing, because the crossfader is processed in the next audio
    // callback.
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Fake a final callback, normally in this case the engine
    // stops the deck
    deck1.playposition.set(9.9999);
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());

    // Expect that we will transition into IDLE mode and receive a track-load
    // request for deck 1 after deck 2 starts playing.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    deck2.playposition.set(0.5);
}

TEST_F(AutoDJProcessorTest, FadeToDeck2_Pause_Transition) {
    pProcessor->setTransitionMode(AutoDJProcessor::TransitionMode::FixedFullTrack);

    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the left.
    mixer.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Pretend that track is 2 minutes long.
    pTrack->setDuration(120);
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // The track should have been cued at 0.0.
    EXPECT_DOUBLE_EQ(0.0, deck2.playposition.get());

    // Pause transition. Set a negative transition time.
    pProcessor->setTransitionTime(-12);

    // changing the transition time does not re-cue the track
    // The user may has adjusted the cue-ing of the track manually in the mean time
    EXPECT_DOUBLE_EQ(0.0, deck2.playposition.get());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // The newly loaded track should have been seeked back by the trackSamples of transition.
    EXPECT_DOUBLE_EQ(-0.1, deck2.playposition.get());

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Expect that we will transition into LEFT_FADING mode.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_LEFT_FADING));

    // Seek track in deck1 to its end.
    deck1.playposition.set(1.0);

    // We should have transitioned into LEFT_FADING.
    EXPECT_EQ(AutoDJProcessor::ADJ_LEFT_FADING, pProcessor->getState());

    // Deck is still playing, because the crossfader is processed in the next audio
    // callback.
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());

    // Fake a final callback, normally in this case the engine
    // stops the deck
    deck1.play.set(0.0);

    // Expect that we will transition into IDLE mode and request a track load
    // on deck1.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), false));

    // Advance track to the point where crossfading should be over.
    deck2.playposition.set(0.0);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(1.0, mixer.crossfader.get());
}

TEST_F(AutoDJProcessorTest, FadeToDeck2_SeekEnd) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the left.
    mixer.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Seek deck 2 to the very end 99 %
    deck2.playposition.set(0.99);

    // Expect that we will transition into LEFT_FADING mode.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_LEFT_FADING));

    // Seek track to 99.9 % it should fade
    // not 100 % because the final step is done by deck2
    deck1.playposition.set(0.999);
    EXPECT_EQ(AutoDJProcessor::ADJ_LEFT_FADING, pProcessor->getState());

    EXPECT_LT(-1.0, mixer.crossfader.get());

    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(1.0, deck2.play.get());
}

TEST_F(AutoDJProcessorTest, FadeToDeck2_SeekBeforeTransition) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    // Crossfader starts on the left.
    mixer.crossfader.set(-1.0);
    // Pretend a track is playing on deck 1.
    TrackPointer pTrack = newTestTrack();
    // Load track and mark it playing.
    deck1.slotLoadTrack(pTrack, true);
    // Indicate the track loaded successfully.
    deck1.fakeTrackLoadedEvent(pTrack);

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    pAutoDJTableModel->appendTrack(testId);

    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_IDLE));
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel2]"), false));

    // Enable AutoDJ, we immediately transition into IDLE and request a track
    // load on deck2.
    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());

    // Pretend the track load succeeds.
    deck2.slotLoadTrack(pTrack, false);
    deck2.fakeTrackLoadedEvent(pTrack);

    // No change to the mode, crossfader or play states.
    EXPECT_EQ(AutoDJProcessor::ADJ_IDLE, pProcessor->getState());
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    EXPECT_DOUBLE_EQ(1.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Play "to deck" near end
    deck2.play.set(1.0);
    deck2.playposition.set(0.95);

    // Expect that we will transition into LEFT_FADING mode.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_LEFT_FADING));

    // Seek track to 99.9 % it should fade
    // not 100 % because the final step is done by deck2
    deck1.playposition.set(0.999);
    EXPECT_EQ(AutoDJProcessor::ADJ_LEFT_FADING, pProcessor->getState());

    EXPECT_LT(-1.0, mixer.crossfader.get());

    EXPECT_DOUBLE_EQ(0.999, deck1.playposition.get());
    // We expect that the "to Deck" has been seeked to the beginning"
    EXPECT_DOUBLE_EQ(0, deck2.playposition.get());
}

TEST_F(AutoDJProcessorTest, TrackZeroLength) {
    TrackId testId = addTrackToCollection(kTrackLocationTest);
    ASSERT_TRUE(testId.isValid());

    PlaylistTableModel* pAutoDJTableModel = pProcessor->getTableModel();
    // Need two tracks -- one to be loaded in the left deck and one to load in
    // the right deck.
    pAutoDJTableModel->appendTrack(testId);
    pAutoDJTableModel->appendTrack(testId);

    // Expect that we switch into ADJ_ENABLE_P1LOADED first.
    EXPECT_CALL(*pProcessor, emitAutoDJStateChanged(AutoDJProcessor::ADJ_ENABLE_P1LOADED));
    // Expect that we get a load-and-play signal for [Channel1].
    EXPECT_CALL(*pProcessor, emitLoadTrackToPlayer(_, QString("[Channel1]"), true)).Times(2);

    AutoDJProcessor::AutoDJError err = pProcessor->toggleAutoDJ(true);
    EXPECT_EQ(AutoDJProcessor::ADJ_OK, err);
    EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, pProcessor->getState());
    // Sets crossfader left and deck 1 playing.
    EXPECT_DOUBLE_EQ(-1.0, mixer.crossfader.get());
    // ADJ_ENABLE_P1LOADED logic does not set play directly. It waits for the
    // engine to load the track and set the deck playing.
    EXPECT_DOUBLE_EQ(0.0, deck1.play.get());
    EXPECT_DOUBLE_EQ(0.0, deck2.play.get());

    // Pretend to load a faulty track with Duration = 0

    // Load the track and mark it playing (as the loadTrackToPlayer signal would
    // have connected to this eventually).
    TrackPointer pTrack(newTestTrack(testId));
    pTrack->setDuration(0);
    deck1.slotLoadTrack(pTrack, true);

    // Expect that the track is rejected an a new one is loaded
    // Signal that the request to load pTrack succeeded.
    deck1.fakeTrackLoadedEvent(pTrack);
}
