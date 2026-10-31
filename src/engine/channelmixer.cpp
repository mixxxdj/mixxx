#include "engine/channelmixer.h"

#include "engine/effects/engineeffectsmanager.h"
#include "util/assert.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/timer.h"

namespace {

/// Rebuilds the contribution of a channel that exposes sub-channels (a deck
/// playing a stem track) at the same point of the signal flow where regular
/// channels apply their Postfader effect chains: after the fader gains.
///
/// For every sub-channel the fader gain (volume, orientation, crossfader) is
/// applied first and then the Postfader chains registered for the sub-channel
/// handle are processed (the stem QuickEffect and effects assigned to a stem).
/// The results are mixed into pMixBuffer, on which the deck's Prefader chain
/// (the equalizer) and the deck's own Postfader chains (the deck QuickEffect
/// and effects assigned to the deck group) are applied afterwards.
///
/// This way the effect tails keep sounding when the deck is paused or its
/// volume/crossfader is closed, just like for regular stereo channels
/// (see issue #16718).
///
/// pTempBuffer receives one sub-channel at a time and pMixBuffer receives the
/// mixed contribution. Both must hold at least bufferSize samples.
void mixSubChannel(EngineMixer::ChannelInfo* pChannelInfo,
        CSAMPLE* pMixBuffer,
        CSAMPLE* pTempBuffer,
        const ChannelHandle& outputHandle,
        std::size_t bufferSize,
        mixxx::audio::SampleRate sampleRate,
        EngineEffectsManager* pEngineEffectsManager,
        CSAMPLE_GAIN oldGain,
        CSAMPLE_GAIN newGain,
        bool fadeout) {
    EngineChannel* pChannel = pChannelInfo->m_pChannel.get();
    DEBUG_ASSERT(pChannel != nullptr);
    const int subChannelCount = pChannel->subChannelCount();
    DEBUG_ASSERT(subChannelCount > 0);

    SampleUtil::clear(pMixBuffer, bufferSize);
    for (int i = 0; i < subChannelCount; ++i) {
        pChannel->copySubChannel(pTempBuffer, i, bufferSize);
        // Apply the gain first, so that the effects below run post-fader.
        SampleUtil::applyRampingGain(pTempBuffer, oldGain, newGain, bufferSize);
        if (pEngineEffectsManager) {
            pEngineEffectsManager->processPostFaderInPlace(
                    pChannel->subChannelHandle(i),
                    outputHandle,
                    pTempBuffer,
                    bufferSize,
                    sampleRate,
                    pChannelInfo->m_features,
                    CSAMPLE_GAIN_ONE,
                    CSAMPLE_GAIN_ONE,
                    fadeout);
        }
        SampleUtil::add(pMixBuffer, pTempBuffer, bufferSize);
    }
    if (pEngineEffectsManager) {
        // The equalizer is normally applied in EngineDeck before mixing.
        // Here it is applied to the rebuilt mix instead, because the downmix
        // from EngineDeck is not used for this channel.
        pEngineEffectsManager->processPreFaderInPlace(pChannelInfo->m_handle,
                outputHandle,
                pMixBuffer,
                bufferSize,
                sampleRate);
        // The gain has already been applied above, so the deck's own Postfader
        // chains only need to process the mix.
        pEngineEffectsManager->processPostFaderInPlace(pChannelInfo->m_handle,
                outputHandle,
                pMixBuffer,
                bufferSize,
                sampleRate,
                pChannelInfo->m_features,
                CSAMPLE_GAIN_ONE,
                CSAMPLE_GAIN_ONE,
                fadeout);
    }
}

// Returns the number of sub-channels of the channel, 0 for regular channels.
int subChannelCount(const EngineMixer::ChannelInfo* pChannelInfo) {
    return pChannelInfo->m_pChannel ? pChannelInfo->m_pChannel->subChannelCount() : 0;
}

} // anonymous namespace

