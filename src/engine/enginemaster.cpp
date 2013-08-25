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
#include "engine/enginedeck.h"
#include "engineclipping.h"
#include "enginevumeter.h"
#include "enginexfader.h"
#include "engine/sidechain/enginesidechain.h"
#include "sampleutil.h"
#include "effects/effectsmanager.h"
#include "util/timer.h"
#include "playermanager.h"
#include "engine/channelmixer.h"

#ifdef __LADSPA__
#include "engineladspa.h"
#endif

EngineMaster::EngineMaster(ConfigObject<ConfigValue> * _config,
                           const char * group,
                           EffectsManager* pEffectsManager,
                           bool bEnableSidechain)
        : m_pEffectsManager(pEffectsManager),
          m_headphoneMasterGainOld(0),
          m_headphoneVolumeOld(0) {
    m_pWorkerScheduler = new EngineWorkerScheduler(this);
    m_pWorkerScheduler->start();

    m_pEffectsManager->registerChannel(getMasterChannelId());
    m_pEffectsManager->registerChannel(getHeadphoneChannelId());

    // Master sample rate
    m_pMasterSampleRate = new ControlObject(ConfigKey(group, "samplerate"), true, true);
    m_pMasterSampleRate->set(44100.);

    // Latency control
    m_pMasterLatency = new ControlObject(ConfigKey(group, "latency"), true, true);
    m_pMasterAudioBufferSize = new ControlObject(ConfigKey(group, "audio_buffer_size"));
    m_pMasterUnderflowCount = new ControlObject(ConfigKey(group, "underflow_count"), true, true);

    // Master rate
    m_pMasterRate = new ControlPotmeter(ConfigKey(group, "rate"), -1.0, 1.0);

#ifdef __LADSPA__
    // LADSPA
    m_pLadspa = new EngineLADSPA();
#endif

    // Crossfader
    m_pCrossfader = new ControlPotmeter(ConfigKey(group, "crossfader"),-1.,1.);

    // Balance
    m_pBalance = new ControlPotmeter(ConfigKey(group, "balance"), -1., 1.);

    // Master volume
    m_pMasterVolume = new ControlLogpotmeter(ConfigKey(group, "volume"), 5.);

    // Clipping
    m_pClipping = new EngineClipping(group);

    // VU meter:
    m_pVumeter = new EngineVuMeter(group);

    // Headphone volume
    m_pHeadVolume = new ControlLogpotmeter(ConfigKey(group, "headVolume"), 5.);

    // Headphone mix (left/right)
    m_pHeadMix = new ControlPotmeter(ConfigKey(group, "headMix"),-1.,1.);
    m_pHeadMix->setDefaultValue(-1.);
    m_pHeadMix->set(-1.);

    // Headphone Clipping
    m_pHeadClipping = new EngineClipping("");

    // Allocate buffers
    m_pHead = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pMaster = SampleUtil::alloc(MAX_BUFFER_LEN);
    SampleUtil::applyGain(m_pHead, 0, MAX_BUFFER_LEN);
    SampleUtil::applyGain(m_pMaster, 0, MAX_BUFFER_LEN);

    // Starts a thread for recording and shoutcast
    m_pSideChain = bEnableSidechain ? new EngineSideChain(_config) : NULL;

    // X-Fader Setup
    m_pXFaderMode = new ControlPotmeter(
        ConfigKey("[Mixer Profile]", "xFaderMode"), 0., 1.);
    m_pXFaderCurve = new ControlPotmeter(
        ConfigKey("[Mixer Profile]", "xFaderCurve"), 0., 2.);
    m_pXFaderCalibration = new ControlPotmeter(
        ConfigKey("[Mixer Profile]", "xFaderCalibration"), -2., 2.);
    m_pXFaderReverse = new ControlPotmeter(
        ConfigKey("[Mixer Profile]", "xFaderReverse"), 0., 1.);
}

