#include <gtest/gtest.h>

#include <QScopedPointer>
#include <QtDebug>
#include <memory>

#include "control/pollingcontrolproxy.h"
#include "mixxxtest.h"
#include "test/signalpathtest.h"

class StemControlTest : public BaseSignalPathTest {
  protected:
    QString getGroupForStem(QStringView deckGroup, int stemNr) {
        DEBUG_ASSERT(deckGroup.endsWith(QChar(']')) && stemNr <= 4);
        return deckGroup.chopped(1) + QStringLiteral("_Stem") + QChar('0' + stemNr) + QChar(']');
    }
    QString getFxGroupForStem(const QString& deckGroup, int stemNr) {
        return QStringLiteral("[QuickEffectRack1_%1]")
                .arg(getGroupForStem(deckGroup, stemNr));
    }

    void SetUp() override {
        BaseSignalPathTest::SetUp();

        for (int i = 1; i <= 4; i++) {
            ChannelHandleAndGroup stemHandleGroup =
                    m_pEngineMixer->registerChannelGroup(getGroupForStem(m_sGroup1, i));
            m_pChannel1->addStemHandle(stemHandleGroup);
            m_pEffectsManager->addStem(stemHandleGroup);
        }
        for (int i = 1; i <= 4; i++) {
            ChannelHandleAndGroup stemHandleGroup =
                    m_pEngineMixer->registerChannelGroup(getGroupForStem(m_sGroup2, i));
            m_pChannel2->addStemHandle(stemHandleGroup);
            m_pEffectsManager->addStem(stemHandleGroup);
        }
        for (int i = 1; i <= 4; i++) {
            ChannelHandleAndGroup stemHandleGroup =
                    m_pEngineMixer->registerChannelGroup(getGroupForStem(m_sGroup3, i));
            m_pChannel3->addStemHandle(stemHandleGroup);
            m_pEffectsManager->addStem(stemHandleGroup);
        }

        const QString kStemFileLocationTest = getTestDir().filePath("stems/test.stem.mp4");
        TrackPointer pStemFile(Track::newTemporary(kStemFileLocationTest));

        loadTrack(m_pMixerDeck1, pStemFile);
        loadTrack(m_pMixerDeck3, pStemFile);

        m_pPlay = std::make_unique<PollingControlProxy>(m_sGroup1, "play");

        m_pStem1Volume = std::make_unique<PollingControlProxy>(
                getGroupForStem(m_sGroup1, 1), "volume");
        m_pStem2Volume = std::make_unique<PollingControlProxy>(
                getGroupForStem(m_sGroup1, 2), "volume");
        m_pStem3Volume = std::make_unique<PollingControlProxy>(
                getGroupForStem(m_sGroup1, 3), "volume");
        m_pStem4Volume = std::make_unique<PollingControlProxy>(
                getGroupForStem(m_sGroup1, 4), "volume");
        m_pStem1Mute = std::make_unique<PollingControlProxy>(getGroupForStem(m_sGroup1, 1), "mute");
        m_pStem2Mute = std::make_unique<PollingControlProxy>(getGroupForStem(m_sGroup1, 2), "mute");
        m_pStem3Mute = std::make_unique<PollingControlProxy>(getGroupForStem(m_sGroup1, 3), "mute");
        m_pStem4Mute = std::make_unique<PollingControlProxy>(getGroupForStem(m_sGroup1, 4), "mute");
        m_pStem1Color = std::make_unique<PollingControlProxy>(
                getGroupForStem(m_sGroup1, 1), "color");
        m_pStem2Color = std::make_unique<PollingControlProxy>(
                getGroupForStem(m_sGroup1, 2), "color");
        m_pStem3Color = std::make_unique<PollingControlProxy>(
                getGroupForStem(m_sGroup1, 3), "color");
        m_pStem4Color = std::make_unique<PollingControlProxy>(
                getGroupForStem(m_sGroup1, 4), "color");
        m_pStem1FXEnabled = std::make_unique<PollingControlProxy>(
                getFxGroupForStem(m_sGroup1, 1), "enabled");
        m_pStem2FXEnabled = std::make_unique<PollingControlProxy>(
                getFxGroupForStem(m_sGroup1, 2), "enabled");
        m_pStem3FXEnabled = std::make_unique<PollingControlProxy>(
                getFxGroupForStem(m_sGroup1, 3), "enabled");
        m_pStem4FXEnabled = std::make_unique<PollingControlProxy>(
                getFxGroupForStem(m_sGroup1, 4), "enabled");

        m_pStem1FXEnabled->set(0.0);
        m_pStem2FXEnabled->set(0.0);
        m_pStem3FXEnabled->set(0.0);
        m_pStem4FXEnabled->set(0.0);

        m_pStemCount = std::make_unique<PollingControlProxy>(m_sGroup1, "stem_count");
    }

