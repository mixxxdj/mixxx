#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QTest>
#include <QtDebug>

#include "control/controlobject.h"
#include "effects/effectsmanager.h"
#include "engine/bufferscalers/enginebufferscale.h"
#include "engine/channels/enginechannel.h"
#include "engine/channels/enginedeck.h"
#include "engine/controls/ratecontrol.h"
#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "engine/sync/enginesync.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/previewdeck.h"
#include "mixer/sampler.h"
#include "preferences/usersettings.h"
#include "test/mixxxtest.h"
#include "track/track.h"
#include "util/defs.h"
#include "util/memory.h"
#include "util/sample.h"
#include "util/types.h"
#include "waveform/guitick.h"
#include "waveform/visualsmanager.h"

using ::testing::Return;
using ::testing::_;

// Subclass of EngineMaster that provides access to the master buffer object
// for comparison.
class TestEngineMaster : public EngineMaster {
  public:
    TestEngineMaster(UserSettingsPointer _config,
            const QString& group,
            EffectsManager* pEffectsManager,
            ChannelHandleFactoryPointer pChannelHandleFactory,
            bool bEnableSidechain)
            : EngineMaster(_config,
                      group,
                      pEffectsManager,
                      pChannelHandleFactory,
                      bEnableSidechain) {
        m_pMasterEnabled->forceSet(1);
        m_pHeadphoneEnabled->forceSet(1);
        m_pBoothEnabled->forceSet(1);
    }

    CSAMPLE* masterBuffer() {
        return m_pMaster;
    }
};

class BaseSignalPathTest : public MixxxTest {
  protected:
    BaseSignalPathTest() {
        m_pGuiTick = std::make_unique<GuiTick>();
        m_pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();
        m_pNumDecks = new ControlObject(ConfigKey("[Master]", "num_decks"));
        m_pEffectsManager = new EffectsManager(NULL, config(), m_pChannelHandleFactory);
        m_pVisualsManager = new VisualsManager();
        m_pEngineMaster = new TestEngineMaster(m_pConfig, "[Master]",
                                               m_pEffectsManager, m_pChannelHandleFactory,
                                               false);

        m_pMixerDeck1 = new Deck(NULL, m_pConfig, m_pEngineMaster, m_pEffectsManager,
                m_pVisualsManager, EngineChannel::CENTER, m_sGroup1);
        m_pMixerDeck2 = new Deck(NULL, m_pConfig, m_pEngineMaster, m_pEffectsManager,
                m_pVisualsManager, EngineChannel::CENTER, m_sGroup2);
        m_pMixerDeck3 = new Deck(NULL, m_pConfig, m_pEngineMaster, m_pEffectsManager,
                m_pVisualsManager, EngineChannel::CENTER, m_sGroup3);

        m_pChannel1 = m_pMixerDeck1->getEngineDeck();
        m_pChannel2 = m_pMixerDeck2->getEngineDeck();
        m_pChannel3 = m_pMixerDeck3->getEngineDeck();
        m_pPreview1 = new PreviewDeck(NULL, m_pConfig, m_pEngineMaster, m_pEffectsManager,
                m_pVisualsManager, EngineChannel::CENTER, m_sPreviewGroup);
        ControlObject::set(ConfigKey(m_sPreviewGroup, "file_bpm"), 2.0);

        // TODO(owilliams) Tests fail with this turned on because EngineSync is syncing
        // to this sampler.  FIX IT!
        // m_pSampler1 = new Sampler(NULL, m_pConfig,
        //                           m_pEngineMaster, m_pEffectsManager,
        //                           EngineChannel::CENTER, m_sSamplerGroup);
        // ControlObject::getControl(ConfigKey(m_sSamplerGroup, "file_bpm"))->set(2.0);

        addDeck(m_pChannel1);
        addDeck(m_pChannel2);
        addDeck(m_pChannel3);

        m_pEngineSync = m_pEngineMaster->getEngineSync();
        ControlObject::set(ConfigKey("[Master]", "enabled"), 1.0);

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
        delete m_pEngineMaster;
        delete m_pEffectsManager;
        delete m_pVisualsManager;
        delete m_pNumDecks;
        PlayerInfo::destroy();
    }

