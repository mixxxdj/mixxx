#include "engine/sync/abletonlink.h"

#include <QtDebug>
#include <cmath>

#include "engine/sync/enginesync.h"
#include "preferences/usersettings.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("AbletonLink");

} // namespace

AbletonLink::AbletonLink(const QString& group, SyncableListener* pEngineSync)
        : m_link(120.), m_group(group), m_pEngineSync(pEngineSync), m_mode(SYNC_NONE) {
    nonAudioSet();
    audioSafeSet();
    nonAudioPrint();
    audioSafePrint();

    initTestTimer(1000, true);
}

void AbletonLink::setSyncMode(SyncMode mode) {
    m_mode = mode;
}

void AbletonLink::notifyOnlyPlayingSyncable() {
}

void AbletonLink::requestSync() {
}

SyncMode AbletonLink::getSyncMode() const {
    return m_mode;
}

bool AbletonLink::isPlaying() const {
    return false;
}

double AbletonLink::getBpm() const {
    return getBaseBpm();
}

double AbletonLink::getBeatDistance() const {
    ableton::Link::SessionState sessionState = m_link.captureAudioSessionState();
    auto beats = sessionState.beatAtTime(getCurrentBufferPlayTime(), getQuantum());
    return std::fmod(beats, 1.);
}

double AbletonLink::getBaseBpm() const {
    ableton::Link::SessionState sessionState = m_link.captureAudioSessionState();
    return sessionState.tempo();
}

void AbletonLink::setMasterBeatDistance(double beatDistance) {
    ableton::Link::SessionState sessionState = m_link.captureAudioSessionState();

    auto currentBeat = sessionState.beatAtTime(getCurrentBufferPlayTime(), getQuantum());
    auto newBeat = currentBeat - std::fmod(currentBeat, 1.0) + beatDistance;
    sessionState.requestBeatAtTime(newBeat, getCurrentBufferPlayTime(), getQuantum());

    m_link.commitAudioSessionState(sessionState);
}

void AbletonLink::setMasterBpm(double bpm) {
    ableton::Link::SessionState sessionState = m_link.captureAudioSessionState();
    sessionState.setTempo(bpm, getCurrentBufferPlayTime());
    m_link.commitAudioSessionState(sessionState);
}

void AbletonLink::setMasterParams(double beatDistance, double baseBpm, double bpm) {
    Q_UNUSED(baseBpm)
    setMasterBeatDistance(beatDistance);
    setMasterBpm(bpm);
}

void AbletonLink::setInstantaneousBpm(double bpm) {
    setMasterBpm(bpm);
}

void AbletonLink::onCallbackStart(int sampleRate, int bufferSize) {
    int samplesPerChannel = bufferSize / 2;
    double samplesPerMicrosecond = static_cast<double>(sampleRate) / 1000000.;
    double currentLatency = static_cast<double>(samplesPerChannel) / samplesPerMicrosecond;
    m_currentLatency = std::chrono::microseconds{static_cast<long>(currentLatency)};

    updateHostTime(m_sampleTime);
    m_sampleTime += samplesPerChannel;
}

void AbletonLink::onCallbackEnd(int sampleRate, int bufferSize) {
    Q_UNUSED(sampleRate)
    Q_UNUSED(bufferSize)
}
