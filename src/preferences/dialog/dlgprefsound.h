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

#include "preferences/dialog/ui_dlgprefsounddlg.h"
#include "preferences/usersettings.h"
#include "soundio/soundmanagerconfig.h"
#include "soundio/sounddeviceerror.h"
#include "preferences/dlgpreferencepage.h"

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
                 UserSettingsPointer config);
    virtual ~DlgPrefSound();

  signals:
    void loadPaths(const SoundManagerConfig &config);
    void writePaths(SoundManagerConfig *config);
    void refreshOutputDevices(const QList<SoundDevice*> &devices);
    void refreshInputDevices(const QList<SoundDevice*> &devices);
    void updatingAPI();
    void updatedAPI();

  public slots:
    void slotUpdate(); // called on show
    void slotApply();  // called on ok button
    void slotResetToDefaults();
    void bufferUnderflow(double count);
    void masterLatencyChanged(double latency);
    void headDelayChanged(double value);
    void masterDelayChanged(double value);
    void masterMixChanged(int value);
    void masterEnabledChanged(double value);
    void masterOutputModeComboBoxChanged(int value);
    void masterMonoMixdownChanged(double value);
    void talkoverMixComboBoxChanged(int value);
    void talkoverMixChanged(double value);

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
    void refreshDevices();
    void settingChanged();
    void queryClicked();

  private:
    void initializePaths();
    void connectSoundItem(DlgPrefSoundItem *item);
    void loadSettings(const SoundManagerConfig &config);
    void insertItem(DlgPrefSoundItem *pItem, QVBoxLayout *pLayout);

    SoundManager *m_pSoundManager;
    PlayerManager *m_pPlayerManager;
    UserSettingsPointer m_pConfig;
    ControlProxy* m_pMasterAudioLatencyOverloadCount;
    ControlProxy* m_pMasterLatency;
    ControlProxy* m_pHeadDelay;
    ControlProxy* m_pMasterDelay;
    ControlProxy* m_pKeylockEngine;
    ControlProxy* m_pMasterEnabled;
    ControlProxy* m_pMasterMonoMixdown;
    ControlProxy* m_pMasterTalkoverMix;
    QList<SoundDevice*> m_inputDevices;
    QList<SoundDevice*> m_outputDevices;
    bool m_settingsModified;
    SoundManagerConfig m_config;
    bool m_loading;
};

#endif
