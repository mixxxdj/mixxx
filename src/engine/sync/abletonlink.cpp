#include "engine/sync/abletonlink.h"

#include <QtDebug>
#include <cmath>

#include "control/controlobject.h"
#include "engine/sync/enginesync.h"
#include "moc_abletonlink.cpp"
#include "preferences/usersettings.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("AbletonLink");
constexpr mixxx::Bpm kDefaultBpm(9999.9);
} // namespace

AbletonLink::AbletonLink(const QString& group, EngineSync* pEngineSync)
        : m_group(group),
          m_pEngineSync(pEngineSync),
          m_syncMode(SyncMode::None),
          m_oldTempo(kDefaultBpm),
          m_audioBufferTimeMicros(0),
          m_absTimeWhenPrevOutputBufferReachesDac(0),
          m_sampleTimeAtStartCallback(0),
          m_pLink(std::make_unique<ableton::BasicLink<MixxxClockRef>>(120.0)),
          m_pLinkButton(std::make_unique<ControlPushButton>(ConfigKey(group, "sync_enabled"))),
          m_pNumLinkPeers(std::make_unique<ControlObject>(ConfigKey(group, "num_peers"))) {
    m_timeAtStartCallback = m_pLink->clock().micros();

    m_pLinkButton->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pLinkButton->setStates(2);

    connect(m_pLinkButton.get(),
            &ControlObject::valueChanged,
            this,
            &AbletonLink::slotControlSyncEnabled);

    m_pNumLinkPeers->setReadOnly();
    m_pNumLinkPeers->forceSet(0);

    // The callback is invoked on a Link - managed thread.
    // The callback is the only entity, which access m_pNumLinkPeers
    m_pLink->setNumPeersCallback([this](std::size_t numPeers) {
        m_pNumLinkPeers->forceSet(numPeers);
    });

    m_pLink->enable(false);

    // Start/Stop sync makes not much sense for a DJ set
    m_pLink->enableStartStopSync(false);

    audioThreadDebugOutput();
}

AbletonLink::~AbletonLink() {
    // Stop Link activity and remove callbacks before destroying ControlObjects.
    m_pLink->setNumPeersCallback([](std::size_t) {});
    m_pLink->enable(false);

    // Disconnect the connection from the Link GUI button
    if (m_pLinkButton) {
        QObject::disconnect(m_pLinkButton.get(),
                &ControlObject::valueChanged,
                this,
                &AbletonLink::slotControlSyncEnabled);
    }

    // Destroy control objects before releasing Link.
    m_pNumLinkPeers.reset();
    m_pLinkButton.reset();

    // Finally release Link
    m_pLink.reset();
}

void AbletonLink::slotControlSyncEnabled(double controButtonlValue) {
    m_pLink->enable(controButtonlValue > 0);
}

void AbletonLink::setSyncMode(SyncMode syncMode) {
    m_syncMode = syncMode;
}

void AbletonLink::notifyUniquePlaying() {
}

void AbletonLink::requestSync() {
    qDebug() << "AbletonLink::requestSync()";
}

SyncMode AbletonLink::getSyncMode() const {
    return m_syncMode;
}

bool AbletonLink::isPlaying() const {
    if (!m_pLink->isEnabled()) {
        return false;
    }
    if (m_pLink->numPeers() < 1) {
        return false;
    }

    // Note, that ableton::Link::SessionState.isPlaying() is an optional Ableton
    // Link feature (Start/Stop sync) and shouldn't be taken into account here.
    return true;
}
bool AbletonLink::isAudible() const {
    return false;
}
bool AbletonLink::isQuantized() const {
    return true;
}

mixxx::Bpm AbletonLink::getBpm() const {
    return getBaseBpm();
}

double AbletonLink::getBeatDistance() const {
    const auto sessionState = m_pLink->captureAudioSessionState();
    const auto beats = sessionState.beatAtTime(
            m_absTimeWhenPrevOutputBufferReachesDac, getQuantum());
    return std::fmod(beats, 1.0);
}

mixxx::Bpm AbletonLink::getBaseBpm() const {
    const auto sessionState = m_pLink->captureAudioSessionState();
    return mixxx::Bpm(sessionState.tempo());
}

void AbletonLink::updateLeaderBeatDistance(double beatDistance) {
    auto sessionState = m_pLink->captureAudioSessionState();
    const auto currentBeat = sessionState.beatAtTime(
            m_absTimeWhenPrevOutputBufferReachesDac, getQuantum());
    const auto newBeat = currentBeat - std::fmod(currentBeat, 1.0) + beatDistance;

    sessionState.requestBeatAtTime(newBeat, m_absTimeWhenPrevOutputBufferReachesDac, getQuantum());
    m_pLink->commitAudioSessionState(sessionState);
}

