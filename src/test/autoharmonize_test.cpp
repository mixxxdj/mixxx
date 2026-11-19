#include <gtest/gtest.h>

#include <QCheckBox>

#include "control/controlobject.h"
#include "library/library_prefs.h"
#include "mixer/basetrackplayer.h"
#include "preferences/dialog/dlgprefautodj.h"
#include "preferences/dialog/dlgprefkey.h"
#include "preferences/keydetectionsettings.h"
#include "proto/keys.pb.h"
#include "test/mockedenginebackendtest.h"
#include "test/mixxxtest.h"
#include "track/keyutils.h"

namespace {

const ConfigKey kAutoDjHarmonizeKey(QStringLiteral("[Auto DJ]"),
        QStringLiteral("AutoHarmonizeToPrevious"));
const ConfigKey kManualHarmonizeKey(QStringLiteral("[Key]"),
        QStringLiteral("AutoHarmonizeOnManualLoad"));

const mixxx::track::io::key::ChromaticKey kLoadedKey =
        mixxx::track::io::key::F_SHARP_MAJOR;
const mixxx::track::io::key::ChromaticKey kOtherKey =
        mixxx::track::io::key::C_MAJOR;

double compatibleOffset(mixxx::track::io::key::ChromaticKey fileKey,
        mixxx::track::io::key::ChromaticKey otherKey,
        double otherDistance) {
    return static_cast<double>(KeyUtils::shortestStepsToCompatibleKey(fileKey, otherKey)) +
            otherDistance;
}

// Publish an effective key on a real deck the same way KeyControl does when
// its pitch control moves, then read back the key and visual distance sync_key
// would use. The expected pitch is computed from those readbacks.
struct OtherDeckKey {
    mixxx::track::io::key::ChromaticKey key;
    double distance;
};

OtherDeckKey publishOtherDeckKey(const QString& group, double pitchSemitones) {
    ControlObject::set(ConfigKey(group, "file_key"), KeyUtils::keyToNumericValue(kOtherKey));
    ControlObject::set(ConfigKey(group, "pitch"), pitchSemitones);
    OtherDeckKey result;
    result.key = KeyUtils::keyFromNumericValue(ControlObject::get(ConfigKey(group, "key")));
    result.distance = ControlObject::get(ConfigKey(group, "visual_key_distance"));
    return result;
}

} // namespace

class AutoHarmonizeManualLoadTest : public MockedEngineBackendTest {
  protected:
    void SetUp() override {
        MockedEngineBackendTest::SetUp();
        config()->setValue(ConfigKey("[Controls]", "SpeedAutoReset"),
                BaseTrackPlayer::TrackLoadReset::RESET_NONE);
    }

    void setOptions(bool manual, bool autoDj) {
        config()->setValue(kManualHarmonizeKey, manual);
        config()->setValue(kAutoDjHarmonizeKey, autoDj);
    }

    struct LoadResult {
        // Pitch after the pre-load write, once KeyControl has absorbed it,
        // and before slotTrackLoaded. Harmonize is the only later pitch write
        // when SpeedAutoReset is RESET_NONE and the file key does not change.
        double pitchBefore;
        double pitch;
    };

    // Calls the shipped main-deck load path (slotTrackLoaded) after the file
    // key is on the track.
    LoadResult loadDeck1(mixxx::track::io::key::ChromaticKey key, double initialPitch) {
        if (key == mixxx::track::io::key::INVALID) {
            m_pTrack1->resetKeys();
        } else {
            m_pTrack1->setKey(key, mixxx::track::io::key::USER);
        }
        const ConfigKey pitchKey(m_sGroup1, "pitch");
        ControlObject::set(pitchKey, initialPitch);
        LoadResult result;
        result.pitchBefore = ControlObject::get(pitchKey);
        m_pMixerDeck1->slotTrackLoaded(m_pTrack1, TrackPointer());
        EXPECT_EQ(m_pTrack1, m_pMixerDeck1->getLoadedTrack());
        result.pitch = ControlObject::get(pitchKey);
        return result;
    }
};

TEST_F(AutoHarmonizeManualLoadTest, ManualLoadHarmonizesWhenKeyOptionOn) {
    setOptions(true, false);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.3);
    ASSERT_NE(mixxx::track::io::key::INVALID, other.key);
    const double expected = compatibleOffset(kLoadedKey, other.key, other.distance);
    const auto helper = KeyUtils::matchKeyPitchSemitones(kLoadedKey, other.key, other.distance);
    ASSERT_TRUE(helper.has_value());
    EXPECT_DOUBLE_EQ(expected, *helper);
    ASSERT_NE(expected * 2.0, expected);

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    ASSERT_NE(loaded.pitchBefore, expected);
    EXPECT_DOUBLE_EQ(expected, loaded.pitch);
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, ManualLoadLeavesPitchWhenKeyOptionOff) {
    setOptions(false, true);
    publishOtherDeckKey(m_sGroup2, 0.3);
    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(loaded.pitchBefore, loaded.pitch);
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, BothOptionsApplyOneOffsetOnManualLoad) {
    setOptions(true, true);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.3);
    const double expected = compatibleOffset(kLoadedKey, other.key, other.distance);
    ASSERT_NE(0.0, expected);

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(expected, loaded.pitch);
    EXPECT_NE(expected * 2.0, loaded.pitch);
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, AutoDjMarkBlocksManualHarmonize) {
    setOptions(true, true);
    publishOtherDeckKey(m_sGroup2, 0.3);
    m_pMixerDeck1->setPendingAutoDjAutoload(true);

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(loaded.pitchBefore, loaded.pitch);
    // The manual path must not consume the AutoDJ autoload mark.
    EXPECT_TRUE(m_pMixerDeck1->isPendingAutoDjAutoload());
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, InvalidKeyLeavesPitchAndStoredKey) {
    setOptions(true, true);
    publishOtherDeckKey(m_sGroup2, 0.0);
    const LoadResult loaded = loadDeck1(mixxx::track::io::key::INVALID, 4.0);
    EXPECT_DOUBLE_EQ(loaded.pitchBefore, loaded.pitch);
    EXPECT_EQ(mixxx::track::io::key::INVALID, m_pTrack1->getKey());
    EXPECT_EQ(m_pTrack1, m_pMixerDeck1->getLoadedTrack());
}

