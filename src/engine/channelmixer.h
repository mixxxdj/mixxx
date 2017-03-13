#ifndef CHANNELMIXER_H
#define CHANNELMIXER_H

#include "util/types.h"
#include "engine/enginemaster.h"

class ChannelMixer {
  public:
    static void mixChannels(
        const EngineMaster::GainCalculator& gainCalculator,
        EngineMaster::FastVector<EngineMaster::ChannelInfo*, kMaxChannels>* activeChannels,
        EngineMaster::FastVector<EngineMaster::GainCache, kMaxChannels>* channelGainCache,
        CSAMPLE* pOutput,
        unsigned int iBufferSize);
    static void mixChannelsRamping(
        const EngineMaster::GainCalculator& gainCalculator,
        EngineMaster::FastVector<EngineMaster::ChannelInfo*, kMaxChannels>* activeChannels,
        EngineMaster::FastVector<EngineMaster::GainCache, kMaxChannels>* channelGainCache,
        CSAMPLE* pOutput,
        unsigned int iBufferSize);
};

#endif /* CHANNELMIXER_H */
