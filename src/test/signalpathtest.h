#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QTest>
#include <QtDebug>
#include <memory>

#include "control/controlindicatortimer.h"
#include "control/controlobject.h"
#include "effects/effectsmanager.h"
#include "engine/bufferscalers/enginebufferscale.h"
#include "engine/channels/enginechannel.h"
#include "engine/channels/enginedeck.h"
#include "engine/controls/ratecontrol.h"
#include "engine/enginebuffer.h"
#include "engine/enginemixer.h"
#include "engine/sync/enginesync.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/previewdeck.h"
#include "mixer/sampler.h"
#include "preferences/usersettings.h"
#include "test/mixxxtest.h"
#include "test/soundsourceproviderregistration.h"
#include "track/track.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"
#ifdef __RUBBERBAND__
#include "engine/bufferscalers/rubberbandworkerpool.h"
#endif

using ::testing::Return;
using ::testing::_;

#define EXPECT_FRAMEPOS_EQ(pos1, pos2)                    \
    EXPECT_EQ((pos1).isValid(), (pos2).isValid());        \
    if ((pos1).isValid()) {                               \
        EXPECT_DOUBLE_EQ((pos1).value(), (pos2).value()); \
    }

#define EXPECT_FRAMEPOS_EQ_CONTROL(position, control)                    \
    {                                                                    \
        const auto controlPos =                                          \
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid( \
                        control->get());                                 \
        EXPECT_FRAMEPOS_EQ(position, controlPos);                        \
    }

class TestEngineMixer : public EngineMixer {
  public:
    TestEngineMixer(UserSettingsPointer _config,
            const QString& group,
            EffectsManager* pEffectsManager,
            ChannelHandleFactoryPointer pChannelHandleFactory,
            bool bEnableSidechain)
            : EngineMixer(_config,
                      group,
                      pEffectsManager,
                      pChannelHandleFactory,
                      bEnableSidechain) {
        m_pMainEnabled->forceSet(1);
        m_pHeadphoneEnabled->forceSet(1);
        m_pBoothEnabled->forceSet(1);
    }
};

class BaseSignalPathTest : public MixxxTest, SoundSourceProviderRegistration {
  protected:
    BaseSignalPathTest() {
        m_pControlIndicatorTimer = std::make_unique<mixxx::ControlIndicatorTimer>();
        m_pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();
        m_pNumDecks = new ControlObject(ConfigKey(
                QStringLiteral("[App]"), QStringLiteral("num_decks")));
        m_pEffectsManager = new EffectsManager(config(), m_pChannelHandleFactory);
        m_pEngineMixer = new TestEngineMixer(m_pConfig,
                m_sMainGroup,
                m_pEffectsManager,
                m_pChannelHandleFactory,
                false);

        m_pMixerDeck1 = new Deck(nullptr,
                m_pConfig,
                m_pEngineMixer,
                m_pEffectsManager,
                EngineChannel::CENTER,
                m_pEngineMixer->registerChannelGroup(m_sGroup1));
        m_pMixerDeck2 = new Deck(nullptr,
                m_pConfig,
                m_pEngineMixer,
                m_pEffectsManager,
                EngineChannel::CENTER,
                m_pEngineMixer->registerChannelGroup(m_sGroup2));
        m_pMixerDeck3 = new Deck(nullptr,
                m_pConfig,
                m_pEngineMixer,
                m_pEffectsManager,
                EngineChannel::CENTER,
                m_pEngineMixer->registerChannelGroup(m_sGroup3));

        m_pChannel1 = m_pMixerDeck1->getEngineDeck();
        m_pChannel2 = m_pMixerDeck2->getEngineDeck();
        m_pChannel3 = m_pMixerDeck3->getEngineDeck();
        m_pPreview1 = new PreviewDeck(nullptr,
                m_pConfig,
                m_pEngineMixer,
                m_pEffectsManager,
                EngineChannel::CENTER,
                m_pEngineMixer->registerChannelGroup(m_sPreviewGroup));
        ControlObject::set(ConfigKey(m_sPreviewGroup, "file_bpm"), 2.0);

        // TODO(owilliams) Tests fail with this turned on because EngineSync is syncing
        // to this sampler.  FIX IT!
        // m_pSampler1 = new Sampler(NULL, m_pConfig,
        //                           m_pEngineMixer, m_pEffectsManager,
        //                           EngineChannel::CENTER, m_sSamplerGroup);
        // ControlObject::getControl(ConfigKey(m_sSamplerGroup, "file_bpm"))->set(2.0);

        addDeck(m_pChannel1);
        addDeck(m_pChannel2);
        addDeck(m_pChannel3);

        m_pEngineSync = m_pEngineMixer->getEngineSync();
        ControlObject::set(ConfigKey(m_sMainGroup, "enabled"), 1.0);

        PlayerInfo::create();
    }

    ~BaseSignalPathTest() override {
        delete m_pMixerDeck1;
        delete m_pMixerDeck2;
        delete m_pMixerDeck3;
        m_pChannel1 = NULL;
        m_pChannel2 = NULL;
        m_pChannel3 = NULL;
        m_pEngineSync = NULL;
        delete m_pPreview1;

        // Deletes all EngineChannels added to it.
        delete m_pEngineMixer;
        delete m_pEffectsManager;
        delete m_pNumDecks;
        PlayerInfo::destroy();
    }

    void SetUp() override {
#ifdef __RUBBERBAND__
        RubberBandWorkerPool::createInstance();
#endif
    }

    void TearDown() override {
#ifdef __RUBBERBAND__
        RubberBandWorkerPool::destroy();
#endif
    }

