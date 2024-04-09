#pragma once

#include <QObject>
#include <QVarLengthArray>
#include <atomic>
#include <gsl/pointers>
#include <memory>

#include "audio/types.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/channelhandle.h"
#include "engine/channels/enginechannel.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/engineobject.h"
#include "preferences/usersettings.h"
#include "recording/recordingmanager.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"
#include "util/parented_ptr.h"
#include "util/samplebuffer.h"
#include "util/types.h"

class EngineWorkerScheduler;
class EngineVuMeter;
class ControlPotmeter;
class ControlPushButton;
class EngineSideChain;
class EffectsManager;
class EngineEffectsManager;
class EngineSync;
class EngineTalkoverDucking;
class EngineDelay;

// The number of channels to pre-allocate in various structures in the
// engine. Prevents memory allocation in EngineMixer::addChannel.
static constexpr int kPreallocatedChannels = 64;

class EngineMixer : public QObject, public AudioSource {
    Q_OBJECT
  public:
    EngineMixer(UserSettingsPointer pConfig,
            const QString& group,
            EffectsManager* pEffectsManager,
            ChannelHandleFactoryPointer pChannelHandleFactory,
            bool bEnableSidechain);
    ~EngineMixer() override;

    // Get access to the sample buffers. None of these are thread safe. Only to
    // be called by SoundManager.
    std::span<const CSAMPLE> buffer(const AudioOutput& output) const override;

    ChannelHandleAndGroup registerChannelGroup(const QString& group) {
        return ChannelHandleAndGroup(
                   m_pChannelHandleFactory->getOrCreateHandle(group), group);
    }

    // Register the sound I/O that does not correspond to any EngineChannel object
    void registerNonEngineChannelSoundIO(gsl::not_null<SoundManager*> pSoundManager);

    // WARNING: These methods are called by the main thread. They should only
    // touch the volatile bool connected indicators (see below). However, when
    // these methods are called the callback is guaranteed to be inactive
    // (SoundManager closes all devices before calling these). This may change
    // in the future.
    void onOutputConnected(const AudioOutput& output) override;
    void onOutputDisconnected(const AudioOutput& output) override;
    void onInputConnected(const AudioInput& input);
    void onInputDisconnected(const AudioInput& input);

    void process(const std::size_t bufferSize);

    // Add an EngineChannel to the mixing engine. This is not thread safe --
    // only call it before the engine has started mixing.
    void addChannel(std::unique_ptr<EngineChannel> pChannel);
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

    // Provide access to the sync lock so enginebuffers can know what their rate controller is.
    EngineSync* getEngineSync() const{
        return m_pEngineSync.get();
    }

    // These are really only exposed for tests to use.
    std::span<const CSAMPLE> getMainBuffer() const;
    std::span<const CSAMPLE> getBoothBuffer() const;
    std::span<const CSAMPLE> getHeadphoneBuffer() const;
    std::span<const CSAMPLE> getOutputBusBuffer(unsigned int i) const;
    std::span<const CSAMPLE> getDeckBuffer(unsigned int i) const;
    std::span<const CSAMPLE> getChannelBuffer(const QString& name) const;
    std::span<const CSAMPLE> getSidechainBuffer() const;

    EngineSideChain* getSideChain() const {
        return m_pEngineSideChain.get();
    }

    CSAMPLE_GAIN getMainGain(int channelIndex) const;

    struct ChannelInfo {
        ChannelInfo(int index)
                : m_index(index) {
        }
        ChannelHandle m_handle{};
        std::unique_ptr<EngineChannel> m_pChannel{nullptr};
        mixxx::SampleBuffer m_pBuffer{};
        std::unique_ptr<ControlObject> m_pVolumeControl{nullptr};
        std::unique_ptr<ControlPushButton> m_pMuteControl{nullptr};
        GroupFeatureState m_features{};
        int m_index;
    };

    struct GainCache {
        CSAMPLE_GAIN m_gain;
        bool m_fadeout;
    };

