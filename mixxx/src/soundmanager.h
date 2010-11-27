/***************************************************************************
                          soundmanager.h
                             -------------------
    begin                : Sun Aug 15, 2007
    copyright            : (C) 2007 Albert Santoni
    email                : gamegod \a\t users.sf.net
 ***************************************************************************/

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

#include "configobject.h"
#include "controlobject.h"
#include "defs.h"
#ifdef __VINYLCONTROL__
#include "vinylcontrolproxy.h"
#endif
#include "soundmanagerconfig.h"
#include <QTimer>

class SoundDevice;
class EngineMaster;
class AudioOutput;
class AudioInput;

#define MIXXX_PORTAUDIO_JACK_STRING "JACK Audio Connection Kit"
#define MIXXX_PORTAUDIO_ALSA_STRING "ALSA"
#define MIXXX_PORTAUDIO_OSS_STRING "OSS"
#define MIXXX_PORTAUDIO_ASIO_STRING "ASIO"
#define MIXXX_PORTAUDIO_DIRECTSOUND_STRING "Windows DirectSound"
#define MIXXX_PORTAUDIO_COREAUDIO_STRING "Core Audio"

class SoundManager : public QObject
{
    Q_OBJECT
    
    public:
        SoundManager(ConfigObject<ConfigValue> *pConfig, EngineMaster *_master);
        ~SoundManager();
        const EngineMaster* getEngine() const;
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
#ifdef __VINYLCONTROL__        
        bool hasVinylInput(int deck);
#endif
        int setConfig(SoundManagerConfig config);
        void checkConfig();
        QHash<AudioOutput, const CSAMPLE*>
            requestBuffer(QList<AudioOutput> outputs, unsigned long iFramesPerBuffer);
        void pushBuffer(QList<AudioInput> inputs, short *inputBuffer, 
                        unsigned long iFramesPerBuffer, unsigned int iFrameSize);
    signals:
        void devicesUpdated(); // emitted when all the pointers to SoundDevices go stale
    public slots:
        void sync();
    private:
        EngineMaster *m_pMaster;
        ConfigObject<ConfigValue> *m_pConfig;
        QList<SoundDevice*> m_devices;
        QList<unsigned int> m_samplerates;
        QString m_hostAPI;
        QHash<AudioOutput, const CSAMPLE*> m_outputBuffers;
        QHash<AudioInput, short*> m_inputBuffers; /** Audio received from input */
#ifdef __VINYLCONTROL__
        QList<VinylControlProxy*> m_VinylControl;
        QHash<VinylControlProxy*, AudioInput> m_VinylMapping;
#endif        
        unsigned int iNumDevicesOpenedForOutput;
        unsigned int iNumDevicesOpenedForInput;
        unsigned int iNumDevicesHaveRequestedBuffer;
        QMutex requestBufferMutex;
        SoundManagerConfig m_config;
        SoundDevice *m_pErrorDevice;
#ifdef __PORTAUDIO__
        bool m_paInitialized;
        unsigned int m_jackSampleRate;
#endif
};

#endif
