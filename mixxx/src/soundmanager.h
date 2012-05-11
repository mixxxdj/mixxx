/**
 * @file soundmanager.h
 * @author Albert Santoni <gamegod at users dot sf dot net>
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20070815
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QObject>
#include "defs.h"
#include "configobject.h"
#include "soundmanagerconfig.h"
#include "controlobjectthreadmain.h"

class SoundDevice;
class EngineMaster;
class AudioOutput;
class AudioInput;
class AudioSource;
class AudioDestination;

#define MIXXX_PORTAUDIO_JACK_STRING "JACK Audio Connection Kit"
#define MIXXX_PORTAUDIO_ALSA_STRING "ALSA"
#define MIXXX_PORTAUDIO_OSS_STRING "OSS"
#define MIXXX_PORTAUDIO_ASIO_STRING "ASIO"
#define MIXXX_PORTAUDIO_DIRECTSOUND_STRING "Windows DirectSound"
#define MIXXX_PORTAUDIO_COREAUDIO_STRING "Core Audio"

#define SOUNDMANAGER_DISCONNECTED 0
#define SOUNDMANAGER_CONNECTING 1
#define SOUNDMANAGER_CONNECTED 2

class SoundManager : public QObject {
    Q_OBJECT
public:
    SoundManager(ConfigObject<ConfigValue> *pConfig, EngineMaster *_master);
    ~SoundManager();
    const EngineMaster* getEngine() const; // this shouldn't exist
    QList<SoundDevice*> getDeviceList(QString filterAPI, bool bOutputDevices, bool bInputDevices);
    void closeDevices();
    void clearDeviceList();
    void queryDevices();
    int setupDevices();
    SoundDevice* getErrorDevice() const;
    QList<unsigned int> getSampleRates(QString api) const;
    QList<unsigned int> getSampleRates() const;
    QList<QString> getHostAPIList() const;
    SoundManagerConfig getConfig() const;
    int setConfig(SoundManagerConfig config);
    void checkConfig();
    QHash<AudioOutput, const CSAMPLE*> requestBuffer(
        QList<AudioOutput> outputs, unsigned long iFramesPerBuffer,
        SoundDevice *device, double streamTime = 0);
    void pushBuffer(QList<AudioInput> inputs, short *inputBuffer,
        unsigned long iFramesPerBuffer, unsigned int iFrameSize);
    void registerOutput(AudioOutput output, const AudioSource *src);
    void registerInput(AudioInput input, AudioDestination *dest);
    QList<AudioOutput> registeredOutputs() const;
    QList<AudioInput> registeredInputs() const;
signals:
    void devicesUpdated(); // emitted when pointers to SoundDevices go stale
    void devicesSetup(); // emitted when the sound devices have been set up
    void outputRegistered(AudioOutput output, const AudioSource *src);
    void inputRegistered(AudioInput input, AudioDestination *dest);
private:
    void clearOperativeVariables();
    void setJACKName() const;

    EngineMaster *m_pMaster;
    ConfigObject<ConfigValue> *m_pConfig;
    QList<SoundDevice*> m_devices;
    QList<unsigned int> m_samplerates;
    QString m_hostAPI;
    QHash<AudioOutput, const CSAMPLE*> m_outputBuffers;
    QHash<AudioInput, short*> m_inputBuffers;
    QHash<SoundDevice*, long> m_deviceFrameCount; // used in dead code
    // Clock reference, used to make sure the same device triggers buffer
    // refresh every $latency-ms period
    SoundDevice* m_pClkRefDevice;
    int m_outputDevicesOpened;
    int m_inputDevicesOpened;
    QMutex requestBufferMutex;
    SoundManagerConfig m_config;
    SoundDevice *m_pErrorDevice;
#ifdef __PORTAUDIO__
    bool m_paInitialized;
    unsigned int m_jackSampleRate;
#endif
    QHash<AudioOutput, const AudioSource*> m_registeredSources;
    QHash<AudioInput, AudioDestination*> m_registeredDestinations;

    ControlObjectThreadMain* m_pControlObjectLatency;
    ControlObjectThreadMain* m_pControlObjectSampleRate;
    ControlObject* m_pControlObjectSoundStatus;
    ControlObjectThreadMain* m_pControlObjectVinylControlMode;
    ControlObjectThreadMain* m_pControlObjectVinylControlMode1;
    ControlObjectThreadMain* m_pControlObjectVinylControlMode2;
    ControlObjectThreadMain* m_pControlObjectVinylControlGain;
};

#endif
