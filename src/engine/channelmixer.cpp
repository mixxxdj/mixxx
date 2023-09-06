#include "engine/channelmixer.h"

#include "util/sample.h"
#include "util/timer.h"

// static
void ChannelMixer::applyEffectsAndMixChannels(const EngineMaster::GainCalculator& gainCalculator,
        const QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>& activeChannels,
        QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
        CSAMPLE* pOutput,
        const ChannelHandle& outputHandle,
        unsigned int iBufferSize,
        unsigned int iSampleRate,
        EngineEffectsManager* pEngineEffectsManager) {
    // Signal flow overview:
    // 1. Clear pOutput buffer
    // 2. Calculate gains for each channel
    // 3. Pass each channel's calculated gain and input buffer to pEngineEffectsManager, which then:
    //     A) Copies each channel input buffer to a temporary buffer
    //     B) Applies gain to the temporary buffer
    //     C) Processes effects on the temporary buffer
    //     D) Mixes the temporary buffer into pOutput
    // The original channel input buffers are not modified.
    SampleUtil::clear(pOutput, iBufferSize);
    ScopedTimer t(u"EngineMaster::applyEffectsAndMixChannels");
    for (auto* pChannelInfo : activeChannels) {
        EngineMaster::GainCache& gainCache = (*channelGainCache)[pChannelInfo->m_index];
        CSAMPLE_GAIN oldGain = gainCache.m_gain;
        CSAMPLE_GAIN newGain;
        bool fadeout = gainCache.m_fadeout ||
                (pChannelInfo->m_pChannel &&
                        !pChannelInfo->m_pChannel->isActive());
        if (fadeout) {
            newGain = 0;
            gainCache.m_fadeout = false;
        } else {
            newGain = gainCalculator.getGain(pChannelInfo);
        }
        gainCache.m_gain = newGain;
        pEngineEffectsManager->processPostFaderAndMix(pChannelInfo->m_handle,
                outputHandle,
                pChannelInfo->m_pBuffer,
                pOutput,
                iBufferSize,
                iSampleRate,
                pChannelInfo->m_features,
                oldGain,
                newGain,
                fadeout);
    }
}

void ChannelMixer::applyEffectsInPlaceAndMixChannels(
        const EngineMaster::GainCalculator& gainCalculator,
        const QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>&
                activeChannels,
        QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>*
                channelGainCache,
        CSAMPLE* pOutput,
        const ChannelHandle& outputHandle,
        unsigned int iBufferSize,
        unsigned int iSampleRate,
        EngineEffectsManager* pEngineEffectsManager) {
    // Signal flow overview:
    // 1. Calculate gains for each channel
    // 2. Pass each channel's calculated gain and input buffer to pEngineEffectsManager, which then:
    //    A) Applies the calculated gain to the channel buffer, modifying the original input buffer
    //    B) Applies effects to the buffer, modifying the original input buffer
    // 4. Mix the channel buffers together to make pOutput, overwriting the pOutput buffer from the last engine callback
    ScopedTimer t(u"EngineMaster::applyEffectsInPlaceAndMixChannels");
    SampleUtil::clear(pOutput, iBufferSize);
    for (auto* pChannelInfo : activeChannels) {
        EngineMaster::GainCache& gainCache = (*channelGainCache)[pChannelInfo->m_index];
        CSAMPLE_GAIN oldGain = gainCache.m_gain;
        CSAMPLE_GAIN newGain;
        bool fadeout = gainCache.m_fadeout ||
                (pChannelInfo->m_pChannel &&
                        !pChannelInfo->m_pChannel->isActive());
        if (fadeout) {
            newGain = 0;
            gainCache.m_fadeout = false;
        } else {
            newGain = gainCalculator.getGain(pChannelInfo);
        }
        gainCache.m_gain = newGain;
        pEngineEffectsManager->processPostFaderInPlace(pChannelInfo->m_handle,
                outputHandle,
                pChannelInfo->m_pBuffer,
                iBufferSize,
                iSampleRate,
                pChannelInfo->m_features,
                oldGain,
                newGain,
                fadeout);
        SampleUtil::add(pOutput, pChannelInfo->m_pBuffer, iBufferSize);
    }
}