    void addDeck(EngineDeck* pDeck) {
        ControlObject::set(ConfigKey(pDeck->getGroup(), "main_mix"), 1.0);
        ControlObject::set(ConfigKey(pDeck->getGroup(), "rate_dir"), kDefaultRateDir);
        ControlObject::set(ConfigKey(pDeck->getGroup(), "rateRange"), kDefaultRateRange);
        m_pNumDecks->set(m_pNumDecks->get() + 1);
    }

    void loadTrack(Deck* pDeck, TrackPointer pTrack) {
        EngineDeck* pEngineDeck = pDeck->getEngineDeck();
        if (pEngineDeck->getEngineBuffer()->isTrackLoaded()) {
            pEngineDeck->getEngineBuffer()->ejectTrack();
        }
        pDeck->slotLoadTrack(pTrack,
#ifdef __STEM__
                mixxx::StemChannelSelection(),
#endif
                false);

        // Wait for the track to load.
        ProcessBuffer();
        for (int i = 0; i < 2000; ++i) {
            if (pEngineDeck->getEngineBuffer()->isTrackLoaded()) {
                break;
            }
            QTest::qSleep(1); // sleep 1 ms for waiting 2 s at max
        }
        DEBUG_ASSERT(pEngineDeck->getEngineBuffer()->isTrackLoaded());
    }

    // Asserts that the contents of the output buffer matches a reference
    // data file where each float sample value must be within the delta to pass.
    // To create a reference file, just run the test. It will fail, but the test
    // will write out the actual buffers to the testdata/reference_buffers/
    // directory. Remove the ".actual" extension to create the file the test
    // will compare against.  On the next run, the test should pass.
    // Use tools/AudioPlot.py to look at the reference file and make sure it
    // looks correct.  Each line of the generated file contains the left sample
    // followed by the right sample.

    void assertBufferMatchesReference(std::span<const CSAMPLE> buffer,
            const QString& reference_title,
            const double delta = .0001) {
        QFile f(getTestDir().filePath(QStringLiteral("reference_buffers/") + reference_title));
        bool pass = true;
        std::size_t i = 0;
        // If the file is not there, we will fail and write out the .actual
        // reference file.
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&f);
            // Note: We will only compare as many values as there are in the reference file.
            for (; i < buffer.size() && !in.atEnd(); i += 2) {
                QStringList line = in.readLine().split(',');
                if (line.length() != 2) {
                    qWarning() << "Unexpected line length in reference file";
                    pass = false;
                    break;
                }
                bool ok = false;
                const double gold_value0 = line[0].toDouble(&ok);
                ASSERT_TRUE(ok);
                const double gold_value1 = line[1].toDouble(&ok);
                ASSERT_TRUE(ok);
                if (fabs(gold_value0 - buffer[i]) > delta) {
                    qWarning() << "Golden check failed at index" << i << ", "
                               << gold_value0 << "vs" << buffer[i];
                    pass = false;
                }
                if (fabs(gold_value1 - buffer[i + 1]) > delta) {
                    qWarning() << "Golden check failed at index" << i + 1 << ", "
                               << gold_value1 << "vs" << buffer[i + 1];
                    pass = false;
                }
            }
        }
        // Fail if either we didn't pass, or the comparison file was empty.
        if (!pass || i == 0) {
            QString fname_actual = reference_title + ".actual";
            qWarning() << "Buffer does not match" << reference_title
                       << ", actual buffer written to "
                       << "reference_buffers/" + fname_actual;
            QFile actual(getTestDir().filePath(
                    QStringLiteral("reference_buffers/") + fname_actual));
            ASSERT_TRUE(actual.open(QFile::WriteOnly | QFile::Text));
            QTextStream out(&actual);
            for (std::size_t i = 0; i < buffer.size(); i += 2) {
                out << QString("%1,%2\n").arg(buffer[i]).arg(buffer[i + 1]);
            }
            actual.close();
            EXPECT_TRUE(false);
        }
        f.close();
    }

    double getRateSliderValue(double rate) const {
        return (rate - 1.0) / kRateRangeDivisor;
    }

    void ProcessBuffer() {
        qDebug() << "------- Process Buffer -------";
        m_pEngineMixer->process(kProcessBufferSize, std::chrono::microseconds(0));
    }

    ChannelHandleFactoryPointer m_pChannelHandleFactory;
    ControlObject* m_pNumDecks;
    std::unique_ptr<mixxx::ControlIndicatorTimer> m_pControlIndicatorTimer;
    EffectsManager* m_pEffectsManager;
    EngineSync* m_pEngineSync;
    TestEngineMixer* m_pEngineMixer;
    Deck *m_pMixerDeck1, *m_pMixerDeck2, *m_pMixerDeck3;
    EngineDeck *m_pChannel1, *m_pChannel2, *m_pChannel3;
    PreviewDeck* m_pPreview1;

    static const QString m_sMainGroup;
    static const QString m_sInternalClockGroup;
    static const QString m_sGroup1;
    static const QString m_sGroup2;
    static const QString m_sGroup3;
    static const QString m_sPreviewGroup;
    static const QString m_sSamplerGroup;
    static const double kDefaultRateRange;
    static const double kDefaultRateDir;
    static const double kRateRangeDivisor;
    static const int kProcessBufferSize;
};

class SignalPathTest : public BaseSignalPathTest {
  protected:
    void SetUp() override {
        BaseSignalPathTest::SetUp();
        const QString kTrackLocationTest = getTestDir().filePath(QStringLiteral("sine-30.wav"));
        TrackPointer pTrack(Track::newTemporary(kTrackLocationTest));

        loadTrack(m_pMixerDeck1, pTrack);
        loadTrack(m_pMixerDeck2, pTrack);
        loadTrack(m_pMixerDeck3, pTrack);
    }

    void TearDown() override {
        BaseSignalPathTest::TearDown();
    }
};
