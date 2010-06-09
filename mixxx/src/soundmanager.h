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
#include <QTimer>

class SoundDevice;
class EngineMaster;

#define MIXXX_PORTAUDIO_JACK_STRING "JACK Audio Connection Kit"
#define MIXXX_PORTAUDIO_ALSA_STRING "ALSA"
#define MIXXX_PORTAUDIO_OSS_STRING "OSS"
#define MIXXX_PORTAUDIO_ASIO_STRING "ASIO"
#define MIXXX_PORTAUDIO_DIRECTSOUND_STRING "Windows DirectSound"
#define MIXXX_PORTAUDIO_COREAUDIO_STRING "Core Audio"

enum AudioSourceType { 
    SOURCE_MASTER,
    SOURCE_HEADPHONES,
    SOURCE_DECK,
    SOURCE_PASSTHROUGH,
    SOURCE_MICROPHONE
};

typedef struct _AudioSource {
    AudioSourceType type;
    unsigned int channelBase; //Base channel on the audio device
    unsigned int channels;    //total channels (e.g. 2 for stereo)
    unsigned int index;       //Index of indexed sources (eg. decks)
} AudioSource;

enum AudioReceiverType {
    RECEIVER_VINYLCONTROL,
    RECEIVER_MICROPHONE,
    RECEIVER_PASSTHROUGH
};

typedef struct _AudioReceiver {
    AudioReceiverType type;
    unsigned int channelBase; //Base channel on the audio device
    unsigned int channels;    //total channels (e.g. 2 for stereo)
    unsigned int index;       //Index of indexed receivers (eg. vinylctrl)
} AudioReceiver;

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
        int setHostAPI(QString api);
        QString getHostAPI();
        CSAMPLE** requestBuffer(QList<AudioSource> srcs, unsigned long iFramesPerBuffer);
        CSAMPLE* pushBuffer(QList<AudioReceiver> recvs, short *inputBuffer, 
                            unsigned long iFramesPerBuffer, unsigned int iFrameSize);
        static QString getStringFromSource(const AudioSource& src) const;
        static QString getStringFromReceiver(const AudioReceiver& recv) const;
    public slots:
        void sync();
    private:
        EngineMaster *m_pMaster;
        ConfigObject<ConfigValue> *m_pConfig;
        QList<SoundDevice*> m_devices;
        QList<QString> m_samplerates;
        QString m_hostAPI;
        QHash<AudioSource, CSAMPLE*> m_pStreamBuffers;
        QHash<AudioReceiver, short*> m_pReceiverBuffers; /** Audio received from input */
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
