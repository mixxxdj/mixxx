#pragma once

#include <QGridLayout>
#include <QGroupBox>
#include <QSlider>
#include <memory>

#include "control/pollingcontrolproxy.h"
#include "preferences/constants.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefsounddlg.h"
#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"
#include "soundio/soundmanagerconfig.h"
#include "soundio/soundmanagerutil.h"
#include "util/parented_ptr.h"

class ControlObject;
class ControlProxy;
class DlgPrefSoundItem;
class PlayerManager;
class SoundDevice;
class SoundDeviceId;
class SoundManager;

// TODO(bkgood) (n-decks) establish a signal/slot connection with a signal
// on EngineMaster that emits every time a channel is added, and a slot here
// that updates the dialog accordingly.

class DlgPrefSound : public DlgPreferencePage, public Ui::DlgPrefSoundDlg  {
    Q_OBJECT;
  public:
    DlgPrefSound(QWidget* parent,
            std::shared_ptr<SoundManager> soundManager,
            UserSettingsPointer pSettings);

    void selectIOTab(mixxx::preferences::SoundHardwareTab tab);

    QUrl helpUrl() const override;
    bool okayToClose() const override;

  signals:
    void loadPaths(const SoundManagerConfig &config);
    void writePath(const AudioPath* pPath, SoundManagerConfig* config);
    void writePaths(SoundManagerConfig *config);
    void refreshOutputDevices(const QList<SoundDevicePointer>& devices);
    void refreshInputDevices(const QList<SoundDevicePointer>& devices);
    void addOutputDevice(SoundDevicePointer pDevice);
    void addInputDevice(SoundDevicePointer pDevice);
    void removeOutputDevice(SoundDevicePointer pDevice);
    void removeInputDevice(SoundDevicePointer pDevice);
    void updatingAPI();
    void updatedAPI();
    void deviceChannelsUpdated(SoundDevicePointer devices);

  public slots:
    void slotUpdate() override; // called on show
    void slotApply() override;  // called on ok button
    void slotResetToDefaults() override;
    void bufferUnderflow(double count);
    void slotResetUnderflowCounter();
    void outputLatencyChanged(double latency);
    void latencyCompensationSpinboxChanged(double value);
    void mainDelaySpinboxChanged(double value);
    void headDelaySpinboxChanged(double value);
    void boothDelaySpinboxChanged(double value);
    void mainMixChanged(int value);
    void mainEnabledChanged(double value);
    void mainOutputModeComboBoxChanged(int value);
    void mainMonoMixdownChanged(double value);
    void micMonitorModeComboBoxChanged(int value);

  private slots:
    void addPath(const AudioOutput& output);
    void addPath(const AudioInput& input);
    void loadSettings();
    void apiChanged(int index);
    void updateAPIs();
    void sampleRateChanged(int index);
    void audioBufferChanged(int index);
    void updateAudioBufferSizes(int sampleRateIndex);
    void syncBuffersChanged(int index);
    void engineClockChanged(int index);
    void refreshDevices();
    void settingChanged();
    void deviceChanged();
    void deviceChannelsChanged();
    void configuredDeviceNotFound();
    void queryClicked();
#ifdef __RUBBERBAND__
    void updateKeylockDualThreadingCheckbox();
    void updateKeylockMultithreading(bool enabled);
#endif
    void addDevice(SoundDevicePointer pDevice);
    void removeDevice(SoundDevicePointer pDevice);
    void updateDeviceChannels(SoundDevicePointer pDevice);
    void updateSampleRates(const QList<mixxx::audio::SampleRate>& sampleRates);
    void invalidateConfig();
    void refreshHardwareDevices();

  private:
    void initializePaths();
    void connectSoundItem(DlgPrefSoundItem *item);
    void loadSettings(const SoundManagerConfig &config);
    void insertItem(DlgPrefSoundItem *pItem, QVBoxLayout *pLayout);
    void checkLatencyCompensation();
#ifdef __PIPEWIRE__
    void initPipewire();
    void updateGraphDriver(int driverId);
#endif
    void hardwareVolumeAdded(uint32_t deviceId, const QString& name, uint32_t index, bool isInput);
    void addHardwareVolume(uint32_t deviceId, const QString& name, uint32_t index);

    std::shared_ptr<SoundManager> m_pSoundManager;
    UserSettingsPointer m_pSettings;
    SoundManagerConfig m_config;

    PollingControlProxy m_pLatencyCompensation;
    PollingControlProxy m_pMainDelay;
    PollingControlProxy m_pHeadDelay;
    PollingControlProxy m_pBoothDelay;
    PollingControlProxy m_pMicMonitorMode;
    PollingControlProxy m_pKeylockEngine;

    parented_ptr<ControlProxy> m_pAudioLatencyOverloadCount;
    parented_ptr<ControlProxy> m_pOutputLatencyMs;
    parented_ptr<ControlProxy> m_pMainEnabled;
    parented_ptr<ControlProxy> m_pMainMonoMixdown;

    QList<SoundDevicePointer> m_inputDevices;
    QList<SoundDevicePointer> m_outputDevices;
    QHash<DlgPrefSoundItem*, QPair<SoundDeviceId, int>> m_selectedOutputChannelIndices;
    QHash<DlgPrefSoundItem*, QPair<SoundDeviceId, int>> m_selectedInputChannelIndices;
    bool m_settingsModified;
    bool m_bLatencyChanged;
    bool m_bSkipConfigClear;
    bool m_loading;
    bool m_configValid;

#ifdef __PIPEWIRE__
    parented_ptr<QCheckBox> m_pipewireCheckBox;
    parented_ptr<QCheckBox> m_pipewirePatchbayCheckBox;
    parented_ptr<ControlProxy> m_pPipewirePatchbay;
    parented_ptr<QCheckBox> m_forceBufferSize;
    parented_ptr<QCheckBox> m_forceSamplerate;
    parented_ptr<ControlProxy> m_cpSamplerate;
    parented_ptr<ControlProxy> m_cpBufferSize;
    parented_ptr<ControlProxy> m_cpLatencyParamsMismatch;
    QLabel* m_latencyParamsMismatchText;
    parented_ptr<ControlProxy> m_pNodeDriver;
    QLabel* m_pPipewireDriver;
#endif

    struct HardwareDevice {
        struct Volume {
            parented_ptr<QSlider> slider;
            parented_ptr<ControlProxy> value;
            QLabel* label;
        };
        QString name;
        std::unordered_map<uint32_t, Volume> volumes;
    };
    parented_ptr<ControlProxy> m_cpMainVolumeRoute;
    parented_ptr<ControlProxy> m_cpHeadVolumeRoute;
    parented_ptr<ControlProxy> m_cpBoothVolumeRoute;
    parented_ptr<ControlProxy> m_cpMainVolumeDevice;
    parented_ptr<ControlProxy> m_cpHeadVolumeDevice;
    parented_ptr<ControlProxy> m_cpBoothVolumeDevice;
    parented_ptr<ControlProxy> m_cpManualVolumeDevice;
    std::unordered_map<uint32_t, HardwareDevice> m_hardwareDevices;
};
