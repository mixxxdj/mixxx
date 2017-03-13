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
#include "controlaudiotaperpot.h"
#include "controlpotmeter.h"
#include "controlaudiotaperpot.h"
#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "engine/engineworkerscheduler.h"
#include "engine/enginedeck.h"
#include "engine/enginebuffer.h"
#include "engine/enginechannel.h"
#include "engine/enginetalkoverducking.h"
#include "engine/enginevumeter.h"
#include "engine/enginexfader.h"
#include "engine/enginedelay.h"
#include "engine/sidechain/enginesidechain.h"
#include "engine/sync/enginesync.h"
#include "sampleutil.h"
#include "engine/effects/engineeffectsmanager.h"
#include "effects/effectsmanager.h"
#include "util/timer.h"
#include "util/trace.h"
#include "util/defs.h"
#include "playermanager.h"
#include "engine/channelmixer.h"

EngineMaster::EngineMaster(ConfigObject<ConfigValue>* _config,
                           const char* group,
                           EffectsManager* pEffectsManager,
                           bool bEnableSidechain,
                           bool bRampingGain)
        : m_pEngineEffectsManager(pEffectsManager ? pEffectsManager->getEngineEffectsManager() : NULL),
          m_bRampingGain(bRampingGain),
          m_masterVolumeOld(0.0),
          m_headphoneMasterGainOld(0.0),
          m_headphoneVolumeOld(1.0) {
    m_bBusOutputConnected[0] = false;
    m_bBusOutputConnected[1] = false;
    m_bBusOutputConnected[2] = false;
    m_pWorkerScheduler = new EngineWorkerScheduler(this);
    m_pWorkerScheduler->start(QThread::HighPriority);

    if (pEffectsManager) {
        pEffectsManager->registerGroup(getMasterGroup());
        pEffectsManager->registerGroup(getHeadphoneGroup());
        pEffectsManager->registerGroup(getBusLeftGroup());
        pEffectsManager->registerGroup(getBusCenterGroup());
        pEffectsManager->registerGroup(getBusRightGroup());
    }

    // Master sample rate
    m_pMasterSampleRate = new ControlObject(ConfigKey(group, "samplerate"), true, true);
    m_pMasterSampleRate->set(44100.);

    // Latency control
    m_pMasterLatency = new ControlObject(ConfigKey(group, "latency"), true, true);
    m_pMasterAudioBufferSize = new ControlObject(ConfigKey(group, "audio_buffer_size"));
    m_pAudioLatencyOverloadCount = new ControlObject(ConfigKey(group, "audio_latency_overload_count"), true, true);
    m_pAudioLatencyUsage = new ControlPotmeter(ConfigKey(group, "audio_latency_usage"), 0.0, 0.25);
    m_pAudioLatencyOverload  = new ControlPotmeter(ConfigKey(group, "audio_latency_overload"), 0.0, 1.0);

    // Master rate
    m_pMasterRate = new ControlPotmeter(ConfigKey(group, "rate"), -1.0, 1.0);

    // Master sync controller
    m_pMasterSync = new EngineSync(_config);

    // The last-used bpm value is saved in the destructor of EngineSync.
    double default_bpm = _config->getValueString(ConfigKey("[InternalClock]", "bpm"),
                                                 "124.0").toDouble();
    ControlObject::getControl(ConfigKey("[InternalClock]","bpm"))->set(default_bpm);

    // Crossfader
    m_pCrossfader = new ControlPotmeter(ConfigKey(group, "crossfader"), -1., 1.);

    // Balance
    m_pBalance = new ControlPotmeter(ConfigKey(group, "balance"), -1., 1.);

    // Master gain
    m_pMasterGain = new ControlAudioTaperPot(ConfigKey(group, "gain"), -14, 14, 0.5);

    // Legacy: the master "gain" control used to be named "volume" in Mixxx
    // 1.11.0 and earlier. See Bug #1306253.
    ControlDoublePrivate::insertAlias(ConfigKey(group, "volume"),
                                      ConfigKey(group, "gain"));

    // VU meter:
    m_pVumeter = new EngineVuMeter(group);

    m_pMasterDelay = new EngineDelay(group, ConfigKey(group, "delay"));
    m_pHeadDelay = new EngineDelay(group, ConfigKey(group, "headDelay"));

    // Headphone volume
    m_pHeadGain = new ControlAudioTaperPot(ConfigKey(group, "headGain"), -14, 14, 0.5);

    // Legacy: the headphone "headGain" control used to be named "headVolume" in
    // Mixxx 1.11.0 and earlier. See Bug #1306253.
    ControlDoublePrivate::insertAlias(ConfigKey(group, "headVolume"),
                                      ConfigKey(group, "headGain"));

    // Headphone mix (left/right)
    m_pHeadMix = new ControlPotmeter(ConfigKey(group, "headMix"),-1.,1.);
    m_pHeadMix->setDefaultValue(-1.);
    m_pHeadMix->set(-1.);

    // Master / Headphone split-out mode (for devices with only one output).
    m_pHeadSplitEnabled = new ControlPushButton(ConfigKey(group, "headSplit"));
    m_pHeadSplitEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pHeadSplitEnabled->set(0.0);

    m_pTalkoverDucking = new EngineTalkoverDucking(_config, group);

    // Allocate buffers
    m_pHead = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pMaster = SampleUtil::alloc(MAX_BUFFER_LEN);
    SampleUtil::clear(m_pHead, MAX_BUFFER_LEN);
    SampleUtil::clear(m_pMaster, MAX_BUFFER_LEN);

    // Setup the output buses
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; ++o) {
        m_pOutputBusBuffers[o] = SampleUtil::alloc(MAX_BUFFER_LEN);
        SampleUtil::clear(m_pOutputBusBuffers[o], MAX_BUFFER_LEN);
    }

    // Starts a thread for recording and shoutcast
    m_pSideChain = bEnableSidechain ? new EngineSideChain(_config) : NULL;

    // X-Fader Setup
    m_pXFaderMode = new ControlPushButton(
            ConfigKey("[Mixer Profile]", "xFaderMode"));
    m_pXFaderMode->setButtonMode(ControlPushButton::TOGGLE);
    m_pXFaderCurve = new ControlPotmeter(
            ConfigKey("[Mixer Profile]", "xFaderCurve"), 0., 2.);
    m_pXFaderCalibration = new ControlPotmeter(
            ConfigKey("[Mixer Profile]", "xFaderCalibration"), -2., 2.);
    m_pXFaderReverse = new ControlPushButton(
            ConfigKey("[Mixer Profile]", "xFaderReverse"));
    m_pXFaderReverse->setButtonMode(ControlPushButton::TOGGLE);

    m_pKeylockEngine = new ControlObject(ConfigKey(group, "keylock_engine"),
                                         true, false, true);
    m_pKeylockEngine->set(_config->getValueString(
            ConfigKey(group, "keylock_engine")).toDouble());

    m_pMasterEnabled = new ControlObject(ConfigKey(group, "enabled"),
            true, false, true);  // persist = true
    m_pMasterMonoMixdown = new ControlObject(ConfigKey(group, "mono_mixdown"),
            true, false, true);  // persist = true
    m_pHeadphoneEnabled = new ControlObject(ConfigKey(group, "headEnabled"));


    // Note: the EQ Rack is set in EffectsManager::setupDefaults();
}

