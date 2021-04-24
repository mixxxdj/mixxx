#pragma once

#include <QObject>
#include <QVarLengthArray>

#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/engineobject.h"
#include "engine/channels/enginechannel.h"
#include "engine/channelhandle.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"
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
    EngineMaster(UserSettingsPointer pConfig,
            const QString& group,
            EffectsManager* pEffectsManager,
            ChannelHandleFactoryPointer pChannelHandleFactory,
            bool bEnableSidechain);
    virtual ~EngineMaster();

    // Get access to the sample buffers. None of these are thread safe. Only to
    // be called by SoundManager.
    const CSAMPLE* buffer(const AudioOutput& output) const;

    ChannelHandleAndGroup registerChannelGroup(const QString& group) {
        return ChannelHandleAndGroup(
                   m_pChannelHandleFactory->getOrCreateHandle(group), group);
    }

    // Register the sound I/O that does not correspond to any EngineChannel object
    void registerNonEngineChannelSoundIO(SoundManager* pSoundManager);

    // WARNING: These methods are called by the main thread. They should only
    // touch the volatile bool connected indicators (see below). However, when
    // these methods are called the callback is guaranteed to be inactive
    // (SoundManager closes all devices before calling these). This may change
    // in the future.
    virtual void onOutputConnected(const AudioOutput& output);
    virtual void onOutputDisconnected(const AudioOutput& output);
    void onInputConnected(const AudioInput& input);
    void onInputDisconnected(const AudioInput& input);

    void process(const int iBufferSize);

    // Add an EngineChannel to the mixing engine. This is not thread safe --
    // only call it before the engine has started mixing.
    void addChannel(EngineChannel* pChannel);
    EngineChannel* getChannel(const QString& group);
    static inline CSAMPLE_GAIN gainForOrientation(EngineChannel::ChannelOrientation orientation,
            CSAMPLE_GAIN leftGain,
            CSAMPLE_GAIN centerGain,
            CSAMPLE_GAIN rightGain) {
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
    const CSAMPLE* getBoothBuffer() const;
    const CSAMPLE* getHeadphoneBuffer() const;
    const CSAMPLE* getOutputBusBuffer(unsigned int i) const;
    const CSAMPLE* getDeckBuffer(unsigned int i) const;
    const CSAMPLE* getChannelBuffer(const QString& name) const;
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
        GroupFeatureState m_features;
        int m_index;
    };

    struct GainCache {
        CSAMPLE m_gain;
        bool m_fadeout;
    };

    class GainCalculator {
      public:
        virtual ~GainCalculator() = default;
        virtual CSAMPLE_GAIN getGain(ChannelInfo* pChannelInfo) const = 0;
    };
    class PflGainCalculator : public GainCalculator {
      public:
        inline CSAMPLE_GAIN getGain(ChannelInfo* pChannelInfo) const override {
            Q_UNUSED(pChannelInfo);
            return m_dGain;
        }
        inline void setGain(CSAMPLE_GAIN dGain) {
            m_dGain = dGain;
        }

      private:
        CSAMPLE_GAIN m_dGain;
    };
    class TalkoverGainCalculator : public GainCalculator {
      public:
        inline CSAMPLE_GAIN getGain(ChannelInfo* pChannelInfo) const override {
            return static_cast<CSAMPLE_GAIN>(pChannelInfo->m_pVolumeControl->get());
        }
    };
    class OrientationVolumeGainCalculator : public GainCalculator {
      public:
        OrientationVolumeGainCalculator()
                : m_dLeftGain(1.0),
                  m_dCenterGain(1.0),
                  m_dRightGain(1.0),
                  m_dTalkoverDuckingGain(1.0) {
        }

        inline CSAMPLE_GAIN getGain(ChannelInfo* pChannelInfo) const {
            const CSAMPLE_GAIN channelVolume = static_cast<CSAMPLE_GAIN>(
                    pChannelInfo->m_pVolumeControl->get());
            const CSAMPLE_GAIN orientationGain = EngineMaster::gainForOrientation(
                    pChannelInfo->m_pChannel->getOrientation(),
                    m_dLeftGain,
                    m_dCenterGain,
                    m_dRightGain);
            return channelVolume * orientationGain * m_dTalkoverDuckingGain;
        }

        inline void setGains(CSAMPLE_GAIN leftGain,
                CSAMPLE_GAIN centerGain,
                CSAMPLE_GAIN rightGain,
                CSAMPLE_GAIN talkoverDuckingGain) {
            m_dLeftGain = leftGain;
            m_dCenterGain = centerGain;
            m_dRightGain = rightGain;
            m_dTalkoverDuckingGain = talkoverDuckingGain;
        }

      private:
        CSAMPLE_GAIN m_dLeftGain;
        CSAMPLE_GAIN m_dCenterGain;
        CSAMPLE_GAIN m_dRightGain;
        CSAMPLE_GAIN m_dTalkoverDuckingGain;
    };

    enum class MicMonitorMode {
        // These are out of order with how they are listed in DlgPrefSound for backwards
        // compatibility with Mixxx 2.0 user settings. In Mixxx 2.0, before the
        // booth output was added, this was a binary option without
        // the MASTER_AND_BOOTH mode.
        MASTER = 0,
        DIRECT_MONITOR,
        MASTER_AND_BOOTH
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

    // ControlObjects for switching off unnecessary processing
    // These are protected so tests can set them
    ControlObject* m_pMasterEnabled;
    ControlObject* m_pHeadphoneEnabled;
    ControlObject* m_pBoothEnabled;

  private:
    // Processes active channels. The master sync channel (if any) is processed
    // first and all others are processed after. Populates m_activeChannels,
    // m_activeBusChannels, m_activeHeadphoneChannels, and
    // m_activeTalkoverChannels with each channel that is active for the
    // respective output.
    void processChannels(int iBufferSize);

    ChannelHandleFactoryPointer m_pChannelHandleFactory;
    void applyMasterEffects();
    void processHeadphones(const CSAMPLE_GAIN masterMixGainInHeadphones);
    bool sidechainMixRequired() const;

    EngineEffectsManager* m_pEngineEffectsManager;

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

    unsigned int m_iSampleRate;
    unsigned int m_iBufferSize;

    // Mixing buffers for each output.
    CSAMPLE* m_pOutputBusBuffers[3];
    CSAMPLE* m_pBooth;
    CSAMPLE* m_pHead;
    CSAMPLE* m_pTalkover;
    CSAMPLE* m_pTalkoverHeadphones;
    CSAMPLE* m_pSidechainMix;

    EngineWorkerScheduler* m_pWorkerScheduler;
    EngineSync* m_pMasterSync;

    ControlObject* m_pMasterGain;
    ControlObject* m_pBoothGain;
    ControlObject* m_pHeadGain;
    ControlObject* m_pMasterSampleRate;
    ControlObject* m_pMasterLatency;
    ControlObject* m_pMasterAudioBufferSize;
    ControlObject* m_pAudioLatencyOverloadCount;
    ControlObject* m_pNumMicsConfigured;
    ControlPotmeter* m_pAudioLatencyUsage;
    ControlPotmeter* m_pAudioLatencyOverload;
    EngineTalkoverDucking* m_pTalkoverDucking;
    EngineDelay* m_pMasterDelay;
    EngineDelay* m_pHeadDelay;
    EngineDelay* m_pBoothDelay;
    EngineDelay* m_pLatencyCompensationDelay;

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
    CSAMPLE_GAIN m_masterGainOld;
    CSAMPLE_GAIN m_boothGainOld;
    CSAMPLE_GAIN m_headphoneMasterGainOld;
    CSAMPLE_GAIN m_headphoneGainOld;
    CSAMPLE_GAIN m_balleftOld;
    CSAMPLE_GAIN m_balrightOld;
    const ChannelHandleAndGroup m_masterHandle;
    const ChannelHandleAndGroup m_headphoneHandle;
    const ChannelHandleAndGroup m_masterOutputHandle;
    const ChannelHandleAndGroup m_busTalkoverHandle;
    const ChannelHandleAndGroup m_busCrossfaderLeftHandle;
    const ChannelHandleAndGroup m_busCrossfaderCenterHandle;
    const ChannelHandleAndGroup m_busCrossfaderRightHandle;

    // Mix two Mono channels. This is useful for outdoor gigs
    ControlObject* m_pMasterMonoMixdown;
    ControlObject* m_pMicMonitorMode;

    volatile bool m_bBusOutputConnected[3];
    bool m_bExternalRecordBroadcastInputConnected;
};
