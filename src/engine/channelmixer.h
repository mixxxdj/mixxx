#ifndef CHANNELMIXER_H
#define CHANNELMIXER_H

#include "defs.h"
#include "engine/enginemaster.h"

class ChannelMixer {
  public:
    static void mixChannels(
        const QList<EngineMaster::ChannelInfo*>& channels,
        const EngineMaster::GainCalculator& gainCalculator,
        unsigned int channelBitvector,
        unsigned int maxChannels,
        QList<CSAMPLE>* channelGainCache,
        CSAMPLE* pOutput,
        unsigned int iBufferSize);
    static void mixChannelsRamping(
        const QList<EngineMaster::ChannelInfo*>& channels,
        const EngineMaster::GainCalculator& gainCalculator,
        unsigned int channelBitvector,
        unsigned int maxChannels,
        QList<CSAMPLE>* channelGainCache,
        CSAMPLE* pOutput,
        unsigned int iBufferSize);
};

#endif /* CHANNELMIXER_H */