EngineMaster::~EngineMaster() {
    qDebug() << "in ~EngineMaster()";
    delete m_pKeylockEngine;
    delete m_pCrossfader;
    delete m_pBalance;
    delete m_pHeadMix;
    delete m_pHeadSplitEnabled;
    delete m_pMasterGain;
    delete m_pHeadGain;
    delete m_pTalkoverDucking;
    delete m_pVumeter;
    delete m_pSideChain;
    delete m_pMasterDelay;
    delete m_pHeadDelay;

    delete m_pXFaderReverse;
    delete m_pXFaderCalibration;
    delete m_pXFaderCurve;
    delete m_pXFaderMode;

    delete m_pMasterSync;
    delete m_pMasterSampleRate;
    delete m_pMasterLatency;
    delete m_pMasterAudioBufferSize;
    delete m_pMasterRate;
    delete m_pAudioLatencyOverloadCount;
    delete m_pAudioLatencyUsage;
    delete m_pAudioLatencyOverload;

    delete m_pMasterEnabled;
    delete m_pMasterMonoMixdown;
    delete m_pHeadphoneEnabled;

    SampleUtil::free(m_pHead);
    SampleUtil::free(m_pMaster);
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        SampleUtil::free(m_pOutputBusBuffers[o]);
    }

    delete m_pWorkerScheduler;

    QMutableListIterator<ChannelInfo*> channel_it(m_channels);
    while (channel_it.hasNext()) {
        ChannelInfo* pChannelInfo = channel_it.next();
        channel_it.remove();
        SampleUtil::free(pChannelInfo->m_pBuffer);
        delete pChannelInfo->m_pChannel;
        delete pChannelInfo->m_pVolumeControl;
        delete pChannelInfo->m_pMuteControl;
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

    // Clear talkover compressor for the next round of gain calculation.
    m_pTalkoverDucking->clearKeys();

    EngineChannel* pMasterChannel = m_pMasterSync->getMaster();
    m_activeChannels.clear();
    m_activeChannels.reserve(m_channels.size());
    QList<ChannelInfo*>::iterator it = m_channels.begin();
    for (unsigned int channel_number = 0;
         it != m_channels.end(); ++it, ++channel_number) {
        ChannelInfo* pChannelInfo = *it;
        EngineChannel* pChannel = pChannelInfo->m_pChannel;

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

        // If necessary, add the channel to the list of buffers to process.
        if (needsProcessing) {
            if (pChannel == pMasterChannel) {
                // If this is the sync master, it should be processed first.
                m_activeChannels.prepend(pChannelInfo);
            } else {
                m_activeChannels.append(pChannelInfo);
            }
        }
    }

    // Now that the list is built and ordered, do the processing.
    foreach (ChannelInfo* pChannelInfo, m_activeChannels) {
        EngineChannel* pChannel = pChannelInfo->m_pChannel;
        pChannel->process(pChannelInfo->m_pBuffer, iBufferSize);

        if (m_pTalkoverDucking->getMode() != EngineTalkoverDucking::OFF &&
                pChannel->isTalkover()) {
            m_pTalkoverDucking->processKey(pChannelInfo->m_pBuffer, iBufferSize);
        }
    }

    // After all the engines have been processed, trigger post-processing
    // which ensures that all channels are updating certain values at the
    // same point in time.  This prevents sync from failing depending on
    // if the sync target was processed before or after the sync origin.
    foreach (ChannelInfo* pChannelInfo, m_activeChannels) {
        pChannelInfo->m_pChannel->postProcess(iBufferSize);
    }
}

