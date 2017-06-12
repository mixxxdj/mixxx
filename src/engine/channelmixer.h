#ifndef CHANNELMIXER_H
#define CHANNELMIXER_H

#include <QVarLengthArray>

#include "util/types.h"
#include "engine/enginemaster.h"
#include "effects/engineeffectsmanager.h"

class ChannelMixer {
  public:
    static void mixChannels(
        const EngineMaster::GainCalculator& gainCalculator,
        QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
        QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
        CSAMPLE* pOutput,
        unsigned int iBufferSize);
    static void mixChannelsRamping(
        const EngineMaster::GainCalculator& gainCalculator,
        QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
        QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
        CSAMPLE* pOutput,
        unsigned int iBufferSize);
    static void applyEffectsInPlaceAndMixChannels(
        const EngineMaster::GainCalculator& gainCalculator,
        QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
        QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
        CSAMPLE* pOutput, const ChannelHandle& outputHandle,
        unsigned int iBufferSize,
        unsigned int iSampleRate,
        EngineEffectsManager* pEngineEffectsManager);
    static void applyEffectsInPlaceAndMixChannelsRamping(
        const EngineMaster::GainCalculator& gainCalculator,
        QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
        QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
        CSAMPLE* pOutput, const ChannelHandle& outputHandle,
        unsigned int iBufferSize,
        unsigned int iSampleRate,
        EngineEffectsManager* pEngineEffectsManager);
    static void applyEffectsAndMixChannels(
        const EngineMaster::GainCalculator& gainCalculator,
        QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
        QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
        CSAMPLE* pOutput, const ChannelHandle& outputHandle,
        unsigned int iBufferSize,
        unsigned int iSampleRate,
        EngineEffectsManager* pEngineEffectsManager);
    static void applyEffectsAndMixChannelsRamping(
        const EngineMaster::GainCalculator& gainCalculator,
        QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
        QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
        CSAMPLE* pOutput, const ChannelHandle& outputHandle,
        unsigned int iBufferSize,
        unsigned int iSampleRate,
        EngineEffectsManager* pEngineEffectsManager);
};

#endif /* CHANNELMIXER_H */