    void setCurrentPosition(mixxx::audio::FramePos position) {
        m_pChannel1->getEngineBuffer()->queueNewPlaypos(position, EngineBuffer::SEEK_STANDARD);
        ProcessBuffer();
    }

    void loadTrack(Deck* pDeck, TrackPointer pTrack) {
        // Because there is connection across the main thread in caching reader
        // thread, we need to manually process the Qt event loop to trigger
        // `BaseTrackPlayerImpl::slotTrackLoaded` Here is the chain of
        // connections (Symbol (thread)) EngineDeck::slotLoadTrack (main) ->
        // EngineBuffer::loadTrack (main) -> CachingReader*::newTrack (main) ->
        // CachingReaderWorker::trackLoaded (CachingReader) ->
        // EngineBuffer::loaded (CachingReader, direct) ->
        // BaseTrackPlayerImpl::slotTrackLoaded  (main)

        TrackPointer pLoadedTrack;
        QMetaObject::Connection connection = QObject::connect(pDeck,
                &BaseTrackPlayerImpl::newTrackLoaded,
                [&pLoadedTrack]( // clazy:exclude=lambda-in-connect
                        TrackPointer pNewTrack) { pLoadedTrack = pNewTrack; });
        BaseSignalPathTest::loadTrack(pDeck, pTrack);

        for (int i = 0; i < 10000; ++i) {
            if (pLoadedTrack == pTrack) {
                break;
            }
            int maxtime = 1; // ms
            QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, maxtime);
            // 1 ms for waiting 10 s at max
        }
        QObject::disconnect(connection);
        if (pLoadedTrack != pTrack) {
            qWarning() << "Timeout: failed loading track" << pTrack->getLocation();
        }
    }

    std::unique_ptr<PollingControlProxy> m_pPlay;
    std::unique_ptr<PollingControlProxy> m_pStem1Volume;
    std::unique_ptr<PollingControlProxy> m_pStem2Volume;
    std::unique_ptr<PollingControlProxy> m_pStem3Volume;
    std::unique_ptr<PollingControlProxy> m_pStem4Volume;
    std::unique_ptr<PollingControlProxy> m_pStem1Mute;
    std::unique_ptr<PollingControlProxy> m_pStem2Mute;
    std::unique_ptr<PollingControlProxy> m_pStem3Mute;
    std::unique_ptr<PollingControlProxy> m_pStem4Mute;
    std::unique_ptr<PollingControlProxy> m_pStem1Color;
    std::unique_ptr<PollingControlProxy> m_pStem2Color;
    std::unique_ptr<PollingControlProxy> m_pStem3Color;
    std::unique_ptr<PollingControlProxy> m_pStem4Color;
    std::unique_ptr<PollingControlProxy> m_pStem1FXEnabled;
    std::unique_ptr<PollingControlProxy> m_pStem2FXEnabled;
    std::unique_ptr<PollingControlProxy> m_pStem3FXEnabled;
    std::unique_ptr<PollingControlProxy> m_pStem4FXEnabled;
    std::unique_ptr<PollingControlProxy> m_pStemCount;
};

TEST_F(StemControlTest, StemCount) {
    EXPECT_EQ(m_pStemCount->get(), 4.0);

    QString kTrackLocationTest = getTestDir().filePath(QStringLiteral("sine-30.wav"));
    TrackPointer pTrack(Track::newTemporary(kTrackLocationTest));
    loadTrack(m_pMixerDeck1, pTrack);

    EXPECT_EQ(m_pStemCount->get(), 0.0);

    kTrackLocationTest = getTestDir().filePath("stems/test.stem.mp4");
    pTrack = Track::newTemporary(kTrackLocationTest);
    loadTrack(m_pMixerDeck1, pTrack);

    EXPECT_EQ(m_pStemCount->get(), 4.0);
}

