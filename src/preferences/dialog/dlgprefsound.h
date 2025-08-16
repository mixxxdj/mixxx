#pragma once

#include <memory>

#include "control/pollingcontrolproxy.h"
#include "defs_urls.h"
#include "preferences/constants.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefsounddlg.h"
#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"
#include "soundio/sounddevicestatus.h"
#include "soundio/soundmanagerconfig.h"
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

  signals:
    void loadPaths(const SoundManagerConfig &config);
    void writePaths(SoundManagerConfig *config);
    void refreshOutputDevices(const QList<SoundDevicePointer>& devices);
    void refreshInputDevices(const QList<SoundDevicePointer>& devices);
    void updatingAPI();
    void updatedAPI();

  public slots:
    void slotUpdate() override; // called on show
    void slotApply() override;  // called on ok button
    void slotResetToDefaults() override;
    void bufferUnderflow(double count);
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

  private:
    void initializePaths();
    void connectSoundItem(DlgPrefSoundItem *item);
    void loadSettings(const SoundManagerConfig &config);
    void insertItem(DlgPrefSoundItem *pItem, QVBoxLayout *pLayout);
    void checkLatencyCompensation();

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
};
