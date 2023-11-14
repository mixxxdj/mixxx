#include "engine/sync/abletonlink.h"

#include <QtDebug>
#include <cmath>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
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
          m_mode(SyncMode::None),
          m_oldTempo(kDefaultBpm),
          m_audioBufferTimeMicros(0),
          m_absTimeWhenPrevOutputBufferReachesDac(0),
          m_sampleTimeAtStartCallback(0) {
    m_pLink = std::make_unique<ableton::Link>(120.);
    m_timeAtStartCallback = m_pLink->clock().micros();

    m_pLinkButton = std::make_unique<ControlPushButton>(ConfigKey(group, "sync_enabled"));
    m_pLinkButton->setButtonMode(ControlPushButton::TOGGLE);
    m_pLinkButton->setStates(2);

    connect(m_pLinkButton.get(),
            &ControlObject::valueChanged,
            this,
            &AbletonLink::slotControlSyncEnabled,
            Qt::DirectConnection);

    m_pNumLinkPeers = std::make_unique<ControlObject>(ConfigKey(m_group, "num_peers"));
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

    nonAudioThreadDebugOutput();
    audioThreadDebugOutput();

    initTestTimer(1000, true);
}

AbletonLink::~AbletonLink() {
    m_pLink.reset();
}

void AbletonLink::slotControlSyncEnabled(double value) {
    qDebug() << "AbletonLink::slotControlSyncEnabled" << value;
    if (value > 0) {
        m_pLink->enable(true);
    } else {
        m_pLink->enable(false);
    }
}

void AbletonLink::setSyncMode(SyncMode mode) {
    m_mode = mode;
}

void AbletonLink::notifyUniquePlaying() {
}

void AbletonLink::requestSync() {
    qDebug() << "AbletonLink::requestSync()";
}

SyncMode AbletonLink::getSyncMode() const {
    return m_mode;
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
    ableton::Link::SessionState sessionState = m_pLink->captureAudioSessionState();
    auto beats = sessionState.beatAtTime(m_absTimeWhenPrevOutputBufferReachesDac, getQuantum());
    return std::fmod(beats, 1.);
}

mixxx::Bpm AbletonLink::getBaseBpm() const {
    ableton::Link::SessionState sessionState = m_pLink->captureAudioSessionState();
    mixxx::Bpm tempo(sessionState.tempo());
    return tempo;
}

void AbletonLink::updateLeaderBeatDistance(double beatDistance) {
    ableton::Link::SessionState sessionState = m_pLink->captureAudioSessionState();
    auto currentBeat = sessionState.beatAtTime(
            m_absTimeWhenPrevOutputBufferReachesDac, getQuantum());
    auto newBeat = currentBeat - std::fmod(currentBeat, 1.0) + beatDistance;
    sessionState.requestBeatAtTime(newBeat, m_absTimeWhenPrevOutputBufferReachesDac, getQuantum());

    m_pLink->commitAudioSessionState(sessionState);
}

void AbletonLink::forceUpdateLeaderBeatDistance(double beatDistance) {
    ableton::Link::SessionState sessionState = m_pLink->captureAudioSessionState();
    auto currentBeat = sessionState.beatAtTime(
            m_absTimeWhenPrevOutputBufferReachesDac, getQuantum());
    auto newBeat = currentBeat - std::fmod(currentBeat, 1.0) + beatDistance;

    sessionState.forceBeatAtTime(newBeat, m_absTimeWhenPrevOutputBufferReachesDac, getQuantum());

    m_pLink->commitAudioSessionState(sessionState);
}

void AbletonLink::updateLeaderBpm(mixxx::Bpm bpm) {
    ableton::Link::SessionState sessionState = m_pLink->captureAudioSessionState();
    sessionState.setTempo(bpm.value(), m_absTimeWhenPrevOutputBufferReachesDac);
    m_pLink->commitAudioSessionState(sessionState);
}

void AbletonLink::notifyLeaderParamSource() {
    // TODO: Not implemented yet
}

