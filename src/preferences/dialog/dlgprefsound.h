#pragma once

#include <memory>

#include "defs_urls.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefsounddlg.h"
#include "preferences/usersettings.h"
#include "soundio/sounddevice.h"
#include "soundio/sounddeviceerror.h"
#include "soundio/soundmanagerconfig.h"

class SoundManager;
class PlayerManager;
class ControlObject;
class SoundDevice;
class DlgPrefSoundItem;
class ControlProxy;

/*
 * TODO(bkgood) (n-decks) establish a signal/slot connection with a signal
 * on EngineMaster that emits every time a channel is added, and a slot here
 * that updates the dialog accordingly.
 */

/**
 * Class representing a preferences pane to configure sound devices for Mixxx.
 */
class DlgPrefSound : public DlgPreferencePage, public Ui::DlgPrefSoundDlg  {
    Q_OBJECT;
  public:
    DlgPrefSound(QWidget* parent,
            std::shared_ptr<SoundManager> soundManager,
            UserSettingsPointer pSettings);
    virtual ~DlgPrefSound();

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
    void masterLatencyChanged(double latency);
    void latencyCompensationSpinboxChanged(double value);
    void masterDelaySpinboxChanged(double value);
    void headDelaySpinboxChanged(double value);
    void boothDelaySpinboxChanged(double value);
    void masterMixChanged(int value);
    void masterEnabledChanged(double value);
    void masterOutputModeComboBoxChanged(int value);
    void masterMonoMixdownChanged(double value);
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
    void deviceSettingChanged();
    void queryClicked();

  private:
    void initializePaths();
    void connectSoundItem(DlgPrefSoundItem *item);
    void loadSettings(const SoundManagerConfig &config);
    void insertItem(DlgPrefSoundItem *pItem, QVBoxLayout *pLayout);
    void checkLatencyCompensation();
    bool eventFilter(QObject* object, QEvent* event) override;

    std::shared_ptr<SoundManager> m_pSoundManager;
    UserSettingsPointer m_pSettings;
    SoundManagerConfig m_config;
    ControlProxy* m_pMasterAudioLatencyOverloadCount;
    ControlProxy* m_pMasterLatency;
    ControlProxy* m_pHeadDelay;
    ControlProxy* m_pMasterDelay;
    ControlProxy* m_pBoothDelay;
    ControlProxy* m_pLatencyCompensation;
    ControlProxy* m_pKeylockEngine;
    ControlProxy* m_pMasterEnabled;
    ControlProxy* m_pMasterMonoMixdown;
    ControlProxy* m_pMicMonitorMode;
    QList<SoundDevicePointer> m_inputDevices;
    QList<SoundDevicePointer> m_outputDevices;
    bool m_settingsModified;
    bool m_bLatencyChanged;
    bool m_bSkipConfigClear;
    bool m_loading;
};
