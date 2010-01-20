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
#include "enginemaster.h"
#include "enginevolume.h"
#include "enginechannel.h"
#include "engineclipping.h"
#include "engineflanger.h"
#include "enginevumeter.h"
#include "enginexfader.h"
#include "enginesidechain.h"
#include "sampleutil.h"

#ifdef __LADSPA__
#include "engineladspa.h"
#endif


EngineMaster::EngineMaster(ConfigObject<ConfigValue> * _config,
                           const char * group) {

    // Flanger
    flanger = new EngineFlanger("[Flanger]");

#ifdef __LADSPA__
    // LADSPA
    ladspa = new EngineLADSPA();
#endif

    // Crossfader
    crossfader = new ControlPotmeter(ConfigKey(group, "crossfader"),-1.,1.);

    // Transform buttons

    // TODO(XXX) These aren't used anymore. Created only for backwards
    // compatibility.
    new ControlPushButton(ConfigKey("[Channel1]", "transform"));
    new ControlPushButton(ConfigKey("[Channel2]", "transform"));

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

    flanger1 = flanger->getButtonCh1();
    flanger2 = flanger->getButtonCh2();

    Q_ASSERT(flanger1);
    Q_ASSERT(flanger2);

    // Allocate buffers
    m_pHead = new CSAMPLE[MAX_BUFFER_LEN];
    m_pMaster = new CSAMPLE[MAX_BUFFER_LEN];

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

    delete [] m_pHead;
    delete [] m_pMaster;

    QMutableListIterator<CSAMPLE*> buffer_it(m_channelBuffers);
    while (buffer_it.hasNext()) {
        CSAMPLE* buffer = buffer_it.next();
        buffer_it.remove();
        delete [] buffer;
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
    // TODO(XXX) re-enable
    //buffer1->setPitchIndpTimeStretch(b);
    //buffer2->setPitchIndpTimeStretch(b);
}

const CSAMPLE* EngineMaster::getMasterBuffer()
{
    return m_pMaster;
}

const CSAMPLE* EngineMaster::getHeadphoneBuffer()
{
    return m_pHead;
}

void EngineMaster::process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize)
{
    CSAMPLE **pOutput = (CSAMPLE**)pOut;

    // Prepare each channel for output
    // TODO(XXX) per-channel flanger

    QListIterator<EngineChannel*> channel_iter(m_channels);
    QList<CSAMPLE*> pflChannels;
    QList<QPair<CSAMPLE*, EngineChannel::ChannelOrientation> > masterChannels;
    int channel_number = 0;
    while (channel_iter.hasNext()) {
        EngineChannel* channel = channel_iter.next();
        CSAMPLE* buffer = m_channelBuffers[channel_number];
        channel->process(NULL, buffer, iBufferSize);
        if (channel->isPFL()) {
            pflChannels.push_back(buffer);
        }
        masterChannels.push_back(QPair<CSAMPLE*, EngineChannel::ChannelOrientation>(buffer, channel->getOrientation()));
        channel_number++;
    }

    // Perform the master mix.
    // TODO(XXX) support channel orientations

    // Crossfader and Transform buttons
    //set gain levels;
    float c1_gain, c2_gain;
    // TODO(XXX) replace get()'s with valueChanged slots
    EngineXfader::getXfadeGains(c1_gain, c2_gain,
                                crossfader->get(), xFaderCurve->get(),
                                xFaderCalibration->get());

    if (masterChannels.size() == 0) {
        memset(m_pMaster, 0, sizeof(m_pMaster[0]) * iBufferSize);
    } else if (masterChannels.size() == 1) {
        QPair<CSAMPLE*, EngineChannel::ChannelOrientation>& channel =
                masterChannels[0];
        CSAMPLE* buffer = channel.first;
        EngineChannel::ChannelOrientation orientation = channel.second;
        memcpy(m_pMaster, buffer, sizeof(m_pMaster[0]) * iBufferSize);
        // Apply gain
        double gain = gainForOrientation(orientation, c1_gain, 1.0f, c2_gain);
        SampleUtil::applyGain(m_pMaster, gain, iBufferSize);
        // if (gain != 1.0f) {
        //     for (int i = 0; i < iBufferSize; ++i) {
        //         m_pMaster[i] *= gain;
        //     }
        // }
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
        // for (int i = 0; i < iBufferSize; ++i) {
        //     m_pMaster[i] = gain1 * buffer1[i] + gain2 * buffer2[i];
        // }
    } else {
        QListIterator<QPair<CSAMPLE*, EngineChannel::ChannelOrientation> > master_iter(masterChannels);
        memset(m_pMaster, 0, sizeof(m_pMaster[0]) * iBufferSize);
        while(master_iter.hasNext()) {
            QPair<CSAMPLE*, EngineChannel::ChannelOrientation> channel =
                    master_iter.next();
            CSAMPLE* buffer = channel.first;
            EngineChannel::ChannelOrientation orientation = channel.second;
            double gain = gainForOrientation(orientation, c1_gain, 1.0f, c2_gain);
            SampleUtil::addWithGain(m_pMaster, buffer, gain, iBufferSize);
        }

        // for (int i = 0; i < iBufferSize; ++i) {
        //     master_iter.toFront();
        //     while(master_iter.hasNext()) {
        //         QPair<CSAMPLE*, EngineChannel::ChannelOrientation> channel =
        //                 master_iter.next();
        //         CSAMPLE* buffer = channel.first;
        //         EngineChannel::ChannelOrientation orientation = channel.second;
        //         double gain = gainForOrientation(orientation, c1_gain, 1.0f, c2_gain);
        //         m_pMaster[i] += buffer[i] * gain;

        //     }
        // }
    }

    // Master volume
    volume->process(m_pMaster, m_pMaster, iBufferSize);

#ifdef __LADSPA__
    // LADPSA master effects
    ladspa->process(m_pMaster, m_pMaster, iBufferSize);
#endif

    // Process the flanger on master if flanger is enabled on both channels
    //if (flanger1->get()==1. && flanger2->get()==1.)
    //    flanger->process(m_pMaster, m_pMaster, iBufferSize);

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

    //Perform balancing on main out
    SampleUtil::applyAlternatingGain(m_pMaster, balleft, balright, iBufferSize);
    // for (int i = 0; i < iBufferSize; i += 2) {
    //     m_pMaster[i] = m_pMaster[i]*balleft;
    //     m_pMaster[i+1] = m_pMaster[i+1]*balright;
    // }

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
    // for (int i = 0; i < iBufferSize; ++i) {
    //     m_pHead[i] = m_pMaster[i]*cmaster_gain;
    // }
    SampleUtil::copyWithGain(m_pHead, m_pMaster, cmaster_gain, iBufferSize);

    if (pflChannels.size() == 0) {
        // Do nothing
    } else if (pflChannels.size() == 1) {
        // Apply gain TODO(XXX) SSE
        CSAMPLE* buffer = pflChannels[0];
        SampleUtil::addWithGain(m_pHead, buffer, chead_gain, iBufferSize);
        // for (int i = 0; i < iBufferSize; ++i) {
        //     m_pHead[i] += buffer[i]*chead_gain;
        // }
    } else if (pflChannels.size() == 2) {
        CSAMPLE* buffer1 = pflChannels[0];
        CSAMPLE* buffer2 = pflChannels[1];
        SampleUtil::add2WithGain(m_pHead,
                                 buffer1, chead_gain,
                                 buffer2, chead_gain,
                                 iBufferSize);
        // for (int i = 0; i < iBufferSize; ++i) {
        //     m_pHead[i] += buffer1[i]*chead_gain + buffer2[i]*chead_gain;
        // }
    } else {
        QListIterator<CSAMPLE*> pfl_iter(pflChannels);
        while(pfl_iter.hasNext()) {
            CSAMPLE* buffer = pfl_iter.next();
            SampleUtil::addWithGain(m_pHead, buffer, chead_gain, iBufferSize);
        }
        // for (int i = 0; i < iBufferSize; ++i) {
        //     pfl_iter.toFront();
        //     while(pfl_iter.hasNext()) {
        //         CSAMPLE* buffer = pfl_iter.next();
        //         m_pHead[i] += buffer[i];
        //     }
        // }
    }

    // Head volume and clipping
    head_volume->process(m_pHead, m_pHead, iBufferSize);
    head_clipping->process(m_pHead, m_pHead, iBufferSize);

    //Master/headphones interleaving is now done in
    //SoundManager::requestBuffer() - Albert Nov 18/07
}

void EngineMaster::addChannel(EngineChannel* pChannel) {
    m_channelBuffers.push_back(new CSAMPLE[MAX_BUFFER_LEN]);
    m_channels.push_back(pChannel);
}

int EngineMaster::numChannels() {
    return m_channels.size();
}

const CSAMPLE* EngineMaster::getChannelBuffer(int i) {
    if (i >= 0 && i < numChannels()) {
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