TEST_F(AutoHarmonizeManualLoadTest, CompatiblePairWritesZeroOffset) {
    setOptions(true, false);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.0);
    const double expected = compatibleOffset(kOtherKey, other.key, other.distance);
    EXPECT_DOUBLE_EQ(0.0, expected);

    const LoadResult loaded = loadDeck1(kOtherKey, 4.0);
    EXPECT_DOUBLE_EQ(expected, loaded.pitch);
    EXPECT_EQ(kOtherKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, PreviewLoadIsNotHarmonized) {
    setOptions(true, true);
    publishOtherDeckKey(m_sGroup2, 0.3);
    TrackPointer pPreview = m_pPreview1->loadFakeTrack(false, 120.0);
    pPreview->setKey(kLoadedKey, mixxx::track::io::key::USER);
    const ConfigKey previewPitch(m_sPreviewGroup, "pitch");
    ControlObject::set(previewPitch, 2.5);
    const double pitchBefore = ControlObject::get(previewPitch);

    m_pPreview1->slotTrackLoaded(pPreview, TrackPointer());

    EXPECT_DOUBLE_EQ(pitchBefore, ControlObject::get(previewPitch));
    EXPECT_EQ(kLoadedKey, pPreview->getKey());
    EXPECT_EQ(pPreview, m_pPreview1->getLoadedTrack());
}

TEST_F(MixxxTest, HarmonizePreferencesAreIndependent) {
    ControlObject keyNotation(mixxx::library::prefs::kKeyNotationConfigKey);

    DlgPrefAutoDJ autoDjPage(nullptr, config());
    DlgPrefKey keyPage(nullptr, config());
    autoDjPage.slotUpdate();

    QCheckBox* pAutoDj = autoDjPage.findChild<QCheckBox*>(
            QStringLiteral("AutoHarmonizeToPreviousCheckBox"));
    QCheckBox* pManual = keyPage.findChild<QCheckBox*>(
            QStringLiteral("bAutoHarmonizeOnManualLoad"));
    ASSERT_NE(pAutoDj, nullptr);
    ASSERT_NE(pManual, nullptr);
    EXPECT_FALSE(pAutoDj->isChecked());
    EXPECT_FALSE(pManual->isChecked());
    // Unset keys use the false default. A true default here would hide that.
    EXPECT_FALSE(config()->getValue<bool>(kAutoDjHarmonizeKey, false));
    EXPECT_FALSE(config()->getValue<bool>(kManualHarmonizeKey, false));

    pAutoDj->setChecked(true);
    autoDjPage.slotApply();
    EXPECT_TRUE(config()->getValue<bool>(kAutoDjHarmonizeKey, false));
    EXPECT_FALSE(config()->getValue<bool>(kManualHarmonizeKey, false));

    pManual->setChecked(true);
    keyPage.slotApply();
    EXPECT_TRUE(config()->getValue<bool>(kManualHarmonizeKey, false));
    EXPECT_TRUE(config()->getValue<bool>(kAutoDjHarmonizeKey, false));

    autoDjPage.slotResetToDefaults();
    EXPECT_FALSE(pAutoDj->isChecked());
    EXPECT_TRUE(pManual->isChecked());
    // Reset changes the widget only. The other page's saved value stays.
    EXPECT_TRUE(config()->getValue<bool>(kManualHarmonizeKey, false));
    autoDjPage.slotApply();
    // Stored false must win over a true fallback, so this is not "unset".
    EXPECT_FALSE(config()->getValue<bool>(kAutoDjHarmonizeKey, true));
    EXPECT_TRUE(config()->getValue<bool>(kManualHarmonizeKey, false));

    keyPage.slotResetToDefaults();
    EXPECT_FALSE(pManual->isChecked());
    EXPECT_FALSE(pAutoDj->isChecked());
    EXPECT_FALSE(config()->getValue<bool>(kAutoDjHarmonizeKey, true));
    keyPage.slotApply();
    EXPECT_FALSE(config()->getValue<bool>(kManualHarmonizeKey, true));
    EXPECT_FALSE(config()->getValue<bool>(kAutoDjHarmonizeKey, true));

    // Applying one page again does not revive the other key.
    pAutoDj->setChecked(true);
    autoDjPage.slotApply();
    keyPage.slotUpdate();
    EXPECT_FALSE(pManual->isChecked());
    EXPECT_FALSE(config()->getValue<bool>(kManualHarmonizeKey, true));
}