// static
void ChannelMixer::applyEffectsAndMixChannels(const EngineMixer::GainCalculator& gainCalculator,
        const QVarLengthArray<EngineMixer::ChannelInfo*, kPreallocatedChannels>& activeChannels,
        QVarLengthArray<EngineMixer::GainCache, kPreallocatedChannels>* channelGainCache,
        CSAMPLE* pOutput,
        const ChannelHandle& outputHandle,
        std::size_t bufferSize,
        mixxx::audio::SampleRate sampleRate,
        EngineEffectsManager* pEngineEffectsManager,
        CSAMPLE* pTempBuffer) {
    // Signal flow overview:
    // 1. Clear pOutput buffer
    // 2. Calculate gains for each channel
    // 3. Pass each channel's calculated gain and input buffer to pEngineEffectsManager, which then:
    //     A) Copies each channel input buffer to a temporary buffer
    //     B) Applies gain to the temporary buffer
    //     C) Processes effects on the temporary buffer
    //     D) Mixes the temporary buffer into pOutput
    //     Channels with sub-channels (stem decks) rebuild their contribution
    //     from the single stems in the temporary buffer instead, so that the
    //     per-stem effects run after the gain as well.
    // The original channel input buffers are not modified.
    SampleUtil::clear(pOutput, bufferSize);
    ScopedTimer t(QStringLiteral("EngineMixer::applyEffectsAndMixChannels"));
    for (auto* pChannelInfo : activeChannels) {
        EngineMixer::GainCache& gainCache = (*channelGainCache)[pChannelInfo->m_index];
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
        if (subChannelCount(pChannelInfo) > 0) {
            // Build the post-fader contribution in the second half of the
            // temporary buffer; the channel input buffer is not modified.
            CSAMPLE* pMixBuffer = pTempBuffer + kMaxEngineSamples;
            mixSubChannel(pChannelInfo,
                    pMixBuffer,
                    pTempBuffer,
                    outputHandle,
                    bufferSize,
                    sampleRate,
                    pEngineEffectsManager,
                    oldGain,
                    newGain,
                    fadeout);
            SampleUtil::add(pOutput, pMixBuffer, bufferSize);
        } else {
            pEngineEffectsManager->processPostFaderAndMix(pChannelInfo->m_handle,
                    outputHandle,
                    pChannelInfo->m_pBuffer.data(),
                    pOutput,
                    bufferSize,
                    sampleRate,
                    pChannelInfo->m_features,
                    oldGain,
                    newGain,
                    fadeout);
        }
    }
}

void ChannelMixer::applyEffectsInPlaceAndMixChannels(
        const EngineMixer::GainCalculator& gainCalculator,
        const QVarLengthArray<EngineMixer::ChannelInfo*, kPreallocatedChannels>&
                activeChannels,
        QVarLengthArray<EngineMixer::GainCache, kPreallocatedChannels>*
                channelGainCache,
        CSAMPLE* pOutput,
        const ChannelHandle& outputHandle,
        std::size_t bufferSize,
        mixxx::audio::SampleRate sampleRate,
        EngineEffectsManager* pEngineEffectsManager,
        CSAMPLE* pTempBuffer) {
    // Signal flow overview:
    // 1. Calculate gains for each channel
    // 2. Pass each channel's calculated gain and input buffer to pEngineEffectsManager, which then:
    //    A) Applies the calculated gain to the channel buffer, modifying the original input buffer
    //    B) Applies effects to the buffer, modifying the original input buffer
    // 4. Mix the channel buffers together to make pOutput, overwriting the pOutput buffer from the last engine callback
    ScopedTimer t(QStringLiteral("EngineMixer::applyEffectsInPlaceAndMixChannels"));
    SampleUtil::clear(pOutput, bufferSize);
    for (auto* pChannelInfo : activeChannels) {
        EngineMixer::GainCache& gainCache = (*channelGainCache)[pChannelInfo->m_index];
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
        if (subChannelCount(pChannelInfo) > 0) {
            // Rebuild the channel buffer from the single sub-channels, with
            // the gain applied and the per-stem effects processed, followed
            // by the equalizer and the deck's own effects (see mixSubChannel).
            mixSubChannel(pChannelInfo,
                    pChannelInfo->m_pBuffer.data(),
                    pTempBuffer,
                    outputHandle,
                    bufferSize,
                    sampleRate,
                    pEngineEffectsManager,
                    oldGain,
                    newGain,
                    fadeout);
        } else {
            pEngineEffectsManager->processPostFaderInPlace(pChannelInfo->m_handle,
                    outputHandle,
                    pChannelInfo->m_pBuffer.data(),
                    bufferSize,
                    sampleRate,
                    pChannelInfo->m_features,
                    oldGain,
                    newGain,
                    fadeout);
        }
        SampleUtil::add(pOutput, pChannelInfo->m_pBuffer.data(), bufferSize);
    }
}