EngineMaster::~EngineMaster() {
    qDebug() << "in ~EngineMaster()";
    delete m_pCrossfader;
    delete m_pBalance;
    delete m_pHeadMix;
    delete m_pMasterVolume;
    delete m_pHeadVolume;
    delete m_pClipping;
    delete m_pVumeter;
    delete m_pHeadClipping;
    delete m_pSideChain;

    delete m_pXFaderReverse;
    delete m_pXFaderCalibration;
    delete m_pXFaderCurve;
    delete m_pXFaderMode;

    delete m_pMasterSampleRate;
    delete m_pMasterLatency;
    delete m_pMasterAudioBufferSize;
    delete m_pMasterRate;
    delete m_pMasterUnderflowCount;

    SampleUtil::free(m_pHead);
    SampleUtil::free(m_pMaster);

    delete m_pWorkerScheduler;

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

const CSAMPLE* EngineMaster::getMasterBuffer() const {
    return m_pMaster;
}

const CSAMPLE* EngineMaster::getHeadphoneBuffer() const {
    return m_pHead;
}

void EngineMaster::process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize) {
    static bool haveSetName = false;
    if (!haveSetName) {
        QThread::currentThread()->setObjectName("Engine");
        haveSetName = true;
    }
    ScopedTimer t("EngineMaster::process");

    CSAMPLE **pOutput = (CSAMPLE**)pOut;
    Q_UNUSED(pOutput);

    // Prepare each channel for output

    // Bitvector of enabled channels
    const unsigned int maxChannels = 32;
    unsigned int masterOutput = 0;
    unsigned int headphoneOutput = 0;

    // Compute headphone mix
    // Head phone left/right mix
    CSAMPLE cf_val = m_pHeadMix->get();
    CSAMPLE chead_gain = 0.5*(-cf_val+1.);
    CSAMPLE cmaster_gain = 0.5*(cf_val+1.);
    // qDebug() << "head val " << cf_val << ", head " << chead_gain
    //          << ", master " << cmaster_gain;

    Timer timer("EngineMaster::process channels");
    QList<ChannelInfo*>::iterator it = m_channels.begin();
    for (unsigned int channel_number = 0;
         it != m_channels.end(); ++it, ++channel_number) {
        ChannelInfo* pChannelInfo = *it;
        EngineChannel* pChannel = pChannelInfo->m_pChannel;

        if (!pChannel->isActive()) {
            continue;
        }

        bool needsProcessing = false;
        if (pChannel->isMaster()) {
            masterOutput |= (1 << channel_number);
            needsProcessing = true;
        }

        // If the channel is enabled for previewing in headphones, copy it
        // over to the headphone buffer
        if (pChannel->isPFL()) {
            headphoneOutput |= (1 << channel_number);
            needsProcessing = true;
        }

        // Process the buffer if necessary
        if (needsProcessing) {
            pChannel->process(NULL, pChannelInfo->m_pBuffer, iBufferSize);
        }
    }
    timer.elapsed(true);

    // Mix all the enabled headphone channels together.
    m_headphoneGain.setGain(chead_gain);

    ChannelMixer::mixChannels(m_channels, m_headphoneGain, headphoneOutput,
                              maxChannels, &m_channelHeadphoneGainCache,
                              m_pHead, iBufferSize);

    // Calculate the crossfader gains for left and right side of the crossfader
    double c1_gain, c2_gain;
    EngineXfader::getXfadeGains(m_pCrossfader->get(), m_pXFaderCurve->get(),
                                m_pXFaderCalibration->get(),
                                m_pXFaderMode->get() == MIXXX_XFADER_CONSTPWR,
                                m_pXFaderReverse->get() == 1.0,
                                &c1_gain, &c2_gain);

    // Now set the gains for overall volume and the left, center, right gains.
    m_masterGain.setGains(m_pMasterVolume->get(), c1_gain, 1.0, c2_gain);

    // Perform the master mix
    ChannelMixer::mixChannels(m_channels, m_masterGain, masterOutput,
                              maxChannels, &m_channelMasterGainCache,
                              m_pMaster, iBufferSize);

    // Process master channel effects
    m_pEffectsManager->process(getMasterChannelId(), m_pMaster, m_pMaster, iBufferSize);

#ifdef __LADSPA__
    // LADPSA master effects
    m_pLadspa->process(m_pMaster, m_pMaster, iBufferSize);
#endif

    // Clipping
    m_pClipping->process(m_pMaster, m_pMaster, iBufferSize);

    // Balance values
    CSAMPLE balright = 1.;
    CSAMPLE balleft = 1.;
    CSAMPLE bal = m_pBalance->get();
    if (bal>0.)
        balleft -= bal;
    else if (bal<0.)
        balright += bal;

    // Perform balancing on main out
    SampleUtil::applyAlternatingGain(m_pMaster, balleft, balright, iBufferSize);

    // Update VU meter (it does not return anything). Needs to be here so that
    // master balance is reflected in the VU meter.
    if (m_pVumeter != NULL) {
        m_pVumeter->process(m_pMaster, m_pMaster, iBufferSize);
    }

    //Submit master samples to the side chain to do shoutcasting, recording,
    //etc.  (cpu intensive non-realtime tasks)
    if (m_pSideChain != NULL) {
        m_pSideChain->writeSamples(m_pMaster, iBufferSize);
    }

    // Add master to headphone with appropriate gain
    SampleUtil::addWithRampingGain(m_pHead, m_pMaster, m_headphoneMasterGainOld,
                                   cmaster_gain, iBufferSize);
    m_headphoneMasterGainOld = cmaster_gain;

    // Process headphone channel effects
    m_pEffectsManager->process(getHeadphoneChannelId(), m_pHead, m_pHead, iBufferSize);

    // Head volume and clipping
    CSAMPLE headphoneVolume = m_pHeadVolume->get();
    SampleUtil::applyRampingGain(m_pHead, m_headphoneVolumeOld, headphoneVolume, iBufferSize);
    m_headphoneVolumeOld = headphoneVolume;
    m_pHeadClipping->process(m_pHead, m_pHead, iBufferSize);

    //Master/headphones interleaving is now done in
    //SoundManager::requestBuffer() - Albert Nov 18/07

    // We're close to the end of the callback. Wake up the engine worker
    // scheduler so that it runs the workers.
    m_pWorkerScheduler->runWorkers();
}

