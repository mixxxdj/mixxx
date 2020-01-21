#include "engine/channelmixer.h"
#include "util/sample.h"
#include "util/timer.h"
////////////////////////////////////////////////////////
// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //
// SEE tools/generate_sample_functions.py             //
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
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_0active");
    } else if (totalActive == 1) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_1active");
        CSAMPLE_GAIN oldGain[1];
        CSAMPLE_GAIN newGain[1];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
    } else if (totalActive == 2) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_2active");
        CSAMPLE_GAIN oldGain[2];
        CSAMPLE_GAIN newGain[2];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
    } else if (totalActive == 3) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_3active");
        CSAMPLE_GAIN oldGain[3];
        CSAMPLE_GAIN newGain[3];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
    } else if (totalActive == 4) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_4active");
        CSAMPLE_GAIN oldGain[4];
        CSAMPLE_GAIN newGain[4];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
    } else if (totalActive == 5) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_5active");
        CSAMPLE_GAIN oldGain[5];
        CSAMPLE_GAIN newGain[5];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
    } else if (totalActive == 6) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_6active");
        CSAMPLE_GAIN oldGain[6];
        CSAMPLE_GAIN newGain[6];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
    } else if (totalActive == 7) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_7active");
        CSAMPLE_GAIN oldGain[7];
        CSAMPLE_GAIN newGain[7];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
    } else if (totalActive == 8) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_8active");
        CSAMPLE_GAIN oldGain[8];
        CSAMPLE_GAIN newGain[8];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
    } else if (totalActive == 9) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_9active");
        CSAMPLE_GAIN oldGain[9];
        CSAMPLE_GAIN newGain[9];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
    } else if (totalActive == 10) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_10active");
        CSAMPLE_GAIN oldGain[10];
        CSAMPLE_GAIN newGain[10];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
    } else if (totalActive == 11) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_11active");
        CSAMPLE_GAIN oldGain[11];
        CSAMPLE_GAIN newGain[11];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
    } else if (totalActive == 12) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_12active");
        CSAMPLE_GAIN oldGain[12];
        CSAMPLE_GAIN newGain[12];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
    } else if (totalActive == 13) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_13active");
        CSAMPLE_GAIN oldGain[13];
        CSAMPLE_GAIN newGain[13];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
    } else if (totalActive == 14) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_14active");
        CSAMPLE_GAIN oldGain[14];
        CSAMPLE_GAIN newGain[14];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
    } else if (totalActive == 15) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_15active");
        CSAMPLE_GAIN oldGain[15];
        CSAMPLE_GAIN newGain[15];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
    } else if (totalActive == 16) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_16active");
        CSAMPLE_GAIN oldGain[16];
        CSAMPLE_GAIN newGain[16];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
    } else if (totalActive == 17) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_17active");
        CSAMPLE_GAIN oldGain[17];
        CSAMPLE_GAIN newGain[17];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
    } else if (totalActive == 18) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_18active");
        CSAMPLE_GAIN oldGain[18];
        CSAMPLE_GAIN newGain[18];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
    } else if (totalActive == 19) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_19active");
        CSAMPLE_GAIN oldGain[19];
        CSAMPLE_GAIN newGain[19];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
    } else if (totalActive == 20) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_20active");
        CSAMPLE_GAIN oldGain[20];
        CSAMPLE_GAIN newGain[20];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
    } else if (totalActive == 21) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_21active");
        CSAMPLE_GAIN oldGain[21];
        CSAMPLE_GAIN newGain[21];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
    } else if (totalActive == 22) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_22active");
        CSAMPLE_GAIN oldGain[22];
        CSAMPLE_GAIN newGain[22];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
    } else if (totalActive == 23) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_23active");
        CSAMPLE_GAIN oldGain[23];
        CSAMPLE_GAIN newGain[23];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
    } else if (totalActive == 24) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_24active");
        CSAMPLE_GAIN oldGain[24];
        CSAMPLE_GAIN newGain[24];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
    } else if (totalActive == 25) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_25active");
        CSAMPLE_GAIN oldGain[25];
        CSAMPLE_GAIN newGain[25];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
    } else if (totalActive == 26) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_26active");
        CSAMPLE_GAIN oldGain[26];
        CSAMPLE_GAIN newGain[26];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
    } else if (totalActive == 27) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_27active");
        CSAMPLE_GAIN oldGain[27];
        CSAMPLE_GAIN newGain[27];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
    } else if (totalActive == 28) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_28active");
        CSAMPLE_GAIN oldGain[28];
        CSAMPLE_GAIN newGain[28];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        oldGain[27] = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            newGain[27] = 0;
            gainCache27.m_fadeout = false;
        } else {
            newGain[27] = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel27->m_handle, outputHandle, pBuffer27, pOutput, iBufferSize, iSampleRate, pChannel27->m_features, oldGain[27], newGain[27]);
    } else if (totalActive == 29) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_29active");
        CSAMPLE_GAIN oldGain[29];
        CSAMPLE_GAIN newGain[29];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        oldGain[27] = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            newGain[27] = 0;
            gainCache27.m_fadeout = false;
        } else {
            newGain[27] = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        oldGain[28] = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            newGain[28] = 0;
            gainCache28.m_fadeout = false;
        } else {
            newGain[28] = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel27->m_handle, outputHandle, pBuffer27, pOutput, iBufferSize, iSampleRate, pChannel27->m_features, oldGain[27], newGain[27]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel28->m_handle, outputHandle, pBuffer28, pOutput, iBufferSize, iSampleRate, pChannel28->m_features, oldGain[28], newGain[28]);
    } else if (totalActive == 30) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_30active");
        CSAMPLE_GAIN oldGain[30];
        CSAMPLE_GAIN newGain[30];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        oldGain[27] = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            newGain[27] = 0;
            gainCache27.m_fadeout = false;
        } else {
            newGain[27] = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        oldGain[28] = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            newGain[28] = 0;
            gainCache28.m_fadeout = false;
        } else {
            newGain[28] = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        oldGain[29] = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            newGain[29] = 0;
            gainCache29.m_fadeout = false;
        } else {
            newGain[29] = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = newGain[29];
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel27->m_handle, outputHandle, pBuffer27, pOutput, iBufferSize, iSampleRate, pChannel27->m_features, oldGain[27], newGain[27]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel28->m_handle, outputHandle, pBuffer28, pOutput, iBufferSize, iSampleRate, pChannel28->m_features, oldGain[28], newGain[28]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel29->m_handle, outputHandle, pBuffer29, pOutput, iBufferSize, iSampleRate, pChannel29->m_features, oldGain[29], newGain[29]);
    } else if (totalActive == 31) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_31active");
        CSAMPLE_GAIN oldGain[31];
        CSAMPLE_GAIN newGain[31];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        oldGain[27] = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            newGain[27] = 0;
            gainCache27.m_fadeout = false;
        } else {
            newGain[27] = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        oldGain[28] = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            newGain[28] = 0;
            gainCache28.m_fadeout = false;
        } else {
            newGain[28] = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        oldGain[29] = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            newGain[29] = 0;
            gainCache29.m_fadeout = false;
        } else {
            newGain[29] = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = newGain[29];
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int channelIndex30 = pChannel30->m_index;
        EngineMaster::GainCache& gainCache30 = (*channelGainCache)[channelIndex30];
        oldGain[30] = gainCache30.m_gain;
        if (gainCache30.m_fadeout) {
            newGain[30] = 0;
            gainCache30.m_fadeout = false;
        } else {
            newGain[30] = gainCalculator.getGain(pChannel30);
        }
        gainCache30.m_gain = newGain[30];
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel27->m_handle, outputHandle, pBuffer27, pOutput, iBufferSize, iSampleRate, pChannel27->m_features, oldGain[27], newGain[27]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel28->m_handle, outputHandle, pBuffer28, pOutput, iBufferSize, iSampleRate, pChannel28->m_features, oldGain[28], newGain[28]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel29->m_handle, outputHandle, pBuffer29, pOutput, iBufferSize, iSampleRate, pChannel29->m_features, oldGain[29], newGain[29]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel30->m_handle, outputHandle, pBuffer30, pOutput, iBufferSize, iSampleRate, pChannel30->m_features, oldGain[30], newGain[30]);
    } else if (totalActive == 32) {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_32active");
        CSAMPLE_GAIN oldGain[32];
        CSAMPLE_GAIN newGain[32];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        oldGain[27] = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            newGain[27] = 0;
            gainCache27.m_fadeout = false;
        } else {
            newGain[27] = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        oldGain[28] = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            newGain[28] = 0;
            gainCache28.m_fadeout = false;
        } else {
            newGain[28] = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        oldGain[29] = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            newGain[29] = 0;
            gainCache29.m_fadeout = false;
        } else {
            newGain[29] = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = newGain[29];
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int channelIndex30 = pChannel30->m_index;
        EngineMaster::GainCache& gainCache30 = (*channelGainCache)[channelIndex30];
        oldGain[30] = gainCache30.m_gain;
        if (gainCache30.m_fadeout) {
            newGain[30] = 0;
            gainCache30.m_fadeout = false;
        } else {
            newGain[30] = gainCalculator.getGain(pChannel30);
        }
        gainCache30.m_gain = newGain[30];
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel31 = activeChannels->at(31);
        const int channelIndex31 = pChannel31->m_index;
        EngineMaster::GainCache& gainCache31 = (*channelGainCache)[channelIndex31];
        oldGain[31] = gainCache31.m_gain;
        if (gainCache31.m_fadeout) {
            newGain[31] = 0;
            gainCache31.m_fadeout = false;
        } else {
            newGain[31] = gainCalculator.getGain(pChannel31);
        }
        gainCache31.m_gain = newGain[31];
        CSAMPLE* pBuffer31 = pChannel31->m_pBuffer;
        // Process effects for each channel and mix the processed signal into pOutput
        pEngineEffectsManager->processPostFaderAndMix(pChannel0->m_handle, outputHandle, pBuffer0, pOutput, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel1->m_handle, outputHandle, pBuffer1, pOutput, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel2->m_handle, outputHandle, pBuffer2, pOutput, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel3->m_handle, outputHandle, pBuffer3, pOutput, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel4->m_handle, outputHandle, pBuffer4, pOutput, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel5->m_handle, outputHandle, pBuffer5, pOutput, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel6->m_handle, outputHandle, pBuffer6, pOutput, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel7->m_handle, outputHandle, pBuffer7, pOutput, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel8->m_handle, outputHandle, pBuffer8, pOutput, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel9->m_handle, outputHandle, pBuffer9, pOutput, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel10->m_handle, outputHandle, pBuffer10, pOutput, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel11->m_handle, outputHandle, pBuffer11, pOutput, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel12->m_handle, outputHandle, pBuffer12, pOutput, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel13->m_handle, outputHandle, pBuffer13, pOutput, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel14->m_handle, outputHandle, pBuffer14, pOutput, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel15->m_handle, outputHandle, pBuffer15, pOutput, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel16->m_handle, outputHandle, pBuffer16, pOutput, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel17->m_handle, outputHandle, pBuffer17, pOutput, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel18->m_handle, outputHandle, pBuffer18, pOutput, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel19->m_handle, outputHandle, pBuffer19, pOutput, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel20->m_handle, outputHandle, pBuffer20, pOutput, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel21->m_handle, outputHandle, pBuffer21, pOutput, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel22->m_handle, outputHandle, pBuffer22, pOutput, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel23->m_handle, outputHandle, pBuffer23, pOutput, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel24->m_handle, outputHandle, pBuffer24, pOutput, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel25->m_handle, outputHandle, pBuffer25, pOutput, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel26->m_handle, outputHandle, pBuffer26, pOutput, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel27->m_handle, outputHandle, pBuffer27, pOutput, iBufferSize, iSampleRate, pChannel27->m_features, oldGain[27], newGain[27]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel28->m_handle, outputHandle, pBuffer28, pOutput, iBufferSize, iSampleRate, pChannel28->m_features, oldGain[28], newGain[28]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel29->m_handle, outputHandle, pBuffer29, pOutput, iBufferSize, iSampleRate, pChannel29->m_features, oldGain[29], newGain[29]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel30->m_handle, outputHandle, pBuffer30, pOutput, iBufferSize, iSampleRate, pChannel30->m_features, oldGain[30], newGain[30]);
        pEngineEffectsManager->processPostFaderAndMix(pChannel31->m_handle, outputHandle, pBuffer31, pOutput, iBufferSize, iSampleRate, pChannel31->m_features, oldGain[31], newGain[31]);
    } else {
        //ScopedTimer t("EngineMaster::applyEffectsAndMixChannels_Over32active");
        for (int i = 0; i < activeChannels->size(); ++i) {
            EngineMaster::ChannelInfo* pChannelInfo = activeChannels->at(i);
            const int channelIndex = pChannelInfo->m_index;
            EngineMaster::GainCache& gainCache = (*channelGainCache)[channelIndex];
            CSAMPLE_GAIN oldGain = gainCache.m_gain;
            CSAMPLE_GAIN newGain;
            if (gainCache.m_fadeout) {
                newGain = 0;
                gainCache.m_fadeout = false;
            } else {
                newGain = gainCalculator.getGain(pChannelInfo);
            }
            gainCache.m_gain = newGain;
            CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;
            pEngineEffectsManager->processPostFaderAndMix(pChannelInfo->m_handle, outputHandle, pBuffer, pOutput, iBufferSize, iSampleRate, pChannelInfo->m_features, oldGain, newGain);
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
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_0active");
        SampleUtil::clear(pOutput, iBufferSize);
    } else if (totalActive == 1) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_1active");
        CSAMPLE_GAIN oldGain[1];
        CSAMPLE_GAIN newGain[1];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i];
        }
    } else if (totalActive == 2) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_2active");
        CSAMPLE_GAIN oldGain[2];
        CSAMPLE_GAIN newGain[2];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i];
        }
    } else if (totalActive == 3) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_3active");
        CSAMPLE_GAIN oldGain[3];
        CSAMPLE_GAIN newGain[3];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i];
        }
    } else if (totalActive == 4) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_4active");
        CSAMPLE_GAIN oldGain[4];
        CSAMPLE_GAIN newGain[4];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i];
        }
    } else if (totalActive == 5) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_5active");
        CSAMPLE_GAIN oldGain[5];
        CSAMPLE_GAIN newGain[5];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i];
        }
    } else if (totalActive == 6) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_6active");
        CSAMPLE_GAIN oldGain[6];
        CSAMPLE_GAIN newGain[6];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i];
        }
    } else if (totalActive == 7) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_7active");
        CSAMPLE_GAIN oldGain[7];
        CSAMPLE_GAIN newGain[7];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i];
        }
    } else if (totalActive == 8) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_8active");
        CSAMPLE_GAIN oldGain[8];
        CSAMPLE_GAIN newGain[8];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i];
        }
    } else if (totalActive == 9) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_9active");
        CSAMPLE_GAIN oldGain[9];
        CSAMPLE_GAIN newGain[9];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i];
        }
    } else if (totalActive == 10) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_10active");
        CSAMPLE_GAIN oldGain[10];
        CSAMPLE_GAIN newGain[10];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i];
        }
    } else if (totalActive == 11) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_11active");
        CSAMPLE_GAIN oldGain[11];
        CSAMPLE_GAIN newGain[11];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i];
        }
    } else if (totalActive == 12) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_12active");
        CSAMPLE_GAIN oldGain[12];
        CSAMPLE_GAIN newGain[12];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i];
        }
    } else if (totalActive == 13) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_13active");
        CSAMPLE_GAIN oldGain[13];
        CSAMPLE_GAIN newGain[13];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i];
        }
    } else if (totalActive == 14) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_14active");
        CSAMPLE_GAIN oldGain[14];
        CSAMPLE_GAIN newGain[14];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i];
        }
    } else if (totalActive == 15) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_15active");
        CSAMPLE_GAIN oldGain[15];
        CSAMPLE_GAIN newGain[15];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i];
        }
    } else if (totalActive == 16) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_16active");
        CSAMPLE_GAIN oldGain[16];
        CSAMPLE_GAIN newGain[16];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i];
        }
    } else if (totalActive == 17) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_17active");
        CSAMPLE_GAIN oldGain[17];
        CSAMPLE_GAIN newGain[17];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i];
        }
    } else if (totalActive == 18) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_18active");
        CSAMPLE_GAIN oldGain[18];
        CSAMPLE_GAIN newGain[18];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i];
        }
    } else if (totalActive == 19) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_19active");
        CSAMPLE_GAIN oldGain[19];
        CSAMPLE_GAIN newGain[19];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i];
        }
    } else if (totalActive == 20) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_20active");
        CSAMPLE_GAIN oldGain[20];
        CSAMPLE_GAIN newGain[20];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i];
        }
    } else if (totalActive == 21) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_21active");
        CSAMPLE_GAIN oldGain[21];
        CSAMPLE_GAIN newGain[21];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i];
        }
    } else if (totalActive == 22) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_22active");
        CSAMPLE_GAIN oldGain[22];
        CSAMPLE_GAIN newGain[22];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i];
        }
    } else if (totalActive == 23) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_23active");
        CSAMPLE_GAIN oldGain[23];
        CSAMPLE_GAIN newGain[23];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i];
        }
    } else if (totalActive == 24) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_24active");
        CSAMPLE_GAIN oldGain[24];
        CSAMPLE_GAIN newGain[24];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i];
        }
    } else if (totalActive == 25) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_25active");
        CSAMPLE_GAIN oldGain[25];
        CSAMPLE_GAIN newGain[25];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i];
        }
    } else if (totalActive == 26) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_26active");
        CSAMPLE_GAIN oldGain[26];
        CSAMPLE_GAIN newGain[26];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i];
        }
    } else if (totalActive == 27) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_27active");
        CSAMPLE_GAIN oldGain[27];
        CSAMPLE_GAIN newGain[27];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i];
        }
    } else if (totalActive == 28) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_28active");
        CSAMPLE_GAIN oldGain[28];
        CSAMPLE_GAIN newGain[28];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        oldGain[27] = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            newGain[27] = 0;
            gainCache27.m_fadeout = false;
        } else {
            newGain[27] = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel27->m_handle, outputHandle, pBuffer27, iBufferSize, iSampleRate, pChannel27->m_features, oldGain[27], newGain[27]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i] + pBuffer27[i];
        }
    } else if (totalActive == 29) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_29active");
        CSAMPLE_GAIN oldGain[29];
        CSAMPLE_GAIN newGain[29];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        oldGain[27] = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            newGain[27] = 0;
            gainCache27.m_fadeout = false;
        } else {
            newGain[27] = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        oldGain[28] = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            newGain[28] = 0;
            gainCache28.m_fadeout = false;
        } else {
            newGain[28] = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel27->m_handle, outputHandle, pBuffer27, iBufferSize, iSampleRate, pChannel27->m_features, oldGain[27], newGain[27]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel28->m_handle, outputHandle, pBuffer28, iBufferSize, iSampleRate, pChannel28->m_features, oldGain[28], newGain[28]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i] + pBuffer27[i] + pBuffer28[i];
        }
    } else if (totalActive == 30) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_30active");
        CSAMPLE_GAIN oldGain[30];
        CSAMPLE_GAIN newGain[30];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        oldGain[27] = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            newGain[27] = 0;
            gainCache27.m_fadeout = false;
        } else {
            newGain[27] = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        oldGain[28] = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            newGain[28] = 0;
            gainCache28.m_fadeout = false;
        } else {
            newGain[28] = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        oldGain[29] = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            newGain[29] = 0;
            gainCache29.m_fadeout = false;
        } else {
            newGain[29] = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = newGain[29];
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel27->m_handle, outputHandle, pBuffer27, iBufferSize, iSampleRate, pChannel27->m_features, oldGain[27], newGain[27]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel28->m_handle, outputHandle, pBuffer28, iBufferSize, iSampleRate, pChannel28->m_features, oldGain[28], newGain[28]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel29->m_handle, outputHandle, pBuffer29, iBufferSize, iSampleRate, pChannel29->m_features, oldGain[29], newGain[29]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i] + pBuffer27[i] + pBuffer28[i] + pBuffer29[i];
        }
    } else if (totalActive == 31) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_31active");
        CSAMPLE_GAIN oldGain[31];
        CSAMPLE_GAIN newGain[31];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        oldGain[27] = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            newGain[27] = 0;
            gainCache27.m_fadeout = false;
        } else {
            newGain[27] = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        oldGain[28] = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            newGain[28] = 0;
            gainCache28.m_fadeout = false;
        } else {
            newGain[28] = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        oldGain[29] = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            newGain[29] = 0;
            gainCache29.m_fadeout = false;
        } else {
            newGain[29] = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = newGain[29];
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int channelIndex30 = pChannel30->m_index;
        EngineMaster::GainCache& gainCache30 = (*channelGainCache)[channelIndex30];
        oldGain[30] = gainCache30.m_gain;
        if (gainCache30.m_fadeout) {
            newGain[30] = 0;
            gainCache30.m_fadeout = false;
        } else {
            newGain[30] = gainCalculator.getGain(pChannel30);
        }
        gainCache30.m_gain = newGain[30];
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel27->m_handle, outputHandle, pBuffer27, iBufferSize, iSampleRate, pChannel27->m_features, oldGain[27], newGain[27]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel28->m_handle, outputHandle, pBuffer28, iBufferSize, iSampleRate, pChannel28->m_features, oldGain[28], newGain[28]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel29->m_handle, outputHandle, pBuffer29, iBufferSize, iSampleRate, pChannel29->m_features, oldGain[29], newGain[29]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel30->m_handle, outputHandle, pBuffer30, iBufferSize, iSampleRate, pChannel30->m_features, oldGain[30], newGain[30]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i] + pBuffer27[i] + pBuffer28[i] + pBuffer29[i] + pBuffer30[i];
        }
    } else if (totalActive == 32) {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_32active");
        CSAMPLE_GAIN oldGain[32];
        CSAMPLE_GAIN newGain[32];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        oldGain[0] = gainCache0.m_gain;
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int channelIndex1 = pChannel1->m_index;
        EngineMaster::GainCache& gainCache1 = (*channelGainCache)[channelIndex1];
        oldGain[1] = gainCache1.m_gain;
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int channelIndex2 = pChannel2->m_index;
        EngineMaster::GainCache& gainCache2 = (*channelGainCache)[channelIndex2];
        oldGain[2] = gainCache2.m_gain;
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int channelIndex3 = pChannel3->m_index;
        EngineMaster::GainCache& gainCache3 = (*channelGainCache)[channelIndex3];
        oldGain[3] = gainCache3.m_gain;
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int channelIndex4 = pChannel4->m_index;
        EngineMaster::GainCache& gainCache4 = (*channelGainCache)[channelIndex4];
        oldGain[4] = gainCache4.m_gain;
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int channelIndex5 = pChannel5->m_index;
        EngineMaster::GainCache& gainCache5 = (*channelGainCache)[channelIndex5];
        oldGain[5] = gainCache5.m_gain;
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int channelIndex6 = pChannel6->m_index;
        EngineMaster::GainCache& gainCache6 = (*channelGainCache)[channelIndex6];
        oldGain[6] = gainCache6.m_gain;
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int channelIndex7 = pChannel7->m_index;
        EngineMaster::GainCache& gainCache7 = (*channelGainCache)[channelIndex7];
        oldGain[7] = gainCache7.m_gain;
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int channelIndex8 = pChannel8->m_index;
        EngineMaster::GainCache& gainCache8 = (*channelGainCache)[channelIndex8];
        oldGain[8] = gainCache8.m_gain;
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int channelIndex9 = pChannel9->m_index;
        EngineMaster::GainCache& gainCache9 = (*channelGainCache)[channelIndex9];
        oldGain[9] = gainCache9.m_gain;
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int channelIndex10 = pChannel10->m_index;
        EngineMaster::GainCache& gainCache10 = (*channelGainCache)[channelIndex10];
        oldGain[10] = gainCache10.m_gain;
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int channelIndex11 = pChannel11->m_index;
        EngineMaster::GainCache& gainCache11 = (*channelGainCache)[channelIndex11];
        oldGain[11] = gainCache11.m_gain;
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int channelIndex12 = pChannel12->m_index;
        EngineMaster::GainCache& gainCache12 = (*channelGainCache)[channelIndex12];
        oldGain[12] = gainCache12.m_gain;
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int channelIndex13 = pChannel13->m_index;
        EngineMaster::GainCache& gainCache13 = (*channelGainCache)[channelIndex13];
        oldGain[13] = gainCache13.m_gain;
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int channelIndex14 = pChannel14->m_index;
        EngineMaster::GainCache& gainCache14 = (*channelGainCache)[channelIndex14];
        oldGain[14] = gainCache14.m_gain;
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int channelIndex15 = pChannel15->m_index;
        EngineMaster::GainCache& gainCache15 = (*channelGainCache)[channelIndex15];
        oldGain[15] = gainCache15.m_gain;
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int channelIndex16 = pChannel16->m_index;
        EngineMaster::GainCache& gainCache16 = (*channelGainCache)[channelIndex16];
        oldGain[16] = gainCache16.m_gain;
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int channelIndex17 = pChannel17->m_index;
        EngineMaster::GainCache& gainCache17 = (*channelGainCache)[channelIndex17];
        oldGain[17] = gainCache17.m_gain;
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int channelIndex18 = pChannel18->m_index;
        EngineMaster::GainCache& gainCache18 = (*channelGainCache)[channelIndex18];
        oldGain[18] = gainCache18.m_gain;
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int channelIndex19 = pChannel19->m_index;
        EngineMaster::GainCache& gainCache19 = (*channelGainCache)[channelIndex19];
        oldGain[19] = gainCache19.m_gain;
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int channelIndex20 = pChannel20->m_index;
        EngineMaster::GainCache& gainCache20 = (*channelGainCache)[channelIndex20];
        oldGain[20] = gainCache20.m_gain;
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int channelIndex21 = pChannel21->m_index;
        EngineMaster::GainCache& gainCache21 = (*channelGainCache)[channelIndex21];
        oldGain[21] = gainCache21.m_gain;
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int channelIndex22 = pChannel22->m_index;
        EngineMaster::GainCache& gainCache22 = (*channelGainCache)[channelIndex22];
        oldGain[22] = gainCache22.m_gain;
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int channelIndex23 = pChannel23->m_index;
        EngineMaster::GainCache& gainCache23 = (*channelGainCache)[channelIndex23];
        oldGain[23] = gainCache23.m_gain;
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int channelIndex24 = pChannel24->m_index;
        EngineMaster::GainCache& gainCache24 = (*channelGainCache)[channelIndex24];
        oldGain[24] = gainCache24.m_gain;
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int channelIndex25 = pChannel25->m_index;
        EngineMaster::GainCache& gainCache25 = (*channelGainCache)[channelIndex25];
        oldGain[25] = gainCache25.m_gain;
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int channelIndex26 = pChannel26->m_index;
        EngineMaster::GainCache& gainCache26 = (*channelGainCache)[channelIndex26];
        oldGain[26] = gainCache26.m_gain;
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int channelIndex27 = pChannel27->m_index;
        EngineMaster::GainCache& gainCache27 = (*channelGainCache)[channelIndex27];
        oldGain[27] = gainCache27.m_gain;
        if (gainCache27.m_fadeout) {
            newGain[27] = 0;
            gainCache27.m_fadeout = false;
        } else {
            newGain[27] = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int channelIndex28 = pChannel28->m_index;
        EngineMaster::GainCache& gainCache28 = (*channelGainCache)[channelIndex28];
        oldGain[28] = gainCache28.m_gain;
        if (gainCache28.m_fadeout) {
            newGain[28] = 0;
            gainCache28.m_fadeout = false;
        } else {
            newGain[28] = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int channelIndex29 = pChannel29->m_index;
        EngineMaster::GainCache& gainCache29 = (*channelGainCache)[channelIndex29];
        oldGain[29] = gainCache29.m_gain;
        if (gainCache29.m_fadeout) {
            newGain[29] = 0;
            gainCache29.m_fadeout = false;
        } else {
            newGain[29] = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = newGain[29];
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int channelIndex30 = pChannel30->m_index;
        EngineMaster::GainCache& gainCache30 = (*channelGainCache)[channelIndex30];
        oldGain[30] = gainCache30.m_gain;
        if (gainCache30.m_fadeout) {
            newGain[30] = 0;
            gainCache30.m_fadeout = false;
        } else {
            newGain[30] = gainCalculator.getGain(pChannel30);
        }
        gainCache30.m_gain = newGain[30];
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel31 = activeChannels->at(31);
        const int channelIndex31 = pChannel31->m_index;
        EngineMaster::GainCache& gainCache31 = (*channelGainCache)[channelIndex31];
        oldGain[31] = gainCache31.m_gain;
        if (gainCache31.m_fadeout) {
            newGain[31] = 0;
            gainCache31.m_fadeout = false;
        } else {
            newGain[31] = gainCalculator.getGain(pChannel31);
        }
        gainCache31.m_gain = newGain[31];
        CSAMPLE* pBuffer31 = pChannel31->m_pBuffer;
        // Process effects for each channel in place
        pEngineEffectsManager->processPostFaderInPlace(pChannel0->m_handle, outputHandle, pBuffer0, iBufferSize, iSampleRate, pChannel0->m_features, oldGain[0], newGain[0]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel1->m_handle, outputHandle, pBuffer1, iBufferSize, iSampleRate, pChannel1->m_features, oldGain[1], newGain[1]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel2->m_handle, outputHandle, pBuffer2, iBufferSize, iSampleRate, pChannel2->m_features, oldGain[2], newGain[2]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel3->m_handle, outputHandle, pBuffer3, iBufferSize, iSampleRate, pChannel3->m_features, oldGain[3], newGain[3]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel4->m_handle, outputHandle, pBuffer4, iBufferSize, iSampleRate, pChannel4->m_features, oldGain[4], newGain[4]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel5->m_handle, outputHandle, pBuffer5, iBufferSize, iSampleRate, pChannel5->m_features, oldGain[5], newGain[5]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel6->m_handle, outputHandle, pBuffer6, iBufferSize, iSampleRate, pChannel6->m_features, oldGain[6], newGain[6]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel7->m_handle, outputHandle, pBuffer7, iBufferSize, iSampleRate, pChannel7->m_features, oldGain[7], newGain[7]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel8->m_handle, outputHandle, pBuffer8, iBufferSize, iSampleRate, pChannel8->m_features, oldGain[8], newGain[8]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel9->m_handle, outputHandle, pBuffer9, iBufferSize, iSampleRate, pChannel9->m_features, oldGain[9], newGain[9]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel10->m_handle, outputHandle, pBuffer10, iBufferSize, iSampleRate, pChannel10->m_features, oldGain[10], newGain[10]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel11->m_handle, outputHandle, pBuffer11, iBufferSize, iSampleRate, pChannel11->m_features, oldGain[11], newGain[11]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel12->m_handle, outputHandle, pBuffer12, iBufferSize, iSampleRate, pChannel12->m_features, oldGain[12], newGain[12]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel13->m_handle, outputHandle, pBuffer13, iBufferSize, iSampleRate, pChannel13->m_features, oldGain[13], newGain[13]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel14->m_handle, outputHandle, pBuffer14, iBufferSize, iSampleRate, pChannel14->m_features, oldGain[14], newGain[14]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel15->m_handle, outputHandle, pBuffer15, iBufferSize, iSampleRate, pChannel15->m_features, oldGain[15], newGain[15]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel16->m_handle, outputHandle, pBuffer16, iBufferSize, iSampleRate, pChannel16->m_features, oldGain[16], newGain[16]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel17->m_handle, outputHandle, pBuffer17, iBufferSize, iSampleRate, pChannel17->m_features, oldGain[17], newGain[17]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel18->m_handle, outputHandle, pBuffer18, iBufferSize, iSampleRate, pChannel18->m_features, oldGain[18], newGain[18]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel19->m_handle, outputHandle, pBuffer19, iBufferSize, iSampleRate, pChannel19->m_features, oldGain[19], newGain[19]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel20->m_handle, outputHandle, pBuffer20, iBufferSize, iSampleRate, pChannel20->m_features, oldGain[20], newGain[20]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel21->m_handle, outputHandle, pBuffer21, iBufferSize, iSampleRate, pChannel21->m_features, oldGain[21], newGain[21]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel22->m_handle, outputHandle, pBuffer22, iBufferSize, iSampleRate, pChannel22->m_features, oldGain[22], newGain[22]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel23->m_handle, outputHandle, pBuffer23, iBufferSize, iSampleRate, pChannel23->m_features, oldGain[23], newGain[23]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel24->m_handle, outputHandle, pBuffer24, iBufferSize, iSampleRate, pChannel24->m_features, oldGain[24], newGain[24]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel25->m_handle, outputHandle, pBuffer25, iBufferSize, iSampleRate, pChannel25->m_features, oldGain[25], newGain[25]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel26->m_handle, outputHandle, pBuffer26, iBufferSize, iSampleRate, pChannel26->m_features, oldGain[26], newGain[26]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel27->m_handle, outputHandle, pBuffer27, iBufferSize, iSampleRate, pChannel27->m_features, oldGain[27], newGain[27]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel28->m_handle, outputHandle, pBuffer28, iBufferSize, iSampleRate, pChannel28->m_features, oldGain[28], newGain[28]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel29->m_handle, outputHandle, pBuffer29, iBufferSize, iSampleRate, pChannel29->m_features, oldGain[29], newGain[29]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel30->m_handle, outputHandle, pBuffer30, iBufferSize, iSampleRate, pChannel30->m_features, oldGain[30], newGain[30]);
        pEngineEffectsManager->processPostFaderInPlace(pChannel31->m_handle, outputHandle, pBuffer31, iBufferSize, iSampleRate, pChannel31->m_features, oldGain[31], newGain[31]);
        // Mix the effected channel buffers together to replace the old pOutput from the last engine callback
        for (unsigned int i = 0; i < iBufferSize; ++i) {
            pOutput[i] = pBuffer0[i] + pBuffer1[i] + pBuffer2[i] + pBuffer3[i] + pBuffer4[i] + pBuffer5[i] + pBuffer6[i] + pBuffer7[i] + pBuffer8[i] + pBuffer9[i] + pBuffer10[i] + pBuffer11[i] + pBuffer12[i] + pBuffer13[i] + pBuffer14[i] + pBuffer15[i] + pBuffer16[i] + pBuffer17[i] + pBuffer18[i] + pBuffer19[i] + pBuffer20[i] + pBuffer21[i] + pBuffer22[i] + pBuffer23[i] + pBuffer24[i] + pBuffer25[i] + pBuffer26[i] + pBuffer27[i] + pBuffer28[i] + pBuffer29[i] + pBuffer30[i] + pBuffer31[i];
        }
    } else {
        //ScopedTimer t("EngineMaster::applyEffectsInPlaceAndMixChannels_Over32active");
        SampleUtil::clear(pOutput, iBufferSize);
        for (int i = 0; i < activeChannels->size(); ++i) {
            EngineMaster::ChannelInfo* pChannelInfo = activeChannels->at(i);
            const int channelIndex = pChannelInfo->m_index;
            EngineMaster::GainCache& gainCache = (*channelGainCache)[channelIndex];
            CSAMPLE_GAIN oldGain = gainCache.m_gain;
            CSAMPLE_GAIN newGain;
            if (gainCache.m_fadeout) {
                newGain = 0;
                gainCache.m_fadeout = false;
            } else {
                newGain = gainCalculator.getGain(pChannelInfo);
            }
            gainCache.m_gain = newGain;
            CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;
            pEngineEffectsManager->processPostFaderInPlace(pChannelInfo->m_handle, outputHandle, pBuffer, iBufferSize, iSampleRate, pChannelInfo->m_features, oldGain, newGain);
            SampleUtil::add(pOutput, pBuffer, iBufferSize);
        }
    }
}
