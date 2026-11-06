#pragma once

#include <QVarLengthArray>

#include "audio/types.h"
#include "engine/enginemixer.h"
#include "util/types.h"

class ChannelMixer {
  public:
    // Temporary buffer for the functions below: pTempBuffer must point to at
    // least 2 * kMaxEngineSamples samples. The first kMaxEngineSamples samples
    // are used as a per-sub-channel work buffer, the second half is used as
    // the mix buffer of applyEffectsAndMixChannels().

    // This does not modify the input channel buffers. All manipulation of the input
    // channel buffers is done after copying to a temporary buffer, then they are mixed
    // to make the output buffer.
    static void applyEffectsAndMixChannels(
            const EngineMixer::GainCalculator& gainCalculator,
            const QVarLengthArray<EngineMixer::ChannelInfo*,
                    kPreallocatedChannels>& activeChannels,
            QVarLengthArray<EngineMixer::GainCache, kPreallocatedChannels>*
                    channelGainCache,
            CSAMPLE* pOutput,
            const ChannelHandle& outputHandle,
            std::size_t bufferSize,
            mixxx::audio::SampleRate sampleRate,
            EngineEffectsManager* pEngineEffectsManager,
            CSAMPLE* pTempBuffer);
    // This does modify the input channel buffers, then mixes them to make the output buffer.
    static void applyEffectsInPlaceAndMixChannels(
            const EngineMixer::GainCalculator& gainCalculator,
            const QVarLengthArray<EngineMixer::ChannelInfo*,
                    kPreallocatedChannels>& activeChannels,
            QVarLengthArray<EngineMixer::GainCache, kPreallocatedChannels>*
                    channelGainCache,
            CSAMPLE* pOutput,
            const ChannelHandle& outputHandle,
            std::size_t bufferSize,
            mixxx::audio::SampleRate sampleRate,
            EngineEffectsManager* pEngineEffectsManager,
            CSAMPLE* pTempBuffer);
};