    class GainCalculator {
      public:
        virtual ~GainCalculator() = default;
        virtual CSAMPLE_GAIN getGain(ChannelInfo* pChannelInfo) const = 0;
    };
    class PflGainCalculator final : public GainCalculator {
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
    class TalkoverGainCalculator final : public GainCalculator {
      public:
        inline CSAMPLE_GAIN getGain(ChannelInfo* pChannelInfo) const override {
            return static_cast<CSAMPLE_GAIN>(pChannelInfo->m_pVolumeControl->get());
        }
    };
    class OrientationVolumeGainCalculator final : public GainCalculator {
      public:
        OrientationVolumeGainCalculator()
                : m_dLeftGain(1.0),
                  m_dCenterGain(1.0),
                  m_dRightGain(1.0) {
        }

        inline CSAMPLE_GAIN getGain(ChannelInfo* pChannelInfo) const override {
            const CSAMPLE_GAIN channelVolume = static_cast<CSAMPLE_GAIN>(
                    pChannelInfo->m_pVolumeControl->get());
            const CSAMPLE_GAIN orientationGain = EngineMixer::gainForOrientation(
                    pChannelInfo->m_pChannel->getOrientation(),
                    m_dLeftGain,
                    m_dCenterGain,
                    m_dRightGain);
            return channelVolume * orientationGain;
        }

        inline void setGains(CSAMPLE_GAIN leftGain,
                CSAMPLE_GAIN centerGain,
                CSAMPLE_GAIN rightGain) {
            m_dLeftGain = leftGain;
            m_dCenterGain = centerGain;
            m_dRightGain = rightGain;
        }

      private:
        CSAMPLE_GAIN m_dLeftGain;
        CSAMPLE_GAIN m_dCenterGain;
        CSAMPLE_GAIN m_dRightGain;
    };

    enum class MicMonitorMode {
        // These are out of order with how they are listed in DlgPrefSound for backwards
        // compatibility with Mixxx 2.0 user settings. In Mixxx 2.0, before the
        // booth output was added, this was a binary option without
        // the MainAndBooth mode.
        Main = 0,
        DirectMonitor,
        MainAndBooth,
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
    // The main buffer is protected so it can be accessed by test subclasses.
    mixxx::SampleBuffer m_main;

    // ControlObjects for switching off unnecessary processing
    // These are protected so tests can set them
    std::unique_ptr<ControlObject> m_pMainEnabled;
    std::unique_ptr<ControlObject> m_pHeadphoneEnabled;
    std::unique_ptr<ControlObject> m_pBoothEnabled;

  private:
    // Processes active channels. The sync lock channel (if any) is processed
    // first and all others are processed after. Populates m_activeChannels,
    // m_activeBusChannels, m_activeHeadphoneChannels, and
    // m_activeTalkoverChannels with each channel that is active for the
    // respective output.
    void processChannels(std::size_t bufferSize);

    ChannelHandleFactoryPointer m_pChannelHandleFactory;
    void applyMainEffects(std::size_t bufferSize);
    void processHeadphones(
            const CSAMPLE_GAIN mainMixGainInHeadphones,
            std::size_t bufferSize);
    bool sidechainMixRequired() const;

    // non-owning. lifetime bound to EffectsManager
    EngineEffectsManager* m_pEngineEffectsManager;

    // List of channels added to the engine.
    QVarLengthArray<std::unique_ptr<ChannelInfo>, kPreallocatedChannels> m_channels;

    // The previous gain of each channel for each mixing output (main,
    // headphone, talkover).
    QVarLengthArray<GainCache, kPreallocatedChannels> m_channelMainGainCache;
    QVarLengthArray<GainCache, kPreallocatedChannels> m_channelHeadphoneGainCache;
    QVarLengthArray<GainCache, kPreallocatedChannels> m_channelTalkoverGainCache;

    // Pre-allocated buffers for performing channel mixing in the callback.
    QVarLengthArray<ChannelInfo*, kPreallocatedChannels> m_activeChannels;
    QVarLengthArray<ChannelInfo*, kPreallocatedChannels> m_activeBusChannels[3];
    QVarLengthArray<ChannelInfo*, kPreallocatedChannels> m_activeHeadphoneChannels;
    QVarLengthArray<ChannelInfo*, kPreallocatedChannels> m_activeTalkoverChannels;

