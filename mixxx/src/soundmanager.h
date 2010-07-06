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
#include "audiopath.h"
#ifdef __VINYLCONTROL__
#include "vinylcontrolproxy.h"
#endif
#include <QTimer>

class SoundDevice;
class EngineMaster;

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
        QList<SoundDevice*> getDeviceList(QString filterAPI, bool bOutputDevices, bool bInputDevices);
        void closeDevices();
        void clearDeviceList();
        void queryDevices();
        int setupDevices();
        void setDefaults(bool api=true, bool devices=true, bool other=true);
        QList<QString> getSamplerateList();
        QList<QString> getHostAPIList();
        QString getHostAPI() const;
        void setHostAPI(QString api);
        float getSampleRate() const;
        void setSampleRate(float sampleRate);
        unsigned int getFramesPerBuffer() const;
        void setFramesPerBuffer(unsigned int framesPerBuffer);
        QHash<AudioSource, const CSAMPLE*>
            requestBuffer(QList<AudioSource> srcs, unsigned long iFramesPerBuffer);
        void pushBuffer(QList<AudioReceiver> recvs, short *inputBuffer, 
                        unsigned long iFramesPerBuffer, unsigned int iFrameSize);
    public slots:
        void sync();
    private:
        EngineMaster *m_pMaster;
        ConfigObject<ConfigValue> *m_pConfig;
        QList<SoundDevice*> m_devices;
        QList<QString> m_samplerates;
        QString m_hostAPI;
        float m_sampleRate;
        unsigned int m_framesPerBuffer;
        QHash<AudioSource, const CSAMPLE*> m_sourceBuffers;
        QHash<AudioReceiver, short*> m_receiverBuffers; /** Audio received from input */
#ifdef __VINYLCONTROL__
        QList<VinylControlProxy*> m_VinylControl;
#endif        
        unsigned int iNumDevicesOpenedForOutput;
        unsigned int iNumDevicesOpenedForInput;
        unsigned int iNumDevicesHaveRequestedBuffer;
        QMutex requestBufferMutex;
        QTimer m_controlObjSyncTimer;
};

#endif
