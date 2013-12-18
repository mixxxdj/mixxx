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

#include <QtDebug>
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
#include "engine/sidechain/enginesidechain.h"
#include "engine/sync/enginesync.h"
#include "sampleutil.h"
#include "util/timer.h"
#include "playermanager.h"
#include "engine/channelmixer.h"

#ifdef __LADSPA__
#include "engineladspa.h"
#endif

EngineMaster::EngineMaster(ConfigObject<ConfigValue> * _config,
                           const char * group,
                           bool bEnableSidechain,
                           bool bRampingGain)
        : m_bRampingGain(bRampingGain),
          m_headphoneMasterGainOld(0.0),
          m_headphoneVolumeOld(1.0) {
    m_pWorkerScheduler = new EngineWorkerScheduler(this);
    m_pWorkerScheduler->start();

    // Master sample rate
    m_pMasterSampleRate = new ControlObject(ConfigKey(group, "samplerate"), true, true);
    m_pMasterSampleRate->set(44100.);

    // Latency control
    m_pMasterLatency = new ControlObject(ConfigKey(group, "latency"), true, true);
    m_pMasterAudioBufferSize = new ControlObject(ConfigKey(group, "audio_buffer_size"));
    m_pMasterUnderflowCount = new ControlObject(ConfigKey(group, "underflow_count"), true, true);

    // Master rate
    m_pMasterRate = new ControlPotmeter(ConfigKey(group, "rate"), -1.0, 1.0);

    // Master sync controller
    m_pMasterSync = new EngineSync(_config);

    // The last-used bpm value is saved in the destructor of EngineSync.
    double default_bpm = _config->getValueString(ConfigKey("[InternalClock]", "bpm"),
                                                 "124.0").toDouble();
    ControlObject::getControl(ConfigKey("[InternalClock]","bpm"))->set(default_bpm);

#ifdef __LADSPA__
    // LADSPA
    m_pLadspa = new EngineLADSPA();
#endif

    // Crossfader
    m_pCrossfader = new ControlPotmeter(ConfigKey(group, "crossfader"), -1., 1.);

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

    // Setup the output buses
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; ++o) {
        struct OutputBus* bus = &m_outputBus[o];
        bus->m_pBuffer = SampleUtil::alloc(MAX_BUFFER_LEN);
        SampleUtil::applyGain(bus->m_pBuffer, 0, MAX_BUFFER_LEN);
        bus->m_gain.setGains(1.0,
                             o == EngineChannel::LEFT ? 1.0 : 0.0,
                             o == EngineChannel::CENTER ? 1.0 : 0.0,
                             o == EngineChannel::RIGHT ? 1.0 : 0.0);
    }

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

    delete m_pMasterSync;
    delete m_pMasterSampleRate;
    delete m_pMasterLatency;
    delete m_pMasterAudioBufferSize;
    delete m_pMasterRate;
    delete m_pMasterUnderflowCount;

    SampleUtil::free(m_pHead);
    SampleUtil::free(m_pMaster);
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        SampleUtil::free(m_outputBus[o].m_pBuffer);
    }

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

void EngineMaster::processChannels(unsigned int* busChannelConnectionFlags,
                                   unsigned int* headphoneOutput,
                                   int iBufferSize) {
    ScopedTimer timer("EngineMaster::processChannels");

    QList<ChannelInfo*>::iterator it = m_channels.begin();
    QList<ChannelInfo*>::iterator master_it = NULL;

    // Find the Sync Master and process it first then process all the slaves
    // (and skip the master).

    EngineChannel* pMasterChannel = m_pMasterSync->getMaster();
    if (pMasterChannel != NULL) {
        for (unsigned int channel_number = 0;
             it != m_channels.end(); ++it, ++channel_number) {
            ChannelInfo* pChannelInfo = *it;
            EngineChannel* pChannel = pChannelInfo->m_pChannel;
            if (!pChannel || !pChannel->isActive()) {
               continue;
            }

            if (pMasterChannel == pChannel) {
                master_it = it;

                // Proceed with the processing as below.
                bool needsProcessing = false;
                if (pChannel->isMaster()) {
                    busChannelConnectionFlags[pChannel->getOrientation()] |= (1 << channel_number);
                    needsProcessing = true;
                }

                // If the channel is enabled for previewing in headphones, copy it
                // over to the headphone buffer
                if (pChannel->isPFL()) {
                    *headphoneOutput |= (1 << channel_number);
                    needsProcessing = true;
                }

                // Process the buffer if necessary, which it damn well better be
                if (needsProcessing) {
                    pChannel->process(NULL, pChannelInfo->m_pBuffer, iBufferSize);
                }
                break;
            }
        }
    }

    it = m_channels.begin();
    for (unsigned int channel_number = 0;
         it != m_channels.end(); ++it, ++channel_number) {
        ChannelInfo* pChannelInfo = *it;
        EngineChannel* pChannel = pChannelInfo->m_pChannel;

        // Skip the master since we already processed it.
        if (it == master_it) {
            continue;
        }

        // Skip inactive channels.
        if (!pChannel || !pChannel->isActive()) {
            continue;
        }

        bool needsProcessing = false;
        if (pChannel->isMaster()) {
            busChannelConnectionFlags[pChannel->getOrientation()] |= (1 << channel_number);
            needsProcessing = true;
        }

        // If the channel is enabled for previewing in headphones, copy it
        // over to the headphone buffer
        if (pChannel->isPFL()) {
            *headphoneOutput |= (1 << channel_number);
            needsProcessing = true;
        }

        // Process the buffer if necessary
        if (needsProcessing) {
            pChannel->process(NULL, pChannelInfo->m_pBuffer, iBufferSize);
        }
    }
}

