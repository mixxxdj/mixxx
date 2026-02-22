#pragma once

#include <QVarLengthArray>

#include "audio/types.h"
#include "engine/enginemixer.h"
#include "util/types.h"

class ChannelMixer {
  public:
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
            unsigned int iBufferSize,
            mixxx::audio::SampleRate sampleRate,
            EngineEffectsManager* pEngineEffectsManager);
    // This does modify the input channel buffers, then mixes them to make the output buffer.
    static void applyEffectsInPlaceAndMixChannels(
            const EngineMixer::GainCalculator& gainCalculator,
            const QVarLengthArray<EngineMixer::ChannelInfo*,
                    kPreallocatedChannels>& activeChannels,
            QVarLengthArray<EngineMixer::GainCache, kPreallocatedChannels>*
                    channelGainCache,
            CSAMPLE* pOutput,
            const ChannelHandle& outputHandle,
            unsigned int iBufferSize,
            mixxx::audio::SampleRate sampleRate,
            EngineEffectsManager* pEngineEffectsManager);
};
