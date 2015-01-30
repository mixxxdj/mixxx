#include "engine/channelmixer.h"
#include "util/timer.h"
#include "sampleutil.h"
////////////////////////////////////////////////////////
// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //
// SEE scripts/generate_sample_functions.py           //
////////////////////////////////////////////////////////

// static
void ChannelMixer::mixChannels(const EngineMaster::GainCalculator& gainCalculator,
                               EngineMaster::FastVector<EngineMaster::ChannelInfo*, kMaxChannels>* activeChannels,
                               EngineMaster::FastVector<CSAMPLE, kMaxChannels>* channelGainCache,
                               CSAMPLE* pOutput,
                               unsigned int iBufferSize) {
    unsigned int totalActive = activeChannels->size();
    if (totalActive == 0) {
        ScopedTimer t("EngineMaster::mixChannels_0active");
        SampleUtil::clear(pOutput, iBufferSize);
    } else if (totalActive == 1) {
        ScopedTimer t("EngineMaster::mixChannels_1active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        SampleUtil::copy1WithGain(pOutput,
                                  pBuffer0, newGain0,
                                  iBufferSize);
    } else if (totalActive == 2) {
        ScopedTimer t("EngineMaster::mixChannels_2active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        SampleUtil::copy2WithGain(pOutput,
                                  pBuffer0, newGain0,
                                  pBuffer1, newGain1,
                                  iBufferSize);
    } else if (totalActive == 3) {
        ScopedTimer t("EngineMaster::mixChannels_3active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        SampleUtil::copy3WithGain(pOutput,
                                  pBuffer0, newGain0,
                                  pBuffer1, newGain1,
                                  pBuffer2, newGain2,
                                  iBufferSize);
    } else if (totalActive == 4) {
        ScopedTimer t("EngineMaster::mixChannels_4active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        SampleUtil::copy4WithGain(pOutput,
                                  pBuffer0, newGain0,
                                  pBuffer1, newGain1,
                                  pBuffer2, newGain2,
                                  pBuffer3, newGain3,
                                  iBufferSize);
    } else if (totalActive == 5) {
        ScopedTimer t("EngineMaster::mixChannels_5active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        SampleUtil::copy5WithGain(pOutput,
                                  pBuffer0, newGain0,
                                  pBuffer1, newGain1,
                                  pBuffer2, newGain2,
                                  pBuffer3, newGain3,
                                  pBuffer4, newGain4,
                                  iBufferSize);
    } else if (totalActive == 6) {
        ScopedTimer t("EngineMaster::mixChannels_6active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        SampleUtil::copy6WithGain(pOutput,
                                  pBuffer0, newGain0,
                                  pBuffer1, newGain1,
                                  pBuffer2, newGain2,
                                  pBuffer3, newGain3,
                                  pBuffer4, newGain4,
                                  pBuffer5, newGain5,
                                  iBufferSize);
    } else if (totalActive == 7) {
        ScopedTimer t("EngineMaster::mixChannels_7active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        SampleUtil::copy7WithGain(pOutput,
                                  pBuffer0, newGain0,
                                  pBuffer1, newGain1,
                                  pBuffer2, newGain2,
                                  pBuffer3, newGain3,
                                  pBuffer4, newGain4,
                                  pBuffer5, newGain5,
                                  pBuffer6, newGain6,
                                  iBufferSize);
    } else if (totalActive == 8) {
        ScopedTimer t("EngineMaster::mixChannels_8active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        SampleUtil::copy8WithGain(pOutput,
                                  pBuffer0, newGain0,
                                  pBuffer1, newGain1,
                                  pBuffer2, newGain2,
                                  pBuffer3, newGain3,
                                  pBuffer4, newGain4,
                                  pBuffer5, newGain5,
                                  pBuffer6, newGain6,
                                  pBuffer7, newGain7,
                                  iBufferSize);
    } else if (totalActive == 9) {
        ScopedTimer t("EngineMaster::mixChannels_9active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        SampleUtil::copy9WithGain(pOutput,
                                  pBuffer0, newGain0,
                                  pBuffer1, newGain1,
                                  pBuffer2, newGain2,
                                  pBuffer3, newGain3,
                                  pBuffer4, newGain4,
                                  pBuffer5, newGain5,
                                  pBuffer6, newGain6,
                                  pBuffer7, newGain7,
                                  pBuffer8, newGain8,
                                  iBufferSize);
    } else if (totalActive == 10) {
        ScopedTimer t("EngineMaster::mixChannels_10active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        SampleUtil::copy10WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   iBufferSize);
    } else if (totalActive == 11) {
        ScopedTimer t("EngineMaster::mixChannels_11active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        SampleUtil::copy11WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   iBufferSize);
    } else if (totalActive == 12) {
        ScopedTimer t("EngineMaster::mixChannels_12active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        SampleUtil::copy12WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   iBufferSize);
    } else if (totalActive == 13) {
        ScopedTimer t("EngineMaster::mixChannels_13active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        SampleUtil::copy13WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   iBufferSize);
    } else if (totalActive == 14) {
        ScopedTimer t("EngineMaster::mixChannels_14active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        SampleUtil::copy14WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   iBufferSize);
    } else if (totalActive == 15) {
        ScopedTimer t("EngineMaster::mixChannels_15active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        SampleUtil::copy15WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   iBufferSize);
    } else if (totalActive == 16) {
        ScopedTimer t("EngineMaster::mixChannels_16active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        SampleUtil::copy16WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   iBufferSize);
    } else if (totalActive == 17) {
        ScopedTimer t("EngineMaster::mixChannels_17active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        SampleUtil::copy17WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   iBufferSize);
    } else if (totalActive == 18) {
        ScopedTimer t("EngineMaster::mixChannels_18active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        SampleUtil::copy18WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   iBufferSize);
    } else if (totalActive == 19) {
        ScopedTimer t("EngineMaster::mixChannels_19active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        SampleUtil::copy19WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   iBufferSize);
    } else if (totalActive == 20) {
        ScopedTimer t("EngineMaster::mixChannels_20active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        SampleUtil::copy20WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   iBufferSize);
    } else if (totalActive == 21) {
        ScopedTimer t("EngineMaster::mixChannels_21active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        SampleUtil::copy21WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   iBufferSize);
    } else if (totalActive == 22) {
        ScopedTimer t("EngineMaster::mixChannels_22active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        SampleUtil::copy22WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   pBuffer21, newGain21,
                                   iBufferSize);
    } else if (totalActive == 23) {
        ScopedTimer t("EngineMaster::mixChannels_23active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        SampleUtil::copy23WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   pBuffer21, newGain21,
                                   pBuffer22, newGain22,
                                   iBufferSize);
    } else if (totalActive == 24) {
        ScopedTimer t("EngineMaster::mixChannels_24active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        SampleUtil::copy24WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   pBuffer21, newGain21,
                                   pBuffer22, newGain22,
                                   pBuffer23, newGain23,
                                   iBufferSize);
    } else if (totalActive == 25) {
        ScopedTimer t("EngineMaster::mixChannels_25active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        SampleUtil::copy25WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   pBuffer21, newGain21,
                                   pBuffer22, newGain22,
                                   pBuffer23, newGain23,
                                   pBuffer24, newGain24,
                                   iBufferSize);
    } else if (totalActive == 26) {
        ScopedTimer t("EngineMaster::mixChannels_26active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        SampleUtil::copy26WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   pBuffer21, newGain21,
                                   pBuffer22, newGain22,
                                   pBuffer23, newGain23,
                                   pBuffer24, newGain24,
                                   pBuffer25, newGain25,
                                   iBufferSize);
    } else if (totalActive == 27) {
        ScopedTimer t("EngineMaster::mixChannels_27active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        SampleUtil::copy27WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   pBuffer21, newGain21,
                                   pBuffer22, newGain22,
                                   pBuffer23, newGain23,
                                   pBuffer24, newGain24,
                                   pBuffer25, newGain25,
                                   pBuffer26, newGain26,
                                   iBufferSize);
    } else if (totalActive == 28) {
        ScopedTimer t("EngineMaster::mixChannels_28active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int pChannelIndex27 = pChannel27->m_index;
        CSAMPLE_GAIN newGain27 = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain27;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        SampleUtil::copy28WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   pBuffer21, newGain21,
                                   pBuffer22, newGain22,
                                   pBuffer23, newGain23,
                                   pBuffer24, newGain24,
                                   pBuffer25, newGain25,
                                   pBuffer26, newGain26,
                                   pBuffer27, newGain27,
                                   iBufferSize);
    } else if (totalActive == 29) {
        ScopedTimer t("EngineMaster::mixChannels_29active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int pChannelIndex27 = pChannel27->m_index;
        CSAMPLE_GAIN newGain27 = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain27;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int pChannelIndex28 = pChannel28->m_index;
        CSAMPLE_GAIN newGain28 = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain28;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        SampleUtil::copy29WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   pBuffer21, newGain21,
                                   pBuffer22, newGain22,
                                   pBuffer23, newGain23,
                                   pBuffer24, newGain24,
                                   pBuffer25, newGain25,
                                   pBuffer26, newGain26,
                                   pBuffer27, newGain27,
                                   pBuffer28, newGain28,
                                   iBufferSize);
    } else if (totalActive == 30) {
        ScopedTimer t("EngineMaster::mixChannels_30active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int pChannelIndex27 = pChannel27->m_index;
        CSAMPLE_GAIN newGain27 = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain27;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int pChannelIndex28 = pChannel28->m_index;
        CSAMPLE_GAIN newGain28 = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain28;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int pChannelIndex29 = pChannel29->m_index;
        CSAMPLE_GAIN newGain29 = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain29;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        SampleUtil::copy30WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   pBuffer21, newGain21,
                                   pBuffer22, newGain22,
                                   pBuffer23, newGain23,
                                   pBuffer24, newGain24,
                                   pBuffer25, newGain25,
                                   pBuffer26, newGain26,
                                   pBuffer27, newGain27,
                                   pBuffer28, newGain28,
                                   pBuffer29, newGain29,
                                   iBufferSize);
    } else if (totalActive == 31) {
        ScopedTimer t("EngineMaster::mixChannels_31active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int pChannelIndex27 = pChannel27->m_index;
        CSAMPLE_GAIN newGain27 = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain27;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int pChannelIndex28 = pChannel28->m_index;
        CSAMPLE_GAIN newGain28 = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain28;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int pChannelIndex29 = pChannel29->m_index;
        CSAMPLE_GAIN newGain29 = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain29;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int pChannelIndex30 = pChannel30->m_index;
        CSAMPLE_GAIN newGain30 = gainCalculator.getGain(pChannel30);
        (*channelGainCache)[pChannelIndex30] = newGain30;
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        SampleUtil::copy31WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   pBuffer21, newGain21,
                                   pBuffer22, newGain22,
                                   pBuffer23, newGain23,
                                   pBuffer24, newGain24,
                                   pBuffer25, newGain25,
                                   pBuffer26, newGain26,
                                   pBuffer27, newGain27,
                                   pBuffer28, newGain28,
                                   pBuffer29, newGain29,
                                   pBuffer30, newGain30,
                                   iBufferSize);
    } else if (totalActive == 32) {
        ScopedTimer t("EngineMaster::mixChannels_32active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int pChannelIndex27 = pChannel27->m_index;
        CSAMPLE_GAIN newGain27 = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain27;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int pChannelIndex28 = pChannel28->m_index;
        CSAMPLE_GAIN newGain28 = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain28;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int pChannelIndex29 = pChannel29->m_index;
        CSAMPLE_GAIN newGain29 = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain29;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int pChannelIndex30 = pChannel30->m_index;
        CSAMPLE_GAIN newGain30 = gainCalculator.getGain(pChannel30);
        (*channelGainCache)[pChannelIndex30] = newGain30;
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel31 = activeChannels->at(31);
        const int pChannelIndex31 = pChannel31->m_index;
        CSAMPLE_GAIN newGain31 = gainCalculator.getGain(pChannel31);
        (*channelGainCache)[pChannelIndex31] = newGain31;
        CSAMPLE* pBuffer31 = pChannel31->m_pBuffer;
        SampleUtil::copy32WithGain(pOutput,
                                   pBuffer0, newGain0,
                                   pBuffer1, newGain1,
                                   pBuffer2, newGain2,
                                   pBuffer3, newGain3,
                                   pBuffer4, newGain4,
                                   pBuffer5, newGain5,
                                   pBuffer6, newGain6,
                                   pBuffer7, newGain7,
                                   pBuffer8, newGain8,
                                   pBuffer9, newGain9,
                                   pBuffer10, newGain10,
                                   pBuffer11, newGain11,
                                   pBuffer12, newGain12,
                                   pBuffer13, newGain13,
                                   pBuffer14, newGain14,
                                   pBuffer15, newGain15,
                                   pBuffer16, newGain16,
                                   pBuffer17, newGain17,
                                   pBuffer18, newGain18,
                                   pBuffer19, newGain19,
                                   pBuffer20, newGain20,
                                   pBuffer21, newGain21,
                                   pBuffer22, newGain22,
                                   pBuffer23, newGain23,
                                   pBuffer24, newGain24,
                                   pBuffer25, newGain25,
                                   pBuffer26, newGain26,
                                   pBuffer27, newGain27,
                                   pBuffer28, newGain28,
                                   pBuffer29, newGain29,
                                   pBuffer30, newGain30,
                                   pBuffer31, newGain31,
                                   iBufferSize);
    } else {
        // Set pOutput to all 0s
        SampleUtil::clear(pOutput, iBufferSize);
        for (unsigned int i = 0; i < activeChannels->size(); ++i) {
            EngineMaster::ChannelInfo* pChannelInfo = activeChannels->at(i);
            CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;
            CSAMPLE gain = gainCalculator.getGain(pChannelInfo);
            SampleUtil::addWithGain(pOutput, pBuffer, gain, iBufferSize);
        }
    }
}
void ChannelMixer::mixChannelsRamping(const EngineMaster::GainCalculator& gainCalculator,
                                      EngineMaster::FastVector<EngineMaster::ChannelInfo*, kMaxChannels>* activeChannels,
                                      EngineMaster::FastVector<CSAMPLE, kMaxChannels>* channelGainCache,
                                      CSAMPLE* pOutput,
                                      unsigned int iBufferSize) {
    unsigned int totalActive = activeChannels->size();
    if (totalActive == 0) {
        ScopedTimer t("EngineMaster::mixChannels_0active");
        SampleUtil::clear(pOutput, iBufferSize);
    } else if (totalActive == 1) {
        ScopedTimer t("EngineMaster::mixChannels_1active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        SampleUtil::copy1WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         iBufferSize);
    } else if (totalActive == 2) {
        ScopedTimer t("EngineMaster::mixChannels_2active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        SampleUtil::copy2WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         pBuffer1, oldGain1, newGain1,
                                         iBufferSize);
    } else if (totalActive == 3) {
        ScopedTimer t("EngineMaster::mixChannels_3active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        SampleUtil::copy3WithRampingGain(pOutput,
                                         pBuffer0, oldGain0, newGain0,
                                         pBuffer1, oldGain1, newGain1,
                                         pBuffer2, oldGain2, newGain2,
                                         iBufferSize);
    } else if (totalActive == 4) {
        ScopedTimer t("EngineMaster::mixChannels_4active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
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
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN oldGain23 = (*channelGainCache)[pChannelIndex23];
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
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
    } else if (totalActive == 25) {
        ScopedTimer t("EngineMaster::mixChannels_25active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN oldGain23 = (*channelGainCache)[pChannelIndex23];
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN oldGain24 = (*channelGainCache)[pChannelIndex24];
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        SampleUtil::copy25WithRampingGain(pOutput,
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
                                          pBuffer24, oldGain24, newGain24,
                                          iBufferSize);
    } else if (totalActive == 26) {
        ScopedTimer t("EngineMaster::mixChannels_26active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN oldGain23 = (*channelGainCache)[pChannelIndex23];
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN oldGain24 = (*channelGainCache)[pChannelIndex24];
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN oldGain25 = (*channelGainCache)[pChannelIndex25];
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        SampleUtil::copy26WithRampingGain(pOutput,
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
                                          pBuffer24, oldGain24, newGain24,
                                          pBuffer25, oldGain25, newGain25,
                                          iBufferSize);
    } else if (totalActive == 27) {
        ScopedTimer t("EngineMaster::mixChannels_27active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN oldGain23 = (*channelGainCache)[pChannelIndex23];
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN oldGain24 = (*channelGainCache)[pChannelIndex24];
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN oldGain25 = (*channelGainCache)[pChannelIndex25];
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN oldGain26 = (*channelGainCache)[pChannelIndex26];
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        SampleUtil::copy27WithRampingGain(pOutput,
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
                                          pBuffer24, oldGain24, newGain24,
                                          pBuffer25, oldGain25, newGain25,
                                          pBuffer26, oldGain26, newGain26,
                                          iBufferSize);
    } else if (totalActive == 28) {
        ScopedTimer t("EngineMaster::mixChannels_28active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN oldGain23 = (*channelGainCache)[pChannelIndex23];
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN oldGain24 = (*channelGainCache)[pChannelIndex24];
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN oldGain25 = (*channelGainCache)[pChannelIndex25];
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN oldGain26 = (*channelGainCache)[pChannelIndex26];
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int pChannelIndex27 = pChannel27->m_index;
        CSAMPLE_GAIN oldGain27 = (*channelGainCache)[pChannelIndex27];
        CSAMPLE_GAIN newGain27 = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain27;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        SampleUtil::copy28WithRampingGain(pOutput,
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
                                          pBuffer24, oldGain24, newGain24,
                                          pBuffer25, oldGain25, newGain25,
                                          pBuffer26, oldGain26, newGain26,
                                          pBuffer27, oldGain27, newGain27,
                                          iBufferSize);
    } else if (totalActive == 29) {
        ScopedTimer t("EngineMaster::mixChannels_29active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN oldGain23 = (*channelGainCache)[pChannelIndex23];
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN oldGain24 = (*channelGainCache)[pChannelIndex24];
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN oldGain25 = (*channelGainCache)[pChannelIndex25];
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN oldGain26 = (*channelGainCache)[pChannelIndex26];
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int pChannelIndex27 = pChannel27->m_index;
        CSAMPLE_GAIN oldGain27 = (*channelGainCache)[pChannelIndex27];
        CSAMPLE_GAIN newGain27 = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain27;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int pChannelIndex28 = pChannel28->m_index;
        CSAMPLE_GAIN oldGain28 = (*channelGainCache)[pChannelIndex28];
        CSAMPLE_GAIN newGain28 = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain28;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        SampleUtil::copy29WithRampingGain(pOutput,
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
                                          pBuffer24, oldGain24, newGain24,
                                          pBuffer25, oldGain25, newGain25,
                                          pBuffer26, oldGain26, newGain26,
                                          pBuffer27, oldGain27, newGain27,
                                          pBuffer28, oldGain28, newGain28,
                                          iBufferSize);
    } else if (totalActive == 30) {
        ScopedTimer t("EngineMaster::mixChannels_30active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN oldGain23 = (*channelGainCache)[pChannelIndex23];
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN oldGain24 = (*channelGainCache)[pChannelIndex24];
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN oldGain25 = (*channelGainCache)[pChannelIndex25];
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN oldGain26 = (*channelGainCache)[pChannelIndex26];
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int pChannelIndex27 = pChannel27->m_index;
        CSAMPLE_GAIN oldGain27 = (*channelGainCache)[pChannelIndex27];
        CSAMPLE_GAIN newGain27 = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain27;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int pChannelIndex28 = pChannel28->m_index;
        CSAMPLE_GAIN oldGain28 = (*channelGainCache)[pChannelIndex28];
        CSAMPLE_GAIN newGain28 = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain28;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int pChannelIndex29 = pChannel29->m_index;
        CSAMPLE_GAIN oldGain29 = (*channelGainCache)[pChannelIndex29];
        CSAMPLE_GAIN newGain29 = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain29;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        SampleUtil::copy30WithRampingGain(pOutput,
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
                                          pBuffer24, oldGain24, newGain24,
                                          pBuffer25, oldGain25, newGain25,
                                          pBuffer26, oldGain26, newGain26,
                                          pBuffer27, oldGain27, newGain27,
                                          pBuffer28, oldGain28, newGain28,
                                          pBuffer29, oldGain29, newGain29,
                                          iBufferSize);
    } else if (totalActive == 31) {
        ScopedTimer t("EngineMaster::mixChannels_31active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN oldGain23 = (*channelGainCache)[pChannelIndex23];
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN oldGain24 = (*channelGainCache)[pChannelIndex24];
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN oldGain25 = (*channelGainCache)[pChannelIndex25];
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN oldGain26 = (*channelGainCache)[pChannelIndex26];
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int pChannelIndex27 = pChannel27->m_index;
        CSAMPLE_GAIN oldGain27 = (*channelGainCache)[pChannelIndex27];
        CSAMPLE_GAIN newGain27 = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain27;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int pChannelIndex28 = pChannel28->m_index;
        CSAMPLE_GAIN oldGain28 = (*channelGainCache)[pChannelIndex28];
        CSAMPLE_GAIN newGain28 = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain28;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int pChannelIndex29 = pChannel29->m_index;
        CSAMPLE_GAIN oldGain29 = (*channelGainCache)[pChannelIndex29];
        CSAMPLE_GAIN newGain29 = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain29;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int pChannelIndex30 = pChannel30->m_index;
        CSAMPLE_GAIN oldGain30 = (*channelGainCache)[pChannelIndex30];
        CSAMPLE_GAIN newGain30 = gainCalculator.getGain(pChannel30);
        (*channelGainCache)[pChannelIndex30] = newGain30;
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        SampleUtil::copy31WithRampingGain(pOutput,
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
                                          pBuffer24, oldGain24, newGain24,
                                          pBuffer25, oldGain25, newGain25,
                                          pBuffer26, oldGain26, newGain26,
                                          pBuffer27, oldGain27, newGain27,
                                          pBuffer28, oldGain28, newGain28,
                                          pBuffer29, oldGain29, newGain29,
                                          pBuffer30, oldGain30, newGain30,
                                          iBufferSize);
    } else if (totalActive == 32) {
        ScopedTimer t("EngineMaster::mixChannels_32active");
        EngineMaster::ChannelInfo* pChannel0 = activeChannels->at(0);
        const int pChannelIndex0 = pChannel0->m_index;
        CSAMPLE_GAIN oldGain0 = (*channelGainCache)[pChannelIndex0];
        CSAMPLE_GAIN newGain0 = gainCalculator.getGain(pChannel0);
        (*channelGainCache)[pChannelIndex0] = newGain0;
        CSAMPLE* pBuffer0 = pChannel0->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel1 = activeChannels->at(1);
        const int pChannelIndex1 = pChannel1->m_index;
        CSAMPLE_GAIN oldGain1 = (*channelGainCache)[pChannelIndex1];
        CSAMPLE_GAIN newGain1 = gainCalculator.getGain(pChannel1);
        (*channelGainCache)[pChannelIndex1] = newGain1;
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel2 = activeChannels->at(2);
        const int pChannelIndex2 = pChannel2->m_index;
        CSAMPLE_GAIN oldGain2 = (*channelGainCache)[pChannelIndex2];
        CSAMPLE_GAIN newGain2 = gainCalculator.getGain(pChannel2);
        (*channelGainCache)[pChannelIndex2] = newGain2;
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel3 = activeChannels->at(3);
        const int pChannelIndex3 = pChannel3->m_index;
        CSAMPLE_GAIN oldGain3 = (*channelGainCache)[pChannelIndex3];
        CSAMPLE_GAIN newGain3 = gainCalculator.getGain(pChannel3);
        (*channelGainCache)[pChannelIndex3] = newGain3;
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel4 = activeChannels->at(4);
        const int pChannelIndex4 = pChannel4->m_index;
        CSAMPLE_GAIN oldGain4 = (*channelGainCache)[pChannelIndex4];
        CSAMPLE_GAIN newGain4 = gainCalculator.getGain(pChannel4);
        (*channelGainCache)[pChannelIndex4] = newGain4;
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel5 = activeChannels->at(5);
        const int pChannelIndex5 = pChannel5->m_index;
        CSAMPLE_GAIN oldGain5 = (*channelGainCache)[pChannelIndex5];
        CSAMPLE_GAIN newGain5 = gainCalculator.getGain(pChannel5);
        (*channelGainCache)[pChannelIndex5] = newGain5;
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel6 = activeChannels->at(6);
        const int pChannelIndex6 = pChannel6->m_index;
        CSAMPLE_GAIN oldGain6 = (*channelGainCache)[pChannelIndex6];
        CSAMPLE_GAIN newGain6 = gainCalculator.getGain(pChannel6);
        (*channelGainCache)[pChannelIndex6] = newGain6;
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel7 = activeChannels->at(7);
        const int pChannelIndex7 = pChannel7->m_index;
        CSAMPLE_GAIN oldGain7 = (*channelGainCache)[pChannelIndex7];
        CSAMPLE_GAIN newGain7 = gainCalculator.getGain(pChannel7);
        (*channelGainCache)[pChannelIndex7] = newGain7;
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel8 = activeChannels->at(8);
        const int pChannelIndex8 = pChannel8->m_index;
        CSAMPLE_GAIN oldGain8 = (*channelGainCache)[pChannelIndex8];
        CSAMPLE_GAIN newGain8 = gainCalculator.getGain(pChannel8);
        (*channelGainCache)[pChannelIndex8] = newGain8;
        CSAMPLE* pBuffer8 = pChannel8->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel9 = activeChannels->at(9);
        const int pChannelIndex9 = pChannel9->m_index;
        CSAMPLE_GAIN oldGain9 = (*channelGainCache)[pChannelIndex9];
        CSAMPLE_GAIN newGain9 = gainCalculator.getGain(pChannel9);
        (*channelGainCache)[pChannelIndex9] = newGain9;
        CSAMPLE* pBuffer9 = pChannel9->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel10 = activeChannels->at(10);
        const int pChannelIndex10 = pChannel10->m_index;
        CSAMPLE_GAIN oldGain10 = (*channelGainCache)[pChannelIndex10];
        CSAMPLE_GAIN newGain10 = gainCalculator.getGain(pChannel10);
        (*channelGainCache)[pChannelIndex10] = newGain10;
        CSAMPLE* pBuffer10 = pChannel10->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel11 = activeChannels->at(11);
        const int pChannelIndex11 = pChannel11->m_index;
        CSAMPLE_GAIN oldGain11 = (*channelGainCache)[pChannelIndex11];
        CSAMPLE_GAIN newGain11 = gainCalculator.getGain(pChannel11);
        (*channelGainCache)[pChannelIndex11] = newGain11;
        CSAMPLE* pBuffer11 = pChannel11->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel12 = activeChannels->at(12);
        const int pChannelIndex12 = pChannel12->m_index;
        CSAMPLE_GAIN oldGain12 = (*channelGainCache)[pChannelIndex12];
        CSAMPLE_GAIN newGain12 = gainCalculator.getGain(pChannel12);
        (*channelGainCache)[pChannelIndex12] = newGain12;
        CSAMPLE* pBuffer12 = pChannel12->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel13 = activeChannels->at(13);
        const int pChannelIndex13 = pChannel13->m_index;
        CSAMPLE_GAIN oldGain13 = (*channelGainCache)[pChannelIndex13];
        CSAMPLE_GAIN newGain13 = gainCalculator.getGain(pChannel13);
        (*channelGainCache)[pChannelIndex13] = newGain13;
        CSAMPLE* pBuffer13 = pChannel13->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel14 = activeChannels->at(14);
        const int pChannelIndex14 = pChannel14->m_index;
        CSAMPLE_GAIN oldGain14 = (*channelGainCache)[pChannelIndex14];
        CSAMPLE_GAIN newGain14 = gainCalculator.getGain(pChannel14);
        (*channelGainCache)[pChannelIndex14] = newGain14;
        CSAMPLE* pBuffer14 = pChannel14->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel15 = activeChannels->at(15);
        const int pChannelIndex15 = pChannel15->m_index;
        CSAMPLE_GAIN oldGain15 = (*channelGainCache)[pChannelIndex15];
        CSAMPLE_GAIN newGain15 = gainCalculator.getGain(pChannel15);
        (*channelGainCache)[pChannelIndex15] = newGain15;
        CSAMPLE* pBuffer15 = pChannel15->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel16 = activeChannels->at(16);
        const int pChannelIndex16 = pChannel16->m_index;
        CSAMPLE_GAIN oldGain16 = (*channelGainCache)[pChannelIndex16];
        CSAMPLE_GAIN newGain16 = gainCalculator.getGain(pChannel16);
        (*channelGainCache)[pChannelIndex16] = newGain16;
        CSAMPLE* pBuffer16 = pChannel16->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel17 = activeChannels->at(17);
        const int pChannelIndex17 = pChannel17->m_index;
        CSAMPLE_GAIN oldGain17 = (*channelGainCache)[pChannelIndex17];
        CSAMPLE_GAIN newGain17 = gainCalculator.getGain(pChannel17);
        (*channelGainCache)[pChannelIndex17] = newGain17;
        CSAMPLE* pBuffer17 = pChannel17->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel18 = activeChannels->at(18);
        const int pChannelIndex18 = pChannel18->m_index;
        CSAMPLE_GAIN oldGain18 = (*channelGainCache)[pChannelIndex18];
        CSAMPLE_GAIN newGain18 = gainCalculator.getGain(pChannel18);
        (*channelGainCache)[pChannelIndex18] = newGain18;
        CSAMPLE* pBuffer18 = pChannel18->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel19 = activeChannels->at(19);
        const int pChannelIndex19 = pChannel19->m_index;
        CSAMPLE_GAIN oldGain19 = (*channelGainCache)[pChannelIndex19];
        CSAMPLE_GAIN newGain19 = gainCalculator.getGain(pChannel19);
        (*channelGainCache)[pChannelIndex19] = newGain19;
        CSAMPLE* pBuffer19 = pChannel19->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel20 = activeChannels->at(20);
        const int pChannelIndex20 = pChannel20->m_index;
        CSAMPLE_GAIN oldGain20 = (*channelGainCache)[pChannelIndex20];
        CSAMPLE_GAIN newGain20 = gainCalculator.getGain(pChannel20);
        (*channelGainCache)[pChannelIndex20] = newGain20;
        CSAMPLE* pBuffer20 = pChannel20->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel21 = activeChannels->at(21);
        const int pChannelIndex21 = pChannel21->m_index;
        CSAMPLE_GAIN oldGain21 = (*channelGainCache)[pChannelIndex21];
        CSAMPLE_GAIN newGain21 = gainCalculator.getGain(pChannel21);
        (*channelGainCache)[pChannelIndex21] = newGain21;
        CSAMPLE* pBuffer21 = pChannel21->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel22 = activeChannels->at(22);
        const int pChannelIndex22 = pChannel22->m_index;
        CSAMPLE_GAIN oldGain22 = (*channelGainCache)[pChannelIndex22];
        CSAMPLE_GAIN newGain22 = gainCalculator.getGain(pChannel22);
        (*channelGainCache)[pChannelIndex22] = newGain22;
        CSAMPLE* pBuffer22 = pChannel22->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel23 = activeChannels->at(23);
        const int pChannelIndex23 = pChannel23->m_index;
        CSAMPLE_GAIN oldGain23 = (*channelGainCache)[pChannelIndex23];
        CSAMPLE_GAIN newGain23 = gainCalculator.getGain(pChannel23);
        (*channelGainCache)[pChannelIndex23] = newGain23;
        CSAMPLE* pBuffer23 = pChannel23->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel24 = activeChannels->at(24);
        const int pChannelIndex24 = pChannel24->m_index;
        CSAMPLE_GAIN oldGain24 = (*channelGainCache)[pChannelIndex24];
        CSAMPLE_GAIN newGain24 = gainCalculator.getGain(pChannel24);
        (*channelGainCache)[pChannelIndex24] = newGain24;
        CSAMPLE* pBuffer24 = pChannel24->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel25 = activeChannels->at(25);
        const int pChannelIndex25 = pChannel25->m_index;
        CSAMPLE_GAIN oldGain25 = (*channelGainCache)[pChannelIndex25];
        CSAMPLE_GAIN newGain25 = gainCalculator.getGain(pChannel25);
        (*channelGainCache)[pChannelIndex25] = newGain25;
        CSAMPLE* pBuffer25 = pChannel25->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel26 = activeChannels->at(26);
        const int pChannelIndex26 = pChannel26->m_index;
        CSAMPLE_GAIN oldGain26 = (*channelGainCache)[pChannelIndex26];
        CSAMPLE_GAIN newGain26 = gainCalculator.getGain(pChannel26);
        (*channelGainCache)[pChannelIndex26] = newGain26;
        CSAMPLE* pBuffer26 = pChannel26->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel27 = activeChannels->at(27);
        const int pChannelIndex27 = pChannel27->m_index;
        CSAMPLE_GAIN oldGain27 = (*channelGainCache)[pChannelIndex27];
        CSAMPLE_GAIN newGain27 = gainCalculator.getGain(pChannel27);
        (*channelGainCache)[pChannelIndex27] = newGain27;
        CSAMPLE* pBuffer27 = pChannel27->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel28 = activeChannels->at(28);
        const int pChannelIndex28 = pChannel28->m_index;
        CSAMPLE_GAIN oldGain28 = (*channelGainCache)[pChannelIndex28];
        CSAMPLE_GAIN newGain28 = gainCalculator.getGain(pChannel28);
        (*channelGainCache)[pChannelIndex28] = newGain28;
        CSAMPLE* pBuffer28 = pChannel28->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel29 = activeChannels->at(29);
        const int pChannelIndex29 = pChannel29->m_index;
        CSAMPLE_GAIN oldGain29 = (*channelGainCache)[pChannelIndex29];
        CSAMPLE_GAIN newGain29 = gainCalculator.getGain(pChannel29);
        (*channelGainCache)[pChannelIndex29] = newGain29;
        CSAMPLE* pBuffer29 = pChannel29->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel30 = activeChannels->at(30);
        const int pChannelIndex30 = pChannel30->m_index;
        CSAMPLE_GAIN oldGain30 = (*channelGainCache)[pChannelIndex30];
        CSAMPLE_GAIN newGain30 = gainCalculator.getGain(pChannel30);
        (*channelGainCache)[pChannelIndex30] = newGain30;
        CSAMPLE* pBuffer30 = pChannel30->m_pBuffer;
        EngineMaster::ChannelInfo* pChannel31 = activeChannels->at(31);
        const int pChannelIndex31 = pChannel31->m_index;
        CSAMPLE_GAIN oldGain31 = (*channelGainCache)[pChannelIndex31];
        CSAMPLE_GAIN newGain31 = gainCalculator.getGain(pChannel31);
        (*channelGainCache)[pChannelIndex31] = newGain31;
        CSAMPLE* pBuffer31 = pChannel31->m_pBuffer;
        SampleUtil::copy32WithRampingGain(pOutput,
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
                                          pBuffer24, oldGain24, newGain24,
                                          pBuffer25, oldGain25, newGain25,
                                          pBuffer26, oldGain26, newGain26,
                                          pBuffer27, oldGain27, newGain27,
                                          pBuffer28, oldGain28, newGain28,
                                          pBuffer29, oldGain29, newGain29,
                                          pBuffer30, oldGain30, newGain30,
                                          pBuffer31, oldGain31, newGain31,
                                          iBufferSize);
    } else {
        // Set pOutput to all 0s
        SampleUtil::clear(pOutput, iBufferSize);
        for (unsigned int i = 0; i < activeChannels->size(); ++i) {
            EngineMaster::ChannelInfo* pChannelInfo = activeChannels->at(i);
            CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;
            CSAMPLE gain = gainCalculator.getGain(pChannelInfo);
            SampleUtil::addWithGain(pOutput, pBuffer, gain, iBufferSize);
        }
    }
}
