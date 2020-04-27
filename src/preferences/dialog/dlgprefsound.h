/**
 * @file dlgprefsound.h
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100625
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFSOUND_H
#define DLGPREFSOUND_H

#include "defs_urls.h"
#include "preferences/dialog/ui_dlgprefsounddlg.h"
#include "preferences/dlgpreferencepage.h"
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
    DlgPrefSound(QWidget *parent, SoundManager *soundManager,
                 PlayerManager* pPlayerManager,
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
    void slotUpdate(); // called on show
    void slotApply();  // called on ok button
    void slotResetToDefaults();
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
    void addPath(AudioOutput output);
    void addPath(AudioInput input);
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

    SoundManager *m_pSoundManager;
    PlayerManager *m_pPlayerManager;
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

#endif
