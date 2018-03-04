#include "engine/channelmixer.h"
#include "util/sample.h"
#include "util/timer.h"
////////////////////////////////////////////////////////
// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //
// SEE scripts/generate_sample_functions.py           //
////////////////////////////////////////////////////////

// static
void ChannelMixer::applyEffectsAndMixChannels(const EngineMaster::GainCalculator& gainCalculator,
                                              QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
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
    int totalActive = activeChannels->size();
    SampleUtil::clear(pOutput, iBufferSize);
    if (totalActive == 0) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_0active");
    } else if (totalActive == 1) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_1active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
    } else if (totalActive == 2) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_2active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
    } else if (totalActive == 3) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_3active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
    } else if (totalActive == 4) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_4active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
    } else if (totalActive == 5) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_5active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
    } else if (totalActive == 6) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_6active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
    } else if (totalActive == 7) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_7active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
    } else if (totalActive == 8) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_8active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
    } else if (totalActive == 9) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_9active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
    } else if (totalActive == 10) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_10active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
    } else if (totalActive == 11) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_11active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
    } else if (totalActive == 12) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_12active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
    } else if (totalActive == 13) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_13active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
    } else if (totalActive == 14) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_14active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
    } else if (totalActive == 15) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_15active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
    } else if (totalActive == 16) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_16active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
    } else if (totalActive == 17) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_17active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
    } else if (totalActive == 18) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_18active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
    } else if (totalActive == 19) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_19active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
    } else if (totalActive == 20) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_20active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
    } else if (totalActive == 21) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_21active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
    } else if (totalActive == 22) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_22active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features);
    } else if (totalActive == 23) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_23active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features);
    } else if (totalActive == 24) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_24active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features);
    } else if (totalActive == 25) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_25active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features);
    } else if (totalActive == 26) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_26active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features);
    } else if (totalActive == 27) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_27active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features);
    } else if (totalActive == 28) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_28active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        pChannel27->m_features.volume_fader_old = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            pChannel27->m_features.volume_fader_new = 0;
            gainCache27.m_fadeout = false;
        } else {
            pChannel27->m_features.volume_fader_new = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = pChannel27->m_features.volume_fader_new;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel27->m_handle, outputHandle, pBuffer27, pOutput, iBufferSize, iSampleRate, pChannel27->m_features);
    } else if (totalActive == 29) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_29active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        pChannel27->m_features.volume_fader_old = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            pChannel27->m_features.volume_fader_new = 0;
            gainCache27.m_fadeout = false;
        } else {
            pChannel27->m_features.volume_fader_new = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = pChannel27->m_features.volume_fader_new;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        pChannel28->m_features.volume_fader_old = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            pChannel28->m_features.volume_fader_new = 0;
            gainCache28.m_fadeout = false;
        } else {
            pChannel28->m_features.volume_fader_new = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = pChannel28->m_features.volume_fader_new;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel27->m_handle, outputHandle, pBuffer27, pOutput, iBufferSize, iSampleRate, pChannel27->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel28->m_handle, outputHandle, pBuffer28, pOutput, iBufferSize, iSampleRate, pChannel28->m_features);
    } else if (totalActive == 30) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_30active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        pChannel27->m_features.volume_fader_old = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            pChannel27->m_features.volume_fader_new = 0;
            gainCache27.m_fadeout = false;
        } else {
            pChannel27->m_features.volume_fader_new = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = pChannel27->m_features.volume_fader_new;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        pChannel28->m_features.volume_fader_old = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            pChannel28->m_features.volume_fader_new = 0;
            gainCache28.m_fadeout = false;
        } else {
            pChannel28->m_features.volume_fader_new = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = pChannel28->m_features.volume_fader_new;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        pChannel29->m_features.volume_fader_old = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            pChannel29->m_features.volume_fader_new = 0;
            gainCache29.m_fadeout = false;
        } else {
            pChannel29->m_features.volume_fader_new = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = pChannel29->m_features.volume_fader_new;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel27->m_handle, outputHandle, pBuffer27, pOutput, iBufferSize, iSampleRate, pChannel27->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel28->m_handle, outputHandle, pBuffer28, pOutput, iBufferSize, iSampleRate, pChannel28->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel29->m_handle, outputHandle, pBuffer29, pOutput, iBufferSize, iSampleRate, pChannel29->m_features);
    } else if (totalActive == 31) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_31active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        pChannel27->m_features.volume_fader_old = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            pChannel27->m_features.volume_fader_new = 0;
            gainCache27.m_fadeout = false;
        } else {
            pChannel27->m_features.volume_fader_new = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = pChannel27->m_features.volume_fader_new;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        pChannel28->m_features.volume_fader_old = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            pChannel28->m_features.volume_fader_new = 0;
            gainCache28.m_fadeout = false;
        } else {
            pChannel28->m_features.volume_fader_new = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = pChannel28->m_features.volume_fader_new;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        pChannel29->m_features.volume_fader_old = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            pChannel29->m_features.volume_fader_new = 0;
            gainCache29.m_fadeout = false;
        } else {
            pChannel29->m_features.volume_fader_new = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = pChannel29->m_features.volume_fader_new;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int channelIndex30 = pChannel30->m_index;
        EngineMaster::GainCache& gainCache30 = (*channelGainCache)[channelIndex30];
        pChannel30->m_features.volume_fader_old = gainCache30.m_gain;
        if (gainCache30.m_fadeout) {
            pChannel30->m_features.volume_fader_new = 0;
            gainCache30.m_fadeout = false;
        } else {
            pChannel30->m_features.volume_fader_new = gainCalculator.getGain(pChannel30);
        }
        gainCache30.m_gain = pChannel30->m_features.volume_fader_new;
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel27->m_handle, outputHandle, pBuffer27, pOutput, iBufferSize, iSampleRate, pChannel27->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel28->m_handle, outputHandle, pBuffer28, pOutput, iBufferSize, iSampleRate, pChannel28->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel29->m_handle, outputHandle, pBuffer29, pOutput, iBufferSize, iSampleRate, pChannel29->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel30->m_handle, outputHandle, pBuffer30, pOutput, iBufferSize, iSampleRate, pChannel30->m_features);
    } else if (totalActive == 32) {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_32active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        pChannel27->m_features.volume_fader_old = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            pChannel27->m_features.volume_fader_new = 0;
            gainCache27.m_fadeout = false;
        } else {
            pChannel27->m_features.volume_fader_new = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = pChannel27->m_features.volume_fader_new;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        pChannel28->m_features.volume_fader_old = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            pChannel28->m_features.volume_fader_new = 0;
            gainCache28.m_fadeout = false;
        } else {
            pChannel28->m_features.volume_fader_new = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = pChannel28->m_features.volume_fader_new;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        pChannel29->m_features.volume_fader_old = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            pChannel29->m_features.volume_fader_new = 0;
            gainCache29.m_fadeout = false;
        } else {
            pChannel29->m_features.volume_fader_new = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = pChannel29->m_features.volume_fader_new;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int channelIndex30 = pChannel30->m_index;
        EngineMaster::GainCache& gainCache30 = (*channelGainCache)[channelIndex30];
        pChannel30->m_features.volume_fader_old = gainCache30.m_gain;
        if (gainCache30.m_fadeout) {
            pChannel30->m_features.volume_fader_new = 0;
            gainCache30.m_fadeout = false;
        } else {
            pChannel30->m_features.volume_fader_new = gainCalculator.getGain(pChannel30);
        }
        gainCache30.m_gain = pChannel30->m_features.volume_fader_new;
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel31 = activeChannels->at(31);
        const int channelIndex31 = pChannel31->m_index;
        EngineMaster::GainCache& gainCache31 = (*channelGainCache)[channelIndex31];
        pChannel31->m_features.volume_fader_old = gainCache31.m_gain;
        if (gainCache31.m_fadeout) {
            pChannel31->m_features.volume_fader_new = 0;
            gainCache31.m_fadeout = false;
        } else {
            pChannel31->m_features.volume_fader_new = gainCalculator.getGain(pChannel31);
        }
        gainCache31.m_gain = pChannel31->m_features.volume_fader_new;
        CSAMPLE* pBuffer31 = pChannel31->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel27->m_handle, outputHandle, pBuffer27, pOutput, iBufferSize, iSampleRate, pChannel27->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel28->m_handle, outputHandle, pBuffer28, pOutput, iBufferSize, iSampleRate, pChannel28->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel29->m_handle, outputHandle, pBuffer29, pOutput, iBufferSize, iSampleRate, pChannel29->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel30->m_handle, outputHandle, pBuffer30, pOutput, iBufferSize, iSampleRate, pChannel30->m_features);
        pEngineEffectsManager->processPostFaderAndMix(pChannel31->m_handle, outputHandle, pBuffer31, pOutput, iBufferSize, iSampleRate, pChannel31->m_features);
    } else {
        ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_Over32active");
        for (int i = 0; i < activeChannels->size(); ++i) {
            EngineMaster::ChannelInfo* pChannelInfo = activeChannels->at(i);
            const int channelIndex = pChannelInfo->m_index;
            EngineMaster::GainCache& gainCache = (*channelGainCache)[channelIndex];
            pChannelInfo->m_features.volume_fader_old = gainCache.m_gain;
            if (gainCache.m_fadeout) {
                pChannelInfo->m_features.volume_fader_new = 0;
                gainCache.m_fadeout = false;
            } else {
                pChannelInfo->m_features.volume_fader_new = gainCalculator.getGain(pChannelInfo);
            }
            gainCache.m_gain = pChannelInfo->m_features.volume_fader_new;
            CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;
            pEngineEffectsManager->processPostFaderAndMix(pChannelInfo->m_handle, outputHandle, pBuffer, pOutput, iBufferSize, iSampleRate, pChannelInfo->m_features);
        }
    }
}
void ChannelMixer::applyEffectsInPlaceAndMixChannels(const EngineMaster::GainCalculator& gainCalculator,
                                                     QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
                                                     QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
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
    int totalActive = activeChannels->size();
    if (totalActive == 0) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_0active");
        SampleUtil::clear(pOutput, iBufferSize);
    } else if (totalActive == 1) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_1active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i];
        }
    } else if (totalActive == 2) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_2active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i];
        }
    } else if (totalActive == 3) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_3active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i];
        }
    } else if (totalActive == 4) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_4active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i];
        }
    } else if (totalActive == 5) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_5active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i];
        }
    } else if (totalActive == 6) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_6active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i];
        }
    } else if (totalActive == 7) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_7active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i];
        }
    } else if (totalActive == 8) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_8active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i];
        }
    } else if (totalActive == 9) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_9active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i];
        }
    } else if (totalActive == 10) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_10active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i];
        }
    } else if (totalActive == 11) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_11active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i];
        }
    } else if (totalActive == 12) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_12active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i];
        }
    } else if (totalActive == 13) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_13active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i];
        }
    } else if (totalActive == 14) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_14active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i];
        }
    } else if (totalActive == 15) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_15active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i];
        }
    } else if (totalActive == 16) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_16active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i];
        }
    } else if (totalActive == 17) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_17active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i];
        }
    } else if (totalActive == 18) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_18active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i];
        }
    } else if (totalActive == 19) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_19active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i];
        }
    } else if (totalActive == 20) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_20active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i];
        }
    } else if (totalActive == 21) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_21active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i];
        }
    } else if (totalActive == 22) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_22active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i];
        }
    } else if (totalActive == 23) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_23active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i];
        }
    } else if (totalActive == 24) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_24active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i];
        }
    } else if (totalActive == 25) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_25active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i];
        }
    } else if (totalActive == 26) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_26active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i];
        }
    } else if (totalActive == 27) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_27active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i];
        }
    } else if (totalActive == 28) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_28active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        pChannel27->m_features.volume_fader_old = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            pChannel27->m_features.volume_fader_new = 0;
            gainCache27.m_fadeout = false;
        } else {
            pChannel27->m_features.volume_fader_new = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = pChannel27->m_features.volume_fader_new;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel27->m_handle, outputHandle, pBuffer27, iBufferSize, iSampleRate, pChannel27->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i] + pBuffer27[i];
        }
    } else if (totalActive == 29) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_29active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        pChannel27->m_features.volume_fader_old = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            pChannel27->m_features.volume_fader_new = 0;
            gainCache27.m_fadeout = false;
        } else {
            pChannel27->m_features.volume_fader_new = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = pChannel27->m_features.volume_fader_new;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        pChannel28->m_features.volume_fader_old = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            pChannel28->m_features.volume_fader_new = 0;
            gainCache28.m_fadeout = false;
        } else {
            pChannel28->m_features.volume_fader_new = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = pChannel28->m_features.volume_fader_new;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel27->m_handle, outputHandle, pBuffer27, iBufferSize, iSampleRate, pChannel27->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel28->m_handle, outputHandle, pBuffer28, iBufferSize, iSampleRate, pChannel28->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i] + pBuffer27[i] + pBuffer28[i];
        }
    } else if (totalActive == 30) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_30active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        pChannel27->m_features.volume_fader_old = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            pChannel27->m_features.volume_fader_new = 0;
            gainCache27.m_fadeout = false;
        } else {
            pChannel27->m_features.volume_fader_new = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = pChannel27->m_features.volume_fader_new;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        pChannel28->m_features.volume_fader_old = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            pChannel28->m_features.volume_fader_new = 0;
            gainCache28.m_fadeout = false;
        } else {
            pChannel28->m_features.volume_fader_new = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = pChannel28->m_features.volume_fader_new;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        pChannel29->m_features.volume_fader_old = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            pChannel29->m_features.volume_fader_new = 0;
            gainCache29.m_fadeout = false;
        } else {
            pChannel29->m_features.volume_fader_new = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = pChannel29->m_features.volume_fader_new;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel27->m_handle, outputHandle, pBuffer27, iBufferSize, iSampleRate, pChannel27->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel28->m_handle, outputHandle, pBuffer28, iBufferSize, iSampleRate, pChannel28->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel29->m_handle, outputHandle, pBuffer29, iBufferSize, iSampleRate, pChannel29->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i] + pBuffer27[i] + pBuffer28[i] + pBuffer29[i];
        }
    } else if (totalActive == 31) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_31active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        pChannel27->m_features.volume_fader_old = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            pChannel27->m_features.volume_fader_new = 0;
            gainCache27.m_fadeout = false;
        } else {
            pChannel27->m_features.volume_fader_new = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = pChannel27->m_features.volume_fader_new;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        pChannel28->m_features.volume_fader_old = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            pChannel28->m_features.volume_fader_new = 0;
            gainCache28.m_fadeout = false;
        } else {
            pChannel28->m_features.volume_fader_new = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = pChannel28->m_features.volume_fader_new;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        pChannel29->m_features.volume_fader_old = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            pChannel29->m_features.volume_fader_new = 0;
            gainCache29.m_fadeout = false;
        } else {
            pChannel29->m_features.volume_fader_new = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = pChannel29->m_features.volume_fader_new;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int channelIndex30 = pChannel30->m_index;
        EngineMaster::GainCache& gainCache30 = (*channelGainCache)[channelIndex30];
        pChannel30->m_features.volume_fader_old = gainCache30.m_gain;
        if (gainCache30.m_fadeout) {
            pChannel30->m_features.volume_fader_new = 0;
            gainCache30.m_fadeout = false;
        } else {
            pChannel30->m_features.volume_fader_new = gainCalculator.getGain(pChannel30);
        }
        gainCache30.m_gain = pChannel30->m_features.volume_fader_new;
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel27->m_handle, outputHandle, pBuffer27, iBufferSize, iSampleRate, pChannel27->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel28->m_handle, outputHandle, pBuffer28, iBufferSize, iSampleRate, pChannel28->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel29->m_handle, outputHandle, pBuffer29, iBufferSize, iSampleRate, pChannel29->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel30->m_handle, outputHandle, pBuffer30, iBufferSize, iSampleRate, pChannel30->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i] + pBuffer27[i] + pBuffer28[i] + pBuffer29[i] + pBuffer30[i];
        }
    } else if (totalActive == 32) {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_32active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        pChannel0->m_features.volume_fader_old = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            pChannel0->m_features.volume_fader_new = 0;
            gainCache0.m_fadeout = false;
        } else {
            pChannel0->m_features.volume_fader_new = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = pChannel0->m_features.volume_fader_new;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        pChannel1->m_features.volume_fader_old = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            pChannel1->m_features.volume_fader_new = 0;
            gainCache1.m_fadeout = false;
        } else {
            pChannel1->m_features.volume_fader_new = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = pChannel1->m_features.volume_fader_new;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        pChannel2->m_features.volume_fader_old = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            pChannel2->m_features.volume_fader_new = 0;
            gainCache2.m_fadeout = false;
        } else {
            pChannel2->m_features.volume_fader_new = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = pChannel2->m_features.volume_fader_new;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        pChannel3->m_features.volume_fader_old = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            pChannel3->m_features.volume_fader_new = 0;
            gainCache3.m_fadeout = false;
        } else {
            pChannel3->m_features.volume_fader_new = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = pChannel3->m_features.volume_fader_new;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        pChannel4->m_features.volume_fader_old = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            pChannel4->m_features.volume_fader_new = 0;
            gainCache4.m_fadeout = false;
        } else {
            pChannel4->m_features.volume_fader_new = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = pChannel4->m_features.volume_fader_new;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        pChannel5->m_features.volume_fader_old = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            pChannel5->m_features.volume_fader_new = 0;
            gainCache5.m_fadeout = false;
        } else {
            pChannel5->m_features.volume_fader_new = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = pChannel5->m_features.volume_fader_new;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        pChannel6->m_features.volume_fader_old = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            pChannel6->m_features.volume_fader_new = 0;
            gainCache6.m_fadeout = false;
        } else {
            pChannel6->m_features.volume_fader_new = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = pChannel6->m_features.volume_fader_new;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        pChannel7->m_features.volume_fader_old = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            pChannel7->m_features.volume_fader_new = 0;
            gainCache7.m_fadeout = false;
        } else {
            pChannel7->m_features.volume_fader_new = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = pChannel7->m_features.volume_fader_new;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        pChannel8->m_features.volume_fader_old = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            pChannel8->m_features.volume_fader_new = 0;
            gainCache8.m_fadeout = false;
        } else {
            pChannel8->m_features.volume_fader_new = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = pChannel8->m_features.volume_fader_new;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        pChannel9->m_features.volume_fader_old = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            pChannel9->m_features.volume_fader_new = 0;
            gainCache9.m_fadeout = false;
        } else {
            pChannel9->m_features.volume_fader_new = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = pChannel9->m_features.volume_fader_new;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        pChannel10->m_features.volume_fader_old = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            pChannel10->m_features.volume_fader_new = 0;
            gainCache10.m_fadeout = false;
        } else {
            pChannel10->m_features.volume_fader_new = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = pChannel10->m_features.volume_fader_new;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        pChannel11->m_features.volume_fader_old = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            pChannel11->m_features.volume_fader_new = 0;
            gainCache11.m_fadeout = false;
        } else {
            pChannel11->m_features.volume_fader_new = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = pChannel11->m_features.volume_fader_new;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        pChannel12->m_features.volume_fader_old = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            pChannel12->m_features.volume_fader_new = 0;
            gainCache12.m_fadeout = false;
        } else {
            pChannel12->m_features.volume_fader_new = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = pChannel12->m_features.volume_fader_new;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        pChannel13->m_features.volume_fader_old = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            pChannel13->m_features.volume_fader_new = 0;
            gainCache13.m_fadeout = false;
        } else {
            pChannel13->m_features.volume_fader_new = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = pChannel13->m_features.volume_fader_new;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        pChannel14->m_features.volume_fader_old = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            pChannel14->m_features.volume_fader_new = 0;
            gainCache14.m_fadeout = false;
        } else {
            pChannel14->m_features.volume_fader_new = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = pChannel14->m_features.volume_fader_new;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        pChannel15->m_features.volume_fader_old = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            pChannel15->m_features.volume_fader_new = 0;
            gainCache15.m_fadeout = false;
        } else {
            pChannel15->m_features.volume_fader_new = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = pChannel15->m_features.volume_fader_new;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        pChannel16->m_features.volume_fader_old = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            pChannel16->m_features.volume_fader_new = 0;
            gainCache16.m_fadeout = false;
        } else {
            pChannel16->m_features.volume_fader_new = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = pChannel16->m_features.volume_fader_new;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        pChannel17->m_features.volume_fader_old = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            pChannel17->m_features.volume_fader_new = 0;
            gainCache17.m_fadeout = false;
        } else {
            pChannel17->m_features.volume_fader_new = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = pChannel17->m_features.volume_fader_new;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        pChannel18->m_features.volume_fader_old = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            pChannel18->m_features.volume_fader_new = 0;
            gainCache18.m_fadeout = false;
        } else {
            pChannel18->m_features.volume_fader_new = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = pChannel18->m_features.volume_fader_new;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        pChannel19->m_features.volume_fader_old = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            pChannel19->m_features.volume_fader_new = 0;
            gainCache19.m_fadeout = false;
        } else {
            pChannel19->m_features.volume_fader_new = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = pChannel19->m_features.volume_fader_new;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        pChannel20->m_features.volume_fader_old = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            pChannel20->m_features.volume_fader_new = 0;
            gainCache20.m_fadeout = false;
        } else {
            pChannel20->m_features.volume_fader_new = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = pChannel20->m_features.volume_fader_new;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        pChannel21->m_features.volume_fader_old = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            pChannel21->m_features.volume_fader_new = 0;
            gainCache21.m_fadeout = false;
        } else {
            pChannel21->m_features.volume_fader_new = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = pChannel21->m_features.volume_fader_new;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        pChannel22->m_features.volume_fader_old = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            pChannel22->m_features.volume_fader_new = 0;
            gainCache22.m_fadeout = false;
        } else {
            pChannel22->m_features.volume_fader_new = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = pChannel22->m_features.volume_fader_new;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        pChannel23->m_features.volume_fader_old = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            pChannel23->m_features.volume_fader_new = 0;
            gainCache23.m_fadeout = false;
        } else {
            pChannel23->m_features.volume_fader_new = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = pChannel23->m_features.volume_fader_new;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        pChannel24->m_features.volume_fader_old = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            pChannel24->m_features.volume_fader_new = 0;
            gainCache24.m_fadeout = false;
        } else {
            pChannel24->m_features.volume_fader_new = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = pChannel24->m_features.volume_fader_new;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        pChannel25->m_features.volume_fader_old = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            pChannel25->m_features.volume_fader_new = 0;
            gainCache25.m_fadeout = false;
        } else {
            pChannel25->m_features.volume_fader_new = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = pChannel25->m_features.volume_fader_new;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        pChannel26->m_features.volume_fader_old = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            pChannel26->m_features.volume_fader_new = 0;
            gainCache26.m_fadeout = false;
        } else {
            pChannel26->m_features.volume_fader_new = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = pChannel26->m_features.volume_fader_new;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        pChannel27->m_features.volume_fader_old = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            pChannel27->m_features.volume_fader_new = 0;
            gainCache27.m_fadeout = false;
        } else {
            pChannel27->m_features.volume_fader_new = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = pChannel27->m_features.volume_fader_new;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        pChannel28->m_features.volume_fader_old = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            pChannel28->m_features.volume_fader_new = 0;
            gainCache28.m_fadeout = false;
        } else {
            pChannel28->m_features.volume_fader_new = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = pChannel28->m_features.volume_fader_new;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        pChannel29->m_features.volume_fader_old = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            pChannel29->m_features.volume_fader_new = 0;
            gainCache29.m_fadeout = false;
        } else {
            pChannel29->m_features.volume_fader_new = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = pChannel29->m_features.volume_fader_new;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int channelIndex30 = pChannel30->m_index;
        EngineMaster::GainCache& gainCache30 = (*channelGainCache)[channelIndex30];
        pChannel30->m_features.volume_fader_old = gainCache30.m_gain;
        if (gainCache30.m_fadeout) {
            pChannel30->m_features.volume_fader_new = 0;
            gainCache30.m_fadeout = false;
        } else {
            pChannel30->m_features.volume_fader_new = gainCalculator.getGain(pChannel30);
        }
        gainCache30.m_gain = pChannel30->m_features.volume_fader_new;
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel31 = activeChannels->at(31);
        const int channelIndex31 = pChannel31->m_index;
        EngineMaster::GainCache& gainCache31 = (*channelGainCache)[channelIndex31];
        pChannel31->m_features.volume_fader_old = gainCache31.m_gain;
        if (gainCache31.m_fadeout) {
            pChannel31->m_features.volume_fader_new = 0;
            gainCache31.m_fadeout = false;
        } else {
            pChannel31->m_features.volume_fader_new = gainCalculator.getGain(pChannel31);
        }
        gainCache31.m_gain = pChannel31->m_features.volume_fader_new;
        CSAMPLE* pBuffer31 = pChannel31->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel27->m_handle, outputHandle, pBuffer27, iBufferSize, iSampleRate, pChannel27->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel28->m_handle, outputHandle, pBuffer28, iBufferSize, iSampleRate, pChannel28->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel29->m_handle, outputHandle, pBuffer29, iBufferSize, iSampleRate, pChannel29->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel30->m_handle, outputHandle, pBuffer30, iBufferSize, iSampleRate, pChannel30->m_features);
        pEngineEffectsManager->processPostFaderInPlace(pChannel31->m_handle, outputHandle, pBuffer31, iBufferSize, iSampleRate, pChannel31->m_features);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i] + pBuffer27[i] + pBuffer28[i] + pBuffer29[i] + pBuffer30[i] + pBuffer31[i];
        }
    } else {
        ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_Over32active");
        for (int i = 0; i < activeChannels->size(); ++i) {
            EngineMaster::ChannelInfo* pChannelInfo = activeChannels->at(i);
            const int channelIndex = pChannelInfo->m_index;
            EngineMaster::GainCache& gainCache = (*channelGainCache)[channelIndex];
            pChannelInfo->m_features.volume_fader_old = gainCache.m_gain;
            if (gainCache.m_fadeout) {
                pChannelInfo->m_features.volume_fader_new = 0;
                gainCache.m_fadeout = false;
            } else {
                pChannelInfo->m_features.volume_fader_new = gainCalculator.getGain(pChannelInfo);
            }
            gainCache.m_gain = pChannelInfo->m_features.volume_fader_new;
            CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;
            pEngineEffectsManager->processPostFaderInPlace(pChannelInfo->m_handle, outputHandle, pBuffer, iBufferSize, iSampleRate, pChannelInfo->m_features);
            for (unsigned int i = 0; i < iBufferSize; ++i) {
                pOutput[i] += pBuffer[i];
            }
        }
    }
}
