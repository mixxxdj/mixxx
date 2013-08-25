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

#include "controlobject.h"
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
#ifdef __LADSPA__
class EngineLADSPA;
#endif
class EngineVuMeter;
class ControlPotmeter;
class ControlPushButton;
class EngineVinylSoundEmu;
class EngineSideChain;
class EffectsManager;
class SyncWorker;

class EngineMaster : public EngineObject, public AudioSource {
    Q_OBJECT
  public:
    EngineMaster(ConfigObject<ConfigValue>* pConfig,
                 const char* pGroup,
                 EffectsManager* pEffectsManager,
                 bool bEnableSidechain);
    virtual ~EngineMaster();

    // Get access to the sample buffers. None of these are thread safe. Only to
    // be called by SoundManager.
    const CSAMPLE* buffer(AudioOutput output) const;

    const QString getMasterChannelId() const {
        return QString("[Master]");
    }

    const QString getHeadphoneChannelId() const {
        return QString("[Headphone]");
    }

    EffectsManager* getEffectsManager() const {
        return m_pEffectsManager;
    }

    void process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize);

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

    // These are really only exposed for tests to use.
    const CSAMPLE* getMasterBuffer() const;
    const CSAMPLE* getHeadphoneBuffer() const;
    const CSAMPLE* getDeckBuffer(unsigned int i) const;
    const CSAMPLE* getChannelBuffer(QString name) const;

    EngineSideChain* getSideChain() const {
        return m_pSideChain;
    }

    struct ChannelInfo {
        EngineChannel* m_pChannel;
        CSAMPLE* m_pBuffer;
        ControlObject* m_pVolumeControl;
    };

    class GainCalculator {
      public:
        virtual double getGain(ChannelInfo* pChannelInfo) const = 0;
    };
    class ConstantGainCalculator : public GainCalculator {
      public:
        inline double getGain(ChannelInfo* pChannelInfo) const {
            Q_UNUSED(pChannelInfo);
            return m_dGain;
        }
        inline void setGain(double dGain) {
            m_dGain = dGain;
        }
      private:
        double m_dGain;
    };
    class OrientationVolumeGainCalculator : public GainCalculator {
      public:
        OrientationVolumeGainCalculator()
                : m_dVolume(1.0), m_dLeftGain(1.0), m_dCenterGain(1.0), m_dRightGain(1.0) { }

        inline double getGain(ChannelInfo* pChannelInfo) const {
            const double channelVolume = pChannelInfo->m_pVolumeControl->get();
            const double orientationGain = EngineMaster::gainForOrientation(
                pChannelInfo->m_pChannel->getOrientation(),
                m_dLeftGain, m_dCenterGain, m_dRightGain);
            const double gain = m_dVolume * channelVolume * orientationGain;
            return gain;
        }

        inline void setGains(double dVolume, double leftGain, double centerGain, double rightGain) {
            m_dVolume = dVolume;
            m_dLeftGain = leftGain;
            m_dCenterGain = centerGain;
            m_dRightGain = rightGain;
        }

      private:
        double m_dVolume, m_dLeftGain, m_dCenterGain, m_dRightGain;
    };

    EffectsManager* m_pEffectsManager;
    QList<ChannelInfo*> m_channels;
    QList<CSAMPLE> m_channelMasterGainCache;
    QList<CSAMPLE> m_channelHeadphoneGainCache;

    CSAMPLE* m_pMaster;
    CSAMPLE* m_pHead;

    EngineWorkerScheduler* m_pWorkerScheduler;

    ControlObject* m_pMasterVolume;
    ControlObject* m_pHeadVolume;
    ControlObject* m_pMasterSampleRate;
    ControlObject* m_pMasterLatency;
    ControlObject* m_pMasterAudioBufferSize;
    ControlObject* m_pMasterUnderflowCount;
    ControlPotmeter* m_pMasterRate;
    EngineClipping* m_pClipping;
    EngineClipping* m_pHeadClipping;

#ifdef __LADSPA__
    EngineLADSPA* m_pLadspa;
#endif
    EngineVuMeter* m_pVumeter;
    EngineSideChain* m_pSideChain;

    ControlPotmeter* m_pCrossfader;
    ControlPotmeter* m_pHeadMix;
    ControlPotmeter* m_pBalance;
    ControlPotmeter* m_pXFaderMode;
    ControlPotmeter* m_pXFaderCurve;
    ControlPotmeter* m_pXFaderCalibration;
    ControlPotmeter* m_pXFaderReverse;

    ConstantGainCalculator m_headphoneGain;
    OrientationVolumeGainCalculator m_masterGain;
    CSAMPLE m_headphoneMasterGainOld;
    CSAMPLE m_headphoneVolumeOld;
};

#endif
