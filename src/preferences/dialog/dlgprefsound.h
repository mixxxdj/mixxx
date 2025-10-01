#pragma once

#include <memory>

#include "defs_urls.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefsounddlg.h"
#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"
#include "soundio/sounddevicestatus.h"
#include "soundio/soundmanagerconfig.h"

class SoundManager;
class PlayerManager;
class ControlObject;
class SoundDevice;
class SoundDeviceId;
class DlgPrefSoundItem;
class ControlProxy;

// TODO(bkgood) (n-decks) establish a signal/slot connection with a signal
// on EngineMaster that emits every time a channel is added, and a slot here
// that updates the dialog accordingly.

class DlgPrefSound : public DlgPreferencePage, public Ui::DlgPrefSoundDlg  {
    Q_OBJECT;
  public:
    DlgPrefSound(QWidget* parent,
            std::shared_ptr<SoundManager> soundManager,
            UserSettingsPointer pSettings);

    QUrl helpUrl() const override;
    bool okayToClose() const override;

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

  private:
    void initializePaths();
    void connectSoundItem(DlgPrefSoundItem *item);
    void loadSettings(const SoundManagerConfig &config);
    void insertItem(DlgPrefSoundItem *pItem, QVBoxLayout *pLayout);
    void checkLatencyCompensation();

    std::shared_ptr<SoundManager> m_pSoundManager;
    UserSettingsPointer m_pSettings;
    SoundManagerConfig m_config;
    ControlProxy* m_pAudioLatencyOverloadCount;
    ControlProxy* m_pOutputLatencyMs;
    ControlProxy* m_pHeadDelay;
    ControlProxy* m_pMainDelay;
    ControlProxy* m_pBoothDelay;
    ControlProxy* m_pLatencyCompensation;
    ControlProxy* m_pKeylockEngine;
    ControlProxy* m_pMainEnabled;
    ControlProxy* m_pMainMonoMixdown;
    ControlProxy* m_pMicMonitorMode;
    QList<SoundDevicePointer> m_inputDevices;
    QList<SoundDevicePointer> m_outputDevices;
    QHash<DlgPrefSoundItem*, QPair<SoundDeviceId, int>> m_selectedOutputChannelIndices;
    QHash<DlgPrefSoundItem*, QPair<SoundDeviceId, int>> m_selectedInputChannelIndices;
    bool m_settingsModified;
    bool m_bLatencyChanged;
    bool m_bSkipConfigClear;
    bool m_loading;
    bool m_configValid;
};