void EngineMaster::process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize) {
    static bool haveSetName = false;
    if (!haveSetName) {
        QThread::currentThread()->setObjectName("Engine");
        haveSetName = true;
    }
    ScopedTimer t("EngineMaster::process");

    int iSampleRate = static_cast<int>(m_pMasterSampleRate->get());
    // Update internal master sync.
    m_pMasterSync->onCallbackStart(iSampleRate, iBufferSize);

    CSAMPLE **pOutput = (CSAMPLE**)pOut;
    Q_UNUSED(pOutput);

    // Bitvector of enabled channels
    const unsigned int maxChannels = 32;
    unsigned int busChannelConnectionFlags[3] = { 0, 0, 0 };
    unsigned int headphoneOutput = 0;

    // Prepare each channel for output
    Timer timer("EngineMaster::process channels");
    processChannels(busChannelConnectionFlags, &headphoneOutput, iBufferSize);

    // Compute headphone mix
    // Head phone left/right mix
    CSAMPLE cf_val = m_pHeadMix->get();
    CSAMPLE chead_gain = 0.5*(-cf_val+1.);
    CSAMPLE cmaster_gain = 0.5*(cf_val+1.);
    // qDebug() << "head val " << cf_val << ", head " << chead_gain
    //          << ", master " << cmaster_gain;

    timer.elapsed(true);

    // Mix all the enabled headphone channels together.
    m_headphoneGain.setGain(chead_gain);

    if (m_bRampingGain) {
        ChannelMixer::mixChannelsRamping(
            m_channels, m_headphoneGain, headphoneOutput,
            maxChannels, &m_channelHeadphoneGainCache,
            m_pHead, iBufferSize);
    } else {
        ChannelMixer::mixChannels(
            m_channels, m_headphoneGain, headphoneOutput,
            maxChannels, &m_channelHeadphoneGainCache,
            m_pHead, iBufferSize);
    }

    // Make the mix for each output bus
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        if (m_bRampingGain) {
            ChannelMixer::mixChannelsRamping(
                m_channels, m_outputBus[o].m_gain,
                busChannelConnectionFlags[o], maxChannels,
                &m_outputBus[o].m_gainCache,
                m_outputBus[o].m_pBuffer, iBufferSize);
        } else {
            ChannelMixer::mixChannels(
                m_channels, m_outputBus[o].m_gain,
                busChannelConnectionFlags[o], maxChannels,
                &m_outputBus[o].m_gainCache,
                m_outputBus[o].m_pBuffer, iBufferSize);
        }
    }

    // Calculate the crossfader gains for left and right side of the crossfader
    double c1_gain, c2_gain;
    EngineXfader::getXfadeGains(m_pCrossfader->get(), m_pXFaderCurve->get(),
                                m_pXFaderCalibration->get(),
                                m_pXFaderMode->get() == MIXXX_XFADER_CONSTPWR,
                                m_pXFaderReverse->get() == 1.0,
                                &c1_gain, &c2_gain);

    // And mix the 3 buses into the master
    float master_gain = m_pMasterVolume->get();
    SampleUtil::copy3WithGain(m_pMaster,
                              m_outputBus[EngineChannel::LEFT].m_pBuffer, c1_gain*master_gain,
                              m_outputBus[EngineChannel::CENTER].m_pBuffer, master_gain,
                              m_outputBus[EngineChannel::RIGHT].m_pBuffer, c2_gain*master_gain,
                              iBufferSize);

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

    // Submit master samples to the side chain to do shoutcasting, recording,
    // etc. (cpu intensive non-realtime tasks)
    if (m_pSideChain != NULL) {
        m_pSideChain->writeSamples(m_pMaster, iBufferSize);
    }

    // Add master to headphone with appropriate gain
    if (m_bRampingGain) {
        SampleUtil::addWithRampingGain(m_pHead, m_pMaster,
                                       m_headphoneMasterGainOld,
                                       cmaster_gain, iBufferSize);
    } else {
        SampleUtil::addWithGain(m_pHead, m_pMaster, cmaster_gain, iBufferSize);
    }
    m_headphoneMasterGainOld = cmaster_gain;

    // Head volume and clipping
    CSAMPLE headphoneVolume = m_pHeadVolume->get();
    if (m_bRampingGain) {
        SampleUtil::applyRampingGain(m_pHead, m_headphoneVolumeOld,
                                     headphoneVolume, iBufferSize);
    } else {
        SampleUtil::applyGain(m_pHead, headphoneVolume, iBufferSize);
    }
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
    m_channelHeadphoneGainCache.push_back(0);
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        m_outputBus[o].m_gainCache.push_back(0);
    }

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

const CSAMPLE* EngineMaster::getOutputBusBuffer(unsigned int i) const {
    if (i <= EngineChannel::RIGHT)
        return m_outputBus[i].m_pBuffer;
    return NULL;
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
    case AudioOutput::BUS:
        return getOutputBusBuffer(output.getIndex());
        break;
    case AudioOutput::DECK:
        return getDeckBuffer(output.getIndex());
        break;
    default:
        return NULL;
    }
}