TEST_F(StemControlTest, StemColor) {
    EXPECT_EQ(m_pStem1Color->get(), 0xfd << 16 | 0x4a << 8 | 0x4a);
    EXPECT_EQ(m_pStem2Color->get(), 0xff << 16 | 0xff << 8 | 0x00);
    EXPECT_EQ(m_pStem3Color->get(), 0x00 << 16 | 0xe8 << 8 | 0xe8);
    EXPECT_EQ(m_pStem4Color->get(), 0xad << 16 | 0x65 << 8 | 0xff);

    QString kTrackLocationTest = getTestDir().filePath(QStringLiteral("sine-30.wav"));
    TrackPointer pTrack(Track::newTemporary(kTrackLocationTest));
    loadTrack(m_pMixerDeck1, pTrack);

    EXPECT_EQ(m_pStem1Color->get(), -1.0);
    EXPECT_EQ(m_pStem2Color->get(), -1.0);
    EXPECT_EQ(m_pStem3Color->get(), -1.0);
    EXPECT_EQ(m_pStem4Color->get(), -1.0);

    kTrackLocationTest = getTestDir().filePath("stems/test.stem.mp4");
    pTrack = Track::newTemporary(kTrackLocationTest);
    loadTrack(m_pMixerDeck1, pTrack);

    EXPECT_EQ(m_pStem1Color->get(), 0xfd << 16 | 0x4a << 8 | 0x4a);
    EXPECT_EQ(m_pStem2Color->get(), 0xff << 16 | 0xff << 8 | 0x00);
    EXPECT_EQ(m_pStem3Color->get(), 0x00 << 16 | 0xe8 << 8 | 0xe8);
    EXPECT_EQ(m_pStem4Color->get(), 0xad << 16 | 0x65 << 8 | 0xff);
}

TEST_F(StemControlTest, Volume) {
    m_pChannel1->getEngineBuffer()->queueNewPlaypos(
            mixxx::audio::FramePos{0}, EngineBuffer::SEEK_STANDARD);
    m_pPlay->set(1.0);
    m_pStem1Volume->set(0.0);
    m_pStem2Volume->set(0.0);
    m_pStem3Volume->set(0.0);
    m_pStem4Volume->set(0.0);

    // Proceed the buffer a first time to proceed the ramping gain
    m_pEngineMixer->process(kProcessBufferSize);
    m_pEngineMixer->process(kProcessBufferSize);
    assertBufferMatchesReference(m_pEngineMixer->getMainBuffer(),
            QStringLiteral("StemVolumeControlSilence"));

    m_pChannel1->getEngineBuffer()->queueNewPlaypos(
            mixxx::audio::FramePos{0}, EngineBuffer::SEEK_STANDARD);
    m_pStem1Volume->set(1.0);

    // Proceed the buffer a first time to proceed the ramping gain
    m_pEngineMixer->process(kProcessBufferSize);
    m_pEngineMixer->process(kProcessBufferSize);
    assertBufferMatchesReference(m_pEngineMixer->getMainBuffer(),
            QStringLiteral("StemVolumeControlDrumOnly"));

    m_pChannel1->getEngineBuffer()->queueNewPlaypos(
            mixxx::audio::FramePos{0}, EngineBuffer::SEEK_STANDARD);
    m_pStem2Volume->set(0.8);

    // Proceed the buffer a first time to proceed the ramping gain
    m_pEngineMixer->process(kProcessBufferSize);
    m_pEngineMixer->process(kProcessBufferSize);
    assertBufferMatchesReference(m_pEngineMixer->getMainBuffer(),
            QStringLiteral("StemVolumeControlDrumAndBass"));

    m_pChannel1->getEngineBuffer()->queueNewPlaypos(
            mixxx::audio::FramePos{0}, EngineBuffer::SEEK_STANDARD);
    m_pStem1Volume->set(0.5);
    m_pStem3Volume->set(0.2);
    m_pStem4Volume->set(0.4);

    // Proceed the buffer a first time to proceed the ramping gain
    m_pEngineMixer->process(kProcessBufferSize);
    m_pEngineMixer->process(kProcessBufferSize);
    assertBufferMatchesReference(m_pEngineMixer->getMainBuffer(),
            QStringLiteral("StemVolumeControlFull"));
}

