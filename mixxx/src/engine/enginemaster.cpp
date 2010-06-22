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

#include "controlpushbutton.h"
#include "configobject.h"
#include "controlpotmeter.h"
#include "enginemaster.h"
#include "engine/engineworkerscheduler.h"
#include "enginebuffer.h"
#include "enginevolume.h"
#include "enginechannel.h"
#include "engineclipping.h"
#include "engineflanger.h"
#include "enginevumeter.h"
#include "enginevinylsoundemu.h"
#include "enginexfader.h"
#include "enginesidechain.h"
#ifdef __LADSPA__
#include "engineladspa.h"
#endif
#ifdef __VINYLCONTROL__
#include "enginevinylcontrol.h"
#endif
// #include "enginebuffermasterrate.h"
#include <QDebug>

EngineMaster::EngineMaster(ConfigObject<ConfigValue> * _config,
                           EngineBuffer * _buffer1, EngineBuffer * _buffer2,
                           EngineChannel * _channel1, EngineChannel * _channel2,
                           const char * group)
{
    m_pWorkerScheduler = new EngineWorkerScheduler(this);

    buffer1 = _buffer1;
    buffer2 = _buffer2;
    channel1 = _channel1;
    channel2 = _channel2;

    // TODO(XXX) In features_hydra, stick this in addChannel()
    buffer1->bindWorkers(m_pWorkerScheduler);
    buffer2->bindWorkers(m_pWorkerScheduler);

    // Defaults
    master1 = true;
    master2 = true;

    // Flanger
    flanger = new EngineFlanger("[Flanger]");

#ifdef __LADSPA__
    // LADSPA
    ladspa = new EngineLADSPA();
#endif

    // Crossfader
    crossfader = new ControlPotmeter(ConfigKey(group, "crossfader"),-1.,1.);

    // Transform buttons
    transform1 = new ControlPushButton(ConfigKey("[Channel1]", "transform"));
    transform2 = new ControlPushButton(ConfigKey("[Channel2]", "transform"));

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

    // Channel Volume control:
    volume1 = new EngineVolume(ConfigKey("[Channel1]","volume"));
    volume2 = new EngineVolume(ConfigKey("[Channel2]","volume"));

    // Channel VU meter:
    vumeter1 = new EngineVuMeter("[Channel1]");
    vumeter2 = new EngineVuMeter("[Channel2]");

    // Vinyl sound emulation
    vinylsound1 = new EngineVinylSoundEmu(_config, "[Channel1]");
    vinylsound2 = new EngineVinylSoundEmu(_config, "[Channel2]");

    // Mute on active headphone
//     m_pControlObjectHeadphoneMute = new ControlObject(ConfigKey(group,"HeadphoneMute"));

    pfl1 = channel1->getPFL();
    pfl2 = channel2->getPFL();

    flanger1 = flanger->getButtonCh1();
    flanger2 = flanger->getButtonCh2();

    Q_ASSERT(flanger1);
    Q_ASSERT(flanger2);

//     m_pEngineBufferMasterRate = new EngineBufferMasterRate();

    // Allocate buffers
    m_pTemp1 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pTemp2 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pHead = new CSAMPLE[MAX_BUFFER_LEN];
    m_pMaster = new CSAMPLE[MAX_BUFFER_LEN];


    sidechain = new EngineSideChain(_config);

	//X-Fader Setup
	xFaderCurve = new ControlPotmeter(ConfigKey("[Mixer Profile]", "xFaderCurve"), 0., 2.);
	xFaderCalibration = new ControlPotmeter(ConfigKey("[Mixer Profile]", "xFaderCalibration"), -2., 2.);
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
//     delete m_pControlObjectHeadphoneMute;
//     delete m_pEngineBufferMasterRate;
    delete [] m_pTemp1;
    delete [] m_pTemp2;
    delete [] m_pHead;
    delete [] m_pMaster;

}

void EngineMaster::setPitchIndpTimeStretch(bool b)
{
    buffer1->setPitchIndpTimeStretch(b);
    buffer2->setPitchIndpTimeStretch(b);
}

const CSAMPLE* EngineMaster::getMasterBuffer()
{
    return m_pMaster;
}

const CSAMPLE* EngineMaster::getHeadphoneBuffer()
{
    return m_pHead;
}

const CSAMPLE* EngineMaster::getDeckBuffer(unsigned int index) const {
    switch (index) {
    case 0:
        return m_pTemp1;
    case 1:
        return m_pTemp2;
    default:
        qDebug() << "EngineMaster doesn't have deck buffer " << index << "!";
        return NULL; // will likely lead to a segfault if not checked for :) -- bkgood
    }
}

