/***************************************************************************
                          enginemaster.cpp  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QDebug>
#include <QList>
#include <QPair>

#include "controlpushbutton.h"
#include "configobject.h"
#include "controllogpotmeter.h"
#include "controlpotmeter.h"
#include "enginebuffer.h"
#include "enginemaster.h"
#include "engine/engineworkerscheduler.h"
#include "enginebuffer.h"
#include "enginechannel.h"
#include "engineclipping.h"
#include "enginevumeter.h"
#include "enginexfader.h"
#include "enginesidechain.h"
#include "sampleutil.h"

#ifdef __LADSPA__
#include "engineladspa.h"
#endif


EngineMaster::EngineMaster(ConfigObject<ConfigValue> * _config,
                           const char * group) {

    m_pWorkerScheduler = new EngineWorkerScheduler(this);

    // Master sample rate
    ControlObject * sr = new ControlObject(ConfigKey(group, "samplerate"));
    sr->set(44100.);

    // Latency control
    new ControlObject(ConfigKey(group, "latency"));

    // Master rate
    new ControlPotmeter(ConfigKey(group, "rate"), -1.0, 1.0);

#ifdef __LADSPA__
    // LADSPA
    ladspa = new EngineLADSPA();
#endif

    // Crossfader
    crossfader = new ControlPotmeter(ConfigKey(group, "crossfader"),-1.,1.);

    // Balance
    m_pBalance = new ControlPotmeter(ConfigKey(group, "balance"), -1., 1.);

    // Master volume
    m_pMasterVolume = new ControlLogpotmeter(ConfigKey(group, "volume"), 5.);

    // Clipping
    clipping = new EngineClipping(group);

    // VU meter:
    vumeter = new EngineVuMeter(group);

    // Headphone volume
    m_pHeadVolume = new ControlLogpotmeter(ConfigKey(group, "headVolume"), 5.);

    // Headphone mix (left/right)
    head_mix = new ControlPotmeter(ConfigKey(group, "headMix"),-1.,1.);
    head_mix->set(-1.);

    // Headphone Clipping
    head_clipping = new EngineClipping("");

    // Allocate buffers
    m_pHead = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pMaster = SampleUtil::alloc(MAX_BUFFER_LEN);
    memset(m_pHead, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
    memset(m_pMaster, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);

    sidechain = new EngineSideChain(_config);

    //X-Fader Setup
    xFaderCurve = new ControlPotmeter(
        ConfigKey("[Mixer Profile]", "xFaderCurve"), 0., 2.);
    xFaderCalibration = new ControlPotmeter(
        ConfigKey("[Mixer Profile]", "xFaderCalibration"), -2., 2.);
}

EngineMaster::~EngineMaster()
{
    qDebug() << "in ~EngineMaster()";
    delete crossfader;
    delete m_pBalance;
    delete head_mix;
    delete m_pMasterVolume;
    delete m_pHeadVolume;
    delete clipping;
    delete head_clipping;
    delete sidechain;

    SampleUtil::free(m_pHead);
    SampleUtil::free(m_pMaster);

    QMutableListIterator<ChannelInfo*> channel_it(m_channels);
    while (channel_it.hasNext()) {
        ChannelInfo* pChannelInfo = channel_it.next();
        channel_it.remove();
        SampleUtil::free(pChannelInfo->m_pBuffer);
        delete pChannelInfo->m_pChannel;
        delete pChannelInfo->m_pVolumeControl;
        delete pChannelInfo;
    }
}

const CSAMPLE* EngineMaster::getMasterBuffer() const
{
    return m_pMaster;
}

const CSAMPLE* EngineMaster::getHeadphoneBuffer() const
{
    return m_pHead;
}

void EngineMaster::mixChannels(unsigned int channelBitvector, unsigned int maxChannels,
                               CSAMPLE* pOutput, unsigned int iBufferSize) {

    // Crossfader and Transform buttons
    //set gain levels;
    float c1_gain, c2_gain;
    EngineXfader::getXfadeGains(c1_gain, c2_gain,
                                crossfader->get(), xFaderCurve->get(),
                                xFaderCalibration->get());

    // Common case: 2 decks, 4 samplers, 1 mic
    ChannelInfo* pChannel1 = NULL;
    ChannelInfo* pChannel2 = NULL;
    ChannelInfo* pChannel3 = NULL;
    ChannelInfo* pChannel4 = NULL;
    ChannelInfo* pChannel5 = NULL;
    ChannelInfo* pChannel6 = NULL;
    ChannelInfo* pChannel7 = NULL;
    ChannelInfo* pChannel8 = NULL;

    unsigned int totalActive = 0;
    for (unsigned int i = 0; i < maxChannels; ++i) {
        if ((channelBitvector & (1 << i)) == 0) {
            continue;
        }

        ++totalActive;

        if (pChannel1 == NULL) {
            pChannel1 = m_channels[i];
        } else if (pChannel2 == NULL) {
            pChannel2 = m_channels[i];
        } else if (pChannel3 == NULL) {
            pChannel3 = m_channels[i];
        } else if (pChannel4 == NULL) {
            pChannel4 = m_channels[i];
        } else if (pChannel5 == NULL) {
            pChannel5 = m_channels[i];
        } else if (pChannel6 == NULL) {
            pChannel6 = m_channels[i];
        } else if (pChannel7 == NULL) {
            pChannel7 = m_channels[i];
        } else if (pChannel8 == NULL) {
            pChannel8 = m_channels[i];
        }
    }

    if (totalActive == 0) {
        SampleUtil::applyGain(pOutput, 0.0f, iBufferSize);
    } else if (totalActive == 1) {
        CSAMPLE* pBuffer = pChannel1->m_pBuffer;
        EngineChannel::ChannelOrientation orientation = pChannel1->m_pChannel->getOrientation();
        double dVolume = pChannel1->m_pVolumeControl->get();

        // Apply gain
        double gain = gainForOrientation(orientation, c1_gain, 1.0f, c2_gain) * dVolume;
        SampleUtil::copyWithGain(pOutput, pBuffer, gain, iBufferSize);
    } else if (totalActive == 2) {
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        EngineChannel::ChannelOrientation orientation1 = pChannel1->m_pChannel->getOrientation();
        double dVolume1 = pChannel1->m_pVolumeControl->get();

        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        EngineChannel::ChannelOrientation orientation2 = pChannel2->m_pChannel->getOrientation();
        double dVolume2 = pChannel2->m_pVolumeControl->get();

        double gain1 = gainForOrientation(orientation1, c1_gain, 1.0f, c2_gain) * dVolume1;
        double gain2 = gainForOrientation(orientation2, c1_gain, 1.0f, c2_gain) * dVolume2;

        SampleUtil::copy2WithGain(pOutput,
                                  pBuffer1, gain1,
                                  pBuffer2, gain2,
                                  iBufferSize);
    } else {
        // Set pOutput to all 0s
        SampleUtil::applyGain(pOutput, 0.0f, iBufferSize);

        for (unsigned int i = 0; i < maxChannels; ++i) {
            if (channelBitvector & (1 << i)) {
                ChannelInfo* pChannelInfo = m_channels[i];
                CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;
                EngineChannel::ChannelOrientation orientation = pChannelInfo->m_pChannel->getOrientation();
                double dVolume = pChannelInfo->m_pVolumeControl->get();
                double gain = gainForOrientation(orientation, c1_gain, 1.0f, c2_gain) * dVolume;
                SampleUtil::addWithGain(pOutput, pBuffer, gain, iBufferSize);
            }
        }
    }
}

void EngineMaster::process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize)
{
    CSAMPLE **pOutput = (CSAMPLE**)pOut;
    Q_UNUSED(pOutput);

    // Prepare each channel for output

    // Bitvector of enabled channels
    const unsigned int maxChannels = 16;
    unsigned int masterOutput = 0;

    // Compute headphone mix
    // Head phone left/right mix
    float cf_val = head_mix->get();
    float chead_gain = 0.5*(-cf_val+1.);
    float cmaster_gain = 0.5*(cf_val+1.);
    // qDebug() << "head val " << cf_val << ", head " << chead_gain
    //          << ", master " << cmaster_gain;

    // we have to copy PFL channels to the headphone buffer here before we
    // process the master mix, as PFL channels don't have their fader volume
    // applied but the master channels do -- bkgood
    SampleUtil::applyGain(m_pHead, 0.0f, iBufferSize);

    unsigned int totalActive = 0;

    QList<ChannelInfo*>::iterator it = m_channels.begin();
    for (unsigned int channel_number = 0;
         it != m_channels.end(); ++it, ++channel_number) {
        ChannelInfo* pChannelInfo = *it;
        EngineChannel* pChannel = pChannelInfo->m_pChannel;

        if (!pChannel->isActive()) {
            continue;
        }

        totalActive++;
        masterOutput |= (1 << channel_number);

        // Process the buffer
        pChannel->process(NULL, pChannelInfo->m_pBuffer, iBufferSize);

        // If the channel is enabled for previewing in headphones, copy it
        // over to the headphone buffer
        if (pChannel->isPFL()) {
            SampleUtil::addWithGain(m_pHead, pChannelInfo->m_pBuffer, chead_gain, iBufferSize);
        }
    }

    // Perform the master mix.
    mixChannels(masterOutput, maxChannels, m_pMaster, iBufferSize);

    // Master volume
    SampleUtil::applyGain(m_pMaster, m_pMasterVolume->get(), iBufferSize);

#ifdef __LADSPA__
    // LADPSA master effects
    ladspa->process(m_pMaster, m_pMaster, iBufferSize);
#endif

    // Clipping
    clipping->process(m_pMaster, m_pMaster, iBufferSize);

    // Balance values
    float balright = 1.;
    float balleft = 1.;
    float bal = m_pBalance->get();
    if (bal>0.)
        balleft -= bal;
    else if (bal<0.)
        balright += bal;

    // Perform balancing on main out
    SampleUtil::applyAlternatingGain(m_pMaster, balleft, balright, iBufferSize);

    // Update VU meter (it does not return anything). Needs to be here so that
    // master balance is reflected in the VU meter.
    if (vumeter != NULL)
        vumeter->process(m_pMaster, m_pMaster, iBufferSize);

    //Submit master samples to the side chain to do shoutcasting, recording,
    //etc.  (cpu intensive non-realtime tasks)
    sidechain->submitSamples(m_pMaster, iBufferSize);

    // Add master to headphone with appropriate gain
    SampleUtil::addWithGain(m_pHead, m_pMaster, cmaster_gain, iBufferSize);

    // Head volume and clipping
    SampleUtil::applyGain(m_pHead, m_pHeadVolume->get(), iBufferSize);
    head_clipping->process(m_pHead, m_pHead, iBufferSize);

    //Master/headphones interleaving is now done in
    //SoundManager::requestBuffer() - Albert Nov 18/07

    // We're close to the end of the callback. Schedule the workers. Hopefully
    // the work thread doesn't get scheduled between now and then.
    m_pWorkerScheduler->runWorkers();
}

void EngineMaster::addChannel(EngineChannel* pChannel) {
    ChannelInfo* pChannelInfo = new ChannelInfo();
    pChannelInfo->m_pChannel = pChannel;
    pChannelInfo->m_pVolumeControl = new ControlLogpotmeter(
        ConfigKey(pChannel->getGroup(), pChannel->getGroup()), 1.0);
    pChannelInfo->m_pBuffer = SampleUtil::alloc(MAX_BUFFER_LEN);
    memset(pChannelInfo->m_pBuffer, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
    m_channels.push_back(pChannelInfo);
    pChannelInfo->m_pChannel->getEngineBuffer()->bindWorkers(m_pWorkerScheduler);

    // TODO(XXX) WARNING HUGE HACK ALERT In the case of 2-decks, this code hooks
    // the two EngineBuffers together so they can beat-sync off of each other.
    // rryan 6/2010
    if (m_channels.length() == 2) {
        EngineBuffer *pBuffer1 = m_channels[0]->m_pChannel->getEngineBuffer();
        EngineBuffer *pBuffer2 = m_channels[1]->m_pChannel->getEngineBuffer();
        pBuffer1->setOtherEngineBuffer(pBuffer2);
        pBuffer2->setOtherEngineBuffer(pBuffer1);
    }
}

int EngineMaster::numChannels() const {
    return m_channels.size();
}

const CSAMPLE* EngineMaster::getChannelBuffer(unsigned int i) const {
    if (i < numChannels()) {
        return m_channels[i]->m_pBuffer;
    }
    return NULL;
}

// static
double EngineMaster::gainForOrientation(EngineChannel::ChannelOrientation orientation,
                                        double leftGain,
                                        double centerGain,
                                        double rightGain) {
    if (orientation == EngineChannel::LEFT) {
        return leftGain;
    } else if (orientation == EngineChannel::RIGHT) {
        return rightGain;
    }
    return centerGain;
}