void AbletonLink::forceUpdateLeaderBeatDistance(double beatDistance) {
    auto sessionState = m_pLink->captureAudioSessionState();
    const auto currentBeat = sessionState.beatAtTime(
            m_absTimeWhenPrevOutputBufferReachesDac, getQuantum());
    const auto newBeat = currentBeat - std::fmod(currentBeat, 1.0) + beatDistance;

    sessionState.forceBeatAtTime(newBeat, m_absTimeWhenPrevOutputBufferReachesDac, getQuantum());
    m_pLink->commitAudioSessionState(sessionState);
}

void AbletonLink::updateLeaderBpm(mixxx::Bpm bpm) {
    auto sessionState = m_pLink->captureAudioSessionState();
    sessionState.setTempo(bpm.value(), m_absTimeWhenPrevOutputBufferReachesDac);
    m_pLink->commitAudioSessionState(sessionState);
}

void AbletonLink::notifyLeaderParamSource() {
    // In Ableton Link all pears are equal. Therefore nothing differs,
    // if AbletonLink becomes SyncLeader.
    // TODO: Check the special case of half/double BPM sync.
}

void AbletonLink::reinitLeaderParams(double beatDistance, mixxx::Bpm, mixxx::Bpm bpm) {
    updateLeaderBeatDistance(beatDistance);
    updateLeaderBpm(bpm);
}
void AbletonLink::updateInstantaneousBpm(mixxx::Bpm) {
}

/// This method is called at the start of the audio callback.
/// It captures the current time and updates the audio buffer time.
/// If Ableton Link is enabled, it captures the session state and notifies
/// the engine sync about any changes in tempo and beat distance.
void AbletonLink::onCallbackStart(int sampleRate,
        size_t bufferSize,
        std::chrono::microseconds absTimeWhenPrevOutputBufferReachesDac) {
    m_timeAtStartCallback = m_pLink->clock().micros();

    // auto latency = absTimeWhenPrevOutputBufferReachesDac - m_timeAtStartCallback;
    /* qDebug() << "#####################:" << absTimeWhenPrevOutputBufferReachesDac.count()
             << " ##################AbletonLatency " << latency.count()
             << " Delta : "
             << m_absTimeWhenPrevOutputBufferReachesDac.count() -
                    absTimeWhenPrevOutputBufferReachesDac.count();
                    */
    m_audioBufferTimeMicros = std::chrono::microseconds(
            bufferSize / mixxx::audio::ChannelCount::stereo() /
            sampleRate * 1000000);

    m_absTimeWhenPrevOutputBufferReachesDac = absTimeWhenPrevOutputBufferReachesDac;

    if (!m_pLink->isEnabled()) {
        return;
    }

    const auto sessionState = m_pLink->captureAudioSessionState();
    const mixxx::Bpm tempo(sessionState.tempo());
    if (m_oldTempo != tempo) {
        m_oldTempo = tempo;
        m_pEngineSync->notifyRateChanged(this, tempo);
    }

    const auto beats = sessionState.beatAtTime(absTimeWhenPrevOutputBufferReachesDac, getQuantum());
    const auto beatDistance = std::fmod(beats, 1.0);
    m_pEngineSync->notifyBeatDistanceChanged(this, beatDistance);
}

void AbletonLink::onCallbackEnd(int sampleRate, size_t bufferSize) {
    Q_UNUSED(sampleRate)
    Q_UNUSED(bufferSize)
}

// Debug output function, to be called in audio thread
void AbletonLink::audioThreadDebugOutput() {
    kLogger.debug() << "isEnabled()" << m_pLink->isEnabled();
    kLogger.debug() << "numPeers()" << m_pLink->numPeers();

    const auto sessionState = m_pLink->captureAudioSessionState();

    kLogger.debug() << "sessionState.tempo()" << sessionState.tempo();
    kLogger.debug() << "sessionState.beatAtTime()"
                    << sessionState.beatAtTime(m_pLink->clock().micros(), getQuantum());
    kLogger.debug() << "sessionState.phaseAtTime()"
                    << sessionState.phaseAtTime(m_pLink->clock().micros(), getQuantum());
    kLogger.debug() << "sessionState.timeAtBeat(0)"
                    << sessionState.timeAtBeat(0.0, getQuantum()).count();
    kLogger.debug() << "sessionState.isPlaying()" << sessionState.isPlaying();
    kLogger.debug() << "sessionState.timeForIsPlaying()" << sessionState.timeForIsPlaying().count();

    // Est. Delay (micro-seconds) between onCallbackStart() and buffer's first
    // audio sample reaching speakers
    kLogger.debug() << "Est. Delay (us)"
                    << (m_absTimeWhenPrevOutputBufferReachesDac -
                               m_pLink->clock().micros())
                               .count();
}