void EngineMaster::process(const int iBufferSize) {
    static bool haveSetName = false;
    if (!haveSetName) {
        QThread::currentThread()->setObjectName("Engine");
        haveSetName = true;
    }
    Trace t("EngineMaster::process");

    bool masterEnabled = m_pMasterEnabled->get();
    bool headphoneEnabled = m_pHeadphoneEnabled->get();

    unsigned int iSampleRate = static_cast<int>(m_pMasterSampleRate->get());
    if (m_pEngineEffectsManager) {
        m_pEngineEffectsManager->onCallbackStart();
    }

    // Bitvector of enabled channels
    const unsigned int maxChannels = 32;
    unsigned int busChannelConnectionFlags[3] = { 0, 0, 0 };
    unsigned int headphoneOutput = 0;

    // Update internal master sync rate.
    m_pMasterSync->onCallbackStart(iSampleRate, iBufferSize);
    // Prepare each channel for output
    processChannels(busChannelConnectionFlags, &headphoneOutput, iBufferSize);
    // Do internal master sync post-processing
    m_pMasterSync->onCallbackEnd(iSampleRate, iBufferSize);

    // Compute headphone mix
    // Head phone left/right mix
    CSAMPLE chead_gain = 1;
    CSAMPLE cmaster_gain = 0;
    if (masterEnabled) {
        CSAMPLE cf_val = m_pHeadMix->get();
        chead_gain = 0.5 * (-cf_val + 1.);
        cmaster_gain = 0.5 * (cf_val + 1.);
        // qDebug() << "head val " << cf_val << ", head " << chead_gain
        //          << ", master " << cmaster_gain;
    }

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

    // Calculate the crossfader gains for left and right side of the crossfader
    double c1_gain, c2_gain;
    EngineXfader::getXfadeGains(m_pCrossfader->get(), m_pXFaderCurve->get(),
                                m_pXFaderCalibration->get(),
                                m_pXFaderMode->get() == MIXXX_XFADER_CONSTPWR,
                                m_pXFaderReverse->toBool(),
                                &c1_gain, &c2_gain);

    // Channels with the talkover flag should be mixed with the master signal at
    // full master volume.  All other channels should be adjusted by ducking gain.
    m_masterGain.setGains(m_pTalkoverDucking->getGain(iBufferSize / 2),
                          c1_gain, 1.0, c2_gain, 1.0);

    // Make the mix for each output bus. m_masterGain takes care of applying the
    // master volume, the channel volume, and the orientation gain.
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        if (m_bRampingGain) {
            ChannelMixer::mixChannelsRamping(
                m_channels, m_masterGain,
                busChannelConnectionFlags[o], maxChannels,
                &m_channelMasterGainCache,
                m_pOutputBusBuffers[o], iBufferSize);
        } else {
            ChannelMixer::mixChannels(
                m_channels, m_masterGain,
                busChannelConnectionFlags[o], maxChannels,
                &m_channelMasterGainCache,
                m_pOutputBusBuffers[o], iBufferSize);
        }
    }

    // Process master channel effects
    if (m_pEngineEffectsManager) {
        GroupFeatureState busFeatures;
        m_pEngineEffectsManager->process(getBusLeftGroup(), m_pOutputBusBuffers[0],
                                             iBufferSize, iSampleRate, busFeatures);
        m_pEngineEffectsManager->process(getBusCenterGroup(), m_pOutputBusBuffers[1],
                                             iBufferSize, iSampleRate, busFeatures);
        m_pEngineEffectsManager->process(getBusRightGroup(), m_pOutputBusBuffers[2],
                                             iBufferSize, iSampleRate, busFeatures);
    }

    if (masterEnabled) {
        // Mix the three channels together. We already mixed the busses together
        // with the channel gains and overall master gain.
        SampleUtil::copy3WithGain(m_pMaster,
                                  m_pOutputBusBuffers[EngineChannel::LEFT], 1.0,
                                  m_pOutputBusBuffers[EngineChannel::CENTER], 1.0,
                                  m_pOutputBusBuffers[EngineChannel::RIGHT], 1.0,
                                  iBufferSize);

        // Process master channel effects
        if (m_pEngineEffectsManager) {
            GroupFeatureState masterFeatures;
            // Well, this is delayed by one buffer (it's dependent on the
            // output). Oh well.
            if (m_pVumeter != NULL) {
                m_pVumeter->collectFeatures(&masterFeatures);
            }
            m_pEngineEffectsManager->process(getMasterGroup(), m_pMaster,
                                             iBufferSize, iSampleRate,
                                             masterFeatures);
        }

        // Apply master volume after effects.
        CSAMPLE master_volume = m_pMasterGain->get();
        if (m_bRampingGain) {
            SampleUtil::applyRampingGain(m_pMaster, m_masterVolumeOld,
                                         master_volume, iBufferSize);
        } else {
            SampleUtil::applyGain(m_pMaster, master_volume, iBufferSize);
        }
        m_masterVolumeOld = master_volume;

        // Balance values
        CSAMPLE balright = 1.;
        CSAMPLE balleft = 1.;
        CSAMPLE bal = m_pBalance->get();
        if (bal > 0.) {
            balleft -= bal;
        } else if (bal < 0.) {
            balright += bal;
        }

        // Perform balancing on main out
        SampleUtil::applyAlternatingGain(m_pMaster, balleft, balright, iBufferSize);

        // Update VU meter (it does not return anything). Needs to be here so that
        // master balance is reflected in the VU meter.
        if (m_pVumeter != NULL) {
            m_pVumeter->process(m_pMaster, iBufferSize);
        }
        // Submit master samples to the side chain to do shoutcasting, recording,
        // etc. (cpu intensive non-realtime tasks)
        if (m_pSideChain != NULL) {
            m_pSideChain->writeSamples(m_pMaster, iBufferSize);
        }

        // Add master to headphone with appropriate gain
        if (headphoneEnabled) {
            if (m_bRampingGain) {
                SampleUtil::addWithRampingGain(m_pHead, m_pMaster,
                                               m_headphoneMasterGainOld,
                                               cmaster_gain, iBufferSize);
            } else {
                SampleUtil::addWithGain(m_pHead, m_pMaster, cmaster_gain, iBufferSize);
            }
            m_headphoneMasterGainOld = cmaster_gain;
        }
    }


    if (headphoneEnabled) {
        // Process headphone channel effects
        if (m_pEngineEffectsManager) {
            GroupFeatureState headphoneFeatures;
            m_pEngineEffectsManager->process(getHeadphoneGroup(), m_pHead,
                                             iBufferSize, iSampleRate, headphoneFeatures);
        }
        // Head volume
        CSAMPLE headphoneVolume = m_pHeadGain->get();
        if (m_bRampingGain) {
            SampleUtil::applyRampingGain(m_pHead, m_headphoneVolumeOld,
                                         headphoneVolume, iBufferSize);
        } else {
            SampleUtil::applyGain(m_pHead, headphoneVolume, iBufferSize);
        }
        m_headphoneVolumeOld = headphoneVolume;
    }

    if (masterEnabled && headphoneEnabled) {
        // If Head Split is enabled, replace the left channel of the pfl buffer
        // with a mono mix of the headphone buffer, and the right channel of the pfl
        // buffer with a mono mix of the master output buffer.
        if (m_pHeadSplitEnabled->get()) {
            for (int i = 0; i + 1 < iBufferSize; i += 2) {
                m_pHead[i] = (m_pHead[i] + m_pHead[i + 1]) / 2;
                m_pHead[i + 1] = (m_pMaster[i] + m_pMaster[i + 1]) / 2;
            }
        }
    }

    if (m_pMasterMonoMixdown->get()) {
        SampleUtil::mixStereoToMono(m_pMaster, m_pMaster, iBufferSize);
    }

    if (masterEnabled) {
        m_pMasterDelay->process(m_pMaster, iBufferSize);
    } else {
        SampleUtil::clear(m_pMaster, iBufferSize);
    }
    if (headphoneEnabled) {
        m_pHeadDelay->process(m_pHead, iBufferSize);
    }

    // We're close to the end of the callback. Wake up the engine worker
    // scheduler so that it runs the workers.
    m_pWorkerScheduler->runWorkers();
}