void AbletonLink::reinitLeaderParams(double beatDistance, mixxx::Bpm baseBpm, mixxx::Bpm bpm) {
    Q_UNUSED(baseBpm)
    updateLeaderBeatDistance(beatDistance);
    updateLeaderBpm(bpm);
}

void AbletonLink::updateInstantaneousBpm(mixxx::Bpm bpm) {
    Q_UNUSED(bpm)
}

void AbletonLink::onCallbackStart(int sampleRate,
        int bufferSize,
        std::chrono::microseconds absTimeWhenPrevOutputBufferReachesDac) {
    m_timeAtStartCallback = m_pLink->clock().micros();

    // auto latency = absTimeWhenPrevOutputBufferReachesDac - m_timeAtStartCallback;
    /* qDebug() << "#####################:" << absTimeWhenPrevOutputBufferReachesDac.count()
             << " ##################AbletonLatency " << latency.count()
             << " Delta : "
             << m_absTimeWhenPrevOutputBufferReachesDac.count() -
                    absTimeWhenPrevOutputBufferReachesDac.count();
                    */
    m_audioBufferTimeMicros = std::chrono::microseconds((bufferSize / 2 * 1000000) / sampleRate);

    m_absTimeWhenPrevOutputBufferReachesDac = absTimeWhenPrevOutputBufferReachesDac;
    m_absTimeWhenNextOutputBufferReachsDac =
            absTimeWhenPrevOutputBufferReachesDac + m_audioBufferTimeMicros;

    if (m_pLink->isEnabled()) {
        ableton::Link::SessionState sessionState = m_pLink->captureAudioSessionState();
        const mixxx::Bpm tempo(sessionState.tempo());
        if (m_oldTempo != tempo) {
            m_oldTempo = tempo;
            m_pEngineSync->notifyRateChanged(this, tempo);
        }

        auto beats = sessionState.beatAtTime(absTimeWhenPrevOutputBufferReachesDac, getQuantum());
        auto beatDistance = std::fmod(beats, 1.);
        m_pEngineSync->notifyBeatDistanceChanged(this, beatDistance);
    }
}

void AbletonLink::onCallbackEnd(int sampleRate, int bufferSize) {
    Q_UNUSED(sampleRate)
    Q_UNUSED(bufferSize)
}

// Debug output function, to be called in audio thread
void AbletonLink::audioThreadDebugOutput() {
    qDebug() << "isEnabled()" << m_pLink->isEnabled();
    qDebug() << "numPeers()" << m_pLink->numPeers();

    ableton::Link::SessionState sessionState = m_pLink->captureAudioSessionState();

    qDebug() << "sessionState.tempo()" << sessionState.tempo();
    qDebug() << "sessionState.beatAtTime()"
             << sessionState.beatAtTime(m_pLink->clock().micros(), getQuantum());
    qDebug() << "sessionState.phaseAtTime()"
             << sessionState.phaseAtTime(m_pLink->clock().micros(), getQuantum());
    qDebug() << "sessionState.timeAtBeat(0)" << sessionState.timeAtBeat(0.0, getQuantum()).count();
    qDebug() << "sessionState.isPlaying()" << sessionState.isPlaying();
    qDebug() << "sessionState.timeForIsPlaying()" << sessionState.timeForIsPlaying().count();

    // Est. Delay (micro-seconds) between onCallbackStart() and buffer's first
    // audio sample reaching speakers
    qDebug() << "Est. Delay (us)"
             << (m_absTimeWhenPrevOutputBufferReachesDac -
                        m_pLink->clock().micros())
                        .count();
}

// Debug output function, which must not be called in audio thread
void AbletonLink::nonAudioThreadDebugOutput() {
    qDebug() << "isStartStopSyncEnabled()" << m_pLink->isStartStopSyncEnabled();
}

void AbletonLink::initTestTimer(int ms, bool isRepeating) {
    m_pTestTimer = new QTimer(this);
    connect(m_pTestTimer, &QTimer::timeout, this, QOverload<>::of(&AbletonLink::testPrint));
    m_pTestTimer->setSingleShot(!isRepeating);
    m_pTestTimer->start(ms);
}
