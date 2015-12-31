#include "engine/channelmixer.h"
#include "util/timer.h"
#include "util/sample.h"
////////////////////////////////////////////////////////
// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //
// SEE scripts/generate_sample_functions.py           //
////////////////////////////////////////////////////////

// static
void ChannelMixer::mixChannels(const EngineMaster::GainCalculator& gainCalculator,
                               QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
                               QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
                               CSAMPLE* pOutput,
                               unsigned int iBufferSize) {
    int totalActive = activeChannels->size();
    if (totalActive == 0) {
        ScopedTimer t("EngineMaster::mixChannels_0active");
        SampleUtil::clear(pOutput, iBufferSize);
    } else if (totalActive == 1) {
        ScopedTimer t("EngineMaster::mixChannels_1active");
        CSAMPLE_GAIN newGain[1];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
        if (gainCache0.m_fadeout) {
            newGain[0] = 0;
            gainCache0.m_fadeout = false;
        } else {
            newGain[0] = gainCalculator.getGain(pChannel0);
        }
        gainCache0.m_gain = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        SampleUtil::copy1WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  iBufferSize);
    } else if (totalActive == 2) {
        ScopedTimer t("EngineMaster::mixChannels_2active");
        CSAMPLE_GAIN newGain[2];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache1.m_fadeout) {
            newGain[1] = 0;
            gainCache1.m_fadeout = false;
        } else {
            newGain[1] = gainCalculator.getGain(pChannel1);
        }
        gainCache1.m_gain = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        SampleUtil::copy2WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  pBuffer1, newGain[1],
                                  iBufferSize);
    } else if (totalActive == 3) {
        ScopedTimer t("EngineMaster::mixChannels_3active");
        CSAMPLE_GAIN newGain[3];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache2.m_fadeout) {
            newGain[2] = 0;
            gainCache2.m_fadeout = false;
        } else {
            newGain[2] = gainCalculator.getGain(pChannel2);
        }
        gainCache2.m_gain = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        SampleUtil::copy3WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  pBuffer1, newGain[1],
                                  pBuffer2, newGain[2],
                                  iBufferSize);
    } else if (totalActive == 4) {
        ScopedTimer t("EngineMaster::mixChannels_4active");
        CSAMPLE_GAIN newGain[4];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache3.m_fadeout) {
            newGain[3] = 0;
            gainCache3.m_fadeout = false;
        } else {
            newGain[3] = gainCalculator.getGain(pChannel3);
        }
        gainCache3.m_gain = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        SampleUtil::copy4WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  pBuffer1, newGain[1],
                                  pBuffer2, newGain[2],
                                  pBuffer3, newGain[3],
                                  iBufferSize);
    } else if (totalActive == 5) {
        ScopedTimer t("EngineMaster::mixChannels_5active");
        CSAMPLE_GAIN newGain[5];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache4.m_fadeout) {
            newGain[4] = 0;
            gainCache4.m_fadeout = false;
        } else {
            newGain[4] = gainCalculator.getGain(pChannel4);
        }
        gainCache4.m_gain = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        SampleUtil::copy5WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  pBuffer1, newGain[1],
                                  pBuffer2, newGain[2],
                                  pBuffer3, newGain[3],
                                  pBuffer4, newGain[4],
                                  iBufferSize);
    } else if (totalActive == 6) {
        ScopedTimer t("EngineMaster::mixChannels_6active");
        CSAMPLE_GAIN newGain[6];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache5.m_fadeout) {
            newGain[5] = 0;
            gainCache5.m_fadeout = false;
        } else {
            newGain[5] = gainCalculator.getGain(pChannel5);
        }
        gainCache5.m_gain = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        SampleUtil::copy6WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  pBuffer1, newGain[1],
                                  pBuffer2, newGain[2],
                                  pBuffer3, newGain[3],
                                  pBuffer4, newGain[4],
                                  pBuffer5, newGain[5],
                                  iBufferSize);
    } else if (totalActive == 7) {
        ScopedTimer t("EngineMaster::mixChannels_7active");
        CSAMPLE_GAIN newGain[7];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache6.m_fadeout) {
            newGain[6] = 0;
            gainCache6.m_fadeout = false;
        } else {
            newGain[6] = gainCalculator.getGain(pChannel6);
        }
        gainCache6.m_gain = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        SampleUtil::copy7WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  pBuffer1, newGain[1],
                                  pBuffer2, newGain[2],
                                  pBuffer3, newGain[3],
                                  pBuffer4, newGain[4],
                                  pBuffer5, newGain[5],
                                  pBuffer6, newGain[6],
                                  iBufferSize);
    } else if (totalActive == 8) {
        ScopedTimer t("EngineMaster::mixChannels_8active");
        CSAMPLE_GAIN newGain[8];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache7.m_fadeout) {
            newGain[7] = 0;
            gainCache7.m_fadeout = false;
        } else {
            newGain[7] = gainCalculator.getGain(pChannel7);
        }
        gainCache7.m_gain = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        SampleUtil::copy8WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  pBuffer1, newGain[1],
                                  pBuffer2, newGain[2],
                                  pBuffer3, newGain[3],
                                  pBuffer4, newGain[4],
                                  pBuffer5, newGain[5],
                                  pBuffer6, newGain[6],
                                  pBuffer7, newGain[7],
                                  iBufferSize);
    } else if (totalActive == 9) {
        ScopedTimer t("EngineMaster::mixChannels_9active");
        CSAMPLE_GAIN newGain[9];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache8.m_fadeout) {
            newGain[8] = 0;
            gainCache8.m_fadeout = false;
        } else {
            newGain[8] = gainCalculator.getGain(pChannel8);
        }
        gainCache8.m_gain = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        SampleUtil::copy9WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  pBuffer1, newGain[1],
                                  pBuffer2, newGain[2],
                                  pBuffer3, newGain[3],
                                  pBuffer4, newGain[4],
                                  pBuffer5, newGain[5],
                                  pBuffer6, newGain[6],
                                  pBuffer7, newGain[7],
                                  pBuffer8, newGain[8],
                                  iBufferSize);
    } else if (totalActive == 10) {
        ScopedTimer t("EngineMaster::mixChannels_10active");
        CSAMPLE_GAIN newGain[10];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache9.m_fadeout) {
            newGain[9] = 0;
            gainCache9.m_fadeout = false;
        } else {
            newGain[9] = gainCalculator.getGain(pChannel9);
        }
        gainCache9.m_gain = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        SampleUtil::copy10WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   iBufferSize);
    } else if (totalActive == 11) {
        ScopedTimer t("EngineMaster::mixChannels_11active");
        CSAMPLE_GAIN newGain[11];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache10.m_fadeout) {
            newGain[10] = 0;
            gainCache10.m_fadeout = false;
        } else {
            newGain[10] = gainCalculator.getGain(pChannel10);
        }
        gainCache10.m_gain = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        SampleUtil::copy11WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   iBufferSize);
    } else if (totalActive == 12) {
        ScopedTimer t("EngineMaster::mixChannels_12active");
        CSAMPLE_GAIN newGain[12];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache11.m_fadeout) {
            newGain[11] = 0;
            gainCache11.m_fadeout = false;
        } else {
            newGain[11] = gainCalculator.getGain(pChannel11);
        }
        gainCache11.m_gain = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        SampleUtil::copy12WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   iBufferSize);
    } else if (totalActive == 13) {
        ScopedTimer t("EngineMaster::mixChannels_13active");
        CSAMPLE_GAIN newGain[13];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache12.m_fadeout) {
            newGain[12] = 0;
            gainCache12.m_fadeout = false;
        } else {
            newGain[12] = gainCalculator.getGain(pChannel12);
        }
        gainCache12.m_gain = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        SampleUtil::copy13WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   iBufferSize);
    } else if (totalActive == 14) {
        ScopedTimer t("EngineMaster::mixChannels_14active");
        CSAMPLE_GAIN newGain[14];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache13.m_fadeout) {
            newGain[13] = 0;
            gainCache13.m_fadeout = false;
        } else {
            newGain[13] = gainCalculator.getGain(pChannel13);
        }
        gainCache13.m_gain = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        SampleUtil::copy14WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   iBufferSize);
    } else if (totalActive == 15) {
        ScopedTimer t("EngineMaster::mixChannels_15active");
        CSAMPLE_GAIN newGain[15];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache14.m_fadeout) {
            newGain[14] = 0;
            gainCache14.m_fadeout = false;
        } else {
            newGain[14] = gainCalculator.getGain(pChannel14);
        }
        gainCache14.m_gain = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        SampleUtil::copy15WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   iBufferSize);
    } else if (totalActive == 16) {
        ScopedTimer t("EngineMaster::mixChannels_16active");
        CSAMPLE_GAIN newGain[16];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache15.m_fadeout) {
            newGain[15] = 0;
            gainCache15.m_fadeout = false;
        } else {
            newGain[15] = gainCalculator.getGain(pChannel15);
        }
        gainCache15.m_gain = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        SampleUtil::copy16WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   iBufferSize);
    } else if (totalActive == 17) {
        ScopedTimer t("EngineMaster::mixChannels_17active");
        CSAMPLE_GAIN newGain[17];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache16.m_fadeout) {
            newGain[16] = 0;
            gainCache16.m_fadeout = false;
        } else {
            newGain[16] = gainCalculator.getGain(pChannel16);
        }
        gainCache16.m_gain = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        SampleUtil::copy17WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   iBufferSize);
    } else if (totalActive == 18) {
        ScopedTimer t("EngineMaster::mixChannels_18active");
        CSAMPLE_GAIN newGain[18];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache17.m_fadeout) {
            newGain[17] = 0;
            gainCache17.m_fadeout = false;
        } else {
            newGain[17] = gainCalculator.getGain(pChannel17);
        }
        gainCache17.m_gain = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        SampleUtil::copy18WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   iBufferSize);
    } else if (totalActive == 19) {
        ScopedTimer t("EngineMaster::mixChannels_19active");
        CSAMPLE_GAIN newGain[19];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache18.m_fadeout) {
            newGain[18] = 0;
            gainCache18.m_fadeout = false;
        } else {
            newGain[18] = gainCalculator.getGain(pChannel18);
        }
        gainCache18.m_gain = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        SampleUtil::copy19WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   iBufferSize);
    } else if (totalActive == 20) {
        ScopedTimer t("EngineMaster::mixChannels_20active");
        CSAMPLE_GAIN newGain[20];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache19.m_fadeout) {
            newGain[19] = 0;
            gainCache19.m_fadeout = false;
        } else {
            newGain[19] = gainCalculator.getGain(pChannel19);
        }
        gainCache19.m_gain = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        SampleUtil::copy20WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   iBufferSize);
    } else if (totalActive == 21) {
        ScopedTimer t("EngineMaster::mixChannels_21active");
        CSAMPLE_GAIN newGain[21];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache20.m_fadeout) {
            newGain[20] = 0;
            gainCache20.m_fadeout = false;
        } else {
            newGain[20] = gainCalculator.getGain(pChannel20);
        }
        gainCache20.m_gain = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        SampleUtil::copy21WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   iBufferSize);
    } else if (totalActive == 22) {
        ScopedTimer t("EngineMaster::mixChannels_22active");
        CSAMPLE_GAIN newGain[22];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache21.m_fadeout) {
            newGain[21] = 0;
            gainCache21.m_fadeout = false;
        } else {
            newGain[21] = gainCalculator.getGain(pChannel21);
        }
        gainCache21.m_gain = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        SampleUtil::copy22WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   pBuffer21, newGain[21],
                                   iBufferSize);
    } else if (totalActive == 23) {
        ScopedTimer t("EngineMaster::mixChannels_23active");
        CSAMPLE_GAIN newGain[23];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache22.m_fadeout) {
            newGain[22] = 0;
            gainCache22.m_fadeout = false;
        } else {
            newGain[22] = gainCalculator.getGain(pChannel22);
        }
        gainCache22.m_gain = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        SampleUtil::copy23WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   pBuffer21, newGain[21],
                                   pBuffer22, newGain[22],
                                   iBufferSize);
    } else if (totalActive == 24) {
        ScopedTimer t("EngineMaster::mixChannels_24active");
        CSAMPLE_GAIN newGain[24];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache23.m_fadeout) {
            newGain[23] = 0;
            gainCache23.m_fadeout = false;
        } else {
            newGain[23] = gainCalculator.getGain(pChannel23);
        }
        gainCache23.m_gain = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        SampleUtil::copy24WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   pBuffer21, newGain[21],
                                   pBuffer22, newGain[22],
                                   pBuffer23, newGain[23],
                                   iBufferSize);
    } else if (totalActive == 25) {
        ScopedTimer t("EngineMaster::mixChannels_25active");
        CSAMPLE_GAIN newGain[25];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache24.m_fadeout) {
            newGain[24] = 0;
            gainCache24.m_fadeout = false;
        } else {
            newGain[24] = gainCalculator.getGain(pChannel24);
        }
        gainCache24.m_gain = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        SampleUtil::copy25WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   pBuffer21, newGain[21],
                                   pBuffer22, newGain[22],
                                   pBuffer23, newGain[23],
                                   pBuffer24, newGain[24],
                                   iBufferSize);
    } else if (totalActive == 26) {
        ScopedTimer t("EngineMaster::mixChannels_26active");
        CSAMPLE_GAIN newGain[26];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache25.m_fadeout) {
            newGain[25] = 0;
            gainCache25.m_fadeout = false;
        } else {
            newGain[25] = gainCalculator.getGain(pChannel25);
        }
        gainCache25.m_gain = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        SampleUtil::copy26WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   pBuffer21, newGain[21],
                                   pBuffer22, newGain[22],
                                   pBuffer23, newGain[23],
                                   pBuffer24, newGain[24],
                                   pBuffer25, newGain[25],
                                   iBufferSize);
    } else if (totalActive == 27) {
        ScopedTimer t("EngineMaster::mixChannels_27active");
        CSAMPLE_GAIN newGain[27];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache26.m_fadeout) {
            newGain[26] = 0;
            gainCache26.m_fadeout = false;
        } else {
            newGain[26] = gainCalculator.getGain(pChannel26);
        }
        gainCache26.m_gain = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        SampleUtil::copy27WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   pBuffer21, newGain[21],
                                   pBuffer22, newGain[22],
                                   pBuffer23, newGain[23],
                                   pBuffer24, newGain[24],
                                   pBuffer25, newGain[25],
                                   pBuffer26, newGain[26],
                                   iBufferSize);
    } else if (totalActive == 28) {
        ScopedTimer t("EngineMaster::mixChannels_28active");
        CSAMPLE_GAIN newGain[28];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache27.m_fadeout) {
            newGain[27] = 0;
            gainCache27.m_fadeout = false;
        } else {
            newGain[27] = gainCalculator.getGain(pChannel27);
        }
        gainCache27.m_gain = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        SampleUtil::copy28WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   pBuffer21, newGain[21],
                                   pBuffer22, newGain[22],
                                   pBuffer23, newGain[23],
                                   pBuffer24, newGain[24],
                                   pBuffer25, newGain[25],
                                   pBuffer26, newGain[26],
                                   pBuffer27, newGain[27],
                                   iBufferSize);
    } else if (totalActive == 29) {
        ScopedTimer t("EngineMaster::mixChannels_29active");
        CSAMPLE_GAIN newGain[29];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache28.m_fadeout) {
            newGain[28] = 0;
            gainCache28.m_fadeout = false;
        } else {
            newGain[28] = gainCalculator.getGain(pChannel28);
        }
        gainCache28.m_gain = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        SampleUtil::copy29WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   pBuffer21, newGain[21],
                                   pBuffer22, newGain[22],
                                   pBuffer23, newGain[23],
                                   pBuffer24, newGain[24],
                                   pBuffer25, newGain[25],
                                   pBuffer26, newGain[26],
                                   pBuffer27, newGain[27],
                                   pBuffer28, newGain[28],
                                   iBufferSize);
    } else if (totalActive == 30) {
        ScopedTimer t("EngineMaster::mixChannels_30active");
        CSAMPLE_GAIN newGain[30];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache29.m_fadeout) {
            newGain[29] = 0;
            gainCache29.m_fadeout = false;
        } else {
            newGain[29] = gainCalculator.getGain(pChannel29);
        }
        gainCache29.m_gain = newGain[29];
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        SampleUtil::copy30WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   pBuffer21, newGain[21],
                                   pBuffer22, newGain[22],
                                   pBuffer23, newGain[23],
                                   pBuffer24, newGain[24],
                                   pBuffer25, newGain[25],
                                   pBuffer26, newGain[26],
                                   pBuffer27, newGain[27],
                                   pBuffer28, newGain[28],
                                   pBuffer29, newGain[29],
                                   iBufferSize);
    } else if (totalActive == 31) {
        ScopedTimer t("EngineMaster::mixChannels_31active");
        CSAMPLE_GAIN newGain[31];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache30.m_fadeout) {
            newGain[30] = 0;
            gainCache30.m_fadeout = false;
        } else {
            newGain[30] = gainCalculator.getGain(pChannel30);
        }
        gainCache30.m_gain = newGain[30];
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        SampleUtil::copy31WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   pBuffer21, newGain[21],
                                   pBuffer22, newGain[22],
                                   pBuffer23, newGain[23],
                                   pBuffer24, newGain[24],
                                   pBuffer25, newGain[25],
                                   pBuffer26, newGain[26],
                                   pBuffer27, newGain[27],
                                   pBuffer28, newGain[28],
                                   pBuffer29, newGain[29],
                                   pBuffer30, newGain[30],
                                   iBufferSize);
    } else if (totalActive == 32) {
        ScopedTimer t("EngineMaster::mixChannels_32active");
        CSAMPLE_GAIN newGain[32];
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int channelIndex0 = pChannel0->m_index;
        EngineMaster::GainCache& gainCache0 = (*channelGainCache)[channelIndex0];
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
        if (gainCache31.m_fadeout) {
            newGain[31] = 0;
            gainCache31.m_fadeout = false;
        } else {
            newGain[31] = gainCalculator.getGain(pChannel31);
        }
        gainCache31.m_gain = newGain[31];
        CSAMPLE* pBuffer31 = pChannel31->m_pBuffer;
        SampleUtil::copy32WithGain(pOutput,
                                   pBuffer0, newGain[0],
                                   pBuffer1, newGain[1],
                                   pBuffer2, newGain[2],
                                   pBuffer3, newGain[3],
                                   pBuffer4, newGain[4],
                                   pBuffer5, newGain[5],
                                   pBuffer6, newGain[6],
                                   pBuffer7, newGain[7],
                                   pBuffer8, newGain[8],
                                   pBuffer9, newGain[9],
                                   pBuffer10, newGain[10],
                                   pBuffer11, newGain[11],
                                   pBuffer12, newGain[12],
                                   pBuffer13, newGain[13],
                                   pBuffer14, newGain[14],
                                   pBuffer15, newGain[15],
                                   pBuffer16, newGain[16],
                                   pBuffer17, newGain[17],
                                   pBuffer18, newGain[18],
                                   pBuffer19, newGain[19],
                                   pBuffer20, newGain[20],
                                   pBuffer21, newGain[21],
                                   pBuffer22, newGain[22],
                                   pBuffer23, newGain[23],
                                   pBuffer24, newGain[24],
                                   pBuffer25, newGain[25],
                                   pBuffer26, newGain[26],
                                   pBuffer27, newGain[27],
                                   pBuffer28, newGain[28],
                                   pBuffer29, newGain[29],
                                   pBuffer30, newGain[30],
                                   pBuffer31, newGain[31],
                                   iBufferSize);
    } else {
        ScopedTimer t("EngineMaster::mixChannels_%1active", activeChannels->size());
        // Set pOutput to all 0s
        SampleUtil::clear(pOutput, iBufferSize);
        for (int i = 0; i < activeChannels->size(); ++i) {
            EngineMaster::ChannelInfo* pChannelInfo = activeChannels->at(i);
            const int channelIndex = pChannelInfo->m_index;
            EngineMaster::GainCache& gainCache = (*channelGainCache)[channelIndex];
            CSAMPLE_GAIN newGain;
            if (gainCache.m_fadeout) {
                newGain = 0;
                gainCache.m_fadeout = false;
            } else {
                newGain = gainCalculator.getGain(pChannelInfo);
            }
            gainCache.m_gain = newGain;
            CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;
            SampleUtil::addWithGain(pOutput, pBuffer, newGain, iBufferSize);
        }
    }
}
void ChannelMixer::mixChannelsRamping(const EngineMaster::GainCalculator& gainCalculator,
                                      QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels,
                                      QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache,
                                      CSAMPLE* pOutput,
                                      unsigned int iBufferSize) {
    int totalActive = activeChannels->size();
    if (totalActive == 0) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_0active");
        SampleUtil::clear(pOutput, iBufferSize);
    } else if (totalActive == 1) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_1active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy1WithGain(pOutput,
                                      pBuffer0, newGain[0],
                                      iBufferSize);
        } else {
            SampleUtil::copy1WithRampingGain(pOutput,
                                             pBuffer0, oldGain[0], newGain[0],
                                             iBufferSize);
        }
    } else if (totalActive == 2) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_2active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy2WithGain(pOutput,
                                      pBuffer0, newGain[0],
                                      pBuffer1, newGain[1],
                                      iBufferSize);
        } else {
            SampleUtil::copy2WithRampingGain(pOutput,
                                             pBuffer0, oldGain[0], newGain[0],
                                             pBuffer1, oldGain[1], newGain[1],
                                             iBufferSize);
        }
    } else if (totalActive == 3) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_3active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy3WithGain(pOutput,
                                      pBuffer0, newGain[0],
                                      pBuffer1, newGain[1],
                                      pBuffer2, newGain[2],
                                      iBufferSize);
        } else {
            SampleUtil::copy3WithRampingGain(pOutput,
                                             pBuffer0, oldGain[0], newGain[0],
                                             pBuffer1, oldGain[1], newGain[1],
                                             pBuffer2, oldGain[2], newGain[2],
                                             iBufferSize);
        }
    } else if (totalActive == 4) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_4active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy4WithGain(pOutput,
                                      pBuffer0, newGain[0],
                                      pBuffer1, newGain[1],
                                      pBuffer2, newGain[2],
                                      pBuffer3, newGain[3],
                                      iBufferSize);
        } else {
            SampleUtil::copy4WithRampingGain(pOutput,
                                             pBuffer0, oldGain[0], newGain[0],
                                             pBuffer1, oldGain[1], newGain[1],
                                             pBuffer2, oldGain[2], newGain[2],
                                             pBuffer3, oldGain[3], newGain[3],
                                             iBufferSize);
        }
    } else if (totalActive == 5) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_5active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy5WithGain(pOutput,
                                      pBuffer0, newGain[0],
                                      pBuffer1, newGain[1],
                                      pBuffer2, newGain[2],
                                      pBuffer3, newGain[3],
                                      pBuffer4, newGain[4],
                                      iBufferSize);
        } else {
            SampleUtil::copy5WithRampingGain(pOutput,
                                             pBuffer0, oldGain[0], newGain[0],
                                             pBuffer1, oldGain[1], newGain[1],
                                             pBuffer2, oldGain[2], newGain[2],
                                             pBuffer3, oldGain[3], newGain[3],
                                             pBuffer4, oldGain[4], newGain[4],
                                             iBufferSize);
        }
    } else if (totalActive == 6) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_6active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy6WithGain(pOutput,
                                      pBuffer0, newGain[0],
                                      pBuffer1, newGain[1],
                                      pBuffer2, newGain[2],
                                      pBuffer3, newGain[3],
                                      pBuffer4, newGain[4],
                                      pBuffer5, newGain[5],
                                      iBufferSize);
        } else {
            SampleUtil::copy6WithRampingGain(pOutput,
                                             pBuffer0, oldGain[0], newGain[0],
                                             pBuffer1, oldGain[1], newGain[1],
                                             pBuffer2, oldGain[2], newGain[2],
                                             pBuffer3, oldGain[3], newGain[3],
                                             pBuffer4, oldGain[4], newGain[4],
                                             pBuffer5, oldGain[5], newGain[5],
                                             iBufferSize);
        }
    } else if (totalActive == 7) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_7active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy7WithGain(pOutput,
                                      pBuffer0, newGain[0],
                                      pBuffer1, newGain[1],
                                      pBuffer2, newGain[2],
                                      pBuffer3, newGain[3],
                                      pBuffer4, newGain[4],
                                      pBuffer5, newGain[5],
                                      pBuffer6, newGain[6],
                                      iBufferSize);
        } else {
            SampleUtil::copy7WithRampingGain(pOutput,
                                             pBuffer0, oldGain[0], newGain[0],
                                             pBuffer1, oldGain[1], newGain[1],
                                             pBuffer2, oldGain[2], newGain[2],
                                             pBuffer3, oldGain[3], newGain[3],
                                             pBuffer4, oldGain[4], newGain[4],
                                             pBuffer5, oldGain[5], newGain[5],
                                             pBuffer6, oldGain[6], newGain[6],
                                             iBufferSize);
        }
    } else if (totalActive == 8) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_8active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy8WithGain(pOutput,
                                      pBuffer0, newGain[0],
                                      pBuffer1, newGain[1],
                                      pBuffer2, newGain[2],
                                      pBuffer3, newGain[3],
                                      pBuffer4, newGain[4],
                                      pBuffer5, newGain[5],
                                      pBuffer6, newGain[6],
                                      pBuffer7, newGain[7],
                                      iBufferSize);
        } else {
            SampleUtil::copy8WithRampingGain(pOutput,
                                             pBuffer0, oldGain[0], newGain[0],
                                             pBuffer1, oldGain[1], newGain[1],
                                             pBuffer2, oldGain[2], newGain[2],
                                             pBuffer3, oldGain[3], newGain[3],
                                             pBuffer4, oldGain[4], newGain[4],
                                             pBuffer5, oldGain[5], newGain[5],
                                             pBuffer6, oldGain[6], newGain[6],
                                             pBuffer7, oldGain[7], newGain[7],
                                             iBufferSize);
        }
    } else if (totalActive == 9) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_9active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy9WithGain(pOutput,
                                      pBuffer0, newGain[0],
                                      pBuffer1, newGain[1],
                                      pBuffer2, newGain[2],
                                      pBuffer3, newGain[3],
                                      pBuffer4, newGain[4],
                                      pBuffer5, newGain[5],
                                      pBuffer6, newGain[6],
                                      pBuffer7, newGain[7],
                                      pBuffer8, newGain[8],
                                      iBufferSize);
        } else {
            SampleUtil::copy9WithRampingGain(pOutput,
                                             pBuffer0, oldGain[0], newGain[0],
                                             pBuffer1, oldGain[1], newGain[1],
                                             pBuffer2, oldGain[2], newGain[2],
                                             pBuffer3, oldGain[3], newGain[3],
                                             pBuffer4, oldGain[4], newGain[4],
                                             pBuffer5, oldGain[5], newGain[5],
                                             pBuffer6, oldGain[6], newGain[6],
                                             pBuffer7, oldGain[7], newGain[7],
                                             pBuffer8, oldGain[8], newGain[8],
                                             iBufferSize);
        }
    } else if (totalActive == 10) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_10active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy10WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       iBufferSize);
        } else {
            SampleUtil::copy10WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              iBufferSize);
        }
    } else if (totalActive == 11) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_11active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy11WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       iBufferSize);
        } else {
            SampleUtil::copy11WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              iBufferSize);
        }
    } else if (totalActive == 12) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_12active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy12WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       iBufferSize);
        } else {
            SampleUtil::copy12WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              iBufferSize);
        }
    } else if (totalActive == 13) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_13active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy13WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       iBufferSize);
        } else {
            SampleUtil::copy13WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              iBufferSize);
        }
    } else if (totalActive == 14) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_14active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy14WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       iBufferSize);
        } else {
            SampleUtil::copy14WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              iBufferSize);
        }
    } else if (totalActive == 15) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_15active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy15WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       iBufferSize);
        } else {
            SampleUtil::copy15WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              iBufferSize);
        }
    } else if (totalActive == 16) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_16active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy16WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       iBufferSize);
        } else {
            SampleUtil::copy16WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              iBufferSize);
        }
    } else if (totalActive == 17) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_17active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy17WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       iBufferSize);
        } else {
            SampleUtil::copy17WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              iBufferSize);
        }
    } else if (totalActive == 18) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_18active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy18WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       iBufferSize);
        } else {
            SampleUtil::copy18WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              iBufferSize);
        }
    } else if (totalActive == 19) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_19active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy19WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       iBufferSize);
        } else {
            SampleUtil::copy19WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              iBufferSize);
        }
    } else if (totalActive == 20) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_20active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy20WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       iBufferSize);
        } else {
            SampleUtil::copy20WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              iBufferSize);
        }
    } else if (totalActive == 21) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_21active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy21WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       iBufferSize);
        } else {
            SampleUtil::copy21WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              iBufferSize);
        }
    } else if (totalActive == 22) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_22active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy22WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       pBuffer21, newGain[21],
                                       iBufferSize);
        } else {
            SampleUtil::copy22WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              pBuffer21, oldGain[21], newGain[21],
                                              iBufferSize);
        }
    } else if (totalActive == 23) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_23active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy23WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       pBuffer21, newGain[21],
                                       pBuffer22, newGain[22],
                                       iBufferSize);
        } else {
            SampleUtil::copy23WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              pBuffer21, oldGain[21], newGain[21],
                                              pBuffer22, oldGain[22], newGain[22],
                                              iBufferSize);
        }
    } else if (totalActive == 24) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_24active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy24WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       pBuffer21, newGain[21],
                                       pBuffer22, newGain[22],
                                       pBuffer23, newGain[23],
                                       iBufferSize);
        } else {
            SampleUtil::copy24WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              pBuffer21, oldGain[21], newGain[21],
                                              pBuffer22, oldGain[22], newGain[22],
                                              pBuffer23, oldGain[23], newGain[23],
                                              iBufferSize);
        }
    } else if (totalActive == 25) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_25active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy25WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       pBuffer21, newGain[21],
                                       pBuffer22, newGain[22],
                                       pBuffer23, newGain[23],
                                       pBuffer24, newGain[24],
                                       iBufferSize);
        } else {
            SampleUtil::copy25WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              pBuffer21, oldGain[21], newGain[21],
                                              pBuffer22, oldGain[22], newGain[22],
                                              pBuffer23, oldGain[23], newGain[23],
                                              pBuffer24, oldGain[24], newGain[24],
                                              iBufferSize);
        }
    } else if (totalActive == 26) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_26active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy26WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       pBuffer21, newGain[21],
                                       pBuffer22, newGain[22],
                                       pBuffer23, newGain[23],
                                       pBuffer24, newGain[24],
                                       pBuffer25, newGain[25],
                                       iBufferSize);
        } else {
            SampleUtil::copy26WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              pBuffer21, oldGain[21], newGain[21],
                                              pBuffer22, oldGain[22], newGain[22],
                                              pBuffer23, oldGain[23], newGain[23],
                                              pBuffer24, oldGain[24], newGain[24],
                                              pBuffer25, oldGain[25], newGain[25],
                                              iBufferSize);
        }
    } else if (totalActive == 27) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_27active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy27WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       pBuffer21, newGain[21],
                                       pBuffer22, newGain[22],
                                       pBuffer23, newGain[23],
                                       pBuffer24, newGain[24],
                                       pBuffer25, newGain[25],
                                       pBuffer26, newGain[26],
                                       iBufferSize);
        } else {
            SampleUtil::copy27WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              pBuffer21, oldGain[21], newGain[21],
                                              pBuffer22, oldGain[22], newGain[22],
                                              pBuffer23, oldGain[23], newGain[23],
                                              pBuffer24, oldGain[24], newGain[24],
                                              pBuffer25, oldGain[25], newGain[25],
                                              pBuffer26, oldGain[26], newGain[26],
                                              iBufferSize);
        }
    } else if (totalActive == 28) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_28active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy28WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       pBuffer21, newGain[21],
                                       pBuffer22, newGain[22],
                                       pBuffer23, newGain[23],
                                       pBuffer24, newGain[24],
                                       pBuffer25, newGain[25],
                                       pBuffer26, newGain[26],
                                       pBuffer27, newGain[27],
                                       iBufferSize);
        } else {
            SampleUtil::copy28WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              pBuffer21, oldGain[21], newGain[21],
                                              pBuffer22, oldGain[22], newGain[22],
                                              pBuffer23, oldGain[23], newGain[23],
                                              pBuffer24, oldGain[24], newGain[24],
                                              pBuffer25, oldGain[25], newGain[25],
                                              pBuffer26, oldGain[26], newGain[26],
                                              pBuffer27, oldGain[27], newGain[27],
                                              iBufferSize);
        }
    } else if (totalActive == 29) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_29active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy29WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       pBuffer21, newGain[21],
                                       pBuffer22, newGain[22],
                                       pBuffer23, newGain[23],
                                       pBuffer24, newGain[24],
                                       pBuffer25, newGain[25],
                                       pBuffer26, newGain[26],
                                       pBuffer27, newGain[27],
                                       pBuffer28, newGain[28],
                                       iBufferSize);
        } else {
            SampleUtil::copy29WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              pBuffer21, oldGain[21], newGain[21],
                                              pBuffer22, oldGain[22], newGain[22],
                                              pBuffer23, oldGain[23], newGain[23],
                                              pBuffer24, oldGain[24], newGain[24],
                                              pBuffer25, oldGain[25], newGain[25],
                                              pBuffer26, oldGain[26], newGain[26],
                                              pBuffer27, oldGain[27], newGain[27],
                                              pBuffer28, oldGain[28], newGain[28],
                                              iBufferSize);
        }
    } else if (totalActive == 30) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_30active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy30WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       pBuffer21, newGain[21],
                                       pBuffer22, newGain[22],
                                       pBuffer23, newGain[23],
                                       pBuffer24, newGain[24],
                                       pBuffer25, newGain[25],
                                       pBuffer26, newGain[26],
                                       pBuffer27, newGain[27],
                                       pBuffer28, newGain[28],
                                       pBuffer29, newGain[29],
                                       iBufferSize);
        } else {
            SampleUtil::copy30WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              pBuffer21, oldGain[21], newGain[21],
                                              pBuffer22, oldGain[22], newGain[22],
                                              pBuffer23, oldGain[23], newGain[23],
                                              pBuffer24, oldGain[24], newGain[24],
                                              pBuffer25, oldGain[25], newGain[25],
                                              pBuffer26, oldGain[26], newGain[26],
                                              pBuffer27, oldGain[27], newGain[27],
                                              pBuffer28, oldGain[28], newGain[28],
                                              pBuffer29, oldGain[29], newGain[29],
                                              iBufferSize);
        }
    } else if (totalActive == 31) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_31active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy31WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       pBuffer21, newGain[21],
                                       pBuffer22, newGain[22],
                                       pBuffer23, newGain[23],
                                       pBuffer24, newGain[24],
                                       pBuffer25, newGain[25],
                                       pBuffer26, newGain[26],
                                       pBuffer27, newGain[27],
                                       pBuffer28, newGain[28],
                                       pBuffer29, newGain[29],
                                       pBuffer30, newGain[30],
                                       iBufferSize);
        } else {
            SampleUtil::copy31WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              pBuffer21, oldGain[21], newGain[21],
                                              pBuffer22, oldGain[22], newGain[22],
                                              pBuffer23, oldGain[23], newGain[23],
                                              pBuffer24, oldGain[24], newGain[24],
                                              pBuffer25, oldGain[25], newGain[25],
                                              pBuffer26, oldGain[26], newGain[26],
                                              pBuffer27, oldGain[27], newGain[27],
                                              pBuffer28, oldGain[28], newGain[28],
                                              pBuffer29, oldGain[29], newGain[29],
                                              pBuffer30, oldGain[30], newGain[30],
                                              iBufferSize);
        }
    } else if (totalActive == 32) {
        ScopedTimer t("EngineMaster::mixChannelsRamping_32active");
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
        int i = 0;
        for(; i < totalActive; ++i) {
            if (oldGain[i] != newGain[i]) {
                break;
            }
        }
        if (i == totalActive) {
            SampleUtil::copy32WithGain(pOutput,
                                       pBuffer0, newGain[0],
                                       pBuffer1, newGain[1],
                                       pBuffer2, newGain[2],
                                       pBuffer3, newGain[3],
                                       pBuffer4, newGain[4],
                                       pBuffer5, newGain[5],
                                       pBuffer6, newGain[6],
                                       pBuffer7, newGain[7],
                                       pBuffer8, newGain[8],
                                       pBuffer9, newGain[9],
                                       pBuffer10, newGain[10],
                                       pBuffer11, newGain[11],
                                       pBuffer12, newGain[12],
                                       pBuffer13, newGain[13],
                                       pBuffer14, newGain[14],
                                       pBuffer15, newGain[15],
                                       pBuffer16, newGain[16],
                                       pBuffer17, newGain[17],
                                       pBuffer18, newGain[18],
                                       pBuffer19, newGain[19],
                                       pBuffer20, newGain[20],
                                       pBuffer21, newGain[21],
                                       pBuffer22, newGain[22],
                                       pBuffer23, newGain[23],
                                       pBuffer24, newGain[24],
                                       pBuffer25, newGain[25],
                                       pBuffer26, newGain[26],
                                       pBuffer27, newGain[27],
                                       pBuffer28, newGain[28],
                                       pBuffer29, newGain[29],
                                       pBuffer30, newGain[30],
                                       pBuffer31, newGain[31],
                                       iBufferSize);
        } else {
            SampleUtil::copy32WithRampingGain(pOutput,
                                              pBuffer0, oldGain[0], newGain[0],
                                              pBuffer1, oldGain[1], newGain[1],
                                              pBuffer2, oldGain[2], newGain[2],
                                              pBuffer3, oldGain[3], newGain[3],
                                              pBuffer4, oldGain[4], newGain[4],
                                              pBuffer5, oldGain[5], newGain[5],
                                              pBuffer6, oldGain[6], newGain[6],
                                              pBuffer7, oldGain[7], newGain[7],
                                              pBuffer8, oldGain[8], newGain[8],
                                              pBuffer9, oldGain[9], newGain[9],
                                              pBuffer10, oldGain[10], newGain[10],
                                              pBuffer11, oldGain[11], newGain[11],
                                              pBuffer12, oldGain[12], newGain[12],
                                              pBuffer13, oldGain[13], newGain[13],
                                              pBuffer14, oldGain[14], newGain[14],
                                              pBuffer15, oldGain[15], newGain[15],
                                              pBuffer16, oldGain[16], newGain[16],
                                              pBuffer17, oldGain[17], newGain[17],
                                              pBuffer18, oldGain[18], newGain[18],
                                              pBuffer19, oldGain[19], newGain[19],
                                              pBuffer20, oldGain[20], newGain[20],
                                              pBuffer21, oldGain[21], newGain[21],
                                              pBuffer22, oldGain[22], newGain[22],
                                              pBuffer23, oldGain[23], newGain[23],
                                              pBuffer24, oldGain[24], newGain[24],
                                              pBuffer25, oldGain[25], newGain[25],
                                              pBuffer26, oldGain[26], newGain[26],
                                              pBuffer27, oldGain[27], newGain[27],
                                              pBuffer28, oldGain[28], newGain[28],
                                              pBuffer29, oldGain[29], newGain[29],
                                              pBuffer30, oldGain[30], newGain[30],
                                              pBuffer31, oldGain[31], newGain[31],
                                              iBufferSize);
        }
    } else {
        ScopedTimer t("EngineMaster::mixChannelsRamping_%1active", activeChannels->size());
        // Set pOutput to all 0s
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
            SampleUtil::addWithRampingGain(pOutput, pBuffer, oldGain, newGain, iBufferSize);
        }
    }
}
