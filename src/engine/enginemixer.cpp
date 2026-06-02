#include "engine/enginemixer.h"

#include <memory>

#include "audio/types.h"
#include "control/controlaudiotaperpot.h"
#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "effects/effectsmanager.h"
#include "engine/channelmixer.h"
#include "engine/channels/enginechannel.h"
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
#include "preferences/configobject.h"
#include "preferences/usersettings.h"
#include "util/defs.h"
#include "util/parented_ptr.h"
#include "util/sample.h"
#include "util/samplebuffer.h"

namespace {
const QString kAppGroup = QStringLiteral("[App]");
const QString kLegacyGroup = QStringLiteral("[Master]");
const QString kMainGroup = QStringLiteral("[Main]");

const ConfigKey kInternalClockBpmKey{QStringLiteral("[InternalClock]"), QStringLiteral("bpm")};
} // namespace

EngineMixer::EngineMixer(UserSettingsPointer pConfig,
        const QString& group,
        EffectsManager* pEffectsManager,
        ChannelHandleFactoryPointer pChannelHandleFactory,
        bool bEnableSidechain)
        : m_main(kMaxEngineSamples),
          // TODO: Make this read only and make EngineMixer decide whether
          // processing the main mix is necessary.
          m_pMainEnabled(std::make_unique<ControlObject>(
                  ConfigKey(group, "enabled"), true, false, true)),
          m_pHeadphoneEnabled(std::make_unique<ControlObject>(
                  ConfigKey(group, "headEnabled"))),
          m_pBoothEnabled(std::make_unique<ControlObject>(
                  ConfigKey(group, "booth_enabled"))),
          m_pChannelHandleFactory(pChannelHandleFactory),
          m_pEngineEffectsManager(pEffectsManager->getEngineEffectsManager()),
          m_outputBusBuffers({mixxx::SampleBuffer(kMaxEngineSamples),
                  mixxx::SampleBuffer(kMaxEngineSamples),
                  mixxx::SampleBuffer(kMaxEngineSamples)}),
          m_booth(kMaxEngineSamples),
          m_head(kMaxEngineSamples),
          m_talkover(kMaxEngineSamples),
          m_talkoverHeadphones(kMaxEngineSamples),
          m_sidechainMix(kMaxEngineSamples),
          m_pWorkerScheduler(make_parented<EngineWorkerScheduler>(this)),
          m_pEngineSync(std::make_unique<EngineSync>(pConfig)),
          m_pMainGain(std::make_unique<ControlAudioTaperPot>(
                  ConfigKey(group, "gain"), -14, 14, 0.5)),
          m_pBoothGain(std::make_unique<ControlAudioTaperPot>(
                  ConfigKey(group, "booth_gain"), -14, 14, 0.5)),
          m_pHeadGain(std::make_unique<ControlAudioTaperPot>(
                  ConfigKey(group, "headGain"), -14, 14, 0.5)),
          m_pSampleRate(std::make_unique<ControlObject>(
                  ConfigKey(kAppGroup, QStringLiteral("samplerate")),
                  true,
                  true)),
          m_pOutputLatencyMs(std::make_unique<ControlObject>(
                  ConfigKey(kAppGroup, QStringLiteral("output_latency_ms")),
                  true,
                  true)),
          m_pAudioLatencyOverloadCount(
                  std::make_unique<ControlObject>(ConfigKey(kAppGroup,
                          QStringLiteral("audio_latency_overload_count")))),
          m_pAudioLatencyUsage(std::make_unique<ControlObject>(
                  ConfigKey(kAppGroup, QStringLiteral("audio_latency_usage")))),
          m_pAudioLatencyOverload(std::make_unique<ControlObject>(ConfigKey(
                  kAppGroup, QStringLiteral("audio_latency_overload")))),
          m_pTalkoverDucking(
                  std::make_unique<EngineTalkoverDucking>(pConfig, group)),
          m_pMainDelay(
                  std::make_unique<EngineDelay>(ConfigKey(group, "delay"))),
          m_pHeadDelay(
                  std::make_unique<EngineDelay>(ConfigKey(group, "headDelay"))),
          m_pBoothDelay(std::make_unique<EngineDelay>(
                  ConfigKey(group, "boothDelay"))),
          m_pLatencyCompensationDelay(std::make_unique<EngineDelay>(
                  ConfigKey(group, "microphoneLatencyCompensation"))),
          m_pVumeter(std::make_unique<EngineVuMeter>(kMainGroup, kLegacyGroup)),
          // Starts a thread for recording and broadcast
          m_pEngineSideChain(bEnableSidechain
                          ? std::make_unique<EngineSideChain>(
                                    pConfig, m_sidechainMix.data())
                          : nullptr),
          m_pCrossfader(std::make_unique<ControlPotmeter>(
                  ConfigKey(group, "crossfader"), -1., 1.)),
          m_pHeadMix(std::make_unique<ControlPotmeter>(
                  ConfigKey(group, "headMix"), -1., 1.)),
          m_pBalance(std::make_unique<ControlPotmeter>(
                  ConfigKey(group, "balance"), -1., 1.)),
          m_pXFaderMode(std::make_unique<ControlPushButton>(
                  ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderMode"))),
          m_pXFaderCurve(std::make_unique<ControlPotmeter>(
                  ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderCurve"),
                  EngineXfader::kTransformMin,
                  EngineXfader::kTransformMax)),
          m_pXFaderCalibration(std::make_unique<ControlPotmeter>(
                  ConfigKey(
                          EngineXfader::kXfaderConfigKey, "xFaderCalibration"),
                  0.3,
                  1.,
                  true)),
          m_pXFaderReverse(std::make_unique<ControlPushButton>(
                  ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderReverse"))),
          m_pHeadSplitEnabled(std::make_unique<ControlPushButton>(
                  ConfigKey(group, "headSplit"), true, 0.0)),

          m_pKeylockEngine(std::make_unique<ControlObject>(
                  ConfigKey(kAppGroup, QStringLiteral("keylock_engine")),
                  false,
                  false,
                  static_cast<double>(pConfig->getValue(
                          ConfigKey(group, "keylock_engine"),
                          EngineBuffer::defaultKeylockEngine())))),
          m_mainGainOld(0.0f),
          m_boothGainOld(0.0f),
          m_headphoneMainGainOld(0.0f),
          m_headphoneGainOld(1.0f),
          m_duckingGainOld(1.0f),
          m_balleftOld(1.0f),
          m_balrightOld(1.0f),
          m_numMicsConfigured(0),
          m_mainHandle(registerChannelGroup(group)),
          m_headphoneHandle(registerChannelGroup("[Headphone]")),
          m_mainOutputHandle(registerChannelGroup("[MasterOutput]")),
          m_busTalkoverHandle(registerChannelGroup("[BusTalkover]")),
          m_busCrossfaderLeftHandle(registerChannelGroup("[BusLeft]")),
          m_busCrossfaderCenterHandle(registerChannelGroup("[BusCenter]")),
          m_busCrossfaderRightHandle(registerChannelGroup("[BusRight]")),
          m_pMainMonoMixdown(std::make_unique<ControlObject>(
                  ConfigKey(group, "mono_mixdown"), true, false, true)),
          m_pMicMonitorMode(std::make_unique<ControlObject>(
                  ConfigKey(group, "talkover_mix"), true, false, true)) {
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
    m_bExternalRecordBroadcastInputConnected.store(false);
    m_pWorkerScheduler->start(QThread::HighPriority);

    m_pSampleRate->addAlias(ConfigKey(group, QStringLiteral("samplerate")));
    m_pSampleRate->set(44100.);

    m_pOutputLatencyMs->addAlias(ConfigKey(kLegacyGroup, QStringLiteral("latency")));
    m_pAudioLatencyOverloadCount->addAlias(ConfigKey(
            kLegacyGroup, QStringLiteral("audio_latency_overload_count")));
    m_pAudioLatencyUsage->addAlias(ConfigKey(kLegacyGroup, QStringLiteral("audio_latency_usage")));
    m_pAudioLatencyOverload->addAlias(
            ConfigKey(kLegacyGroup, QStringLiteral("audio_latency_overload")));

    // The last-used bpm value is saved in the destructor of EngineSync.
    ControlObject::set(kInternalClockBpmKey, pConfig->getValue(kInternalClockBpmKey, 124.0));

    // Legacy: the main "gain" control used to be named "volume" in Mixxx
    // 1.11.0 and earlier. See issue #7413.
    m_pMainGain->addAlias(ConfigKey(group, QStringLiteral("volume")));

    // Legacy: the headphone "headGain" control used to be named "headVolume" in
    // Mixxx 1.11.0 and earlier. See issue #7413.
    m_pHeadGain->addAlias(ConfigKey(group, QStringLiteral("headVolume")));

    m_pHeadMix->setDefaultValue(-1.);
    m_pHeadMix->set(-1.);

    m_pHeadSplitEnabled->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pHeadSplitEnabled->set(0.0f);

    // zero out otherwise uninitialized buffers
    m_head.clear();
    m_main.clear();
    m_booth.clear();
    m_talkover.clear();
    m_talkoverHeadphones.clear();
    m_sidechainMix.clear();
    for (auto& buffer : m_outputBusBuffers) {
        buffer.clear();
    }

    // X-Fader Setup
    m_pXFaderMode->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pXFaderReverse->setButtonMode(mixxx::control::ButtonMode::Toggle);

    m_pBoothEnabled->setReadOnly();
    m_pHeadphoneEnabled->setReadOnly();

    // Note: the EQ Rack is set in EffectsManager::setupDefaults();
}

EngineMixer::~EngineMixer() = default;

std::span<const CSAMPLE> EngineMixer::getMainBuffer() const {
    return m_main.span();
}

std::span<const CSAMPLE> EngineMixer::getBoothBuffer() const {
    return m_booth.span();
}

std::span<const CSAMPLE> EngineMixer::getHeadphoneBuffer() const {
    return m_head.span();
}

std::span<const CSAMPLE> EngineMixer::getSidechainBuffer() const {
    return m_sidechainMix.span();
}

void EngineMixer::processChannels(std::size_t bufferSize) {
    // Update internal sync lock rate.
    m_pEngineSync->onCallbackStart(m_sampleRate, bufferSize);

    m_activeBusChannels[EngineChannel::LEFT].clear();
    m_activeBusChannels[EngineChannel::CENTER].clear();
    m_activeBusChannels[EngineChannel::RIGHT].clear();
    m_activeHeadphoneChannels.clear();
    m_activeTalkoverChannels.clear();
    m_activeChannels.clear();

    // ScopedTimer timer(QStringLiteral("EngineMixer::processChannels"));
    EngineChannel* pLeaderChannel = m_pEngineSync->getLeaderChannel();
    // Reserve the first place for the main channel which
    // should be processed first
    m_activeChannels.append(nullptr);
    int activeChannelsStartIndex = 1; // Nothing at 0 yet
    for (int i = 0; i < m_channels.size(); ++i) {
        ChannelInfo* pChannelInfo = m_channels[i].get();
        EngineChannel* pChannel = pChannelInfo->m_pChannel.get();

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
    for (int i = activeChannelsStartIndex; i < m_activeChannels.size(); ++i) {
        ChannelInfo* pChannelInfo = m_activeChannels[i];
        auto& pChannel = pChannelInfo->m_pChannel;
        DEBUG_ASSERT(pChannelInfo->m_pBuffer.size() >= static_cast<SINT>(bufferSize));
        pChannel->process(pChannelInfo->m_pBuffer.data(), bufferSize);

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
    m_pEngineSync->onCallbackEnd(m_sampleRate, bufferSize);

    // After all engines have been processed, trigger updates of local bpm values
    // which may have changed based on track position
    std::for_each(m_activeChannels.cbegin() + activeChannelsStartIndex,
            m_activeChannels.cend(),
            [](const auto& pChannelInfo) {
                pChannelInfo->m_pChannel->postProcessLocalBpm();
            });

    // After local bpms are updated, trigger the rest of the post-processing
    // which ensures that all channels are updating certain values at the
    // same point in time. This prevents sync from failing depending on
    // if the sync target was processed before or after the sync origin.
    std::for_each(m_activeChannels.cbegin() + activeChannelsStartIndex,
            m_activeChannels.cend(),
            [bufferSize](const auto& pChannelInfo) {
                pChannelInfo->m_pChannel->postProcess(bufferSize);
            });
}

void EngineMixer::process(const std::size_t bufferSize) {
    DEBUG_ASSERT(bufferSize <= static_cast<int>(kMaxEngineSamples));

    static bool haveSetName = false;
    if (!haveSetName) {
        QThread::currentThread()->setObjectName("Engine");
        haveSetName = true;
    }
    // Trace t("EngineMixer::process");

    bool mainEnabled = m_pMainEnabled->toBool();
    bool boothEnabled = m_pBoothEnabled->toBool();
    bool headphoneEnabled = m_pHeadphoneEnabled->toBool();

    m_sampleRate = mixxx::audio::SampleRate::fromDouble(m_pSampleRate->get());
    // TODO: remove assumption of stereo buffer
    constexpr unsigned int kChannels = 2;
    const unsigned int iFrames = static_cast<unsigned int>(bufferSize) / kChannels;

    if (m_pEngineEffectsManager) {
        m_pEngineEffectsManager->onCallbackStart();
    }

    // Prepare all channels for output
    processChannels(bufferSize);

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
                m_head.data(),
                m_headphoneHandle.handle(),
                bufferSize,
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
                    m_head.data(),
                    bufferSize,
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
            m_talkover.data(),
            m_mainHandle.handle(),
            bufferSize,
            m_sampleRate,
            m_pEngineEffectsManager);

    // Process effects on all microphones mixed together
    // We have no metadata for mixed effect buses, so use an empty GroupFeatureState.
    GroupFeatureState busFeatures;
    if (m_pEngineEffectsManager) {
        m_pEngineEffectsManager->processPostFaderInPlace(
                m_busTalkoverHandle.handle(),
                m_mainHandle.handle(),
                m_talkover.data(),
                bufferSize,
                m_sampleRate,
                busFeatures,
                CSAMPLE_GAIN_ONE,
                CSAMPLE_GAIN_ONE);
    }

    switch (m_pTalkoverDucking->getMode()) {
    case EngineTalkoverDucking::OFF:
        m_pTalkoverDucking->setAboveThreshold(false);
        break;
    case EngineTalkoverDucking::AUTO:
        m_pTalkoverDucking->processKey(m_talkover.data(), bufferSize);
        break;
    case EngineTalkoverDucking::MANUAL:
        m_pTalkoverDucking->setAboveThreshold(!m_activeTalkoverChannels.isEmpty());
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
    // channel volume faders and crossfader.
    m_mainGain.setGains(crossfaderLeftGain, 1.0f, crossfaderRightGain);

    for (int o = EngineChannel::LEFT; o <= EngineChannel::RIGHT; o++) {
        ChannelMixer::applyEffectsInPlaceAndMixChannels(m_mainGain,
                m_activeBusChannels[o],
                &m_channelMainGainCache, // no [o] because the old gain
                                         // follows an orientation switch
                m_outputBusBuffers[o].data(),
                m_mainHandle.handle(),
                bufferSize,
                m_sampleRate,
                m_pEngineEffectsManager);
    }

    // Process crossfader orientation bus channel effects
    if (m_pEngineEffectsManager) {
        m_pEngineEffectsManager->processPostFaderInPlace(
                m_busCrossfaderLeftHandle.handle(),
                m_mainHandle.handle(),
                m_outputBusBuffers[EngineChannel::LEFT].data(),
                bufferSize,
                m_sampleRate,
                busFeatures,
                CSAMPLE_GAIN_ONE,
                CSAMPLE_GAIN_ONE);
        m_pEngineEffectsManager->processPostFaderInPlace(
                m_busCrossfaderCenterHandle.handle(),
                m_mainHandle.handle(),
                m_outputBusBuffers[EngineChannel::CENTER].data(),
                bufferSize,
                m_sampleRate,
                busFeatures,
                CSAMPLE_GAIN_ONE,
                CSAMPLE_GAIN_ONE);
        m_pEngineEffectsManager->processPostFaderInPlace(
                m_busCrossfaderRightHandle.handle(),
                m_mainHandle.handle(),
                m_outputBusBuffers[EngineChannel::RIGHT].data(),
                bufferSize,
                m_sampleRate,
                busFeatures,
                CSAMPLE_GAIN_ONE,
                CSAMPLE_GAIN_ONE);
    }

    if (mainEnabled) {
        // Mix the crossfader orientation buffers together into the main mix.
        // If only one orientation is active, we can skip the expensive 3-way mix.
        bool leftIn = !m_activeBusChannels[EngineChannel::LEFT].isEmpty();
        bool centerIn = !m_activeBusChannels[EngineChannel::CENTER].isEmpty();
        bool rightIn = !m_activeBusChannels[EngineChannel::RIGHT].isEmpty();

        if (leftIn && !centerIn && !rightIn) {
            m_main.copy(m_outputBusBuffers[EngineChannel::LEFT], static_cast<SINT>(bufferSize));
        } else if (!leftIn && centerIn && !rightIn) {
            m_main.copy(m_outputBusBuffers[EngineChannel::CENTER], static_cast<SINT>(bufferSize));
        } else if (!leftIn && !centerIn && rightIn) {
            m_main.copy(m_outputBusBuffers[EngineChannel::RIGHT], static_cast<SINT>(bufferSize));
        } else if (leftIn && centerIn && !rightIn) {
            SampleUtil::copy2WithGain(m_main.data(),
                    m_outputBusBuffers[EngineChannel::LEFT].data(),
                    1.0f,
                    m_outputBusBuffers[EngineChannel::CENTER].data(),
                    1.0f,
                    static_cast<int>(bufferSize));
        } else if (leftIn && !centerIn && rightIn) {
            SampleUtil::copy2WithGain(m_main.data(),
                    m_outputBusBuffers[EngineChannel::LEFT].data(),
                    1.0f,
                    m_outputBusBuffers[EngineChannel::RIGHT].data(),
                    1.0f,
                    static_cast<int>(bufferSize));
        } else if (!leftIn && centerIn && rightIn) {
            SampleUtil::copy2WithGain(m_main.data(),
                    m_outputBusBuffers[EngineChannel::CENTER].data(),
                    1.0f,
                    m_outputBusBuffers[EngineChannel::RIGHT].data(),
                    1.0f,
                    static_cast<int>(bufferSize));
        } else {
            SampleUtil::copy3WithGain(m_main.data(),
                    m_outputBusBuffers[EngineChannel::LEFT].data(),
                    1.0f,
                    m_outputBusBuffers[EngineChannel::CENTER].data(),
                    1.0f,
                    m_outputBusBuffers[EngineChannel::RIGHT].data(),
                    1.0f,
                    static_cast<int>(bufferSize));
        }

        MicMonitorMode configuredMicMonitorMode = static_cast<MicMonitorMode>(
                static_cast<int>(m_pMicMonitorMode->get()));
        const CSAMPLE_GAIN mainGain = static_cast<CSAMPLE_GAIN>(m_pMainGain->get());
        const CSAMPLE_GAIN duckingGain = m_pTalkoverDucking->getGain(iFrames);

        // Process main, booth, and record/broadcast buffers according to the
        // MicMonitorMode configured in DlgPrefSound
        if (configuredMicMonitorMode == MicMonitorMode::Main) {
            applyMainEffects(bufferSize);

            SampleUtil::applyRampingGain(
                    m_main.data(), m_duckingGainOld, duckingGain, bufferSize);
            m_duckingGainOld = duckingGain;

            if (headphoneEnabled) {
                processHeadphones(mainMixGainInHeadphones, bufferSize);
            }

            if (boothEnabled) {
                CSAMPLE_GAIN boothGain = static_cast<CSAMPLE_GAIN>(m_pBoothGain->get());
                SampleUtil::copyWithRampingGain(m_booth.data(),
                        m_main.data(),
                        m_boothGainOld,
                        boothGain,
                        bufferSize);
                m_boothGainOld = boothGain;
            }

            if (m_numMicsConfigured > 0) {
                SampleUtil::add(m_main.data(), m_talkover.data(), bufferSize);
            }

            if (sidechainMixRequired()) {
                m_sidechainMix.copyWithRampingGain(
                        m_main, m_mainGainOld, mainGain, static_cast<SINT>(bufferSize));
            }
        } else if (configuredMicMonitorMode == MicMonitorMode::MainAndBooth) {
            applyMainEffects(bufferSize);

            SampleUtil::applyRampingGain(
                    m_main.data(), m_duckingGainOld, duckingGain, bufferSize);
            m_duckingGainOld = duckingGain;

            if (headphoneEnabled) {
                processHeadphones(mainMixGainInHeadphones, bufferSize);
            }

            if (m_numMicsConfigured > 0) {
                SampleUtil::add(m_main.data(), m_talkover.data(), bufferSize);
            }

            if (boothEnabled) {
                CSAMPLE_GAIN boothGain = static_cast<CSAMPLE_GAIN>(m_pBoothGain->get());
                SampleUtil::copyWithRampingGain(m_booth.data(),
                        m_main.data(),
                        m_boothGainOld,
                        boothGain,
                        bufferSize);
                m_boothGainOld = boothGain;
            }

            if (sidechainMixRequired()) {
                m_sidechainMix.copyWithRampingGain(
                        m_main, m_mainGainOld, mainGain, static_cast<SINT>(bufferSize));
            }
        } else if (configuredMicMonitorMode == MicMonitorMode::DirectMonitor) {
            if (boothEnabled) {
                CSAMPLE_GAIN boothGain = static_cast<CSAMPLE_GAIN>(m_pBoothGain->get());
                SampleUtil::copyWithRampingGain(m_booth.data(),
                        m_main.data(),
                        m_boothGainOld,
                        boothGain,
                        bufferSize);
                m_boothGainOld = boothGain;
            }

            applyMainEffects(bufferSize);

            SampleUtil::applyRampingGain(
                    m_main.data(), m_duckingGainOld, duckingGain, bufferSize);
            m_duckingGainOld = duckingGain;

            if (headphoneEnabled) {
                processHeadphones(mainMixGainInHeadphones, bufferSize);
            }

            if (sidechainMixRequired()) {
                m_sidechainMix.copyWithRampingGain(
                        m_main, m_mainGainOld, mainGain, static_cast<SINT>(bufferSize));

                if (m_numMicsConfigured > 0) {
                    m_pLatencyCompensationDelay->process(
                            m_sidechainMix.data(), bufferSize);
                    SampleUtil::add(m_sidechainMix.data(),
                            m_talkover.data(),
                            bufferSize);
                }
            }
        }

        if (m_pEngineSideChain) {
            m_pEngineSideChain->writeSamples(m_sidechainMix.data(), iFrames);
        }

        if (m_pEngineEffectsManager) {
            GroupFeatureState mainFeatures;
            mainFeatures.gain = m_pMainGain->get();
            m_pEngineEffectsManager->processPostFaderInPlace(
                    m_mainOutputHandle.handle(),
                    m_mainHandle.handle(),
                    m_main.data(),
                    bufferSize,
                    m_sampleRate,
                    mainFeatures);
        }

        CSAMPLE balright = 1.0f;
        CSAMPLE balleft = 1.0f;
        const auto bal = static_cast<CSAMPLE_GAIN>(m_pBalance->get());
        if (bal > 0.0f) {
            balleft -= bal;
        } else if (bal < 0.0f) {
            balright += bal;
        }

        CSAMPLE fVolSumL = 0.0f;
        CSAMPLE fVolSumR = 0.0f;
        const SampleUtil::CLIP_STATUS clipping = SampleUtil::finalizeMainMix(
                m_main.data(),
                bufferSize,
                mainGain,
                m_mainGainOld,
                balleft,
                balright,
                m_balleftOld,
                m_balrightOld,
                &fVolSumL,
                &fVolSumR,
                m_pMainMonoMixdown->toBool());

        m_mainGainOld = mainGain;
        m_balleftOld = balleft;
        m_balrightOld = balright;

        if (m_pVumeter != nullptr) {
            m_pVumeter->processFused(
                    fVolSumL, fVolSumR, clipping, m_sampleRate, bufferSize);
        }
    }

    if (mainEnabled) {
        m_pMainDelay->process(m_main.data(), bufferSize);
    } else {
        m_main.clear(bufferSize);
    }
    if (headphoneEnabled) {
        m_pHeadDelay->process(m_head.data(), bufferSize);
    }
    if (boothEnabled) {
        m_pBoothDelay->process(m_booth.data(), bufferSize);
    }

    // We're close to the end of the callback. Wake up the engine worker
    // scheduler so that it runs the workers.
    m_pWorkerScheduler->runWorkers();
}

void EngineMixer::applyMainEffects(std::size_t bufferSize) {
    // Apply main effects
    if (m_pEngineEffectsManager) {
        GroupFeatureState mainFeatures;
        mainFeatures.gain = m_pMainGain->get();
        m_pEngineEffectsManager->processPostFaderInPlace(m_mainHandle.handle(),
                m_mainHandle.handle(),
                m_main.data(),
                bufferSize,
                m_sampleRate,
                mainFeatures,
                CSAMPLE_GAIN_ONE,
                CSAMPLE_GAIN_ONE);
    }
}

void EngineMixer::processHeadphones(
        const CSAMPLE_GAIN mainMixGainInHeadphones,
        std::size_t bufferSize) {
    // Add main mix to headphones
    SampleUtil::addWithRampingGain(
            m_head.data(),
            m_main.data(),
            m_headphoneMainGainOld,
            mainMixGainInHeadphones,
            bufferSize);
    m_headphoneMainGainOld = mainMixGainInHeadphones;

    // If Head Split is enabled, replace the left channel of the pfl buffer
    // with a mono mix of the headphone buffer, and the right channel of the pfl
    // buffer with a mono mix of the main output buffer.
    if (m_pHeadSplitEnabled->toBool()) {
        // note: LOOP VECTORIZED.
        auto* const M_RESTRICT ph = m_head.data();
        const auto* const M_RESTRICT pm = m_main.data();
        for (std::size_t i = 0; i < bufferSize / 2; ++i) {
            const CSAMPLE headMono = (ph[i * 2] + ph[i * 2 + 1]) * 0.5f;
            const CSAMPLE mainMono = (pm[i * 2] + pm[i * 2 + 1]) * 0.5f;
            ph[i * 2] = headMono;
            ph[i * 2 + 1] = mainMono;
        }
    }

    // Apply headphone gain
    CSAMPLE_GAIN headphoneGain = static_cast<CSAMPLE_GAIN>(m_pHeadGain->get());
    SampleUtil::applyRampingGain(
            m_head.data(),
            m_headphoneGainOld,
            headphoneGain,
            bufferSize);
    m_headphoneGainOld = headphoneGain;
}

void EngineMixer::addChannel(std::unique_ptr<EngineChannel> pChannel) {
    auto pChannelInfo = std::make_unique<ChannelInfo>(m_channels.size());
    pChannel->setChannelIndex(pChannelInfo->m_index);
    const QString& group = pChannel->getGroup();
    // take ownership of the pointer explicitly
    pChannelInfo->m_pChannel = std::move(pChannel);
    pChannelInfo->m_handle = m_pChannelHandleFactory->getOrCreateHandle(group);
    pChannelInfo->m_pVolumeControl = std::make_unique<ControlAudioTaperPot>(
            ConfigKey(group, "volume"), -20, 0, 1);
    pChannelInfo->m_pVolumeControl->setDefaultValue(1.0);
    pChannelInfo->m_pVolumeControl->set(1.0);
    pChannelInfo->m_pMuteControl = std::make_unique<ControlPushButton>(
            ConfigKey(group, "mute"));
    pChannelInfo->m_pMuteControl->setButtonMode(mixxx::control::ButtonMode::PowerWindow);
    pChannelInfo->m_pBuffer = mixxx::SampleBuffer(kMaxEngineSamples);
    pChannelInfo->m_pBuffer.clear();
    EngineBuffer* pBuffer = pChannelInfo->m_pChannel->getEngineBuffer();
    m_channels.append(std::move(pChannelInfo));
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

    if (pBuffer != nullptr) {
        pBuffer->bindWorkers(m_pWorkerScheduler);
    }
}

EngineChannel* EngineMixer::getChannel(const QString& group) {
    // TODO: convert to C++20 ranges using projections or use hash-based lookup based on the group
    for (const auto& pChannelInfo : m_channels) {
        if (pChannelInfo->m_pChannel->getGroup() == group) {
            return pChannelInfo->m_pChannel.get();
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

std::span<const CSAMPLE> EngineMixer::getDeckBuffer(unsigned int i) const {
    return getChannelBuffer(PlayerManager::groupForDeck(i));
}

std::span<const CSAMPLE> EngineMixer::getOutputBusBuffer(unsigned int i) const {
    if (i <= EngineChannel::RIGHT) {
        return m_outputBusBuffers[i].span();
    }
    return {};
}

std::span<const CSAMPLE> EngineMixer::getChannelBuffer(const QString& group) const {
    for (const auto& pChannelInfo : m_channels) {
        if (pChannelInfo->m_pChannel->getGroup() == group) {
            return pChannelInfo->m_pBuffer.span();
        }
    }
    return {};
}

std::span<const CSAMPLE> EngineMixer::buffer(const AudioOutput& output) const {
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
        return {};
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
        m_bExternalRecordBroadcastInputConnected.store(true);
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
        m_bExternalRecordBroadcastInputConnected.store(false);
        break;
    default:
        break;
    }
}

void EngineMixer::registerNonEngineChannelSoundIO(gsl::not_null<SoundManager*> pSoundManager) {
    pSoundManager->registerInput(AudioInput(AudioPathType::RecordBroadcast,
                                         0,
                                         mixxx::audio::ChannelCount::stereo()),
            m_pEngineSideChain.get());

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
    return m_pEngineSideChain && !m_bExternalRecordBroadcastInputConnected.load();
}
