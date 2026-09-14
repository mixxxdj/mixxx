#include "preferences/dialog/dlgprefsounditem.h"

#include <gtest/gtest.h>

#include <QList>
#include <QSharedPointer>

#include "control/controlobject.h"
#include "soundio/sounddevice.h"
#include "soundio/soundmanagerconfig.h"
#include "test/mixxxtest.h"

namespace {

class TestSoundDevice : public SoundDevice {
  public:
    TestSoundDevice(UserSettingsPointer config,
            const QString& name,
            int numOutputs,
            int numInputs = 0)
            : SoundDevice(config, nullptr) {
        m_deviceId.name = name;
        m_strDisplayName = name;
        m_numOutputChannels = mixxx::audio::ChannelCount(numOutputs);
        m_numInputChannels = mixxx::audio::ChannelCount(numInputs);
    }
    SoundDeviceStatus open(bool, int) override {
        return SoundDeviceStatus::Ok;
    }
    bool isOpen() const override {
        return true;
    }
    SoundDeviceStatus close() override {
        return SoundDeviceStatus::Ok;
    }
    void readProcess(SINT) override {
    }
    void writeProcess(SINT) override {
    }
    QString getError() const override {
        return QString();
    }
    mixxx::audio::SampleRate getDefaultSampleRate() const override {
        return mixxx::audio::SampleRate(44100);
    }
};

class DlgPrefSoundItemTest : public MixxxTest {
  protected:
    void SetUp() override {
        MixxxTest::SetUp();
        m_pMainMonoCO = std::make_unique<ControlObject>(ConfigKey(
                QStringLiteral("[Master]"), QStringLiteral("mono_mixdown")));
        m_pBoothMonoCO = std::make_unique<ControlObject>(ConfigKey(
                QStringLiteral("[Booth]"), QStringLiteral("mono_mixdown")));
        m_pHeadphoneMonoCO = std::make_unique<ControlObject>(ConfigKey(
                QStringLiteral("[Headphone]"), QStringLiteral("mono_mixdown")));
        m_pMainMonoCO->set(0.0);
        m_pBoothMonoCO->set(0.0);
        m_pHeadphoneMonoCO->set(0.0);

        m_pDevice = QSharedPointer<TestSoundDevice>::create(
                config(), QStringLiteral("TestCard"), 4, 2);
        m_devices.append(m_pDevice);
    }

    void TearDown() override {
        m_devices.clear();
        m_pDevice.clear();
        m_pHeadphoneMonoCO.reset();
        m_pBoothMonoCO.reset();
        m_pMainMonoCO.reset();
        MixxxTest::TearDown();
    }