void EngineMaster::process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize)
{
    CSAMPLE **pOutput = (CSAMPLE**)pOut;

    //
    // Process the buffer, the channels and the effects:
    //

    if (master1)
    {
        buffer1->process(0, m_pTemp1, iBufferSize);
        vinylsound1->process(m_pTemp1, m_pTemp1, iBufferSize);
        channel1->process(m_pTemp1, m_pTemp1, iBufferSize);
        if (flanger1->get()==1. && flanger2->get()==0.)
            flanger->process(m_pTemp1, m_pTemp1, iBufferSize);
    }

    if (master2)
    {
        buffer2->process(0, m_pTemp2, iBufferSize);
        vinylsound2->process(m_pTemp2, m_pTemp2, iBufferSize);
        channel2->process(m_pTemp2, m_pTemp2, iBufferSize);
        if (flanger1->get()==0. && flanger2->get()==1.)
            flanger->process(m_pTemp2, m_pTemp2, iBufferSize);
    }

    //
    // Output channel:
    //

    //
    // Headphone channel:
    //
    // Head phone left/right mix
    float cf_val = head_mix->get();
    float chead_gain = 0.5*(-cf_val+1.);
    float cmaster_gain = 0.5*(cf_val+1.);
    //qDebug() << "head val " << cf_val << ", head " << chead_gain << ", master " << cmaster_gain;

    if (master1 && pfl1->get()==1. && master2 && pfl2->get()==1.)
    {
        //qDebug() << "both";
        for (int i=0; i<iBufferSize; i++)
            m_pHead[i] = m_pTemp1[i]*chead_gain + m_pTemp2[i]*chead_gain;
    }
    else if (master1 && pfl1->get()==1.)
    {
        //qDebug() << "ch 1";
        for (int i=0; i<iBufferSize; i++)
            m_pHead[i] = m_pTemp1[i]*chead_gain;
    }
    else if (master2 && pfl2->get()==1.)
    {
        //qDebug() << "ch 2";
        for (int i=0; i<iBufferSize; i++)
            m_pHead[i] = m_pTemp2[i]*chead_gain;
    }
    else
    {
        //qDebug() << "none";
        for (int i=0; i<iBufferSize; i++)
            m_pHead[i] = 0.;
    }

    // Volume and vu meters for each channel
    vumeter1->process(m_pTemp1, m_pTemp1, iBufferSize);
    volume1->process(m_pTemp1, m_pTemp1, iBufferSize);
    vumeter2->process(m_pTemp2, m_pTemp2, iBufferSize);
    volume2->process(m_pTemp2, m_pTemp2, iBufferSize);


    // Crossfader and Transform buttons
	/*
    cf_val = crossfader->get();
    //qDebug() << "cf_val: " << cf_val;
    float c1_gain, c2_gain;
    if (cf_val>0)
    {
        if (transform2->get()) {
            c1_gain = 1.;
            c2_gain = 1.-cf_val;
        } else {
            c1_gain = 1.-cf_val;
            c2_gain = 1.;
        }

    }
    else
    {
        if (transform1->get()) {
            c1_gain = 1.+cf_val;
            c2_gain = 1.;
        } else {
            c1_gain = 1.;
            c2_gain = 1.+cf_val;
        }
    }*/
	//set gain levels;
	float c1_gain, c2_gain;
	EngineXfader::getXfadeGains(c1_gain, c2_gain, crossfader->get(), xFaderCurve->get(), xFaderCalibration->get());

    for (int i=0; i<iBufferSize; ++i)
        m_pMaster[i] = (m_pTemp1[i]*c1_gain) + (m_pTemp2[i]*c2_gain);


    // Master volume
    volume->process(m_pMaster, m_pMaster, iBufferSize);

#ifdef __LADSPA__
    ladspa->process(m_pMaster, m_pMaster, iBufferSize);
#endif

    // Process the flanger on master if flanger is enabled on both channels
    if (flanger1->get()==1. && flanger2->get()==1.)
        flanger->process(m_pMaster, m_pMaster, iBufferSize);

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

    for (int i=0; i<iBufferSize; i+=2)
    {
        //Perform balancing on main out
        m_pMaster[i  ] = m_pMaster[i  ]*balleft;
        m_pMaster[i+1] = m_pMaster[i+1]*balright;
    }

    // Add master to headphone
    for (int i=0; i<iBufferSize; i++)
        m_pHead[i] += m_pMaster[i]*cmaster_gain;

    // Head volume and clipping
    head_volume->process(m_pHead, m_pHead, iBufferSize);
    head_clipping->process(m_pHead, m_pHead, iBufferSize);

    int j=0;

    // moved here to take balance into account -elysion
    // Update VU meter (it does not return anything):
    if (vumeter!=0)
        vumeter->process(m_pMaster, m_pMaster, iBufferSize);


    //Submit samples to the side chain to do shoutcasting, recording, etc.
    //(cpu intensive non-realtime tasks)
    sidechain->submitSamples(m_pMaster, iBufferSize);

    //Master/headphones interleaving is now done in SoundManager::requestBuffer() - Albert Nov 18/07


    // We're close to the end of the callback. Schedule the workers. Hopefully
    // the work thread doesn't get scheduled between now and then.
    m_pWorkerScheduler->runWorkers();
}

