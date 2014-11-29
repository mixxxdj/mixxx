/***************************************************************************
                          enginemaster.h  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINEMASTER_H
#define ENGINEMASTER_H

#include <QObject>

#include "controlobject.h"
#include "controlpushbutton.h"
#include "engine/engineobject.h"
#include "engine/enginechannel.h"
#include "soundmanagerutil.h"
#include "recording/recordingmanager.h"

class EngineWorkerScheduler;
class EngineBuffer;
class EngineChannel;
class EngineDeck;
class EngineClipping;
class EngineFlanger;
class EngineVuMeter;
class ControlPotmeter;
class ControlPushButton;
class EngineVinylSoundEmu;
class EngineSideChain;
class EffectsManager;
class EngineEffectsManager;
class SyncWorker;
class GuiTick;
class EngineSync;
class EngineTalkoverDucking;
class EngineDelay;

class EngineMaster : public QObject, public AudioSource {
    Q_OBJECT
  public:
    EngineMaster(ConfigObject<ConfigValue>* pConfig,
                 const char* pGroup,
                 EffectsManager* pEffectsManager,
                 bool bEnableSidechain,
                 bool bRampingGain);
    virtual ~EngineMaster();

    // Get access to the sample buffers. None of these are thread safe. Only to
    // be called by SoundManager.
    const CSAMPLE* buffer(AudioOutput output) const;

    const QString getMasterGroup() const {
        return QString("[Master]");
    }

    const QString getHeadphoneGroup() const {
        return QString("[Headphone]");
    }

    const QString getBusLeftGroup() const {
        return QString("[BusLeft]");
    }

    const QString getBusCenterGroup() const {
        return QString("[BusCenter]");
    }

    const QString getBusRightGroup() const {
        return QString("[BusRight]");
    }


    // WARNING: These methods are called by the main thread. They should only
    // touch the volatile bool connected indicators (see below). However, when
    // these methods are called the callback is guaranteed to be inactive
    // (SoundManager closes all devices before calling these). This may change
    // in the future.
    virtual void onOutputConnected(AudioOutput output);
    virtual void onOutputDisconnected(AudioOutput output);

    void process(const int iBufferSize);

    // Add an EngineChannel to the mixing engine. This is not thread safe --
    // only call it before the engine has started mixing.
    void addChannel(EngineChannel* pChannel);
    EngineChannel* getChannel(QString group);
    static inline double gainForOrientation(EngineChannel::ChannelOrientation orientation,
                                            double leftGain,
                                            double centerGain,
                                            double rightGain) {
        switch (orientation) {
            case EngineChannel::LEFT:
                return leftGain;
            case EngineChannel::RIGHT:
                return rightGain;
            case EngineChannel::CENTER:
            default:
                return centerGain;
        }
    }

    // Provide access to the master sync so enginebuffers can know what their rate controller is.
    EngineSync* getEngineSync() const{
        return m_pMasterSync;
    }

    // These are really only exposed for tests to use.
    const CSAMPLE* getMasterBuffer() const;
    const CSAMPLE* getHeadphoneBuffer() const;
    const CSAMPLE* getOutputBusBuffer(unsigned int i) const;
    const CSAMPLE* getDeckBuffer(unsigned int i) const;
    const CSAMPLE* getChannelBuffer(QString name) const;

    EngineSideChain* getSideChain() const {
        return m_pSideChain;
    }

    struct ChannelInfo {
        ChannelInfo()
                : m_pChannel(NULL),
                  m_pBuffer(NULL),
                  m_pVolumeControl(NULL),
                  m_pMuteControl(NULL) {
        }
        EngineChannel* m_pChannel;
        CSAMPLE* m_pBuffer;
        ControlObject* m_pVolumeControl;
        ControlPushButton* m_pMuteControl;
    };

    class GainCalculator {
      public:
        virtual double getGain(ChannelInfo* pChannelInfo) const = 0;
    };
    class ConstantGainCalculator : public GainCalculator {
      public:
        inline double getGain(ChannelInfo* pChannelInfo) const {
            return pChannelInfo->m_pChannel->isTalkover() ? m_dTalkoverGain : m_dGain;
        }
        inline void setGain(double dGain) {
            m_dGain = dGain;
        }
        inline void setTalkoverGain(double dGain) {
            m_dTalkoverGain = dGain;
        }
      private:
        double m_dGain;
        double m_dTalkoverGain;
    };
    class OrientationVolumeGainCalculator : public GainCalculator {
      public:
        OrientationVolumeGainCalculator()
                : m_dVolume(1.0), m_dLeftGain(1.0), m_dCenterGain(1.0), m_dRightGain(1.0),
                  m_dTalkoverGain(1.0) { }

        inline double getGain(ChannelInfo* pChannelInfo) const {
            if (pChannelInfo->m_pMuteControl->get() > 0.0) {
                return 0.0;
            }
            if (pChannelInfo->m_pChannel->isTalkover()) {
                return m_dTalkoverGain;
            }
            const double channelVolume = pChannelInfo->m_pVolumeControl->get();
            const double orientationGain = EngineMaster::gainForOrientation(
                pChannelInfo->m_pChannel->getOrientation(),
                m_dLeftGain, m_dCenterGain, m_dRightGain);
            return m_dVolume * channelVolume * orientationGain;
        }

        inline void setGains(double dVolume, double leftGain, double centerGain, double rightGain,
                             double talkoverGain) {
            m_dVolume = dVolume;
            m_dLeftGain = leftGain;
            m_dCenterGain = centerGain;
            m_dRightGain = rightGain;
            m_dTalkoverGain = talkoverGain;
        }

      private:
        double m_dVolume, m_dLeftGain, m_dCenterGain, m_dRightGain, m_dTalkoverGain;
    };

  private:
    void mixChannels(unsigned int channelBitvector, unsigned int maxChannels,
                     CSAMPLE* pOutput, unsigned int iBufferSize, GainCalculator* pGainCalculator);

    // Processes active channels. The master sync channel (if any) is processed
    // first and all others are processed after. Sets the i'th bit of
    // masterOutput and headphoneOutput if the i'th channel is enabled for the
    // master output or headphone output, respectively.
    void processChannels(unsigned int* busChannelConnectionFlags,
                         unsigned int* headphoneOutput,
                         int iBufferSize);

    EngineEffectsManager* m_pEngineEffectsManager;
    bool m_bRampingGain;
    QList<ChannelInfo*> m_channels;
    QList<ChannelInfo*> m_activeChannels;
    QList<CSAMPLE> m_channelMasterGainCache;
    QList<CSAMPLE> m_channelHeadphoneGainCache;

    CSAMPLE* m_pOutputBusBuffers[3];
    CSAMPLE* m_pMaster;
    CSAMPLE* m_pHead;

    EngineWorkerScheduler* m_pWorkerScheduler;
    EngineSync* m_pMasterSync;

    ControlObject* m_pMasterGain;
    ControlObject* m_pHeadGain;
    ControlObject* m_pMasterSampleRate;
    ControlObject* m_pMasterLatency;
    ControlObject* m_pMasterAudioBufferSize;
    ControlObject* m_pAudioLatencyOverloadCount;
    ControlPotmeter* m_pMasterRate;
    ControlPotmeter* m_pAudioLatencyUsage;
    ControlPotmeter* m_pAudioLatencyOverload;
    EngineTalkoverDucking* m_pTalkoverDucking;
    EngineDelay* m_pMasterDelay;
    EngineDelay* m_pHeadDelay;

    EngineVuMeter* m_pVumeter;
    EngineSideChain* m_pSideChain;

    ControlPotmeter* m_pCrossfader;
    ControlPotmeter* m_pHeadMix;
    ControlPotmeter* m_pBalance;
    ControlPushButton* m_pXFaderMode;
    ControlPotmeter* m_pXFaderCurve;
    ControlPotmeter* m_pXFaderCalibration;
    ControlPushButton* m_pXFaderReverse;
    ControlPushButton* m_pHeadSplitEnabled;
    ControlObject* m_pKeylockEngine;

    ConstantGainCalculator m_headphoneGain;
    OrientationVolumeGainCalculator m_masterGain;
    CSAMPLE m_masterVolumeOld;
    CSAMPLE m_headphoneMasterGainOld;
    CSAMPLE m_headphoneVolumeOld;

    // Produce the Master Mixxx, not Required if connected to left
    // and right Bus and no recording and broadcast active
    ControlObject* m_pMasterEnabled;
    // Mix two Mono channels. This is useful for outdoor gigs
    ControlObject* m_pMasterMonoMixdown;
    ControlObject* m_pHeadphoneEnabled;

    volatile bool m_bBusOutputConnected[3];
};

#endif
