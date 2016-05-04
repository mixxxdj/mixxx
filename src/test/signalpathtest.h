#ifndef ENGINEBACKENDTEST_H_
#define ENGINEBACKENDTEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>
#include <QTest>

#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "mixer/deck.h"
#include "effects/effectsmanager.h"
#include "engine/enginebuffer.h"
#include "engine/enginebufferscale.h"
#include "engine/enginechannel.h"
#include "engine/enginedeck.h"
#include "engine/enginemaster.h"
#include "engine/ratecontrol.h"
#include "engine/sync/enginesync.h"
#include "mixer/previewdeck.h"
#include "mixer/sampler.h"
#include "test/mixxxtest.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

using ::testing::Return;
using ::testing::_;

// Subclass of EngineMaster that provides access to the master buffer object
// for comparison.
class TestEngineMaster : public EngineMaster {
  public:
    TestEngineMaster(UserSettingsPointer _config,
                     const char* group,
                     EffectsManager* pEffectsManager,
                     bool bEnableSidechain,
                     bool bRampingGain)
        : EngineMaster(_config, group, pEffectsManager,
                       bEnableSidechain, bRampingGain) { }

    CSAMPLE* masterBuffer() {
        return m_pMaster;
    }
};

// TODO: This class has a lot in common with mockedenginebuffer test and they
// both could be refactored to share code.
class SignalPathTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pNumDecks = new ControlObject(ConfigKey("[Master]", "num_decks"));
        m_pEffectsManager = new EffectsManager(NULL, config());
        m_pEngineMaster = new TestEngineMaster(m_pConfig, "[Master]",
                                               m_pEffectsManager, false, false);

        m_pChannel1 = new EngineDeck(
                m_pEngineMaster->registerChannelGroup(m_sGroup1),
                m_pConfig, m_pEngineMaster, m_pEffectsManager,
                EngineChannel::CENTER);
        m_pChannel2 = new EngineDeck(
                m_pEngineMaster->registerChannelGroup(m_sGroup2),
                m_pConfig, m_pEngineMaster, m_pEffectsManager,
                EngineChannel::CENTER);
        m_pChannel3 = new EngineDeck(
                m_pEngineMaster->registerChannelGroup(m_sGroup3),
                m_pConfig, m_pEngineMaster, m_pEffectsManager,
                EngineChannel::CENTER);
        m_pPreview1 = new PreviewDeck(NULL, m_pConfig,
                                     m_pEngineMaster, m_pEffectsManager,
                                     EngineChannel::CENTER, m_sPreviewGroup);
        ControlObject::getControl(ConfigKey(m_sPreviewGroup, "file_bpm"))->set(2.0);

        addDeck(m_pChannel1);
        addDeck(m_pChannel2);
        addDeck(m_pChannel3);

        m_pEngineSync = m_pEngineMaster->getEngineSync();

        loadTrack(m_pChannel1, QDir::currentPath() + "/src/test/sine-30.wav");
        loadTrack(m_pChannel2, QDir::currentPath() + "/src/test/sine-30.wav");
        loadTrack(m_pChannel3, QDir::currentPath() + "/src/test/sine-30.wav");

        ControlObject::set(ConfigKey("[Master]", "enabled"), 1.0);
    }

    void addDeck(EngineDeck* pDeck) {
        m_pEngineMaster->addChannel(pDeck);
        ControlObject::getControl(ConfigKey(pDeck->getGroup(), "master"))
                ->set(1.0);
        ControlObject::getControl(ConfigKey(pDeck->getGroup(), "rate_dir"))
                ->set(kDefaultRateDir);
        ControlObject::getControl(ConfigKey(pDeck->getGroup(), "rateRange"))
                ->set(kDefaultRateRange);
        m_pNumDecks->set(m_pNumDecks->get() + 1);
    }

    void loadTrack(EngineDeck* pDeck, QString path) {
        const QString kTrackLocationTest(path);
        TrackPointer pTrack(Track::newTemporary(kTrackLocationTest));
        pDeck->getEngineBuffer()->loadTrack(pTrack, true);

        // Wait for the track to load.
        ProcessBuffer();
        for (int i = 0; i < 10 && !pDeck->getEngineBuffer()->isTrackLoaded();
                ++i) {
            QTest::qSleep(1000); // millis
        }
        ASSERT_TRUE(pDeck->getEngineBuffer()->isTrackLoaded());
        // For some reason the tracks play by default.  Turn them off.
        ControlObject::set(ConfigKey(pDeck->getGroup(), "play"), 0.0);
    }

    // Asserts that the contents of the output buffer matches a golden example
    // data file where each float sample value must be within the delta to pass.
    // To create a golden file, just run the test. It will fail, but the test
    // will write out the actual buffers to the testdata/golden_buffers/
    // directory. Remove the ".actual" extension to create the file the test
    // will compare against.  On the next run, the test should pass.
    // Use scripts/AudioPlot.py to look at the golden file and make sure it
    // looks correct.  Each line of the generated file contains the left sample
    // followed by the right sample.
    void assertBufferMatchesGolden(CSAMPLE* pBuffer, const int iBufferSize,
                                   QString golden_title, const double delta=.0001) {
        QFile f(QDir::currentPath() + "/src/test/golden_buffers/" + golden_title);
        bool pass = true;
        int i = 0;
        // If the file is not there, we will fail and write out the .actual
        // golden file.
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&f);
            // Note: We will only compare as many values as there are in the golden file.
            for (; i < iBufferSize && !in.atEnd(); i += 2) {
                QStringList line = in.readLine().split(',');
                if (line.length() != 2) {
                    qWarning() << "Unexpected line length in golden file";
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
            QString fname_actual = golden_title + ".actual";
            qWarning() << "Buffer does not match" << golden_title
                       << ", actual buffer written to "
                       << "golden_buffers/" + fname_actual;
            QFile actual(QDir::currentPath() + "/src/test/golden_buffers/" + fname_actual);
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

    virtual void TearDown() {
        m_pChannel1 = NULL;
        m_pChannel2 = NULL;
        m_pChannel3 = NULL;
        m_pEngineSync = NULL;

        // Deletes all EngineChannels added to it.
        delete m_pEngineMaster;
        delete m_pEffectsManager;
        delete m_pNumDecks;
    }

    double getRateSliderValue(double rate) const {
        return (rate - 1.0) / kRateRangeDivisor;
    }

    void ProcessBuffer() {
        m_pEngineMaster->process(kProcessBufferSize);
    }

    ControlObject* m_pNumDecks;

    EffectsManager* m_pEffectsManager;
    EngineSync* m_pEngineSync;
    TestEngineMaster* m_pEngineMaster;
    EngineDeck *m_pChannel1, *m_pChannel2, *m_pChannel3;
    TrackPointer m_pTrack1, m_pTrack2, m_pTrack3;
    PreviewDeck *m_pPreview1;
    Sampler *m_pSampler1;

    static const char* m_sGroup1;
    static const char* m_sGroup2;
    static const char* m_sGroup3;
    static const char* m_sMasterGroup;
    static const char* m_sInternalClockGroup;
    static const char* m_sPreviewGroup;
    static const char* m_sSamplerGroup;
    static const double kDefaultRateRange;
    static const double kDefaultRateDir;
    static const double kRateRangeDivisor;
    static const int kProcessBufferSize;
};

#endif /* ENGINEBACKENDTEST_H_ */