    void addDeck(EngineDeck* pDeck) {
        ControlObject::set(ConfigKey(pDeck->getGroup(), "master"), 1.0);
        ControlObject::set(ConfigKey(pDeck->getGroup(), "rate_dir"), kDefaultRateDir);
        ControlObject::set(ConfigKey(pDeck->getGroup(), "rateRange"), kDefaultRateRange);
        m_pNumDecks->set(m_pNumDecks->get() + 1);
    }

    void loadTrack(Deck* pDeck, TrackPointer pTrack) {
        pDeck->slotLoadTrack(pTrack, false);

        // Wait for the track to load.
        ProcessBuffer();
        EngineDeck* pEngineDeck = pDeck->getEngineDeck();
        while (!pEngineDeck->getEngineBuffer()->isTrackLoaded()) {
            QTest::qSleep(1); // millis
        }
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
    void assertBufferMatchesReference(const CSAMPLE* pBuffer, const int iBufferSize,
                                   QString reference_title, const double delta=.0001) {
        QFile f(QDir::currentPath() + "/src/test/reference_buffers/" + reference_title);
        bool pass = true;
        int i = 0;
        // If the file is not there, we will fail and write out the .actual
        // reference file.
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&f);
            // Note: We will only compare as many values as there are in the reference file.
            for (; i < iBufferSize && !in.atEnd(); i += 2) {
                QStringList line = in.readLine().split(',');
                if (line.length() != 2) {
                    qWarning() << "Unexpected line length in reference file";
                    pass = false;
                    break;
                }
                bool ok = false;
                double gold_value0 = line[0].toDouble(&ok);
                double gold_value1 = line[1].toDouble(&ok);
                EXPECT_TRUE(ok);
                if (fabs(gold_value0 - pBuffer[i]) > delta) {
                    qWarning() << "Golden check failed at index" << i << ", "
                               << gold_value0 << "vs" << pBuffer[i];
                    pass = false;
                }
                if (fabs(gold_value1 - pBuffer[i + 1]) > delta) {
                    qWarning() << "Golden check failed at index" << i + 1 << ", "
                               << gold_value1 << "vs" << pBuffer[i + 1];
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
            QFile actual(QDir::currentPath() + "/src/test/reference_buffers/" + fname_actual);
            ASSERT_TRUE(actual.open(QFile::WriteOnly | QFile::Text));
            QTextStream out(&actual);
            for (int i = 0; i < iBufferSize; i += 2) {
                out << QString("%1,%2\n").arg(pBuffer[i]).arg(pBuffer[i + 1]);
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
        m_pEngineMaster->process(kProcessBufferSize);
    }

    ChannelHandleFactoryPointer m_pChannelHandleFactory;
    ControlObject* m_pNumDecks;
    std::unique_ptr<GuiTick> m_pGuiTick;
    VisualsManager* m_pVisualsManager;
    EffectsManager* m_pEffectsManager;
    EngineSync* m_pEngineSync;
    TestEngineMaster* m_pEngineMaster;
    Deck *m_pMixerDeck1, *m_pMixerDeck2, *m_pMixerDeck3;
    EngineDeck *m_pChannel1, *m_pChannel2, *m_pChannel3;
    PreviewDeck* m_pPreview1;

    static const QString m_sGroup1;
    static const QString m_sGroup2;
    static const QString m_sGroup3;
    static const QString m_sMasterGroup;
    static const QString m_sInternalClockGroup;
    static const QString m_sPreviewGroup;
    static const QString m_sSamplerGroup;
    static const double kDefaultRateRange;
    static const double kDefaultRateDir;
    static const double kRateRangeDivisor;
    static const int kProcessBufferSize;
};

class SignalPathTest : public BaseSignalPathTest {
  protected:
    SignalPathTest() {
        const QString kTrackLocationTest = QDir::currentPath() + "/src/test/sine-30.wav";
        TrackPointer pTrack(Track::newTemporary(kTrackLocationTest));

        loadTrack(m_pMixerDeck1, pTrack);
        loadTrack(m_pMixerDeck2, pTrack);
        loadTrack(m_pMixerDeck3, pTrack);
    }
};
