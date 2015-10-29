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
#include <QVarLengthArray>

#include "controlobject.h"
#include "controlpushbutton.h"
#include "engine/engineobject.h"
#include "engine/enginechannel.h"
#include "engine/channelhandle.h"
#include "soundmanagerutil.h"
#include "recording/recordingmanager.h"

class EngineWorkerScheduler;
class EngineBuffer;
class EngineChannel;
class EngineDeck;
class EngineFlanger;
class EngineVuMeter;
class ControlPotmeter;
class ControlPushButton;
class EngineSideChain;
class EffectsManager;
class EngineEffectsManager;
class SyncWorker;
class GuiTick;
class EngineSync;
class EngineTalkoverDucking;
class EngineDelay;

// The number of channels to pre-allocate in various structures in the
// engine. Prevents memory allocation in EngineMaster::addChannel.
static const int kPreallocatedChannels = 64;

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

    inline const QString& getMasterGroup() const {
        return m_masterHandle.name();
    }

    inline const QString& getHeadphoneGroup() const {
        return m_headphoneHandle.name();
    }

    inline const QString& getBusLeftGroup() const {
        return m_busLeftHandle.name();
    }

    inline const QString& getBusCenterGroup() const {
        return m_busCenterHandle.name();
    }

    inline const QString& getBusRightGroup() const {
        return m_busRightHandle.name();
    }

    ChannelHandleAndGroup registerChannelGroup(const QString& group) {
        return ChannelHandleAndGroup(
                m_channelHandleFactory.getOrCreateHandle(group), group);
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
    EngineChannel* getChannel(const QString& group);
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
    const CSAMPLE* getSidechainBuffer() const;

    EngineSideChain* getSideChain() const {
        return m_pEngineSideChain;
    }

    struct ChannelInfo {
        ChannelInfo(int index)
                : m_pChannel(NULL),
                  m_pBuffer(NULL),
                  m_pVolumeControl(NULL),
                  m_pMuteControl(NULL),
                  m_index(index) {
        }
        ChannelHandle m_handle;
        EngineChannel* m_pChannel;
        CSAMPLE* m_pBuffer;
        ControlObject* m_pVolumeControl;
        ControlPushButton* m_pMuteControl;
        int m_index;
    };

    struct GainCache {
        CSAMPLE m_gain;
        bool m_fadeout;
    };

    class GainCalculator {
      public:
        virtual double getGain(ChannelInfo* pChannelInfo) const = 0;
    };
    class PflGainCalculator : public GainCalculator {
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
    class TalkoverGainCalculator : public GainCalculator {
      public:
        inline double getGain(ChannelInfo* pChannelInfo) const {
            Q_UNUSED(pChannelInfo);
            return 1.0;
        }
    };
    class OrientationVolumeGainCalculator : public GainCalculator {
      public:
        OrientationVolumeGainCalculator()
                : m_dVolume(1.0),
                  m_dLeftGain(1.0),
                  m_dCenterGain(1.0),
                  m_dRightGain(1.0) {
        }

        inline double getGain(ChannelInfo* pChannelInfo) const {
            const double channelVolume = pChannelInfo->m_pVolumeControl->get();
            const double orientationGain = EngineMaster::gainForOrientation(
                    pChannelInfo->m_pChannel->getOrientation(),
                    m_dLeftGain, m_dCenterGain, m_dRightGain);
            return m_dVolume * channelVolume * orientationGain;
        }

        inline void setGains(double dVolume, double leftGain,
                double centerGain, double rightGain) {
            m_dVolume = dVolume;
            m_dLeftGain = leftGain;
            m_dCenterGain = centerGain;
            m_dRightGain = rightGain;
        }

      private:
        double m_dVolume;
        double m_dLeftGain;
        double m_dCenterGain;
        double m_dRightGain;
    };
    template<typename T, unsigned int CAPACITY>
    class FastVector {
      public:
        inline FastVector() : m_size(0), m_data((T*)((void *)m_buffer)) {};
        inline ~FastVector() {
            if (QTypeInfo<T>::isComplex) {
                for (int i = 0; i < m_size; ++i) {
                    m_data[i].~T();
                }
            }
        }
        inline void append(const T& t) {
            if (QTypeInfo<T>::isComplex) {
                new (&m_data[m_size++]) T(t);
            } else {
                m_data[m_size++] = t;
            }
        };
        inline const T& operator[](unsigned int i) const {
            return m_data[i];
        }
        inline T& operator[](unsigned int i) {
            return m_data[i];
        }
        inline const T& at(unsigned int i) const {
            return m_data[i];
        }
        inline void replace(unsigned int i, const T& t) {
            T copy(t);
            m_data[i] = copy;
        }
        inline int size () const {
            return m_size;
        }
      private:
        int m_size;
        T* const m_data;
        // Using a long double buffer guarantees the alignment for any type
        // but avoids the constructor call T();
        long double m_buffer[(CAPACITY * sizeof(T) + sizeof(long double) - 1) /
                             sizeof(long double)];
    };

  protected:
    // The master buffer is protected so it can be accessed by test subclasses.
    CSAMPLE* m_pMaster;

  private:
    void mixChannels(unsigned int channelBitvector, unsigned int maxChannels,
                     CSAMPLE* pOutput, unsigned int iBufferSize, GainCalculator* pGainCalculator);

    // Processes active channels. The master sync channel (if any) is processed
    // first and all others are processed after. Populates m_activeChannels,
    // m_activeBusChannels, m_activeHeadphoneChannels, and
    // m_activeTalkoverChannels with each channel that is active for the
    // respective output.
    void processChannels(int iBufferSize);

    ChannelHandleFactory m_channelHandleFactory;
    EngineEffectsManager* m_pEngineEffectsManager;
    bool m_bRampingGain;

    // List of channels added to the engine.
    QVarLengthArray<ChannelInfo*, kPreallocatedChannels> m_channels;

    // The previous gain of each channel for each mixing output (master,
    // headphone, talkover).
    QVarLengthArray<GainCache, kPreallocatedChannels> m_channelMasterGainCache;
    QVarLengthArray<GainCache, kPreallocatedChannels> m_channelHeadphoneGainCache;
    QVarLengthArray<GainCache, kPreallocatedChannels> m_channelTalkoverGainCache;

    // Pre-allocated buffers for performing channel mixing in the callback.
    QVarLengthArray<ChannelInfo*, kPreallocatedChannels> m_activeChannels;
    QVarLengthArray<ChannelInfo*, kPreallocatedChannels> m_activeBusChannels[3];
    QVarLengthArray<ChannelInfo*, kPreallocatedChannels> m_activeHeadphoneChannels;
    QVarLengthArray<ChannelInfo*, kPreallocatedChannels> m_activeTalkoverChannels;

    // Mixing buffers for each output.
    CSAMPLE* m_pOutputBusBuffers[3];
    CSAMPLE* m_pHead;
    CSAMPLE* m_pTalkover;

    CSAMPLE** m_ppSidechain; // points to master or to talkover buffer

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
    EngineSideChain* m_pEngineSideChain;

    ControlPotmeter* m_pCrossfader;
    ControlPotmeter* m_pHeadMix;
    ControlPotmeter* m_pBalance;
    ControlPushButton* m_pXFaderMode;
    ControlPotmeter* m_pXFaderCurve;
    ControlPotmeter* m_pXFaderCalibration;
    ControlPushButton* m_pXFaderReverse;
    ControlPushButton* m_pHeadSplitEnabled;
    ControlObject* m_pKeylockEngine;

    PflGainCalculator m_headphoneGain;
    TalkoverGainCalculator m_talkoverGain;
    OrientationVolumeGainCalculator m_masterGain;
    CSAMPLE m_masterGainOld;
    CSAMPLE m_headphoneMasterGainOld;
    CSAMPLE m_headphoneGainOld;

    const ChannelHandleAndGroup m_masterHandle;
    const ChannelHandleAndGroup m_headphoneHandle;
    const ChannelHandleAndGroup m_busLeftHandle;
    const ChannelHandleAndGroup m_busCenterHandle;
    const ChannelHandleAndGroup m_busRightHandle;

    // Produce the Master Mixxx, not Required if connected to left
    // and right Bus and no recording and broadcast active
    ControlObject* m_pMasterEnabled;
    // Mix two Mono channels. This is useful for outdoor gigs
    ControlObject* m_pMasterMonoMixdown;
    ControlObject* m_pMasterTalkoverMix;
    ControlObject* m_pHeadphoneEnabled;

    volatile bool m_bBusOutputConnected[3];
};

#endif
