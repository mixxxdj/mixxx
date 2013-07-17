#include "engine/channelmixer.h"
#include "util/timer.h"
#include "sampleutil.h"
////////////////////////////////////////////////////////
// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //
////////////////////////////////////////////////////////

// static
void ChannelMixer::mixChannels(const QList<EngineMaster::ChannelInfo*>& channels,
                               const EngineMaster::GainCalculator& gainCalculator,
                               unsigned int channelBitvector,
                               unsigned int maxChannels,
                               QList<CSAMPLE>* channelGainCache,
                               CSAMPLE* pOutput,
                               unsigned int iBufferSize) { 
    int activeChannels[24] = {
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
    unsigned int totalActive = 0;
    for (unsigned int i = 0; i < maxChannels; ++i) {
        if ((channelBitvector & (1 << i)) == 0) {
            continue;
        }
        if (totalActive < 24) {
            activeChannels[totalActive] = i;
        }
        ++totalActive;
    }
    if (totalActive == 0) {
        ScopedTimer t("EngineMaster::mixChannels_0active");
        SampleUtil::applyGain(pOutput, 0.0f, iBufferSize);
    } else if (totalActive == 1) {
        ScopedTimer t("EngineMaster::mixChannels_1active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        SampleUtil::copy1WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         iBufferSize);
    } else if (totalActive == 2) {
        ScopedTimer t("EngineMaster::mixChannels_2active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        SampleUtil::copy2WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         pBuffer1, oldGain1, newGain1,
                                         iBufferSize);
    } else if (totalActive == 3) {
        ScopedTimer t("EngineMaster::mixChannels_3active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        SampleUtil::copy3WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         pBuffer1, oldGain1, newGain1,
                                         pBuffer2, oldGain2, newGain2,
                                         iBufferSize);
    } else if (totalActive == 4) {
        ScopedTimer t("EngineMaster::mixChannels_4active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        SampleUtil::copy4WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         pBuffer1, oldGain1, newGain1,
                                         pBuffer2, oldGain2, newGain2,
                                         pBuffer3, oldGain3, newGain3,
                                         iBufferSize);
    } else if (totalActive == 5) {
        ScopedTimer t("EngineMaster::mixChannels_5active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        SampleUtil::copy5WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         pBuffer1, oldGain1, newGain1,
                                         pBuffer2, oldGain2, newGain2,
                                         pBuffer3, oldGain3, newGain3,
                                         pBuffer4, oldGain4, newGain4,
                                         iBufferSize);
    } else if (totalActive == 6) {
        ScopedTimer t("EngineMaster::mixChannels_6active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        SampleUtil::copy6WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         pBuffer1, oldGain1, newGain1,
                                         pBuffer2, oldGain2, newGain2,
                                         pBuffer3, oldGain3, newGain3,
                                         pBuffer4, oldGain4, newGain4,
                                         pBuffer5, oldGain5, newGain5,
                                         iBufferSize);
    } else if (totalActive == 7) {
        ScopedTimer t("EngineMaster::mixChannels_7active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        SampleUtil::copy7WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         pBuffer1, oldGain1, newGain1,
                                         pBuffer2, oldGain2, newGain2,
                                         pBuffer3, oldGain3, newGain3,
                                         pBuffer4, oldGain4, newGain4,
                                         pBuffer5, oldGain5, newGain5,
                                         pBuffer6, oldGain6, newGain6,
                                         iBufferSize);
    } else if (totalActive == 8) {
        ScopedTimer t("EngineMaster::mixChannels_8active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        SampleUtil::copy8WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         pBuffer1, oldGain1, newGain1,
                                         pBuffer2, oldGain2, newGain2,
                                         pBuffer3, oldGain3, newGain3,
                                         pBuffer4, oldGain4, newGain4,
                                         pBuffer5, oldGain5, newGain5,
                                         pBuffer6, oldGain6, newGain6,
                                         pBuffer7, oldGain7, newGain7,
                                         iBufferSize);
    } else if (totalActive == 9) {
        ScopedTimer t("EngineMaster::mixChannels_9active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        SampleUtil::copy9WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         pBuffer1, oldGain1, newGain1,
                                         pBuffer2, oldGain2, newGain2,
                                         pBuffer3, oldGain3, newGain3,
                                         pBuffer4, oldGain4, newGain4,
                                         pBuffer5, oldGain5, newGain5,
                                         pBuffer6, oldGain6, newGain6,
                                         pBuffer7, oldGain7, newGain7,
                                         pBuffer8, oldGain8, newGain8,
                                         iBufferSize);
    } else if (totalActive == 10) {
        ScopedTimer t("EngineMaster::mixChannels_10active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        SampleUtil::copy10WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          iBufferSize);
    } else if (totalActive == 11) {
        ScopedTimer t("EngineMaster::mixChannels_11active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        SampleUtil::copy11WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          iBufferSize);
    } else if (totalActive == 12) {
        ScopedTimer t("EngineMaster::mixChannels_12active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        SampleUtil::copy12WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          iBufferSize);
    } else if (totalActive == 13) {
        ScopedTimer t("EngineMaster::mixChannels_13active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        SampleUtil::copy13WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          iBufferSize);
    } else if (totalActive == 14) {
        ScopedTimer t("EngineMaster::mixChannels_14active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        CSAMPLE oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        SampleUtil::copy14WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          pBuffer13, oldGain13, newGain13,
                                          iBufferSize);
    } else if (totalActive == 15) {
        ScopedTimer t("EngineMaster::mixChannels_15active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        CSAMPLE oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        CSAMPLE oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        SampleUtil::copy15WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          pBuffer13, oldGain13, newGain13,
                                          pBuffer14, oldGain14, newGain14,
                                          iBufferSize);
    } else if (totalActive == 16) {
        ScopedTimer t("EngineMaster::mixChannels_16active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        CSAMPLE oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        CSAMPLE oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        CSAMPLE oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        SampleUtil::copy16WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          pBuffer13, oldGain13, newGain13,
                                          pBuffer14, oldGain14, newGain14,
                                          pBuffer15, oldGain15, newGain15,
                                          iBufferSize);
    } else if (totalActive == 17) {
        ScopedTimer t("EngineMaster::mixChannels_17active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        CSAMPLE oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        CSAMPLE oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        CSAMPLE oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        CSAMPLE oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        SampleUtil::copy17WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          pBuffer13, oldGain13, newGain13,
                                          pBuffer14, oldGain14, newGain14,
                                          pBuffer15, oldGain15, newGain15,
                                          pBuffer16, oldGain16, newGain16,
                                          iBufferSize);
    } else if (totalActive == 18) {
        ScopedTimer t("EngineMaster::mixChannels_18active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        CSAMPLE oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        CSAMPLE oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        CSAMPLE oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        CSAMPLE oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        CSAMPLE oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        SampleUtil::copy18WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          pBuffer13, oldGain13, newGain13,
                                          pBuffer14, oldGain14, newGain14,
                                          pBuffer15, oldGain15, newGain15,
                                          pBuffer16, oldGain16, newGain16,
                                          pBuffer17, oldGain17, newGain17,
                                          iBufferSize);
    } else if (totalActive == 19) {
        ScopedTimer t("EngineMaster::mixChannels_19active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        CSAMPLE oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        CSAMPLE oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        CSAMPLE oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        CSAMPLE oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        CSAMPLE oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        CSAMPLE oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        SampleUtil::copy19WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          pBuffer13, oldGain13, newGain13,
                                          pBuffer14, oldGain14, newGain14,
                                          pBuffer15, oldGain15, newGain15,
                                          pBuffer16, oldGain16, newGain16,
                                          pBuffer17, oldGain17, newGain17,
                                          pBuffer18, oldGain18, newGain18,
                                          iBufferSize);
    } else if (totalActive == 20) {
        ScopedTimer t("EngineMaster::mixChannels_20active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        CSAMPLE oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        CSAMPLE oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        CSAMPLE oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        CSAMPLE oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        CSAMPLE oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        CSAMPLE oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        CSAMPLE oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        SampleUtil::copy20WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          pBuffer13, oldGain13, newGain13,
                                          pBuffer14, oldGain14, newGain14,
                                          pBuffer15, oldGain15, newGain15,
                                          pBuffer16, oldGain16, newGain16,
                                          pBuffer17, oldGain17, newGain17,
                                          pBuffer18, oldGain18, newGain18,
                                          pBuffer19, oldGain19, newGain19,
                                          iBufferSize);
    } else if (totalActive == 21) {
        ScopedTimer t("EngineMaster::mixChannels_21active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        CSAMPLE oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        CSAMPLE oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        CSAMPLE oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        CSAMPLE oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        CSAMPLE oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        CSAMPLE oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        CSAMPLE oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        CSAMPLE oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        SampleUtil::copy21WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          pBuffer13, oldGain13, newGain13,
                                          pBuffer14, oldGain14, newGain14,
                                          pBuffer15, oldGain15, newGain15,
                                          pBuffer16, oldGain16, newGain16,
                                          pBuffer17, oldGain17, newGain17,
                                          pBuffer18, oldGain18, newGain18,
                                          pBuffer19, oldGain19, newGain19,
                                          pBuffer20, oldGain20, newGain20,
                                          iBufferSize);
    } else if (totalActive == 22) {
        ScopedTimer t("EngineMaster::mixChannels_22active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        CSAMPLE oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        CSAMPLE oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        CSAMPLE oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        CSAMPLE oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        CSAMPLE oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        CSAMPLE oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        CSAMPLE oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        CSAMPLE oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        CSAMPLE oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        SampleUtil::copy22WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          pBuffer13, oldGain13, newGain13,
                                          pBuffer14, oldGain14, newGain14,
                                          pBuffer15, oldGain15, newGain15,
                                          pBuffer16, oldGain16, newGain16,
                                          pBuffer17, oldGain17, newGain17,
                                          pBuffer18, oldGain18, newGain18,
                                          pBuffer19, oldGain19, newGain19,
                                          pBuffer20, oldGain20, newGain20,
                                          pBuffer21, oldGain21, newGain21,
                                          iBufferSize);
    } else if (totalActive == 23) {
        ScopedTimer t("EngineMaster::mixChannels_23active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        CSAMPLE oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        CSAMPLE oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        CSAMPLE oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        CSAMPLE oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        CSAMPLE oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        CSAMPLE oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        CSAMPLE oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        CSAMPLE oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        CSAMPLE oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        CSAMPLE oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        SampleUtil::copy23WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          pBuffer13, oldGain13, newGain13,
                                          pBuffer14, oldGain14, newGain14,
                                          pBuffer15, oldGain15, newGain15,
                                          pBuffer16, oldGain16, newGain16,
                                          pBuffer17, oldGain17, newGain17,
                                          pBuffer18, oldGain18, newGain18,
                                          pBuffer19, oldGain19, newGain19,
                                          pBuffer20, oldGain20, newGain20,
                                          pBuffer21, oldGain21, newGain21,
                                          pBuffer22, oldGain22, newGain22,
                                          iBufferSize);
    } else if (totalActive == 24) {
        ScopedTimer t("EngineMaster::mixChannels_24active");
        const int pChannelIndex0 = activeChannels[0];
        EngineMaster::ChannelInfo* pChannel0 = channels[pChannelIndex0];
        CSAMPLE oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        const int pChannelIndex1 = activeChannels[1];
        EngineMaster::ChannelInfo* pChannel1 = channels[pChannelIndex1];
        CSAMPLE oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        const int pChannelIndex2 = activeChannels[2];
        EngineMaster::ChannelInfo* pChannel2 = channels[pChannelIndex2];
        CSAMPLE oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        const int pChannelIndex3 = activeChannels[3];
        EngineMaster::ChannelInfo* pChannel3 = channels[pChannelIndex3];
        CSAMPLE oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        const int pChannelIndex4 = activeChannels[4];
        EngineMaster::ChannelInfo* pChannel4 = channels[pChannelIndex4];
        CSAMPLE oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        const int pChannelIndex5 = activeChannels[5];
        EngineMaster::ChannelInfo* pChannel5 = channels[pChannelIndex5];
        CSAMPLE oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        const int pChannelIndex6 = activeChannels[6];
        EngineMaster::ChannelInfo* pChannel6 = channels[pChannelIndex6];
        CSAMPLE oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        const int pChannelIndex7 = activeChannels[7];
        EngineMaster::ChannelInfo* pChannel7 = channels[pChannelIndex7];
        CSAMPLE oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        const int pChannelIndex8 = activeChannels[8];
        EngineMaster::ChannelInfo* pChannel8 = channels[pChannelIndex8];
        CSAMPLE oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        const int pChannelIndex9 = activeChannels[9];
        EngineMaster::ChannelInfo* pChannel9 = channels[pChannelIndex9];
        CSAMPLE oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        const int pChannelIndex10 = activeChannels[10];
        EngineMaster::ChannelInfo* pChannel10 = channels[pChannelIndex10];
        CSAMPLE oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        const int pChannelIndex11 = activeChannels[11];
        EngineMaster::ChannelInfo* pChannel11 = channels[pChannelIndex11];
        CSAMPLE oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        const int pChannelIndex12 = activeChannels[12];
        EngineMaster::ChannelInfo* pChannel12 = channels[pChannelIndex12];
        CSAMPLE oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        const int pChannelIndex13 = activeChannels[13];
        EngineMaster::ChannelInfo* pChannel13 = channels[pChannelIndex13];
        CSAMPLE oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        const int pChannelIndex14 = activeChannels[14];
        EngineMaster::ChannelInfo* pChannel14 = channels[pChannelIndex14];
        CSAMPLE oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        const int pChannelIndex15 = activeChannels[15];
        EngineMaster::ChannelInfo* pChannel15 = channels[pChannelIndex15];
        CSAMPLE oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        const int pChannelIndex16 = activeChannels[16];
        EngineMaster::ChannelInfo* pChannel16 = channels[pChannelIndex16];
        CSAMPLE oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        const int pChannelIndex17 = activeChannels[17];
        EngineMaster::ChannelInfo* pChannel17 = channels[pChannelIndex17];
        CSAMPLE oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        const int pChannelIndex18 = activeChannels[18];
        EngineMaster::ChannelInfo* pChannel18 = channels[pChannelIndex18];
        CSAMPLE oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        const int pChannelIndex19 = activeChannels[19];
        EngineMaster::ChannelInfo* pChannel19 = channels[pChannelIndex19];
        CSAMPLE oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        const int pChannelIndex20 = activeChannels[20];
        EngineMaster::ChannelInfo* pChannel20 = channels[pChannelIndex20];
        CSAMPLE oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        const int pChannelIndex21 = activeChannels[21];
        EngineMaster::ChannelInfo* pChannel21 = channels[pChannelIndex21];
        CSAMPLE oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        const int pChannelIndex22 = activeChannels[22];
        EngineMaster::ChannelInfo* pChannel22 = channels[pChannelIndex22];
        CSAMPLE oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        const int pChannelIndex23 = activeChannels[23];
        EngineMaster::ChannelInfo* pChannel23 = channels[pChannelIndex23];
        CSAMPLE oldGain23 = (*channelGainCache)[pChannelIndex23];
        CSAMPLE newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        SampleUtil::copy24WithRampingGain(pOutput,
                                          pBuffer0, oldGain0, newGain0,
                                          pBuffer1, oldGain1, newGain1,
                                          pBuffer2, oldGain2, newGain2,
                                          pBuffer3, oldGain3, newGain3,
                                          pBuffer4, oldGain4, newGain4,
                                          pBuffer5, oldGain5, newGain5,
                                          pBuffer6, oldGain6, newGain6,
                                          pBuffer7, oldGain7, newGain7,
                                          pBuffer8, oldGain8, newGain8,
                                          pBuffer9, oldGain9, newGain9,
                                          pBuffer10, oldGain10, newGain10,
                                          pBuffer11, oldGain11, newGain11,
                                          pBuffer12, oldGain12, newGain12,
                                          pBuffer13, oldGain13, newGain13,
                                          pBuffer14, oldGain14, newGain14,
                                          pBuffer15, oldGain15, newGain15,
                                          pBuffer16, oldGain16, newGain16,
                                          pBuffer17, oldGain17, newGain17,
                                          pBuffer18, oldGain18, newGain18,
                                          pBuffer19, oldGain19, newGain19,
                                          pBuffer20, oldGain20, newGain20,
                                          pBuffer21, oldGain21, newGain21,
                                          pBuffer22, oldGain22, newGain22,
                                          pBuffer23, oldGain23, newGain23,
                                          iBufferSize);
    } else {
        // Set pOutput to all 0s
        SampleUtil::applyGain(pOutput, 0.0f, iBufferSize);
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
