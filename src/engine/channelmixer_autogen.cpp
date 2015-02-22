#include "engine/channelmixer.h"
#include "util/timer.h"
#include "sampleutil.h"
////////////////////////////////////////////////////////
// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //
// SEE scripts/generate_sample_functions.py           //
////////////////////////////////////////////////////////

// static
void ChannelMixer::mixChannels(const QList<EngineMaster::ChannelInfo*>& channels,
                               const EngineMaster::GainCalculator& gainCalculator,
                               unsigned int channelBitvector,
                               unsigned int maxChannels,
                               QList<CSAMPLE>* channelGainCache,
                               CSAMPLE* pOutput,
                               unsigned int iBufferSize) {
    int activeChannels[32] = {
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1};
    int totalActive = 0;
    for (unsigned int i = 0; i < maxChannels; ++i) {
        if ((channelBitvector & (1 << i)) == 0) {
            continue;
        }
        if (totalActive < 32) {
            activeChannels[totalActive] = i;
        }
        ++totalActive;
    }
    if (totalActive == 0) {
        ScopedTimer t("EngineMaster::mixChannels_0active");
        SampleUtil::clear(pOutput, iBufferSize);
    } else if (totalActive == 1) {
        ScopedTimer t("EngineMaster::mixChannels_1active");
        CSAMPLE_GAIN newGain[1];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        SampleUtil::copy1WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  iBufferSize);
    } else if (totalActive == 2) {
        ScopedTimer t("EngineMaster::mixChannels_2active");
        CSAMPLE_GAIN newGain[2];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        SampleUtil::copy2WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  pBuffer1, newGain[1],
                                  iBufferSize);
    } else if (totalActive == 3) {
        ScopedTimer t("EngineMaster::mixChannels_3active");
        CSAMPLE_GAIN newGain[3];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        SampleUtil::copy3WithGain(pOutput,
                                  pBuffer0, newGain[0],
                                  pBuffer1, newGain[1],
                                  pBuffer2, newGain[2],
                                  iBufferSize);
    } else if (totalActive == 4) {
        ScopedTimer t("EngineMaster::mixChannels_4active");
        CSAMPLE_GAIN newGain[4];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        const int pChannelIndex27 = activeChannels[27];
        EngineMaster::ChannelInfo* pChannel27 = channels[pChannelIndex27];
        newGain[27] = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain[27];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        const int pChannelIndex27 = activeChannels[27];
        EngineMaster::ChannelInfo* pChannel27 = channels[pChannelIndex27];
        newGain[27] = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        const int pChannelIndex28 = activeChannels[28];
        EngineMaster::ChannelInfo* pChannel28 = channels[pChannelIndex28];
        newGain[28] = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain[28];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        const int pChannelIndex27 = activeChannels[27];
        EngineMaster::ChannelInfo* pChannel27 = channels[pChannelIndex27];
        newGain[27] = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        const int pChannelIndex28 = activeChannels[28];
        EngineMaster::ChannelInfo* pChannel28 = channels[pChannelIndex28];
        newGain[28] = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        const int pChannelIndex29 = activeChannels[29];
        EngineMaster::ChannelInfo* pChannel29 = channels[pChannelIndex29];
        newGain[29] = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain[29];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        const int pChannelIndex27 = activeChannels[27];
        EngineMaster::ChannelInfo* pChannel27 = channels[pChannelIndex27];
        newGain[27] = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        const int pChannelIndex28 = activeChannels[28];
        EngineMaster::ChannelInfo* pChannel28 = channels[pChannelIndex28];
        newGain[28] = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        const int pChannelIndex29 = activeChannels[29];
        EngineMaster::ChannelInfo* pChannel29 = channels[pChannelIndex29];
        newGain[29] = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain[29];
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        const int pChannelIndex30 = activeChannels[30];
        EngineMaster::ChannelInfo* pChannel30 = channels[pChannelIndex30];
        newGain[30] = gainCalculator.getGain(pChannel30);
        (*channelGainCache)[pChannelIndex30] = newGain[30];
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
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        const int pChannelIndex27 = activeChannels[27];
        EngineMaster::ChannelInfo* pChannel27 = channels[pChannelIndex27];
        newGain[27] = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        const int pChannelIndex28 = activeChannels[28];
        EngineMaster::ChannelInfo* pChannel28 = channels[pChannelIndex28];
        newGain[28] = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        const int pChannelIndex29 = activeChannels[29];
        EngineMaster::ChannelInfo* pChannel29 = channels[pChannelIndex29];
        newGain[29] = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain[29];
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        const int pChannelIndex30 = activeChannels[30];
        EngineMaster::ChannelInfo* pChannel30 = channels[pChannelIndex30];
        newGain[30] = gainCalculator.getGain(pChannel30);
        (*channelGainCache)[pChannelIndex30] = newGain[30];
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        const int pChannelIndex31 = activeChannels[31];
        EngineMaster::ChannelInfo* pChannel31 = channels[pChannelIndex31];
        newGain[31] = gainCalculator.getGain(pChannel31);
        (*channelGainCache)[pChannelIndex31] = newGain[31];
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
        // Set pOutput to all 0s
        SampleUtil::clear(pOutput, iBufferSize);
        for (unsigned int i = 0; i < maxChannels; ++i) {
            if (channelBitvector & (1 << i)) {
                EngineMaster::ChannelInfo* pChannelInfo = channels[i];
                CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;
                CSAMPLE gain = gainCalculator.getGain(pChannelInfo);
                SampleUtil::addWithGain(pOutput, pBuffer, gain, iBufferSize);
            }
        }
    }
}
void ChannelMixer::mixChannelsRamping(const QList<EngineMaster::ChannelInfo*>& channels,
                                      const EngineMaster::GainCalculator& gainCalculator,
                                      unsigned int channelBitvector,
                                      unsigned int maxChannels,
                                      QList<CSAMPLE>* channelGainCache,
                                      CSAMPLE* pOutput,
                                      unsigned int iBufferSize) {
    int activeChannels[32] = {
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1};
    int totalActive = 0;
    for (unsigned int i = 0; i < maxChannels; ++i) {
        if ((channelBitvector & (1 << i)) == 0) {
            continue;
        }
        if (totalActive < 32) {
            activeChannels[totalActive] = i;
        }
        ++totalActive;
    }
    if (totalActive == 0) {
        ScopedTimer t("EngineMaster::mixChannels_0active");
        SampleUtil::clear(pOutput, iBufferSize);
    } else if (totalActive == 1) {
        ScopedTimer t("EngineMaster::mixChannels_1active");
        CSAMPLE_GAIN oldGain[1];
        CSAMPLE_GAIN newGain[1];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
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
        ScopedTimer t("EngineMaster::mixChannels_2active");
        CSAMPLE_GAIN oldGain[2];
        CSAMPLE_GAIN newGain[2];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
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
        ScopedTimer t("EngineMaster::mixChannels_3active");
        CSAMPLE_GAIN oldGain[3];
        CSAMPLE_GAIN newGain[3];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
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
        ScopedTimer t("EngineMaster::mixChannels_4active");
        CSAMPLE_GAIN oldGain[4];
        CSAMPLE_GAIN newGain[4];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
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
        ScopedTimer t("EngineMaster::mixChannels_5active");
        CSAMPLE_GAIN oldGain[5];
        CSAMPLE_GAIN newGain[5];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
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
        ScopedTimer t("EngineMaster::mixChannels_6active");
        CSAMPLE_GAIN oldGain[6];
        CSAMPLE_GAIN newGain[6];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
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
        ScopedTimer t("EngineMaster::mixChannels_7active");
        CSAMPLE_GAIN oldGain[7];
        CSAMPLE_GAIN newGain[7];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
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
        ScopedTimer t("EngineMaster::mixChannels_8active");
        CSAMPLE_GAIN oldGain[8];
        CSAMPLE_GAIN newGain[8];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
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
        ScopedTimer t("EngineMaster::mixChannels_9active");
        CSAMPLE_GAIN oldGain[9];
        CSAMPLE_GAIN newGain[9];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
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
        ScopedTimer t("EngineMaster::mixChannels_10active");
        CSAMPLE_GAIN oldGain[10];
        CSAMPLE_GAIN newGain[10];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
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
        ScopedTimer t("EngineMaster::mixChannels_11active");
        CSAMPLE_GAIN oldGain[11];
        CSAMPLE_GAIN newGain[11];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
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
        ScopedTimer t("EngineMaster::mixChannels_12active");
        CSAMPLE_GAIN oldGain[12];
        CSAMPLE_GAIN newGain[12];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
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
        ScopedTimer t("EngineMaster::mixChannels_13active");
        CSAMPLE_GAIN oldGain[13];
        CSAMPLE_GAIN newGain[13];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
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
        ScopedTimer t("EngineMaster::mixChannels_14active");
        CSAMPLE_GAIN oldGain[14];
        CSAMPLE_GAIN newGain[14];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
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
        ScopedTimer t("EngineMaster::mixChannels_15active");
        CSAMPLE_GAIN oldGain[15];
        CSAMPLE_GAIN newGain[15];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
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
        ScopedTimer t("EngineMaster::mixChannels_16active");
        CSAMPLE_GAIN oldGain[16];
        CSAMPLE_GAIN newGain[16];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
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
        ScopedTimer t("EngineMaster::mixChannels_17active");
        CSAMPLE_GAIN oldGain[17];
        CSAMPLE_GAIN newGain[17];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
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
        ScopedTimer t("EngineMaster::mixChannels_18active");
        CSAMPLE_GAIN oldGain[18];
        CSAMPLE_GAIN newGain[18];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
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
        ScopedTimer t("EngineMaster::mixChannels_19active");
        CSAMPLE_GAIN oldGain[19];
        CSAMPLE_GAIN newGain[19];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
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
        ScopedTimer t("EngineMaster::mixChannels_20active");
        CSAMPLE_GAIN oldGain[20];
        CSAMPLE_GAIN newGain[20];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
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
        ScopedTimer t("EngineMaster::mixChannels_21active");
        CSAMPLE_GAIN oldGain[21];
        CSAMPLE_GAIN newGain[21];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
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
        ScopedTimer t("EngineMaster::mixChannels_22active");
        CSAMPLE_GAIN oldGain[22];
        CSAMPLE_GAIN newGain[22];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        oldGain[21] = (*channelGainCache)[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
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
        ScopedTimer t("EngineMaster::mixChannels_23active");
        CSAMPLE_GAIN oldGain[23];
        CSAMPLE_GAIN newGain[23];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        oldGain[21] = (*channelGainCache)[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        oldGain[22] = (*channelGainCache)[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
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
        ScopedTimer t("EngineMaster::mixChannels_24active");
        CSAMPLE_GAIN oldGain[24];
        CSAMPLE_GAIN newGain[24];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        oldGain[21] = (*channelGainCache)[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        oldGain[22] = (*channelGainCache)[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        oldGain[23] = (*channelGainCache)[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
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
        ScopedTimer t("EngineMaster::mixChannels_25active");
        CSAMPLE_GAIN oldGain[25];
        CSAMPLE_GAIN newGain[25];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        oldGain[21] = (*channelGainCache)[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        oldGain[22] = (*channelGainCache)[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        oldGain[23] = (*channelGainCache)[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        oldGain[24] = (*channelGainCache)[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
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
        ScopedTimer t("EngineMaster::mixChannels_26active");
        CSAMPLE_GAIN oldGain[26];
        CSAMPLE_GAIN newGain[26];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        oldGain[21] = (*channelGainCache)[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        oldGain[22] = (*channelGainCache)[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        oldGain[23] = (*channelGainCache)[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        oldGain[24] = (*channelGainCache)[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        oldGain[25] = (*channelGainCache)[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
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
        ScopedTimer t("EngineMaster::mixChannels_27active");
        CSAMPLE_GAIN oldGain[27];
        CSAMPLE_GAIN newGain[27];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        oldGain[21] = (*channelGainCache)[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        oldGain[22] = (*channelGainCache)[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        oldGain[23] = (*channelGainCache)[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        oldGain[24] = (*channelGainCache)[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        oldGain[25] = (*channelGainCache)[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        oldGain[26] = (*channelGainCache)[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
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
        ScopedTimer t("EngineMaster::mixChannels_28active");
        CSAMPLE_GAIN oldGain[28];
        CSAMPLE_GAIN newGain[28];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        oldGain[21] = (*channelGainCache)[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        oldGain[22] = (*channelGainCache)[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        oldGain[23] = (*channelGainCache)[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        oldGain[24] = (*channelGainCache)[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        oldGain[25] = (*channelGainCache)[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        oldGain[26] = (*channelGainCache)[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        const int pChannelIndex27 = activeChannels[27];
        EngineMaster::ChannelInfo* pChannel27 = channels[pChannelIndex27];
        oldGain[27] = (*channelGainCache)[pChannelIndex27];
        newGain[27] = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain[27];
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
        ScopedTimer t("EngineMaster::mixChannels_29active");
        CSAMPLE_GAIN oldGain[29];
        CSAMPLE_GAIN newGain[29];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        oldGain[21] = (*channelGainCache)[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        oldGain[22] = (*channelGainCache)[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        oldGain[23] = (*channelGainCache)[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        oldGain[24] = (*channelGainCache)[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        oldGain[25] = (*channelGainCache)[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        oldGain[26] = (*channelGainCache)[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        const int pChannelIndex27 = activeChannels[27];
        EngineMaster::ChannelInfo* pChannel27 = channels[pChannelIndex27];
        oldGain[27] = (*channelGainCache)[pChannelIndex27];
        newGain[27] = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        const int pChannelIndex28 = activeChannels[28];
        EngineMaster::ChannelInfo* pChannel28 = channels[pChannelIndex28];
        oldGain[28] = (*channelGainCache)[pChannelIndex28];
        newGain[28] = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain[28];
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
        ScopedTimer t("EngineMaster::mixChannels_30active");
        CSAMPLE_GAIN oldGain[30];
        CSAMPLE_GAIN newGain[30];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        oldGain[21] = (*channelGainCache)[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        oldGain[22] = (*channelGainCache)[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        oldGain[23] = (*channelGainCache)[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        oldGain[24] = (*channelGainCache)[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        oldGain[25] = (*channelGainCache)[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        oldGain[26] = (*channelGainCache)[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        const int pChannelIndex27 = activeChannels[27];
        EngineMaster::ChannelInfo* pChannel27 = channels[pChannelIndex27];
        oldGain[27] = (*channelGainCache)[pChannelIndex27];
        newGain[27] = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        const int pChannelIndex28 = activeChannels[28];
        EngineMaster::ChannelInfo* pChannel28 = channels[pChannelIndex28];
        oldGain[28] = (*channelGainCache)[pChannelIndex28];
        newGain[28] = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        const int pChannelIndex29 = activeChannels[29];
        EngineMaster::ChannelInfo* pChannel29 = channels[pChannelIndex29];
        oldGain[29] = (*channelGainCache)[pChannelIndex29];
        newGain[29] = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain[29];
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
        ScopedTimer t("EngineMaster::mixChannels_31active");
        CSAMPLE_GAIN oldGain[31];
        CSAMPLE_GAIN newGain[31];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        oldGain[21] = (*channelGainCache)[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        oldGain[22] = (*channelGainCache)[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        oldGain[23] = (*channelGainCache)[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        oldGain[24] = (*channelGainCache)[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        oldGain[25] = (*channelGainCache)[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        oldGain[26] = (*channelGainCache)[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        const int pChannelIndex27 = activeChannels[27];
        EngineMaster::ChannelInfo* pChannel27 = channels[pChannelIndex27];
        oldGain[27] = (*channelGainCache)[pChannelIndex27];
        newGain[27] = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        const int pChannelIndex28 = activeChannels[28];
        EngineMaster::ChannelInfo* pChannel28 = channels[pChannelIndex28];
        oldGain[28] = (*channelGainCache)[pChannelIndex28];
        newGain[28] = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        const int pChannelIndex29 = activeChannels[29];
        EngineMaster::ChannelInfo* pChannel29 = channels[pChannelIndex29];
        oldGain[29] = (*channelGainCache)[pChannelIndex29];
        newGain[29] = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain[29];
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        const int pChannelIndex30 = activeChannels[30];
        EngineMaster::ChannelInfo* pChannel30 = channels[pChannelIndex30];
        oldGain[30] = (*channelGainCache)[pChannelIndex30];
        newGain[30] = gainCalculator.getGain(pChannel30);
        (*channelGainCache)[pChannelIndex30] = newGain[30];
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
        ScopedTimer t("EngineMaster::mixChannels_32active");
        CSAMPLE_GAIN oldGain[32];
        CSAMPLE_GAIN newGain[32];
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        oldGain[0] = (*channelGainCache)[pChannelIndex0];
        newGain[0] = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain[0];
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        oldGain[1] = (*channelGainCache)[pChannelIndex1];
        newGain[1] = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain[1];
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        oldGain[2] = (*channelGainCache)[pChannelIndex2];
        newGain[2] = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain[2];
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        oldGain[3] = (*channelGainCache)[pChannelIndex3];
        newGain[3] = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain[3];
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        oldGain[4] = (*channelGainCache)[pChannelIndex4];
        newGain[4] = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain[4];
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        oldGain[5] = (*channelGainCache)[pChannelIndex5];
        newGain[5] = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain[5];
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        oldGain[6] = (*channelGainCache)[pChannelIndex6];
        newGain[6] = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain[6];
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        oldGain[7] = (*channelGainCache)[pChannelIndex7];
        newGain[7] = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain[7];
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        oldGain[8] = (*channelGainCache)[pChannelIndex8];
        newGain[8] = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain[8];
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        oldGain[9] = (*channelGainCache)[pChannelIndex9];
        newGain[9] = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain[9];
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        oldGain[10] = (*channelGainCache)[pChannelIndex10];
        newGain[10] = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain[10];
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        oldGain[11] = (*channelGainCache)[pChannelIndex11];
        newGain[11] = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain[11];
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        oldGain[12] = (*channelGainCache)[pChannelIndex12];
        newGain[12] = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain[12];
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        oldGain[13] = (*channelGainCache)[pChannelIndex13];
        newGain[13] = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain[13];
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        oldGain[14] = (*channelGainCache)[pChannelIndex14];
        newGain[14] = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain[14];
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        oldGain[15] = (*channelGainCache)[pChannelIndex15];
        newGain[15] = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain[15];
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        oldGain[16] = (*channelGainCache)[pChannelIndex16];
        newGain[16] = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain[16];
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        oldGain[17] = (*channelGainCache)[pChannelIndex17];
        newGain[17] = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain[17];
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        oldGain[18] = (*channelGainCache)[pChannelIndex18];
        newGain[18] = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain[18];
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        oldGain[19] = (*channelGainCache)[pChannelIndex19];
        newGain[19] = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain[19];
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        oldGain[20] = (*channelGainCache)[pChannelIndex20];
        newGain[20] = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain[20];
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        oldGain[21] = (*channelGainCache)[pChannelIndex21];
        newGain[21] = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain[21];
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        oldGain[22] = (*channelGainCache)[pChannelIndex22];
        newGain[22] = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain[22];
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        oldGain[23] = (*channelGainCache)[pChannelIndex23];
        newGain[23] = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain[23];
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        const int pChannelIndex24 = activeChannels[24];
        EngineMaster::ChannelInfo* pChannel24 = channels[pChannelIndex24];
        oldGain[24] = (*channelGainCache)[pChannelIndex24];
        newGain[24] = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain[24];
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        const int pChannelIndex25 = activeChannels[25];
        EngineMaster::ChannelInfo* pChannel25 = channels[pChannelIndex25];
        oldGain[25] = (*channelGainCache)[pChannelIndex25];
        newGain[25] = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain[25];
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        const int pChannelIndex26 = activeChannels[26];
        EngineMaster::ChannelInfo* pChannel26 = channels[pChannelIndex26];
        oldGain[26] = (*channelGainCache)[pChannelIndex26];
        newGain[26] = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain[26];
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        const int pChannelIndex27 = activeChannels[27];
        EngineMaster::ChannelInfo* pChannel27 = channels[pChannelIndex27];
        oldGain[27] = (*channelGainCache)[pChannelIndex27];
        newGain[27] = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain[27];
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        const int pChannelIndex28 = activeChannels[28];
        EngineMaster::ChannelInfo* pChannel28 = channels[pChannelIndex28];
        oldGain[28] = (*channelGainCache)[pChannelIndex28];
        newGain[28] = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain[28];
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        const int pChannelIndex29 = activeChannels[29];
        EngineMaster::ChannelInfo* pChannel29 = channels[pChannelIndex29];
        oldGain[29] = (*channelGainCache)[pChannelIndex29];
        newGain[29] = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain[29];
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        const int pChannelIndex30 = activeChannels[30];
        EngineMaster::ChannelInfo* pChannel30 = channels[pChannelIndex30];
        oldGain[30] = (*channelGainCache)[pChannelIndex30];
        newGain[30] = gainCalculator.getGain(pChannel30);
        (*channelGainCache)[pChannelIndex30] = newGain[30];
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        const int pChannelIndex31 = activeChannels[31];
        EngineMaster::ChannelInfo* pChannel31 = channels[pChannelIndex31];
        oldGain[31] = (*channelGainCache)[pChannelIndex31];
        newGain[31] = gainCalculator.getGain(pChannel31);
        (*channelGainCache)[pChannelIndex31] = newGain[31];
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
        // Set pOutput to all 0s
        SampleUtil::clear(pOutput, iBufferSize);
        for (unsigned int i = 0; i < maxChannels; ++i) {
            if (channelBitvector & (1 << i)) {
                EngineMaster::ChannelInfo* pChannelInfo = channels[i];
                CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;
                CSAMPLE gain = gainCalculator.getGain(pChannelInfo);
                SampleUtil::addWithGain(pOutput, pBuffer, gain, iBufferSize);
            }
        }
    }
}