void EngineMaster::addChannel(EngineChannel* pChannel) {
    ChannelInfo* pChannelInfo = new ChannelInfo();
    pChannelInfo->m_pChannel = pChannel;
    pChannelInfo->m_pVolumeControl = new ControlLogpotmeter(
            ConfigKey(pChannel->getGroup(), "volume"), 1.0);
    pChannelInfo->m_pVolumeControl->setDefaultValue(1.0);
    pChannelInfo->m_pVolumeControl->set(1.0);
    pChannelInfo->m_pBuffer = SampleUtil::alloc(MAX_BUFFER_LEN);
    SampleUtil::applyGain(pChannelInfo->m_pBuffer, 0, MAX_BUFFER_LEN);
    m_channels.push_back(pChannelInfo);
    m_channelMasterGainCache.push_back(0);
    m_channelHeadphoneGainCache.push_back(0);

    EngineBuffer* pBuffer = pChannelInfo->m_pChannel->getEngineBuffer();
    if (pBuffer != NULL) {
        pBuffer->bindWorkers(m_pWorkerScheduler);
        pBuffer->setEngineMaster(this);
    }
}

EngineChannel* EngineMaster::getChannel(QString group) {
    for (QList<ChannelInfo*>::const_iterator i = m_channels.begin();
         i != m_channels.end(); ++i) {
        ChannelInfo* pChannelInfo = *i;
        if (pChannelInfo->m_pChannel->getGroup() == group) {
            return pChannelInfo->m_pChannel;
        }
    }
    return NULL;
}

const CSAMPLE* EngineMaster::getDeckBuffer(unsigned int i) const {
    return getChannelBuffer(PlayerManager::groupForDeck(i));
}

const CSAMPLE* EngineMaster::getChannelBuffer(QString group) const {
    for (QList<ChannelInfo*>::const_iterator i = m_channels.constBegin();
         i != m_channels.constEnd(); ++i) {
        const ChannelInfo* pChannelInfo = *i;
        if (pChannelInfo->m_pChannel->getGroup() == group) {
            return pChannelInfo->m_pBuffer;
        }
    }
    return NULL;
}

const CSAMPLE* EngineMaster::buffer(AudioOutput output) const {
    switch (output.getType()) {
    case AudioOutput::MASTER:
        return getMasterBuffer();
        break;
    case AudioOutput::HEADPHONES:
        return getHeadphoneBuffer();
        break;
    case AudioOutput::DECK:
        return getDeckBuffer(output.getIndex());
        break;
    default:
        return NULL;
    }
}
