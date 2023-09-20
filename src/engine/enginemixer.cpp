#include "engine/enginemixer.h"

#include <QList>
#include <QPair>
#include <QtDebug>

#include "audio/types.h"
#include "control/controlaudiotaperpot.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "effects/effectsmanager.h"
#include "engine/channelmixer.h"
#include "engine/channels/enginechannel.h"
#include "engine/channels/enginedeck.h"
#include "engine/effects/engineeffectsmanager.h"
#include "engine/enginebuffer.h"
#include "engine/enginedelay.h"
#include "engine/enginetalkoverducking.h"
#include "engine/enginevumeter.h"
#include "engine/engineworkerscheduler.h"
#include "engine/enginexfader.h"
#include "engine/sidechain/enginesidechain.h"
#include "engine/sync/enginesync.h"
#include "mixer/playermanager.h"
#include "moc_enginemixer.cpp"
#include "preferences/usersettings.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/timer.h"
#include "util/trace.h"

namespace {
const QString kAppGroup = QStringLiteral("[App]");
} // namespace

EngineMixer::EngineMixer(
        UserSettingsPointer pConfig,
        const QString& group,
        EffectsManager* pEffectsManager,
        ChannelHandleFactoryPointer pChannelHandleFactory,
        bool bEnableSidechain)
        : m_pChannelHandleFactory(pChannelHandleFactory),
          m_pEngineEffectsManager(pEffectsManager->getEngineEffectsManager()),
          m_mainGainOld(0.0),
          m_boothGainOld(0.0),
          m_headphoneMainGainOld(0.0),
          m_headphoneGainOld(1.0),
          m_balleftOld(1.0),
          m_balrightOld(1.0),
          m_numMicsConfigured(0),
          m_mainHandle(registerChannelGroup(group)),
          m_headphoneHandle(registerChannelGroup("[Headphone]")),
          m_mainOutputHandle(registerChannelGroup("[MasterOutput]")),
          m_busTalkoverHandle(registerChannelGroup("[BusTalkover]")),
          m_busCrossfaderLeftHandle(registerChannelGroup("[BusLeft]")),
          m_busCrossfaderCenterHandle(registerChannelGroup("[BusCenter]")),
          m_busCrossfaderRightHandle(registerChannelGroup("[BusRight]")) {
    pEffectsManager->registerInputChannel(m_mainHandle);
    pEffectsManager->registerInputChannel(m_headphoneHandle);
    pEffectsManager->registerOutputChannel(m_mainHandle);
    pEffectsManager->registerOutputChannel(m_headphoneHandle);

    pEffectsManager->registerInputChannel(m_mainOutputHandle);
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

    // Main sample rate
    m_pMainSampleRate = new ControlObject(
            ConfigKey(kAppGroup, QStringLiteral("samplerate")), true, true);
    m_pMainSampleRate->addAlias(ConfigKey(group, QStringLiteral("samplerate")));
    m_pMainSampleRate->set(44100.);

    // Latency control
    m_pMainLatency = new ControlObject(ConfigKey(group, "latency"),
            true,
            true); // reported latency (sometimes correct)
    m_pAudioLatencyOverloadCount = new ControlObject(ConfigKey(group, "audio_latency_overload_count"), true, true);
    m_pAudioLatencyUsage = new ControlPotmeter(ConfigKey(group, "audio_latency_usage"), 0.0, 0.25);
    m_pAudioLatencyOverload  = new ControlPotmeter(ConfigKey(group, "audio_latency_overload"), 0.0, 1.0);

    // Sync controller
    m_pEngineSync = new EngineSync(pConfig);

    // The last-used bpm value is saved in the destructor of EngineSync.
    double default_bpm = pConfig->getValue(
            ConfigKey("[InternalClock]", "bpm"), 124.0);
    ControlObject::set(ConfigKey("[InternalClock]", "bpm"), default_bpm);

    // Crossfader
    m_pCrossfader = new ControlPotmeter(ConfigKey(group, "crossfader"), -1., 1.);

    // Balance
    m_pBalance = new ControlPotmeter(ConfigKey(group, "balance"), -1., 1.);

    // Main gain
    m_pMainGain = new ControlAudioTaperPot(ConfigKey(group, "gain"), -14, 14, 0.5);

    // Booth gain
    m_pBoothGain = new ControlAudioTaperPot(ConfigKey(group, "booth_gain"), -14, 14, 0.5);

    // Legacy: the main "gain" control used to be named "volume" in Mixxx
    // 1.11.0 and earlier. See issue #7413.
    m_pMainGain->addAlias(ConfigKey(group, QStringLiteral("volume")));

    // VU meter:
    m_pVumeter = new EngineVuMeter(group);

    m_pMainDelay = new EngineDelay(group, ConfigKey(group, "delay"));
    m_pHeadDelay = new EngineDelay(group, ConfigKey(group, "headDelay"));
    m_pBoothDelay = new EngineDelay(group, ConfigKey(group, "boothDelay"));
    m_pLatencyCompensationDelay = new EngineDelay(group,
        ConfigKey(group, "microphoneLatencyCompensation"));

    // Headphone volume
    m_pHeadGain = new ControlAudioTaperPot(ConfigKey(group, "headGain"), -14, 14, 0.5);

    // Legacy: the headphone "headGain" control used to be named "headVolume" in
    // Mixxx 1.11.0 and earlier. See issue #7413.
    m_pHeadGain->addAlias(ConfigKey(group, QStringLiteral("headVolume")));

    // Headphone mix (left/right)
    m_pHeadMix = new ControlPotmeter(ConfigKey(group, "headMix"),-1.,1.);
    m_pHeadMix->setDefaultValue(-1.);
    m_pHeadMix->set(-1.);

    // Main / Headphone split-out mode (for devices with only one output).
    m_pHeadSplitEnabled = new ControlPushButton(ConfigKey(group, "headSplit"));
    m_pHeadSplitEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pHeadSplitEnabled->set(0.0);

    m_pTalkoverDucking = new EngineTalkoverDucking(pConfig, group);

    // Allocate buffers
    m_pHead = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pMain = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pBooth = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pTalkover = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pTalkoverHeadphones = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pSidechainMix = SampleUtil::alloc(MAX_BUFFER_LEN);
    SampleUtil::clear(m_pHead, MAX_BUFFER_LEN);
    SampleUtil::clear(m_pMain, MAX_BUFFER_LEN);
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
    m_pEngineSideChain =
            bEnableSidechain ?
                    new EngineSideChain(pConfig, m_pSidechainMix) : nullptr;

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

    m_pKeylockEngine = new ControlObject(ConfigKey(kAppGroup, QStringLiteral("keylock_engine")));
    m_pKeylockEngine->set(static_cast<double>(
            pConfig->getValue(ConfigKey(group, "keylock_engine"),
                    EngineBuffer::defaultKeylockEngine())));

    // TODO: Make this read only and make EngineMixer decide whether
    // processing the main mix is necessary.
    m_pMainEnabled = new ControlObject(ConfigKey(group, "enabled"),
            true,
            false,
            true); // persist = true
    m_pBoothEnabled = new ControlObject(ConfigKey(group, "booth_enabled"));
    m_pBoothEnabled->setReadOnly();
    m_pMainMonoMixdown = new ControlObject(ConfigKey(group, "mono_mixdown"),
            true,
            false,
            true); // persist = true
    m_pMicMonitorMode = new ControlObject(ConfigKey(group, "talkover_mix"),
            true, false, true);  // persist = true
    m_pHeadphoneEnabled = new ControlObject(ConfigKey(group, "headEnabled"));
    m_pHeadphoneEnabled->setReadOnly();

    // Note: the EQ Rack is set in EffectsManager::setupDefaults();
}

