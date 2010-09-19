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
#include "controlpotmeter.h"
#include "enginebuffer.h"
#include "enginemaster.h"
#include "engine/engineworkerscheduler.h"
#include "enginebuffer.h"
#include "enginevolume.h"
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
    volume = new EngineVolume(ConfigKey(group,"volume"), 5.);

    // Clipping
    clipping = new EngineClipping(group);

    // VU meter:
    vumeter = new EngineVuMeter(group);

    // Headphone volume
    head_volume = new EngineVolume(ConfigKey(group, "headVolume"), 5.);

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
    delete volume;
    delete head_volume;
    delete clipping;
    delete head_clipping;
    delete sidechain;

    SampleUtil::free(m_pHead);
    SampleUtil::free(m_pMaster);

    QMutableListIterator<CSAMPLE*> buffer_it(m_channelBuffers);
    while (buffer_it.hasNext()) {
        CSAMPLE* buffer = buffer_it.next();
        buffer_it.remove();
        SampleUtil::free(buffer);
    }

    QMutableListIterator<EngineChannel*> channel_it(m_channels);
    while (channel_it.hasNext()) {
        EngineChannel* channel = channel_it.next();
        channel_it.remove();
        delete channel;
    }

}

void EngineMaster::setPitchIndpTimeStretch(bool b)
{
    QListIterator<EngineChannel*> channel_iter(m_channels);

    while (channel_iter.hasNext()) {
        EngineChannel* pChannel = channel_iter.next();
        pChannel->setPitchIndpTimeStretch(b);
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

void EngineMaster::process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize)
{
    CSAMPLE **pOutput = (CSAMPLE**)pOut;

    // Prepare each channel for output

    QList<CSAMPLE*> pflChannels;
    QList<QPair<CSAMPLE*, EngineChannel::ChannelOrientation> > masterChannels;

    for (int channel_number = 0; channel_number < m_channels.size(); ++channel_number) {
        EngineChannel* channel = m_channels[channel_number];
        CSAMPLE* buffer = m_channelBuffers[channel_number];
        channel->process(NULL, buffer, iBufferSize);

        // If the channel is enabled for previewing in headphones, add it to a
        // list of headphone channels.
        if (channel->isPFL()) {
            pflChannels.push_back(buffer);
        }

        // Add the channel to the list of master output channels.
        masterChannels.push_back(
            QPair<CSAMPLE*, EngineChannel::ChannelOrientation>(
                buffer, channel->getOrientation()));
    }

    // Perform the master mix.

    // Crossfader and Transform buttons
    //set gain levels;
    float c1_gain, c2_gain;
    EngineXfader::getXfadeGains(c1_gain, c2_gain,
                                crossfader->get(), xFaderCurve->get(),
                                xFaderCalibration->get());

    if (masterChannels.size() == 0) {
        SampleUtil::applyGain(m_pMaster, 0.0f, iBufferSize);
    } else if (masterChannels.size() == 1) {
        QPair<CSAMPLE*, EngineChannel::ChannelOrientation>& channel =
                masterChannels[0];
        CSAMPLE* buffer = channel.first;
        EngineChannel::ChannelOrientation orientation = channel.second;

        // Apply gain
        double gain = gainForOrientation(orientation, c1_gain, 1.0f, c2_gain);
        SampleUtil::copyWithGain(m_pMaster, buffer, gain, iBufferSize);
    } else if (masterChannels.size() == 2) {
        QPair<CSAMPLE*, EngineChannel::ChannelOrientation> channel1 =
                masterChannels[0];
        QPair<CSAMPLE*, EngineChannel::ChannelOrientation> channel2 =
                masterChannels[1];
        CSAMPLE* buffer1 = channel1.first;
        CSAMPLE* buffer2 = channel2.first;
        EngineChannel::ChannelOrientation orientation1 = channel1.second;
        EngineChannel::ChannelOrientation orientation2 = channel2.second;
        double gain1 = gainForOrientation(orientation1, c1_gain, 1.0f, c2_gain);
        double gain2 = gainForOrientation(orientation2, c1_gain, 1.0f, c2_gain);

        SampleUtil::copy2WithGain(m_pMaster,
                                  buffer1, gain1,
                                  buffer2, gain2,
                                  iBufferSize);
    } else {
        // Set m_pMaster to all 0s
        SampleUtil::applyGain(m_pMaster, 0.0f, iBufferSize);

        for (int i = 0; i < masterChannels.size(); ++i) {
            QPair<CSAMPLE*, EngineChannel::ChannelOrientation> channel =
                    masterChannels[i];
            CSAMPLE* buffer = channel.first;
            EngineChannel::ChannelOrientation orientation = channel.second;
            double gain = gainForOrientation(orientation, c1_gain, 1.0f, c2_gain);
            SampleUtil::addWithGain(m_pMaster, buffer, gain, iBufferSize);
        }
    }

    // Master volume
    volume->process(m_pMaster, m_pMaster, iBufferSize);

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

    // Compute headphone mix

    // Head phone left/right mix
    float cf_val = head_mix->get();
    float chead_gain = 0.5*(-cf_val+1.);
    float cmaster_gain = 0.5*(cf_val+1.);
    // qDebug() << "head val " << cf_val
    //          << ", head " << chead_gain
    //          << ", master " << cmaster_gain;

    // Set headphone to master with appropriate gain
    SampleUtil::copyWithGain(m_pHead, m_pMaster, cmaster_gain, iBufferSize);

    if (pflChannels.size() == 0) {
        // Do nothing
    } else if (pflChannels.size() == 1) {
        // Apply gain
        CSAMPLE* buffer = pflChannels[0];
        SampleUtil::addWithGain(m_pHead, buffer, chead_gain, iBufferSize);
    } else if (pflChannels.size() == 2) {
        CSAMPLE* buffer1 = pflChannels[0];
        CSAMPLE* buffer2 = pflChannels[1];
        SampleUtil::add2WithGain(m_pHead,
                                 buffer1, chead_gain,
                                 buffer2, chead_gain,
                                 iBufferSize);
    } else {
        for (int i = 0; i < pflChannels.size(); ++i) {
            CSAMPLE* buffer = pflChannels[i];
            SampleUtil::addWithGain(m_pHead, buffer, chead_gain, iBufferSize);
        }
    }

    // Head volume and clipping
    head_volume->process(m_pHead, m_pHead, iBufferSize);
    head_clipping->process(m_pHead, m_pHead, iBufferSize);

    //Master/headphones interleaving is now done in
    //SoundManager::requestBuffer() - Albert Nov 18/07

    // We're close to the end of the callback. Schedule the workers. Hopefully
    // the work thread doesn't get scheduled between now and then.
    m_pWorkerScheduler->runWorkers();
}

void EngineMaster::addChannel(EngineChannel* pChannel) {
    CSAMPLE* pChannelBuffer = SampleUtil::alloc(MAX_BUFFER_LEN);
    memset(pChannelBuffer, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
    m_channelBuffers.push_back(pChannelBuffer);
    m_channels.push_back(pChannel);
    pChannel->getEngineBuffer()->bindWorkers(m_pWorkerScheduler);

    // TODO(XXX) WARNING HUGE HACK ALERT In the case of 2-decks, this code hooks
    // the two EngineBuffers together so they can beat-sync off of each other.
    // rryan 6/2010
    if (m_channels.length() == 2) {
        EngineBuffer *pBuffer1 = m_channels[0]->getEngineBuffer();
        EngineBuffer *pBuffer2 = m_channels[1]->getEngineBuffer();
        pBuffer1->setOtherEngineBuffer(pBuffer2);
        pBuffer2->setOtherEngineBuffer(pBuffer1);
    }
}

int EngineMaster::numChannels() const {
    return m_channels.size();
}

const CSAMPLE* EngineMaster::getChannelBuffer(unsigned int i) const {
    if (i < numChannels()) {
        return m_channelBuffers[i];
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
