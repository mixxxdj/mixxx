#include "engine/controls/enginecontrol.h"

#include "engine/enginebuffer.h"
#include "engine/enginemixer.h"
#include "engine/sync/enginesync.h"
#include "moc_enginecontrol.cpp"

EngineControl::EngineControl(const QString& group,
        UserSettingsPointer pConfig)
        : m_group(group),
          m_pConfig(pConfig),
          m_pEngineMixer(nullptr),
          m_pEngineBuffer(nullptr) {
    setFrameInfo(mixxx::audio::kStartFramePos,
            mixxx::audio::kInvalidFramePos,
            mixxx::audio::SampleRate());
}

EngineControl::~EngineControl() {
}

void EngineControl::process(const double rate,
        mixxx::audio::FramePos currentPosition,
        const std::size_t bufferSize) {
    Q_UNUSED(rate);
    Q_UNUSED(currentPosition);
    Q_UNUSED(bufferSize);
}

void EngineControl::trackLoaded(TrackPointer pNewTrack) {
    Q_UNUSED(pNewTrack);
}

void EngineControl::trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
    Q_UNUSED(pBeats);
}

void EngineControl::hintReader(gsl::not_null<HintVector*>) {
}

void EngineControl::setEngineMixer(EngineMixer* pEngineMixer) {
    m_pEngineMixer = pEngineMixer;
}

void EngineControl::setEngineBuffer(EngineBuffer* pEngineBuffer) {
    m_pEngineBuffer = pEngineBuffer;
}

void EngineControl::setFrameInfo(mixxx::audio::FramePos currentPosition,
        mixxx::audio::FramePos trackEndPosition,
        mixxx::audio::SampleRate sampleRate) {
    m_frameInfo.setValue(FrameInfo{currentPosition, trackEndPosition, sampleRate});
}

QString EngineControl::getGroup() const {
    return m_group;
}

UserSettingsPointer EngineControl::getConfig() {
    return m_pConfig;
}

EngineMixer* EngineControl::getEngineMixer() {
    return m_pEngineMixer;
}

EngineBuffer* EngineControl::getEngineBuffer() {
    return m_pEngineBuffer;
}

void EngineControl::setBeatLoop(mixxx::audio::FramePos startPosition, bool enabled) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->setBeatLoop(startPosition, enabled);
    }
}

void EngineControl::setLoop(mixxx::audio::FramePos startPosition,
        mixxx::audio::FramePos endPosition,
        bool enabled) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->setLoop(startPosition, endPosition, enabled);
    }
}

void EngineControl::seekAbs(mixxx::audio::FramePos position) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->seekAbs(position);
    }
}

void EngineControl::seekExact(mixxx::audio::FramePos position) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->seekExact(position);
    }
}

void EngineControl::seek(double fractionalPosition) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->slotControlSeek(fractionalPosition);
    }
}

EngineBuffer* EngineControl::pickSyncTarget() {
    EngineMixer* pEngineMixer = getEngineMixer();
    if (!pEngineMixer) {
        return nullptr;
    }

    EngineSync* pEngineSync = pEngineMixer->getEngineSync();
    if (!pEngineSync) {
        return nullptr;
    }

    EngineChannel* pThisChannel = pEngineMixer->getChannel(getGroup());
    Syncable* pSyncable = pEngineSync->pickNonSyncSyncTarget(pThisChannel);
    // pickNonSyncSyncTarget can return nullptr, but if it doesn't the Syncable
    // definitely has an EngineChannel.
    if (pSyncable) {
        return pSyncable->getChannel()->getEngineBuffer();
    }
    return nullptr;
}