void EngineMaster::addChannel(EngineChannel* pChannel) {
    ChannelInfo* pChannelInfo = new ChannelInfo();
    pChannelInfo->m_pChannel = pChannel;
    pChannelInfo->m_pVolumeControl = new ControlAudioTaperPot(
            ConfigKey(pChannel->getGroup(), "volume"), -20, 0, 1);
    pChannelInfo->m_pVolumeControl->setDefaultValue(1.0);
    pChannelInfo->m_pVolumeControl->set(1.0);
    pChannelInfo->m_pMuteControl = new ControlPushButton(
        ConfigKey(pChannel->getGroup(), "mute"));
    pChannelInfo->m_pMuteControl->setButtonMode(ControlPushButton::POWERWINDOW);
    pChannelInfo->m_pBuffer = SampleUtil::alloc(MAX_BUFFER_LEN);
    SampleUtil::clear(pChannelInfo->m_pBuffer, MAX_BUFFER_LEN);
    m_channels.push_back(pChannelInfo);
    m_channelHeadphoneGainCache.push_back(0);
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        m_channelMasterGainCache.push_back(0);
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
        return m_pOutputBusBuffers[i];
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

void EngineMaster::onOutputConnected(AudioOutput output) {
    switch (output.getType()) {
        case AudioOutput::MASTER:
            // overwrite config option if a master output is configured
            m_pMasterEnabled->set(1.0);
            break;
        case AudioOutput::HEADPHONES:
            m_pHeadphoneEnabled->set(1.0);
            break;
        case AudioOutput::BUS:
            m_bBusOutputConnected[output.getIndex()] = true;
            break;
        case AudioOutput::DECK:
            // We don't track enabled decks.
            break;
        default:
            break;
    }
}

void EngineMaster::onOutputDisconnected(AudioOutput output) {
    switch (output.getType()) {
        case AudioOutput::MASTER:
            // not used, because we need the master buffer for headphone mix
            // and recording/broadcasting as well
            break;
        case AudioOutput::HEADPHONES:
            m_pHeadphoneEnabled->set(1.0);
            break;
        case AudioOutput::BUS:
            m_bBusOutputConnected[output.getIndex()] = false;
            break;
        case AudioOutput::DECK:
            // We don't track enabled decks.
            break;
        default:
            break;
    }
}
