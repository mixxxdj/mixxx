#include "engine/enginemaster.h"

#include <QtDebug>
#include <QList>
#include <QPair>

#include "preferences/usersettings.h"
#include "control/controlaudiotaperpot.h"
#include "control/controlaudiotaperpot.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "effects/effectsmanager.h"
#include "engine/channelmixer.h"
#include "engine/effects/engineeffectsmanager.h"
#include "engine/enginebuffer.h"
#include "engine/enginebuffer.h"
#include "engine/channels/enginechannel.h"
#include "engine/channels/enginedeck.h"
#include "engine/enginedelay.h"
#include "engine/enginetalkoverducking.h"
#include "engine/enginevumeter.h"
#include "engine/engineworkerscheduler.h"
#include "engine/enginexfader.h"
#include "engine/sidechain/enginesidechain.h"
#include "engine/sync/enginesync.h"
#include "mixer/playermanager.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/timer.h"
#include "util/trace.h"

EngineMaster::EngineMaster(UserSettingsPointer pConfig,
                           const char* group,
                           EffectsManager* pEffectsManager,
                           ChannelHandleFactory* pChannelHandleFactory,
                           bool bEnableSidechain)
        : m_pChannelHandleFactory(pChannelHandleFactory),
          m_pEngineEffectsManager(pEffectsManager ? pEffectsManager->getEngineEffectsManager() : NULL),
          m_masterGainOld(0.0),
          m_boothGainOld(0.0),
          m_headphoneMasterGainOld(0.0),
          m_headphoneGainOld(1.0),
          m_balleftOld(1.0),
          m_balrightOld(1.0),
          m_masterHandle(registerChannelGroup(group)),
          m_headphoneHandle(registerChannelGroup("[Headphone]")),
          m_masterOutputHandle(registerChannelGroup("[MasterOutput]")),
          m_busTalkoverHandle(registerChannelGroup("[BusTalkover]")),
          m_busCrossfaderLeftHandle(registerChannelGroup("[BusLeft]")),
          m_busCrossfaderCenterHandle(registerChannelGroup("[BusCenter]")),
          m_busCrossfaderRightHandle(registerChannelGroup("[BusRight]")) {
    pEffectsManager->registerInputChannel(m_masterHandle);
    pEffectsManager->registerInputChannel(m_headphoneHandle);
    pEffectsManager->registerOutputChannel(m_masterHandle);
    pEffectsManager->registerOutputChannel(m_headphoneHandle);

    pEffectsManager->registerInputChannel(m_masterOutputHandle);
    pEffectsManager->registerInputChannel(m_busTalkoverHandle);
    pEffectsManager->registerInputChannel(m_busCrossfaderLeftHandle);
    pEffectsManager->registerInputChannel(m_busCrossfaderCenterHandle);
    pEffectsManager->registerInputChannel(m_busCrossfaderRightHandle);
    m_bBusOutputConnected[EngineChannel::LEFT] = false;
    m_bBusOutputConnected[EngineChannel::CENTER] = false;
    m_bBusOutputConnected[EngineChannel::RIGHT] = false;
    m_bExternalRecordBroadcastInputConnected = false;
    m_pWorkerScheduler = new EngineWorkerScheduler(this);
    m_pWorkerScheduler->start(QThread::HighPriority);

    // Master sample rate
    m_pMasterSampleRate = new ControlObject(ConfigKey(group, "samplerate"), true, true);
    m_pMasterSampleRate->set(44100.);

    // Latency control
    m_pMasterLatency = new ControlObject(ConfigKey(group, "latency"), true, true);
    m_pMasterAudioBufferSize = new ControlObject(ConfigKey(group, "audio_buffer_size"));
    m_pAudioLatencyOverloadCount = new ControlObject(ConfigKey(group, "audio_latency_overload_count"), true, true);
    m_pAudioLatencyUsage = new ControlPotmeter(ConfigKey(group, "audio_latency_usage"), 0.0, 0.25);
    m_pAudioLatencyOverload  = new ControlPotmeter(ConfigKey(group, "audio_latency_overload"), 0.0, 1.0);

    // Master sync controller
    m_pMasterSync = new EngineSync(pConfig);

    // The last-used bpm value is saved in the destructor of EngineSync.
    double default_bpm = pConfig->getValue(
            ConfigKey("[InternalClock]", "bpm"), 124.0);
    ControlObject::getControl(ConfigKey("[InternalClock]","bpm"))->set(default_bpm);

    // Crossfader
    m_pCrossfader = new ControlPotmeter(ConfigKey(group, "crossfader"), -1., 1.);

    // Balance
    m_pBalance = new ControlPotmeter(ConfigKey(group, "balance"), -1., 1.);

    // Master gain
    m_pMasterGain = new ControlAudioTaperPot(ConfigKey(group, "gain"), -14, 14, 0.5);

    // Booth gain
    m_pBoothGain = new ControlAudioTaperPot(ConfigKey(group, "booth_gain"), -14, 14, 0.5);

    // Legacy: the master "gain" control used to be named "volume" in Mixxx
    // 1.11.0 and earlier. See Bug #1306253.
    ControlDoublePrivate::insertAlias(ConfigKey(group, "volume"),
                                      ConfigKey(group, "gain"));

    // VU meter:
    m_pVumeter = new EngineVuMeter(group);

    m_pMasterDelay = new EngineDelay(group, ConfigKey(group, "delay"));
    m_pHeadDelay = new EngineDelay(group, ConfigKey(group, "headDelay"));
    m_pBoothDelay = new EngineDelay(group, ConfigKey(group, "boothDelay"));
    m_pLatencyCompensationDelay = new EngineDelay(group,
        ConfigKey(group, "microphoneLatencyCompensation"));
    m_pNumMicsConfigured = new ControlObject(ConfigKey(group, "num_mics_configured"));

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

    m_pTalkoverDucking = new EngineTalkoverDucking(pConfig, group);

    // Allocate buffers
    m_pHead = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pMaster = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pBooth = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pTalkover = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pTalkoverHeadphones = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pSidechainMix = SampleUtil::alloc(MAX_BUFFER_LEN);
    SampleUtil::clear(m_pHead, MAX_BUFFER_LEN);
    SampleUtil::clear(m_pMaster, MAX_BUFFER_LEN);
    SampleUtil::clear(m_pBooth, MAX_BUFFER_LEN);
    SampleUtil::clear(m_pTalkover, MAX_BUFFER_LEN);
    SampleUtil::clear(m_pTalkoverHeadphones, MAX_BUFFER_LEN);
    SampleUtil::clear(m_pSidechainMix, MAX_BUFFER_LEN);

    // Setup the output buses
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; ++o) {
        m_pOutputBusBuffers[o] = SampleUtil::alloc(MAX_BUFFER_LEN);
        SampleUtil::clear(m_pOutputBusBuffers[o], MAX_BUFFER_LEN);
    }

    // Starts a thread for recording and broadcast
    m_pEngineSideChain = bEnableSidechain ? new EngineSideChain(pConfig) : NULL;

    // X-Fader Setup
    m_pXFaderMode = new ControlPushButton(
            ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderMode"));
    m_pXFaderMode->setButtonMode(ControlPushButton::TOGGLE);

    m_pXFaderCurve = new ControlPotmeter(
            ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderCurve"),
            EngineXfader::kTransformMin, EngineXfader::kTransformMax);
    m_pXFaderCalibration = new ControlPotmeter(
            ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderCalibration"),
            0.3, 1., true);
    m_pXFaderReverse = new ControlPushButton(
            ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderReverse"));
    m_pXFaderReverse->setButtonMode(ControlPushButton::TOGGLE);

    m_pKeylockEngine = new ControlObject(ConfigKey(group, "keylock_engine"),
                                         true, false, true);
    m_pKeylockEngine->set(pConfig->getValueString(
            ConfigKey(group, "keylock_engine")).toDouble());

    // TODO: Make this read only and make EngineMaster decide whether
    // processing the master mix is necessary.
    m_pMasterEnabled = new ControlObject(ConfigKey(group, "enabled"),
            true, false, true);  // persist = true
    m_pBoothEnabled = new ControlObject(ConfigKey(group, "booth_enabled"));
    m_pBoothEnabled->setReadOnly();
    m_pMasterMonoMixdown = new ControlObject(ConfigKey(group, "mono_mixdown"),
            true, false, true);  // persist = true
    m_pMicMonitorMode = new ControlObject(ConfigKey(group, "talkover_mix"),
            true, false, true);  // persist = true
    m_pHeadphoneEnabled = new ControlObject(ConfigKey(group, "headEnabled"));
    m_pHeadphoneEnabled->setReadOnly();

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
    delete m_pBoothGain;
    delete m_pHeadGain;
    delete m_pTalkoverDucking;
    delete m_pVumeter;
    delete m_pEngineSideChain;
    delete m_pMasterDelay;
    delete m_pHeadDelay;
    delete m_pBoothDelay;
    delete m_pLatencyCompensationDelay;
    delete m_pNumMicsConfigured;

    delete m_pXFaderReverse;
    delete m_pXFaderCalibration;
    delete m_pXFaderCurve;
    delete m_pXFaderMode;

    delete m_pMasterSync;
    delete m_pMasterSampleRate;
    delete m_pMasterLatency;
    delete m_pMasterAudioBufferSize;
    delete m_pAudioLatencyOverloadCount;
    delete m_pAudioLatencyUsage;
    delete m_pAudioLatencyOverload;

    delete m_pMasterEnabled;
    delete m_pBoothEnabled;
    delete m_pMasterMonoMixdown;
    delete m_pMicMonitorMode;
    delete m_pHeadphoneEnabled;

    SampleUtil::free(m_pHead);
    SampleUtil::free(m_pMaster);
    SampleUtil::free(m_pBooth);
    SampleUtil::free(m_pTalkover);
    SampleUtil::free(m_pTalkoverHeadphones);
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        SampleUtil::free(m_pOutputBusBuffers[o]);
    }

    delete m_pWorkerScheduler;

    for (int i = 0; i < m_channels.size(); ++i) {
        ChannelInfo* pChannelInfo = m_channels[i];
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

const CSAMPLE* EngineMaster::getBoothBuffer() const {
    return m_pBooth;
}

const CSAMPLE* EngineMaster::getHeadphoneBuffer() const {
    return m_pHead;
}

const CSAMPLE* EngineMaster::getSidechainBuffer() const {
    return m_pSidechainMix;
}

void EngineMaster::processChannels(int iBufferSize) {
    m_activeBusChannels[EngineChannel::LEFT].clear();
    m_activeBusChannels[EngineChannel::CENTER].clear();
    m_activeBusChannels[EngineChannel::RIGHT].clear();
    m_activeHeadphoneChannels.clear();
    m_activeTalkoverChannels.clear();
    m_activeChannels.clear();

    ScopedTimer timer("EngineMaster::processChannels");
    EngineChannel* pMasterChannel = m_pMasterSync->getMaster();
    // Reserve the first place for the master channel which
    // should be processed first
    m_activeChannels.append(NULL);
    int activeChannelsStartIndex = 1; // Nothing at 0 yet
    for (int i = 0; i < m_channels.size(); ++i) {
        ChannelInfo* pChannelInfo = m_channels[i];
        EngineChannel* pChannel = pChannelInfo->m_pChannel;

        // Skip inactive channels.
        if (!pChannel || !pChannel->isActive()) {
            continue;
        }

        if (pChannel->isTalkoverEnabled() &&
                !pChannelInfo->m_pMuteControl->toBool()) {
            // talkover is an exclusive channel
            // once talkover is enabled it is not used in
            // xFader-Mix
            m_activeTalkoverChannels.append(pChannelInfo);

            // Check if we need to fade out the master channel
            GainCache& gainCache = m_channelMasterGainCache[i];
            if (gainCache.m_gain) {
                gainCache.m_fadeout = true;
                m_activeBusChannels[pChannel->getOrientation()].append(pChannelInfo);
             }
        } else {
            // Check if we need to fade out the channel
            GainCache& gainCache = m_channelTalkoverGainCache[i];
            if (gainCache.m_gain) {
                gainCache.m_fadeout = true;
                m_activeTalkoverChannels.append(pChannelInfo);
            }
            if (pChannel->isMasterEnabled() &&
                    !pChannelInfo->m_pMuteControl->toBool()) {
                // the xFader-Mix
                m_activeBusChannels[pChannel->getOrientation()].append(pChannelInfo);
            } else {
                // Check if we need to fade out the channel
                GainCache& gainCache = m_channelMasterGainCache[i];
                if (gainCache.m_gain) {
                    gainCache.m_fadeout = true;
                    m_activeBusChannels[pChannel->getOrientation()].append(pChannelInfo);
                }
            }
        }

        // If the channel is enabled for previewing in headphones, copy it
        // over to the headphone buffer
        if (pChannel->isPflEnabled()) {
            m_activeHeadphoneChannels.append(pChannelInfo);
        } else {
            // Check if we need to fade out the channel
            GainCache& gainCache = m_channelHeadphoneGainCache[i];
            if (gainCache.m_gain) {
                m_channelHeadphoneGainCache[i].m_fadeout = true;
                m_activeHeadphoneChannels.append(pChannelInfo);
            }
        }

        // If necessary, add the channel to the list of buffers to process.
        if (pChannel == pMasterChannel) {
            // If this is the sync master, it should be processed first.
            m_activeChannels.replace(0, pChannelInfo);
            activeChannelsStartIndex = 0;
        } else {
            m_activeChannels.append(pChannelInfo);
        }
    }

    // Now that the list is built and ordered, do the processing.
    for (int i = activeChannelsStartIndex;
             i < m_activeChannels.size(); ++i) {
        ChannelInfo* pChannelInfo = m_activeChannels[i];
        EngineChannel* pChannel = pChannelInfo->m_pChannel;
        pChannel->process(pChannelInfo->m_pBuffer, iBufferSize);

        // Collect metadata for effects
        if (m_pEngineEffectsManager) {
            GroupFeatureState features;
            pChannel->collectFeatures(&features);
            pChannelInfo->m_features = features;
        }
    }

    // After all the engines have been processed, trigger post-processing
    // which ensures that all channels are updating certain values at the
    // same point in time.  This prevents sync from failing depending on
    // if the sync target was processed before or after the sync origin.
    for (int i = activeChannelsStartIndex;
            i < m_activeChannels.size(); ++i) {
        m_activeChannels[i]->m_pChannel->postProcess(iBufferSize);
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
    bool boothEnabled = m_pBoothEnabled->get();
    bool headphoneEnabled = m_pHeadphoneEnabled->get();

    m_iSampleRate = static_cast<int>(m_pMasterSampleRate->get());
    m_iBufferSize = iBufferSize;
    // TODO: remove assumption of stereo buffer
    const unsigned int kChannels = 2;
    const unsigned int iFrames = iBufferSize / kChannels;

    if (m_pEngineEffectsManager) {
        m_pEngineEffectsManager->onCallbackStart();
    }

    // Update internal master sync rate.
    m_pMasterSync->onCallbackStart(m_iSampleRate, m_iBufferSize);
    // Prepare each channel for output
    processChannels(m_iBufferSize);
    // Do internal master sync post-processing
    m_pMasterSync->onCallbackEnd(m_iSampleRate, m_iBufferSize);

    // Compute headphone mix
    // Head phone left/right mix
    CSAMPLE pflMixGainInHeadphones = 1;
    CSAMPLE masterMixGainInHeadphones = 0;
    if (masterEnabled) {
        CSAMPLE cf_val = m_pHeadMix->get();
        pflMixGainInHeadphones = 0.5 * (-cf_val + 1.);
        masterMixGainInHeadphones = 0.5 * (cf_val + 1.);
        // qDebug() << "head val " << cf_val << ", head " << chead_gain
        //          << ", master " << cmaster_gain;
    }

    // Mix all the PFL enabled channels together.
    m_headphoneGain.setGain(pflMixGainInHeadphones);

    if (headphoneEnabled) {
        // Process effects and mix PFL channels together for the headphones.
        // Effects will be reprocessed post-fader for the crossfader busses
        // and master mix, so the channel input buffers cannot be modified here.
        ChannelMixer::applyEffectsAndMixChannels(
            m_headphoneGain, &m_activeHeadphoneChannels,
            &m_channelHeadphoneGainCache,
            m_pHead, m_headphoneHandle.handle(),
            m_iBufferSize, m_iSampleRate,
            m_pEngineEffectsManager);

        // Process headphone channel effects
        if (m_pEngineEffectsManager) {
            GroupFeatureState headphoneFeatures;
            // If there is only one channel in the headphone mix, use its features
            // for effects processing. This allows for previewing how an effect will
            // sound on a playing deck before turning up the dry/wet knob to make it
            // audible on the master mix. Without this, the effect would sound different
            // in headphones than how it would sound if it was enabled on the deck,
            // for example with tempo synced effects.
            if (m_activeHeadphoneChannels.size() == 1) {
                headphoneFeatures = m_activeHeadphoneChannels.at(0)->m_features;
            }
            m_pEngineEffectsManager->processPostFaderInPlace(
                m_headphoneHandle.handle(),
                m_headphoneHandle.handle(),
                m_pHead,
                m_iBufferSize, m_iSampleRate,
                headphoneFeatures);
        }
    }

    // Mix all the talkover enabled channels together.
    // Effects processing is done in place to avoid unnecessary buffer copying.
    ChannelMixer::applyEffectsInPlaceAndMixChannels(
            m_talkoverGain, &m_activeTalkoverChannels,
            &m_channelTalkoverGainCache,
            m_pTalkover, m_masterHandle.handle(),
            m_iBufferSize, m_iSampleRate, m_pEngineEffectsManager);

    // Process effects on all microphones mixed together
    // We have no metadata for mixed effect buses, so use an empty GroupFeatureState.
    GroupFeatureState busFeatures;
    m_pEngineEffectsManager->processPostFaderInPlace(
            m_busTalkoverHandle.handle(),
            m_masterHandle.handle(),
            m_pTalkover,
            m_iBufferSize, m_iSampleRate, busFeatures);


    // Clear talkover compressor for the next round of gain calculation.
    m_pTalkoverDucking->clearKeys();
    if (m_pTalkoverDucking->getMode() != EngineTalkoverDucking::OFF) {
        m_pTalkoverDucking->processKey(m_pTalkover, m_iBufferSize);
    }

    // Calculate the crossfader gains for left and right side of the crossfader
    double crossfaderLeftGain, crossfaderRightGain;
    EngineXfader::getXfadeGains(m_pCrossfader->get(), m_pXFaderCurve->get(),
                                m_pXFaderCalibration->get(),
                                m_pXFaderMode->get(),
                                m_pXFaderReverse->toBool(),
                                &crossfaderLeftGain, &crossfaderRightGain);

    // Make the mix for each crossfader orientation output bus.
    // m_masterGain takes care of applying the attenuation from
    // channel volume faders, crossfader, and talkover ducking.
    // Talkover is mixed in later according to the configured MicMonitorMode
    m_masterGain.setGains(crossfaderLeftGain, 1.0, crossfaderRightGain,
                            m_pTalkoverDucking->getGain(m_iBufferSize / 2));

    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        ChannelMixer::applyEffectsInPlaceAndMixChannels(
            m_masterGain,
            &m_activeBusChannels[o],
            &m_channelMasterGainCache, // no [o] because the old gain follows an orientation switch
            m_pOutputBusBuffers[o], m_masterHandle.handle(),
            m_iBufferSize, m_iSampleRate, m_pEngineEffectsManager);
    }

    // Process crossfader orientation bus channel effects
    if (m_pEngineEffectsManager) {
        m_pEngineEffectsManager->processPostFaderInPlace(
            m_busCrossfaderLeftHandle.handle(),
            m_masterHandle.handle(),
            m_pOutputBusBuffers[EngineChannel::LEFT],
            m_iBufferSize, m_iSampleRate, busFeatures);
        m_pEngineEffectsManager->processPostFaderInPlace(
            m_busCrossfaderCenterHandle.handle(),
            m_masterHandle.handle(),
            m_pOutputBusBuffers[EngineChannel::CENTER],
            m_iBufferSize, m_iSampleRate, busFeatures);
        m_pEngineEffectsManager->processPostFaderInPlace(
            m_busCrossfaderRightHandle.handle(),
            m_masterHandle.handle(),
            m_pOutputBusBuffers[EngineChannel::RIGHT],
            m_iBufferSize, m_iSampleRate, busFeatures);
    }

    if (masterEnabled) {
        // Mix the crossfader orientation buffers together into the master mix
        SampleUtil::copy3WithGain(m_pMaster,
            m_pOutputBusBuffers[EngineChannel::LEFT], 1.0,
            m_pOutputBusBuffers[EngineChannel::CENTER], 1.0,
            m_pOutputBusBuffers[EngineChannel::RIGHT], 1.0,
            m_iBufferSize);

        MicMonitorMode configuredMicMonitorMode = static_cast<MicMonitorMode>(
            static_cast<int>(m_pMicMonitorMode->get()));

        // Process master, booth, and record/broadcast buffers according to the
        // MicMonitorMode configured in DlgPrefSound
        // TODO(Be): make SampleUtil ramping functions update the old gain variable
        if (configuredMicMonitorMode == MicMonitorMode::MASTER) {
            // Process master channel effects
            // TODO(Be): Move this after mixing in talkover. To apply master effects
            // to both the master and booth in that case will require refactoring
            // the effects system to be able to process the same effects on multiple
            // buffers within the same callback.
            applyMasterEffects();

            if (headphoneEnabled) {
                processHeadphones(masterMixGainInHeadphones);
            }

            // Copy master mix to booth output with booth gain before mixing
            // talkover with master mix
            if (boothEnabled) {
                CSAMPLE boothGain = m_pBoothGain->get();
                SampleUtil::copyWithRampingGain(m_pBooth, m_pMaster,
                                                m_boothGainOld, boothGain,
                                                m_iBufferSize);
                m_boothGainOld = boothGain;
            }

            // Mix talkover into master mix
            if (m_pNumMicsConfigured->get() > 0) {
                SampleUtil::add(m_pMaster, m_pTalkover, m_iBufferSize);
            }

            // Apply master gain
            CSAMPLE master_gain = m_pMasterGain->get();
            SampleUtil::applyRampingGain(m_pMaster, m_masterGainOld,
                                         master_gain, m_iBufferSize);
            m_masterGainOld = master_gain;

            // Record/broadcast signal is the same as the master output
            if (!m_bExternalRecordBroadcastInputConnected) {
                SampleUtil::copy(m_pSidechainMix, m_pMaster, m_iBufferSize);
            }
        } else if (configuredMicMonitorMode == MicMonitorMode::MASTER_AND_BOOTH) {
            // Process master channel effects
            // TODO(Be): Move this after mixing in talkover. For the MASTER only
            // MicMonitorMode above, that will require refactoring the effects system
            // to be able to process the same effects on different buffers
            // within the same callback. For consistency between the MicMonitorModes,
            // process master effects here before mixing in talkover.
            applyMasterEffects();

            if (headphoneEnabled) {
                processHeadphones(masterMixGainInHeadphones);
            }

            // Mix talkover with master
            if (m_pNumMicsConfigured->get() > 0) {
                SampleUtil::add(m_pMaster, m_pTalkover, m_iBufferSize);
            }

            // Copy master mix (with talkover mixed in) to booth output with booth gain
            if (boothEnabled) {
                CSAMPLE boothGain = m_pBoothGain->get();
                SampleUtil::copyWithRampingGain(m_pBooth, m_pMaster,
                                                m_boothGainOld, boothGain,
                                                m_iBufferSize);
                m_boothGainOld = boothGain;
            }

            // Apply master gain
            CSAMPLE master_gain = m_pMasterGain->get();
            SampleUtil::applyRampingGain(m_pMaster, m_masterGainOld,
                                         master_gain, m_iBufferSize);
            m_masterGainOld = master_gain;

            // Record/broadcast signal is the same as the master output
            if (!m_bExternalRecordBroadcastInputConnected) {
                SampleUtil::copy(m_pSidechainMix, m_pMaster, m_iBufferSize);
            }
        } else if (configuredMicMonitorMode == MicMonitorMode::DIRECT_MONITOR) {
            // Skip mixing talkover with the master and booth outputs
            // if using direct monitoring because it is being mixed in hardware
            // without the latency of sending the signal into Mixxx for processing.
            // However, include the talkover mix in the record/broadcast signal.

            // Copy master mix to booth output with booth gain
            if (boothEnabled) {
                CSAMPLE boothGain = m_pBoothGain->get();
                SampleUtil::copyWithRampingGain(m_pBooth, m_pMaster,
                                                m_boothGainOld, boothGain,
                                                m_iBufferSize);
                m_boothGainOld = boothGain;
            }

            // Process master channel effects
            // NOTE(Be): This should occur before mixing in talkover for the
            // record/broadcast signal so the record/broadcast signal is the same
            // as what is heard on the master & booth outputs.
            applyMasterEffects();

            if (headphoneEnabled) {
                processHeadphones(masterMixGainInHeadphones);
            }

            // Apply master gain
            CSAMPLE master_gain = m_pMasterGain->get();
            SampleUtil::applyRampingGain(m_pMaster, m_masterGainOld,
                                         master_gain, m_iBufferSize);
            m_masterGainOld = master_gain;
            if (!m_bExternalRecordBroadcastInputConnected) {
                SampleUtil::copy(m_pSidechainMix, m_pMaster, m_iBufferSize);
            }

            // The talkover signal Mixxx receives is delayed by the round trip latency.
            // There is an output latency between the time Mixxx processes the audio
            // and the user hears it. So if the microphone user plays on beat with
            // what they hear, they will be playing out of sync with the engine's
            // processing by the output latency. Additionally, Mixxx gets input signals
            // delayed by the input latency. By the time Mixxx receives the input signal,
            // a full round trip through the signal chain has elapsed since Mixxx
            // processed the output signal.
            // Although Mixxx receives the input signal delayed, the user hears it mixed
            // in hardware with the master & booth outputs without that
            // latency, so to record/broadcast the same signal that is heard
            // on the master & booth outputs, the master mix must be delayed before
            // mixing the talkover signal for the record/broadcast mix.
            // If not using microphone inputs or recording/broadcasting from
            // a sound card input, skip unnecessary processing here.
            if (m_pNumMicsConfigured->get() > 0
                && !m_bExternalRecordBroadcastInputConnected) {
                // Copy the master mix to a separate buffer before delaying it
                // to avoid delaying the master output.
                m_pLatencyCompensationDelay->process(m_pSidechainMix, m_iBufferSize);
                SampleUtil::add(m_pSidechainMix, m_pTalkover, m_iBufferSize);
            }
        }

        // Submit buffer to the side chain to do broadcasting, recording,
        // etc. (CPU intensive non-realtime tasks)
        // If recording/broadcasting from a sound card input,
        // SoundManager will send the input buffer from the sound card to m_pSidechain
        // so skip sending a buffer to m_pSidechain here.
        if (!m_bExternalRecordBroadcastInputConnected
            && m_pEngineSideChain != nullptr) {
            m_pEngineSideChain->writeSamples(m_pSidechainMix, iFrames);
        }

        // Process effects that apply to master hardware output only but not
        // record/broadcast signal
        if (m_pEngineEffectsManager) {
            GroupFeatureState masterFeatures;
            masterFeatures.has_gain = true;
            masterFeatures.gain = m_pMasterGain->get();
            m_pEngineEffectsManager->processPostFaderInPlace(
                    m_masterOutputHandle.handle(),
                    m_masterHandle.handle(),
                    m_pMaster,
                    m_iBufferSize, m_iSampleRate,
                    masterFeatures);
        }

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
        SampleUtil::applyRampingAlternatingGain(m_pMaster, balleft, balright,
                m_balleftOld, m_balrightOld, iBufferSize);

        m_balleftOld = balleft;
        m_balrightOld = balright;

        // Update VU meter (it does not return anything). Needs to be here so that
        // master balance and talkover is reflected in the VU meter.
        if (m_pVumeter != nullptr) {
            m_pVumeter->process(m_pMaster, m_iBufferSize);
        }
    }

    if (m_pMasterMonoMixdown->get()) {
        SampleUtil::mixStereoToMono(m_pMaster, m_pMaster, m_iBufferSize);
    }

    if (masterEnabled) {
        m_pMasterDelay->process(m_pMaster, m_iBufferSize);
    } else {
        SampleUtil::clear(m_pMaster, m_iBufferSize);
    }
    if (headphoneEnabled) {
        m_pHeadDelay->process(m_pHead, m_iBufferSize);
    }
    if (boothEnabled) {
        m_pBoothDelay->process(m_pBooth, m_iBufferSize);
    }

    // We're close to the end of the callback. Wake up the engine worker
    // scheduler so that it runs the workers.
    m_pWorkerScheduler->runWorkers();
}

void EngineMaster::applyMasterEffects() {
    // Apply master effects
    if (m_pEngineEffectsManager) {
        GroupFeatureState masterFeatures;
        masterFeatures.has_gain = true;
        masterFeatures.gain = m_pMasterGain->get();
        m_pEngineEffectsManager->processPostFaderInPlace(m_masterHandle.handle(),
                                                         m_masterHandle.handle(),
                                                         m_pMaster,
                                                         m_iBufferSize, m_iSampleRate,
                                                         masterFeatures);
    }
}

void EngineMaster::processHeadphones(const double masterMixGainInHeadphones) {
    // Add master mix to headphones
    SampleUtil::addWithRampingGain(m_pHead, m_pMaster,
                                   m_headphoneMasterGainOld,
                                   masterMixGainInHeadphones, m_iBufferSize);
    m_headphoneMasterGainOld = masterMixGainInHeadphones;

    // If Head Split is enabled, replace the left channel of the pfl buffer
    // with a mono mix of the headphone buffer, and the right channel of the pfl
    // buffer with a mono mix of the master output buffer.
    if (m_pHeadSplitEnabled->get()) {
        // note: NOT VECTORIZED because of in place copy
        for (unsigned int i = 0; i + 1 < m_iBufferSize; i += 2) {
            m_pHead[i] = (m_pHead[i] + m_pHead[i + 1]) / 2;
            m_pHead[i + 1] = (m_pMaster[i] + m_pMaster[i + 1]) / 2;
        }
    }

    // Apply headphone gain
    CSAMPLE headphoneGain = m_pHeadGain->get();
    SampleUtil::applyRampingGain(m_pHead, m_headphoneGainOld,
                                 headphoneGain, m_iBufferSize);
    m_headphoneGainOld = headphoneGain;
}

void EngineMaster::addChannel(EngineChannel* pChannel) {
    ChannelInfo* pChannelInfo = new ChannelInfo(m_channels.size());
    pChannelInfo->m_pChannel = pChannel;
    const QString& group = pChannel->getGroup();
    pChannelInfo->m_handle = m_pChannelHandleFactory->getOrCreateHandle(group);
    pChannelInfo->m_pVolumeControl = new ControlAudioTaperPot(
            ConfigKey(group, "volume"), -20, 0, 1);
    pChannelInfo->m_pVolumeControl->setDefaultValue(1.0);
    pChannelInfo->m_pVolumeControl->set(1.0);
    pChannelInfo->m_pMuteControl = new ControlPushButton(
            ConfigKey(group, "mute"));
    pChannelInfo->m_pMuteControl->setButtonMode(ControlPushButton::POWERWINDOW);
    pChannelInfo->m_pBuffer = SampleUtil::alloc(MAX_BUFFER_LEN);
    SampleUtil::clear(pChannelInfo->m_pBuffer, MAX_BUFFER_LEN);
    m_channels.append(pChannelInfo);
    const GainCache gainCacheDefault = {0, false};
    m_channelHeadphoneGainCache.append(gainCacheDefault);
    m_channelTalkoverGainCache.append(gainCacheDefault);
    m_channelMasterGainCache.append(gainCacheDefault);

    // Pre-allocate scratch buffers to avoid memory allocation in the
    // callback. QVarLengthArray does nothing if reserve is called with a size
    // smaller than its pre-allocation.
    m_activeChannels.reserve(m_channels.size());
    m_activeBusChannels[EngineChannel::LEFT].reserve(m_channels.size());
    m_activeBusChannels[EngineChannel::CENTER].reserve(m_channels.size());
    m_activeBusChannels[EngineChannel::RIGHT].reserve(m_channels.size());
    m_activeHeadphoneChannels.reserve(m_channels.size());
    m_activeTalkoverChannels.reserve(m_channels.size());

    EngineBuffer* pBuffer = pChannelInfo->m_pChannel->getEngineBuffer();
    if (pBuffer != NULL) {
        pBuffer->bindWorkers(m_pWorkerScheduler);
    }
}

EngineChannel* EngineMaster::getChannel(const QString& group) {
    for (int i = 0; i < m_channels.size(); ++i) {
        ChannelInfo* pChannelInfo = m_channels[i];
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
    for (int i = 0; i < m_channels.size(); ++i) {
        const ChannelInfo* pChannelInfo = m_channels[i];
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
    case AudioOutput::BOOTH:
        return getBoothBuffer();
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
    case AudioOutput::RECORD_BROADCAST:
        return getSidechainBuffer();
        break;
    default:
        return NULL;
    }
}

void EngineMaster::onOutputConnected(AudioOutput output) {
    switch (output.getType()) {
        case AudioOutput::MASTER:
            // overwrite config option if a master output is configured
            m_pMasterEnabled->forceSet(1.0);
            break;
        case AudioOutput::HEADPHONES:
            m_pMasterEnabled->forceSet(1.0);
            m_pHeadphoneEnabled->forceSet(1.0);
            break;
        case AudioOutput::BOOTH:
            m_pMasterEnabled->forceSet(1.0);
            m_pBoothEnabled->forceSet(1.0);
            break;
        case AudioOutput::BUS:
            m_bBusOutputConnected[output.getIndex()] = true;
            break;
        case AudioOutput::DECK:
            // We don't track enabled decks.
            break;
        case AudioOutput::RECORD_BROADCAST:
            // We don't track enabled sidechain.
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
        case AudioOutput::BOOTH:
            m_pBoothEnabled->forceSet(0.0);
            break;
        case AudioOutput::HEADPHONES:
            m_pHeadphoneEnabled->forceSet(0.0);
            break;
        case AudioOutput::BUS:
            m_bBusOutputConnected[output.getIndex()] = false;
            break;
        case AudioOutput::DECK:
            // We don't track enabled decks.
            break;
        case AudioOutput::RECORD_BROADCAST:
            // We don't track enabled sidechain.
            break;
        default:
            break;
    }
}

void EngineMaster::onInputConnected(AudioInput input) {
    switch (input.getType()) {
      case AudioInput::MICROPHONE:
          m_pNumMicsConfigured->set(m_pNumMicsConfigured->get() + 1);
          break;
      case AudioInput::AUXILIARY:
          // We don't track enabled auxiliary inputs.
          break;
      case AudioInput::VINYLCONTROL:
          // We don't track enabled vinyl control inputs.
          break;
      case AudioInput::RECORD_BROADCAST:
          m_bExternalRecordBroadcastInputConnected = true;
          break;
      default:
          break;
    }
}

void EngineMaster::onInputDisconnected(AudioInput input) {
    switch (input.getType()) {
      case AudioInput::MICROPHONE:
          m_pNumMicsConfigured->set(m_pNumMicsConfigured->get() - 1);
          break;
      case AudioInput::AUXILIARY:
          // We don't track enabled auxiliary inputs.
          break;
      case AudioInput::VINYLCONTROL:
          // We don't track enabled vinyl control inputs.
          break;
      case AudioInput::RECORD_BROADCAST:
          m_bExternalRecordBroadcastInputConnected = false;
          break;
      default:
          break;
    }
}

void EngineMaster::registerNonEngineChannelSoundIO(SoundManager* pSoundManager) {
    pSoundManager->registerInput(AudioInput(AudioPath::RECORD_BROADCAST, 0, 2),
                                 m_pEngineSideChain);

    pSoundManager->registerOutput(AudioOutput(AudioOutput::MASTER, 0, 2), this);
    pSoundManager->registerOutput(AudioOutput(AudioOutput::HEADPHONES, 0, 2), this);
    pSoundManager->registerOutput(AudioOutput(AudioOutput::BOOTH, 0, 2), this);
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        pSoundManager->registerOutput(AudioOutput(AudioOutput::BUS, 0, 2, o), this);
    }
    pSoundManager->registerOutput(AudioOutput(AudioOutput::RECORD_BROADCAST, 0, 2), this);
}