    std::unique_ptr<ControlObject> m_pMainMonoCO;
    std::unique_ptr<ControlObject> m_pBoothMonoCO;
    std::unique_ptr<ControlObject> m_pHeadphoneMonoCO;
    SoundDevicePointer m_pDevice;
    QList<SoundDevicePointer> m_devices;
};

TEST_F(DlgPrefSoundItemTest, MonoCheckboxApplicability) {
    DlgPrefSoundItem mainItem(nullptr, AudioPathType::Main, m_devices, false, 0);
    EXPECT_TRUE(mainItem.isMonoApplicable());
    EXPECT_FALSE(mainItem.monoCheckBox->isHidden());

    DlgPrefSoundItem boothItem(nullptr, AudioPathType::Booth, m_devices, false, 0);
    EXPECT_TRUE(boothItem.isMonoApplicable());
    EXPECT_FALSE(boothItem.monoCheckBox->isHidden());

    DlgPrefSoundItem headphoneItem(
            nullptr, AudioPathType::Headphones, m_devices, false, 0);
    EXPECT_TRUE(headphoneItem.isMonoApplicable());
    EXPECT_FALSE(headphoneItem.monoCheckBox->isHidden());

    DlgPrefSoundItem deckItem(nullptr, AudioPathType::Deck, m_devices, false, 0);
    EXPECT_FALSE(deckItem.isMonoApplicable());
    EXPECT_TRUE(deckItem.monoCheckBox->isHidden());

    DlgPrefSoundItem inputItem(
            nullptr, AudioPathType::Microphone, m_devices, true, 0);
    EXPECT_FALSE(inputItem.isMonoApplicable());
    EXPECT_TRUE(inputItem.monoCheckBox->isHidden());
}

TEST_F(DlgPrefSoundItemTest, MonoPreferencePreservedThroughExternalCOChangeWhileLocked) {
    DlgPrefSoundItem item(nullptr, AudioPathType::Main, m_devices, false, 0);

    // 1. Select device and stereo channel pair (Channels 1-2)
    item.setDevice(m_pDevice->getDeviceId());
    item.channelComboBox->setCurrentIndex(0); // Stereo pair
    ASSERT_EQ(2, item.channelComboBox->itemData(0).toPoint().y());
    EXPECT_TRUE(item.monoCheckBox->isEnabled());
    EXPECT_FALSE(item.monoCheckBox->isChecked());

    // 2. Switch to 1-channel (Channel 1) — checkbox locked checked+disabled
    // Find mono channel item index in combo box
    int monoIndex = -1;
    for (int i = 0; i < item.channelComboBox->count(); ++i) {
        if (item.channelComboBox->itemData(i).toPoint().y() == 1) {
            monoIndex = i;
            break;
        }
    }
    ASSERT_NE(-1, monoIndex);
    item.channelComboBox->setCurrentIndex(monoIndex);
    EXPECT_FALSE(item.monoCheckBox->isEnabled());
    EXPECT_TRUE(item.monoCheckBox->isChecked());

    // 3. Simulate an external CO change while locked in 1-channel mode
    m_pMainMonoCO->set(1.0);

    // 4. Switch back to stereo pair
    item.channelComboBox->setCurrentIndex(0);

    // 5. Assert checkbox is enabled and unchecked (user's stereo preference wasn't clobbered)
    EXPECT_TRUE(item.monoCheckBox->isEnabled());
    EXPECT_FALSE(item.monoCheckBox->isChecked());
}

TEST_F(DlgPrefSoundItemTest, MonoPreferencePreservedThrough1ChannelApplyAndRevert) {
    DlgPrefSoundItem item(nullptr, AudioPathType::Main, m_devices, false, 0);

    // 1. Select stereo pair, Mono unchecked
    item.setDevice(m_pDevice->getDeviceId());
    item.channelComboBox->setCurrentIndex(0); // Stereo
    EXPECT_FALSE(item.monoCheckBox->isChecked());

    // 2. Switch to 1-channel
    int monoIndex = -1;
    for (int i = 0; i < item.channelComboBox->count(); ++i) {
        if (item.channelComboBox->itemData(i).toPoint().y() == 1) {
            monoIndex = i;
            break;
        }
    }
    ASSERT_NE(-1, monoIndex);
    item.channelComboBox->setCurrentIndex(monoIndex);
    EXPECT_TRUE(item.monoCheckBox->isChecked());
    EXPECT_FALSE(item.monoCheckBox->isEnabled());

    // 3. Apply mono setting -> CO updated to 1.0 for 1-channel
    item.applyMonoSetting();
    EXPECT_EQ(1.0, m_pMainMonoCO->get());

    // 4. Switch back to stereo pair -> preference preserved as unchecked
    item.channelComboBox->setCurrentIndex(0);
    EXPECT_TRUE(item.monoCheckBox->isEnabled());
    EXPECT_FALSE(item.monoCheckBox->isChecked());

    // 5. Apply mono setting -> CO updated back to 0.0
    item.applyMonoSetting();
    EXPECT_EQ(0.0, m_pMainMonoCO->get());
}

TEST_F(DlgPrefSoundItemTest, ExternalCOChangeWhileStereoUpdatesCheckbox) {
    DlgPrefSoundItem item(nullptr, AudioPathType::Main, m_devices, false, 0);
    item.setDevice(m_pDevice->getDeviceId());
    item.channelComboBox->setCurrentIndex(0); // Stereo

    EXPECT_FALSE(item.monoCheckBox->isChecked());

    // External CO change to 1.0 while in stereo updates the checkbox
    m_pMainMonoCO->set(1.0);
    EXPECT_TRUE(item.monoCheckBox->isChecked());

    // External CO change to 0.0 updates the checkbox back
    m_pMainMonoCO->set(0.0);
    EXPECT_FALSE(item.monoCheckBox->isChecked());
}

TEST_F(DlgPrefSoundItemTest, ResetMonoToDefaultOnlyResetsStaging) {
    DlgPrefSoundItem item(nullptr, AudioPathType::Main, m_devices, false, 0);
    item.setDevice(m_pDevice->getDeviceId());
    item.channelComboBox->setCurrentIndex(0); // Stereo

    // User checks Mono
    item.monoCheckBox->setChecked(true);
    EXPECT_TRUE(item.monoCheckBox->isChecked());
    EXPECT_EQ(0.0, m_pMainMonoCO->get()); // Not applied yet

    // Reset to defaults unchecks the checkbox
    item.resetMonoToDefault();
    EXPECT_FALSE(item.monoCheckBox->isChecked());
    EXPECT_EQ(0.0, m_pMainMonoCO->get());
}

} // namespace
