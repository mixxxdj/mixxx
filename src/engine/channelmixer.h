#pragma once

#include <QVarLengthArray>

#include "util/types.h"
#include "engine/enginemaster.h"
#include "effects/engineeffectsmanager.h"

class ChannelMixer {
  public:
    // This does not modify the input channel buffers. All manipulation of the input
    // channel buffers is done after copying to a temporary buffer, then they are mixed
    // to make the output buffer.
    static void applyEffectsAndMixChannels(
        const EngineMaster::GainCalculator& gainCalculator,
        QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
        QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
        CSAMPLE* pOutput, const ChannelHandle& outputHandle,
        unsigned int iBufferSize,
        unsigned int iSampleRate,
        EngineEffectsManager* pEngineEffectsManager);
    // This does modify the input channel buffers, then mixes them to make the output buffer.
    static void applyEffectsInPlaceAndMixChannels(
        const EngineMaster::GainCalculator& gainCalculator,
        QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
        QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
        CSAMPLE* pOutput, const ChannelHandle& outputHandle,
        unsigned int iBufferSize,
        unsigned int iSampleRate,
        EngineEffectsManager* pEngineEffectsManager);
};