TEST_F(StemControlTest, VolumeResetOnLoad) {
    m_pStem1Volume->set(0.1);
    m_pStem2Volume->set(0.2);
    m_pStem3Volume->set(0.3);
    m_pStem4Volume->set(0.4);
    m_pStem1Mute->set(1.0);
    m_pStem2Mute->set(1.0);
    m_pStem3Mute->set(0.0);
    m_pStem4Mute->set(1.0);
    m_pConfig->setValue(
            ConfigKey("[Mixer Profile]", "stem_auto_reset"), false);

    QString kTrackLocationTest = getTestDir().filePath(QStringLiteral("sine-30.wav"));
    TrackPointer pTrack(Track::newTemporary(kTrackLocationTest));
    loadTrack(m_pMixerDeck1, pTrack);

    EXPECT_EQ(m_pStem1Volume->get(), 0.1);
    EXPECT_EQ(m_pStem2Volume->get(), 0.2);
    EXPECT_EQ(m_pStem3Volume->get(), 0.3);
    EXPECT_EQ(m_pStem4Volume->get(), 0.4);
    EXPECT_EQ(m_pStem1Mute->get(), 1.0);
    EXPECT_EQ(m_pStem2Mute->get(), 1.0);
    EXPECT_EQ(m_pStem3Mute->get(), 0.0);
    EXPECT_EQ(m_pStem4Mute->get(), 1.0);

    m_pConfig->setValue(
            ConfigKey("[Mixer Profile]", "stem_auto_reset"), true);
    loadTrack(m_pMixerDeck1, pTrack);

    EXPECT_EQ(m_pStem1Volume->get(), 1.0);
    EXPECT_EQ(m_pStem2Volume->get(), 1.0);
    EXPECT_EQ(m_pStem3Volume->get(), 1.0);
    EXPECT_EQ(m_pStem4Volume->get(), 1.0);
    EXPECT_EQ(m_pStem1Mute->get(), 0.0);
    EXPECT_EQ(m_pStem2Mute->get(), 0.0);
    EXPECT_EQ(m_pStem3Mute->get(), 0.0);
    EXPECT_EQ(m_pStem4Mute->get(), 0.0);
}

TEST_F(StemControlTest, Mute) {
    m_pChannel1->getEngineBuffer()->queueNewPlaypos(
            mixxx::audio::FramePos{0}, EngineBuffer::SEEK_STANDARD);
    m_pPlay->set(1.0);
    m_pStem1Mute->set(1.0);
    m_pStem2Mute->set(1.0);
    m_pStem3Mute->set(1.0);
    m_pStem4Mute->set(1.0);

    // Proceed the buffer a first time to proceed the ramping gain
    m_pEngineMixer->process(kProcessBufferSize);
    m_pEngineMixer->process(kProcessBufferSize);
    assertBufferMatchesReference(m_pEngineMixer->getMainBuffer(),
            QStringLiteral("StemVolumeControlSilence")); // Same than volume test

    m_pChannel1->getEngineBuffer()->queueNewPlaypos(
            mixxx::audio::FramePos{0}, EngineBuffer::SEEK_STANDARD);
    m_pStem1Mute->set(0.0);

    // Proceed the buffer a first time to proceed the ramping gain
    m_pEngineMixer->process(kProcessBufferSize);
    m_pEngineMixer->process(kProcessBufferSize);
    assertBufferMatchesReference(m_pEngineMixer->getMainBuffer(),
            QStringLiteral("StemVolumeControlDrumOnly")); // Same than volume test

    m_pChannel1->getEngineBuffer()->queueNewPlaypos(
            mixxx::audio::FramePos{0}, EngineBuffer::SEEK_STANDARD);
    m_pStem2Mute->set(0.0);

    // Proceed the buffer a first time to proceed the ramping gain
    m_pEngineMixer->process(kProcessBufferSize);
    m_pEngineMixer->process(kProcessBufferSize);
    assertBufferMatchesReference(m_pEngineMixer->getMainBuffer(),
            QStringLiteral("StemMuteControlDrumAndBass"));

    m_pChannel1->getEngineBuffer()->queueNewPlaypos(
            mixxx::audio::FramePos{0}, EngineBuffer::SEEK_STANDARD);
    m_pStem3Mute->set(0.0);
    m_pStem4Mute->set(0.0);

    // Proceed the buffer a first time to proceed the ramping gain
    m_pEngineMixer->process(kProcessBufferSize);
    m_pEngineMixer->process(kProcessBufferSize);
    assertBufferMatchesReference(m_pEngineMixer->getMainBuffer(),
            QStringLiteral("StemMuteControlFull"));
}