    mixxx::audio::SampleRate m_sampleRate;

    // Mixing buffers for each output.
    std::array<mixxx::SampleBuffer, 3> m_outputBusBuffers;
    mixxx::SampleBuffer m_booth;
    mixxx::SampleBuffer m_head;
    mixxx::SampleBuffer m_talkover;
    mixxx::SampleBuffer m_talkoverHeadphones;
    mixxx::SampleBuffer m_sidechainMix;

    parented_ptr<EngineWorkerScheduler> m_pWorkerScheduler;
    std::unique_ptr<EngineSync> m_pEngineSync;

    std::unique_ptr<ControlObject> m_pMainGain;
    std::unique_ptr<ControlObject> m_pBoothGain;
    std::unique_ptr<ControlObject> m_pHeadGain;
    std::unique_ptr<ControlObject> m_pSampleRate;
    std::unique_ptr<ControlObject> m_pOutputLatencyMs;
    std::unique_ptr<ControlObject> m_pAudioLatencyOverloadCount;
    std::unique_ptr<ControlObject> m_pAudioLatencyUsage;
    std::unique_ptr<ControlObject> m_pAudioLatencyOverload;
    std::unique_ptr<EngineTalkoverDucking> m_pTalkoverDucking;
    std::unique_ptr<EngineDelay> m_pMainDelay;
    std::unique_ptr<EngineDelay> m_pHeadDelay;
    std::unique_ptr<EngineDelay> m_pBoothDelay;
    std::unique_ptr<EngineDelay> m_pLatencyCompensationDelay;

    std::unique_ptr<EngineVuMeter> m_pVumeter;
    std::unique_ptr<EngineSideChain> m_pEngineSideChain;

    std::unique_ptr<ControlPotmeter> m_pCrossfader;
    std::unique_ptr<ControlPotmeter> m_pHeadMix;
    std::unique_ptr<ControlPotmeter> m_pBalance;
    std::unique_ptr<ControlPushButton> m_pXFaderMode;
    std::unique_ptr<ControlPotmeter> m_pXFaderCurve;
    std::unique_ptr<ControlPotmeter> m_pXFaderCalibration;
    std::unique_ptr<ControlPushButton> m_pXFaderReverse;
    std::unique_ptr<ControlPushButton> m_pHeadSplitEnabled;
    std::unique_ptr<ControlObject> m_pKeylockEngine;

    PflGainCalculator m_headphoneGain;
    TalkoverGainCalculator m_talkoverGain;
    OrientationVolumeGainCalculator m_mainGain;
    CSAMPLE_GAIN m_mainGainOld;
    CSAMPLE_GAIN m_boothGainOld;
    CSAMPLE_GAIN m_headphoneMainGainOld;
    CSAMPLE_GAIN m_headphoneGainOld;
    CSAMPLE_GAIN m_duckingGainOld;
    CSAMPLE_GAIN m_balleftOld;
    CSAMPLE_GAIN m_balrightOld;
    std::atomic<unsigned int> m_numMicsConfigured;
    const ChannelHandleAndGroup m_mainHandle;
    const ChannelHandleAndGroup m_headphoneHandle;
    const ChannelHandleAndGroup m_mainOutputHandle;
    const ChannelHandleAndGroup m_busTalkoverHandle;
    const ChannelHandleAndGroup m_busCrossfaderLeftHandle;
    const ChannelHandleAndGroup m_busCrossfaderCenterHandle;
    const ChannelHandleAndGroup m_busCrossfaderRightHandle;

    // Mix two Mono channels. This is useful for outdoor gigs
    std::unique_ptr<ControlObject> m_pMainMonoMixdown;
    std::unique_ptr<ControlObject> m_pMicMonitorMode;

    // TODO (Swiftb0y): remove volatile (probably supposed to be std::atomic instead).
    volatile bool m_bBusOutputConnected[3];
    bool m_bExternalRecordBroadcastInputConnected;
};