EngineMixer::~EngineMixer() {
    // qDebug() << "in ~EngineMixer()";
    delete m_pKeylockEngine;
    delete m_pCrossfader;
    delete m_pBalance;
    delete m_pHeadMix;
    delete m_pHeadSplitEnabled;
    delete m_pMainGain;
    delete m_pBoothGain;
    delete m_pHeadGain;
    delete m_pTalkoverDucking;
    delete m_pVumeter;
    delete m_pEngineSideChain;
    delete m_pMainDelay;
    delete m_pHeadDelay;
    delete m_pBoothDelay;
    delete m_pLatencyCompensationDelay;

    delete m_pXFaderReverse;
    delete m_pXFaderCalibration;
    delete m_pXFaderCurve;
    delete m_pXFaderMode;

    delete m_pEngineSync;
    delete m_pMainSampleRate;
    delete m_pMainLatency;
    delete m_pAudioLatencyOverloadCount;
    delete m_pAudioLatencyUsage;
    delete m_pAudioLatencyOverload;

    delete m_pMainEnabled;
    delete m_pBoothEnabled;
    delete m_pMainMonoMixdown;
    delete m_pMicMonitorMode;
    delete m_pHeadphoneEnabled;

    SampleUtil::free(m_pHead);
    SampleUtil::free(m_pMain);
    SampleUtil::free(m_pBooth);
    SampleUtil::free(m_pTalkover);
    SampleUtil::free(m_pTalkoverHeadphones);
    SampleUtil::free(m_pSidechainMix);
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

const CSAMPLE* EngineMixer::getMainBuffer() const {
    return m_pMain;
}

const CSAMPLE* EngineMixer::getBoothBuffer() const {
    return m_pBooth;
}

const CSAMPLE* EngineMixer::getHeadphoneBuffer() const {
    return m_pHead;
}

const CSAMPLE* EngineMixer::getSidechainBuffer() const {
    return m_pSidechainMix;
}

void EngineMixer::processChannels(int iBufferSize) {
    // Update internal sync lock rate.
    m_pEngineSync->onCallbackStart(m_sampleRate, iBufferSize);

    m_activeBusChannels[EngineChannel::LEFT].clear();
    m_activeBusChannels[EngineChannel::CENTER].clear();
    m_activeBusChannels[EngineChannel::RIGHT].clear();
    m_activeHeadphoneChannels.clear();
    m_activeTalkoverChannels.clear();
    m_activeChannels.clear();

    // ScopedTimer timer(u"EngineMixer::processChannels");
    EngineChannel* pLeaderChannel = m_pEngineSync->getLeaderChannel();
    // Reserve the first place for the main channel which
    // should be processed first
    m_activeChannels.append(NULL);
    int activeChannelsStartIndex = 1; // Nothing at 0 yet
    for (int i = 0; i < m_channels.size(); ++i) {
        ChannelInfo* pChannelInfo = m_channels[i];
        EngineChannel* pChannel = pChannelInfo->m_pChannel;

        // Skip inactive channels.
        VERIFY_OR_DEBUG_ASSERT(pChannel) {
            continue;
        }

        EngineChannel::ActiveState activeState = pChannel->updateActiveState();
        if (activeState == EngineChannel::ActiveState::Inactive) {
            continue;
        }

        if (pChannel->isTalkoverEnabled() &&
                !pChannelInfo->m_pMuteControl->toBool()) {
            // talkover is an exclusive channel
            // once talkover is enabled it is not used in
            // xFader-Mix
            m_activeTalkoverChannels.append(pChannelInfo);

            // Check if we need to fade out the main channel
            GainCache& gainCache = m_channelMainGainCache[i];
            if (gainCache.m_gain != 0) {
                gainCache.m_fadeout = true;
                m_activeBusChannels[pChannel->getOrientation()].append(pChannelInfo);
            }
        } else {
            // Check if we need to fade out the channel
            GainCache& gainCache = m_channelTalkoverGainCache[i];
            if (gainCache.m_gain != 0) {
                gainCache.m_fadeout = true;
                m_activeTalkoverChannels.append(pChannelInfo);
            }
            if (pChannel->isMainMixEnabled() &&
                    !pChannelInfo->m_pMuteControl->toBool()) {
                // the xFader-Mix
                m_activeBusChannels[pChannel->getOrientation()].append(pChannelInfo);
            } else {
                // Check if we need to fade out the channel
                GainCache& gainCache = m_channelMainGainCache[i];
                if (gainCache.m_gain != 0) {
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
            if (gainCache.m_gain != 0) {
                m_channelHeadphoneGainCache[i].m_fadeout = true;
                m_activeHeadphoneChannels.append(pChannelInfo);
            }
        }

        // If necessary, add the channel to the list of buffers to process.
        if (pChannel == pLeaderChannel) {
            // If this is the sync leader, it should be processed first.
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

    // Do internal sync lock post-processing before the other
    // channels.
    // Note, because we call this on the internal clock first,
    // it will have an up-to-date beatDistance, whereas the other
    // Syncables will not.
    m_pEngineSync->onCallbackEnd(m_sampleRate, iBufferSize);

    // After all the engines have been processed, trigger post-processing
    // which ensures that all channels are updating certain values at the
    // same point in time.  This prevents sync from failing depending on
    // if the sync target was processed before or after the sync origin.
    for (int i = activeChannelsStartIndex;
            i < m_activeChannels.size(); ++i) {
        m_activeChannels[i]->m_pChannel->postProcess(iBufferSize);
    }
}

void EngineMixer::process(const int iBufferSize) {
    static bool haveSetName = false;
    if (!haveSetName) {
        QThread::currentThread()->setObjectName("Engine");
        haveSetName = true;
    }
    // Trace t("EngineMixer::process");

    bool mainEnabled = m_pMainEnabled->toBool();
    bool boothEnabled = m_pBoothEnabled->toBool();
    bool headphoneEnabled = m_pHeadphoneEnabled->toBool();

    m_sampleRate = mixxx::audio::SampleRate::fromDouble(m_pMainSampleRate->get());
    // TODO: remove assumption of stereo buffer
    constexpr unsigned int kChannels = 2;
    const unsigned int iFrames = iBufferSize / kChannels;

    if (m_pEngineEffectsManager) {
        m_pEngineEffectsManager->onCallbackStart();
    }

    // Prepare all channels for output
    processChannels(iBufferSize);

    // Compute headphone mix
    // Head phone left/right mix
    CSAMPLE pflMixGainInHeadphones = 1;
    CSAMPLE mainMixGainInHeadphones = 0;
    if (mainEnabled) {
        const auto cf_val = static_cast<CSAMPLE_GAIN>(m_pHeadMix->get());
        pflMixGainInHeadphones = 0.5f * (-cf_val + 1.0f);
        mainMixGainInHeadphones = 0.5f * (cf_val + 1.0f);
        // qDebug() << "head val " << cf_val << ", head " << chead_gain
        //          << ", main " << mainGain;
    }

    // Mix all the PFL enabled channels together.
    m_headphoneGain.setGain(pflMixGainInHeadphones);

    if (headphoneEnabled) {
        // Process effects and mix PFL channels together for the headphones.
        // Effects will be reprocessed post-fader for the crossfader buses
        // and main mix, so the channel input buffers cannot be modified here.
        ChannelMixer::applyEffectsAndMixChannels(
                m_headphoneGain,
                m_activeHeadphoneChannels,
                &m_channelHeadphoneGainCache,
                m_pHead,
                m_headphoneHandle.handle(),
                iBufferSize,
                m_sampleRate,
                m_pEngineEffectsManager);

        // Process headphone channel effects
        if (m_pEngineEffectsManager) {
            GroupFeatureState headphoneFeatures;
            // If there is only one channel in the headphone mix, use its features
            // for effects processing. This allows for previewing how an effect will
            // sound on a playing deck before turning up the dry/wet knob to make it
            // audible on the main mix. Without this, the effect would sound different
            // in headphones than how it would sound if it was enabled on the deck,
            // for example with tempo synced effects.
            if (m_activeHeadphoneChannels.size() == 1) {
                headphoneFeatures = m_activeHeadphoneChannels.at(0)->m_features;
            }
            m_pEngineEffectsManager->processPostFaderInPlace(
                    m_headphoneHandle.handle(),
                    m_headphoneHandle.handle(),
                    m_pHead,
                    iBufferSize,
                    m_sampleRate,
                    headphoneFeatures);
        }
    }

    // Mix all the talkover enabled channels together.
    // Effects processing is done in place to avoid unnecessary buffer copying.
    ChannelMixer::applyEffectsInPlaceAndMixChannels(
            m_talkoverGain,
            m_activeTalkoverChannels,
            &m_channelTalkoverGainCache,
            m_pTalkover,
            m_mainHandle.handle(),
            iBufferSize,
            m_sampleRate,
            m_pEngineEffectsManager);

    // Process effects on all microphones mixed together
    // We have no metadata for mixed effect buses, so use an empty GroupFeatureState.
    GroupFeatureState busFeatures;
    if (m_pEngineEffectsManager) {
        m_pEngineEffectsManager->processPostFaderInPlace(
                m_busTalkoverHandle.handle(),
                m_mainHandle.handle(),
                m_pTalkover,
                iBufferSize,
                m_sampleRate,
                busFeatures,
                CSAMPLE_GAIN_ONE,
                CSAMPLE_GAIN_ONE,
                false);
    }

    switch (m_pTalkoverDucking->getMode()) {
    case EngineTalkoverDucking::OFF:
        m_pTalkoverDucking->setAboveThreshold(false);
        break;
    case EngineTalkoverDucking::AUTO:
        m_pTalkoverDucking->processKey(m_pTalkover, iBufferSize);
        break;
    case EngineTalkoverDucking::MANUAL:
        m_pTalkoverDucking->setAboveThreshold(m_activeTalkoverChannels.size());
        break;
    default:
        DEBUG_ASSERT("!Unknown Ducking mode");
        m_pTalkoverDucking->setAboveThreshold(false);
        break;
    }

    // Calculate the crossfader gains for left and right side of the crossfader
    CSAMPLE_GAIN crossfaderLeftGain, crossfaderRightGain;
    EngineXfader::getXfadeGains(m_pCrossfader->get(), m_pXFaderCurve->get(),
                                m_pXFaderCalibration->get(),
                                m_pXFaderMode->get(),
                                m_pXFaderReverse->toBool(),
                                &crossfaderLeftGain, &crossfaderRightGain);

    // Make the mix for each crossfader orientation output bus.
    // m_mainGain takes care of applying the attenuation from
    // channel volume faders, crossfader, and talkover ducking.
    // Talkover is mixed in later according to the configured MicMonitorMode
    m_mainGain.setGains(crossfaderLeftGain,
            1.0f,
            crossfaderRightGain,
            m_pTalkoverDucking->getGain(iFrames));

    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        ChannelMixer::applyEffectsInPlaceAndMixChannels(m_mainGain,
                m_activeBusChannels[o],
                &m_channelMainGainCache, // no [o] because the old gain
                                         // follows an orientation switch
                m_pOutputBusBuffers[o],
                m_mainHandle.handle(),
                iBufferSize,
                m_sampleRate,
                m_pEngineEffectsManager);
    }

    // Process crossfader orientation bus channel effects
    if (m_pEngineEffectsManager) {
        m_pEngineEffectsManager->processPostFaderInPlace(
                m_busCrossfaderLeftHandle.handle(),
                m_mainHandle.handle(),
                m_pOutputBusBuffers[EngineChannel::LEFT],
                iBufferSize,
                m_sampleRate,
                busFeatures,
                CSAMPLE_GAIN_ONE,
                CSAMPLE_GAIN_ONE,
                false);
        m_pEngineEffectsManager->processPostFaderInPlace(
                m_busCrossfaderCenterHandle.handle(),
                m_mainHandle.handle(),
                m_pOutputBusBuffers[EngineChannel::CENTER],
                iBufferSize,
                m_sampleRate,
                busFeatures,
                CSAMPLE_GAIN_ONE,
                CSAMPLE_GAIN_ONE,
                false);
        m_pEngineEffectsManager->processPostFaderInPlace(
                m_busCrossfaderRightHandle.handle(),
                m_mainHandle.handle(),
                m_pOutputBusBuffers[EngineChannel::RIGHT],
                iBufferSize,
                m_sampleRate,
                busFeatures,
                CSAMPLE_GAIN_ONE,
                CSAMPLE_GAIN_ONE,
                false);
    }

    if (mainEnabled) {
        // Mix the crossfader orientation buffers together into the main mix
        SampleUtil::copy3WithGain(m_pMain,
                m_pOutputBusBuffers[EngineChannel::LEFT],
                1.0,
                m_pOutputBusBuffers[EngineChannel::CENTER],
                1.0,
                m_pOutputBusBuffers[EngineChannel::RIGHT],
                1.0,
                iBufferSize);

        MicMonitorMode configuredMicMonitorMode = static_cast<MicMonitorMode>(
            static_cast<int>(m_pMicMonitorMode->get()));

        // Process main, booth, and record/broadcast buffers according to the
        // MicMonitorMode configured in DlgPrefSound
        // TODO(Be): make SampleUtil ramping functions update the old gain variable
        if (configuredMicMonitorMode == MicMonitorMode::Main) {
            // Process main channel effects
            // TODO(Be): Move this after mixing in talkover. To apply main effects
            // to both the main and booth in that case will require refactoring
            // the effects system to be able to process the same effects on multiple
            // buffers within the same callback.
            applyMainEffects(iBufferSize);

            if (headphoneEnabled) {
                processHeadphones(mainMixGainInHeadphones, iBufferSize);
            }

            // Copy main mix to booth output with booth gain before mixing
            // talkover with main mix
            if (boothEnabled) {
                CSAMPLE_GAIN boothGain = static_cast<CSAMPLE_GAIN>(m_pBoothGain->get());
                SampleUtil::copyWithRampingGain(
                        m_pBooth,
                        m_pMain,
                        m_boothGainOld,
                        boothGain,
                        iBufferSize);
                m_boothGainOld = boothGain;
            }

            // Mix talkover into main mix
            if (m_numMicsConfigured > 0) {
                SampleUtil::add(m_pMain, m_pTalkover, iBufferSize);
            }

            // Apply main gain
            CSAMPLE_GAIN mainGain = static_cast<CSAMPLE_GAIN>(m_pMainGain->get());
            SampleUtil::applyRampingGain(m_pMain, m_mainGainOld, mainGain, iBufferSize);
            m_mainGainOld = mainGain;

            // Record/broadcast signal is the same as the main output
            if (sidechainMixRequired()) {
                SampleUtil::copy(m_pSidechainMix, m_pMain, iBufferSize);
            }
        } else if (configuredMicMonitorMode == MicMonitorMode::MainAndBooth) {
            // Process main channel effects
            // TODO(Be): Move this after mixing in talkover. For the main output only
            // MicMonitorMode above, that will require refactoring the effects system
            // to be able to process the same effects on different buffers
            // within the same callback. For consistency between the MicMonitorModes,
            // process main effects here before mixing in talkover.
            applyMainEffects(iBufferSize);

            if (headphoneEnabled) {
                processHeadphones(mainMixGainInHeadphones, iBufferSize);
            }

            // Mix talkover with main
            if (m_numMicsConfigured > 0) {
                SampleUtil::add(m_pMain, m_pTalkover, iBufferSize);
            }

            // Copy main mix (with talkover mixed in) to booth output with booth gain
            if (boothEnabled) {
                CSAMPLE_GAIN boothGain = static_cast<CSAMPLE_GAIN>(m_pBoothGain->get());
                SampleUtil::copyWithRampingGain(
                        m_pBooth,
                        m_pMain,
                        m_boothGainOld,
                        boothGain,
                        iBufferSize);
                m_boothGainOld = boothGain;
            }

            // Apply main gain
            CSAMPLE_GAIN mainGain = static_cast<CSAMPLE_GAIN>(m_pMainGain->get());
            SampleUtil::applyRampingGain(
                    m_pMain,
                    m_mainGainOld,
                    mainGain,
                    iBufferSize);
            m_mainGainOld = mainGain;

            // Record/broadcast signal is the same as the main output
            if (sidechainMixRequired()) {
                SampleUtil::copy(m_pSidechainMix, m_pMain, iBufferSize);
            }
        } else if (configuredMicMonitorMode == MicMonitorMode::DirectMonitor) {
            // Skip mixing talkover with the main and booth outputs
            // if using direct monitoring because it is being mixed in hardware
            // without the latency of sending the signal into Mixxx for processing.
            // However, include the talkover mix in the record/broadcast signal.

            // Copy main mix to booth output with booth gain
            if (boothEnabled) {
                CSAMPLE_GAIN boothGain = static_cast<CSAMPLE_GAIN>(m_pBoothGain->get());
                SampleUtil::copyWithRampingGain(
                        m_pBooth,
                        m_pMain,
                        m_boothGainOld,
                        boothGain,
                        iBufferSize);
                m_boothGainOld = boothGain;
            }

            // Process main channel effects
            // NOTE(Be): This should occur before mixing in talkover for the
            // record/broadcast signal so the record/broadcast signal is the same
            // as what is heard on the main & booth outputs.
            applyMainEffects(iBufferSize);

            if (headphoneEnabled) {
                processHeadphones(mainMixGainInHeadphones, iBufferSize);
            }

            // Apply main gain
            CSAMPLE_GAIN mainGain = static_cast<CSAMPLE_GAIN>(m_pMainGain->get());
            SampleUtil::applyRampingGain(
                    m_pMain,
                    m_mainGainOld,
                    mainGain,
                    iBufferSize);
            m_mainGainOld = mainGain;
            if (sidechainMixRequired()) {
                SampleUtil::copy(m_pSidechainMix, m_pMain, iBufferSize);

                if (m_numMicsConfigured > 0) {
                    // The talkover signal Mixxx receives is delayed by the round trip latency.
                    // There is an output latency between the time Mixxx processes the audio
                    // and the user hears it. So if the microphone user plays on beat with
                    // what they hear, they will be playing out of sync with the engine's
                    // processing by the output latency. Additionally, Mixxx gets input signals
                    // delayed by the input latency. By the time Mixxx receives the input signal,
                    // a full round trip through the signal chain has elapsed since Mixxx
                    // processed the output signal.
                    // Although Mixxx receives the input signal delayed, the user hears it mixed
                    // in hardware with the main & booth outputs without that
                    // latency, so to record/broadcast the same signal that is heard
                    // on the main & booth outputs, the main mix must be delayed before
                    // mixing the talkover signal for the record/broadcast mix.
                    // If not using microphone inputs or recording/broadcasting from
                    // a sound card input, skip unnecessary processing here.

                    // Copy the main mix to a separate buffer before delaying it
                    // to avoid delaying the main output.
                    m_pLatencyCompensationDelay->process(m_pSidechainMix, iBufferSize);
                    SampleUtil::add(m_pSidechainMix, m_pTalkover, iBufferSize);
                }
            }
        }

        // Submit buffer to the side chain to do CPU intensive non-realtime
        // tasks like recording. The SoundDeviceNetwork, responsible for
        // passing samples to the network reads directly from m_pSidechainMix,
        // registering it with SoundDevice::addOutput().
        // Note: In case the broadcast/recording input is configured,
        // EngineSideChain::receiveBuffer has copied the input buffer to m_pSidechainMix
        // via before (called by SoundManager::pushInputBuffers())
        if (m_pEngineSideChain) {
            m_pEngineSideChain->writeSamples(m_pSidechainMix, iFrames);
        }

        // Process effects that apply to main hardware output only but not
        // record/broadcast signal
        if (m_pEngineEffectsManager) {
            GroupFeatureState mainFeatures;
            mainFeatures.has_gain = true;
            mainFeatures.gain = m_pMainGain->get();
            m_pEngineEffectsManager->processPostFaderInPlace(
                    m_mainOutputHandle.handle(),
                    m_mainHandle.handle(),
                    m_pMain,
                    iBufferSize,
                    m_sampleRate,
                    mainFeatures);
        }

        // Balance values
        CSAMPLE balright = 1.;
        CSAMPLE balleft = 1.;
        const auto bal = static_cast<CSAMPLE_GAIN>(m_pBalance->get());
        if (bal > 0.) {
            balleft -= bal;
        } else if (bal < 0.) {
            balright += bal;
        }

        // Perform balancing on main out
        SampleUtil::applyRampingAlternatingGain(m_pMain,
                balleft,
                balright,
                m_balleftOld,
                m_balrightOld,
                iBufferSize);

        m_balleftOld = balleft;
        m_balrightOld = balright;

        // Update VU meter (it does not return anything). Needs to be here so that
        // main balance and talkover is reflected in the VU meter.
        if (m_pVumeter != nullptr) {
            m_pVumeter->process(m_pMain, iBufferSize);
        }
    }

    if (m_pMainMonoMixdown->toBool()) {
        SampleUtil::mixStereoToMono(m_pMain, iBufferSize);
    }

    if (mainEnabled) {
        m_pMainDelay->process(m_pMain, iBufferSize);
    } else {
        SampleUtil::clear(m_pMain, iBufferSize);
    }
    if (headphoneEnabled) {
        m_pHeadDelay->process(m_pHead, iBufferSize);
    }
    if (boothEnabled) {
        m_pBoothDelay->process(m_pBooth, iBufferSize);
    }

    // We're close to the end of the callback. Wake up the engine worker
    // scheduler so that it runs the workers.
    m_pWorkerScheduler->runWorkers();
}

void EngineMixer::applyMainEffects(int bufferSize) {
    // Apply main effects
    if (m_pEngineEffectsManager) {
        GroupFeatureState mainFeatures;
        mainFeatures.has_gain = true;
        mainFeatures.gain = m_pMainGain->get();
        m_pEngineEffectsManager->processPostFaderInPlace(m_mainHandle.handle(),
                m_mainHandle.handle(),
                m_pMain,
                bufferSize,
                m_sampleRate,
                mainFeatures,
                CSAMPLE_GAIN_ONE,
                CSAMPLE_GAIN_ONE,
                false);
    }
}

void EngineMixer::processHeadphones(
        const CSAMPLE_GAIN mainMixGainInHeadphones,
        int iBufferSize) {
    // Add main mix to headphones
    SampleUtil::addWithRampingGain(
            m_pHead,
            m_pMain,
            m_headphoneMainGainOld,
            mainMixGainInHeadphones,
            iBufferSize);
    m_headphoneMainGainOld = mainMixGainInHeadphones;

    // If Head Split is enabled, replace the left channel of the pfl buffer
    // with a mono mix of the headphone buffer, and the right channel of the pfl
    // buffer with a mono mix of the main output buffer.
    if (m_pHeadSplitEnabled->toBool()) {
        // note: NOT VECTORIZED because of in place copy
        // with all compilers, except clang >= 14.
        for (SINT i = 0; i + 1 < iBufferSize; i += 2) {
            m_pHead[i] = (m_pHead[i] + m_pHead[i + 1]) / 2;
            m_pHead[i + 1] = (m_pMain[i] + m_pMain[i + 1]) / 2;
        }
    }

    // Apply headphone gain
    CSAMPLE_GAIN headphoneGain = static_cast<CSAMPLE_GAIN>(m_pHeadGain->get());
    SampleUtil::applyRampingGain(
            m_pHead,
            m_headphoneGainOld,
            headphoneGain,
            iBufferSize);
    m_headphoneGainOld = headphoneGain;
}

void EngineMixer::addChannel(EngineChannel* pChannel) {
    ChannelInfo* pChannelInfo = new ChannelInfo(m_channels.size());
    pChannel->setChannelIndex(pChannelInfo->m_index);
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
    constexpr GainCache gainCacheDefault = {0, false};
    m_channelHeadphoneGainCache.append(gainCacheDefault);
    m_channelTalkoverGainCache.append(gainCacheDefault);
    m_channelMainGainCache.append(gainCacheDefault);

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
    if (pBuffer != nullptr) {
        pBuffer->bindWorkers(m_pWorkerScheduler);
    }
}

EngineChannel* EngineMixer::getChannel(const QString& group) {
    for (const ChannelInfo* pChannelInfo : m_channels) {
        if (pChannelInfo->m_pChannel->getGroup() == group) {
            return pChannelInfo->m_pChannel;
        }
    }
    return nullptr;
}

CSAMPLE_GAIN EngineMixer::getMainGain(int channelIndex) const {
    if (channelIndex >= 0 && channelIndex < m_channelMainGainCache.size()) {
        return m_channelMainGainCache[channelIndex].m_gain;
    }
    return CSAMPLE_GAIN_ZERO;
}

const CSAMPLE* EngineMixer::getDeckBuffer(unsigned int i) const {
    return getChannelBuffer(PlayerManager::groupForDeck(i));
}

const CSAMPLE* EngineMixer::getOutputBusBuffer(unsigned int i) const {
    if (i <= EngineChannel::RIGHT) {
        return m_pOutputBusBuffers[i];
    }
    return nullptr;
}

const CSAMPLE* EngineMixer::getChannelBuffer(const QString& group) const {
    for (const ChannelInfo* pChannelInfo : m_channels) {
        if (pChannelInfo->m_pChannel->getGroup() == group) {
            return pChannelInfo->m_pBuffer;
        }
    }
    return nullptr;
}

const CSAMPLE* EngineMixer::buffer(const AudioOutput& output) const {
    switch (output.getType()) {
    case AudioPathType::Main:
        return getMainBuffer();
        break;
    case AudioPathType::Booth:
        return getBoothBuffer();
        break;
    case AudioPathType::Headphones:
        return getHeadphoneBuffer();
        break;
    case AudioPathType::Bus:
        return getOutputBusBuffer(output.getIndex());
        break;
    case AudioPathType::Deck:
        return getDeckBuffer(output.getIndex());
        break;
    case AudioPathType::RecordBroadcast:
        return getSidechainBuffer();
        break;
    default:
        return nullptr;
    }
}

void EngineMixer::onOutputConnected(const AudioOutput& output) {
    switch (output.getType()) {
    case AudioPathType::Main:
        // overwrite config option if a main output is configured
        m_pMainEnabled->forceSet(1.0);
        break;
    case AudioPathType::Headphones:
        m_pMainEnabled->forceSet(1.0);
        m_pHeadphoneEnabled->forceSet(1.0);
        break;
    case AudioPathType::Booth:
        m_pMainEnabled->forceSet(1.0);
        m_pBoothEnabled->forceSet(1.0);
        break;
    case AudioPathType::Bus:
        m_bBusOutputConnected[output.getIndex()] = true;
        break;
    case AudioPathType::Deck:
        // We don't track enabled decks.
        break;
    case AudioPathType::RecordBroadcast:
        // We don't track enabled sidechain.
        break;
    default:
        break;
    }
}

void EngineMixer::onOutputDisconnected(const AudioOutput& output) {
    switch (output.getType()) {
    case AudioPathType::Main:
        // not used, because we need the main buffer for headphone mix
        // and recording/broadcasting as well
        break;
    case AudioPathType::Booth:
        m_pBoothEnabled->forceSet(0.0);
        break;
    case AudioPathType::Headphones:
        m_pHeadphoneEnabled->forceSet(0.0);
        break;
    case AudioPathType::Bus:
        m_bBusOutputConnected[output.getIndex()] = false;
        break;
    case AudioPathType::Deck:
        // We don't track enabled decks.
        break;
    case AudioPathType::RecordBroadcast:
        // We don't track enabled sidechain.
        break;
    default:
        break;
    }
}

void EngineMixer::onInputConnected(const AudioInput& input) {
    switch (input.getType()) {
    case AudioPathType::Microphone:
        m_numMicsConfigured++;
        break;
    case AudioPathType::Auxiliary:
        // We don't track enabled auxiliary inputs.
        break;
    case AudioPathType::VinylControl:
        // We don't track enabled vinyl control inputs.
        break;
    case AudioPathType::RecordBroadcast:
        m_bExternalRecordBroadcastInputConnected = true;
        break;
    default:
        break;
    }
}

void EngineMixer::onInputDisconnected(const AudioInput& input) {
    switch (input.getType()) {
    case AudioPathType::Microphone:
        m_numMicsConfigured--;
        break;
    case AudioPathType::Auxiliary:
        // We don't track enabled auxiliary inputs.
        break;
    case AudioPathType::VinylControl:
        // We don't track enabled vinyl control inputs.
        break;
    case AudioPathType::RecordBroadcast:
        m_bExternalRecordBroadcastInputConnected = false;
        break;
    default:
        break;
    }
}

void EngineMixer::registerNonEngineChannelSoundIO(SoundManager* pSoundManager) {
    pSoundManager->registerInput(AudioInput(AudioPathType::RecordBroadcast,
                                         0,
                                         mixxx::audio::ChannelCount::stereo()),
            m_pEngineSideChain);

    pSoundManager->registerOutput(AudioOutput(AudioPathType::Main,
                                          0,
                                          mixxx::audio::ChannelCount::stereo()),
            this);
    pSoundManager->registerOutput(AudioOutput(AudioPathType::Headphones,
                                          0,
                                          mixxx::audio::ChannelCount::stereo()),
            this);
    pSoundManager->registerOutput(AudioOutput(AudioPathType::Booth,
                                          0,
                                          mixxx::audio::ChannelCount::stereo()),
            this);
    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        pSoundManager->registerOutput(
                AudioOutput(AudioPathType::Bus,
                        0,
                        mixxx::audio::ChannelCount::stereo(),
                        o),
                this);
    }
    pSoundManager->registerOutput(AudioOutput(AudioPathType::RecordBroadcast,
                                          0,
                                          mixxx::audio::ChannelCount::stereo()),
            this);
}

bool EngineMixer::sidechainMixRequired() const {
    return m_pEngineSideChain && !m_bExternalRecordBroadcastInputConnected;
}
