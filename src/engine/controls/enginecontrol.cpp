#include "engine/controls/enginecontrol.h"

#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "engine/sync/enginesync.h"
#include "mixer/playermanager.h"
#include "moc_enginecontrol.cpp"

EngineControl::EngineControl(const QString& group,
        UserSettingsPointer pConfig)
        : m_group(group),
          m_pConfig(pConfig),
          m_pEngineMaster(nullptr),
          m_pEngineBuffer(nullptr) {
    setFrameInfo(mixxx::audio::kStartFramePos,
            mixxx::audio::kInvalidFramePos,
            mixxx::audio::SampleRate());
}

EngineControl::~EngineControl() {
}

void EngineControl::process(const double dRate,
                           const double dCurrentSample,
                           const int iBufferSize) {
    Q_UNUSED(dRate);
    Q_UNUSED(dCurrentSample);
    Q_UNUSED(iBufferSize);
}

void EngineControl::trackLoaded(TrackPointer pNewTrack) {
    Q_UNUSED(pNewTrack);
}

void EngineControl::trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
    Q_UNUSED(pBeats);
}

void EngineControl::hintReader(HintVector*) {
}

void EngineControl::setEngineMaster(EngineMaster* pEngineMaster) {
    m_pEngineMaster = pEngineMaster;
}

void EngineControl::setEngineBuffer(EngineBuffer* pEngineBuffer) {
    m_pEngineBuffer = pEngineBuffer;
}

void EngineControl::setFrameInfo(mixxx::audio::FramePos currentPosition,
        mixxx::audio::FramePos trackEndPosition,
        mixxx::audio::SampleRate sampleRate) {
    FrameInfo info;
    info.currentPosition = currentPosition;
    info.trackEndPosition = trackEndPosition;
    info.sampleRate = sampleRate;
    m_frameInfo.setValue(info);
}

QString EngineControl::getGroup() const {
    return m_group;
}

UserSettingsPointer EngineControl::getConfig() {
    return m_pConfig;
}

EngineMaster* EngineControl::getEngineMaster() {
    return m_pEngineMaster;
}

EngineBuffer* EngineControl::getEngineBuffer() {
    return m_pEngineBuffer;
}

void EngineControl::setBeatLoop(double startPosition, bool enabled) {
    if (m_pEngineBuffer) {
        return m_pEngineBuffer->setBeatLoop(startPosition, enabled);
    }
}

void EngineControl::setLoop(double startPosition, double endPosition, bool enabled) {
    if (m_pEngineBuffer) {
        return m_pEngineBuffer->setLoop(startPosition, endPosition, enabled);
    }
}

void EngineControl::seekAbs(mixxx::audio::FramePos position) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->slotControlSeekAbs(position.toEngineSamplePos());
    }
}

void EngineControl::seekExact(mixxx::audio::FramePos position) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->slotControlSeekExact(position.toEngineSamplePos());
    }
}

void EngineControl::seek(double fractionalPosition) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->slotControlSeek(fractionalPosition);
    }
}

EngineBuffer* EngineControl::pickSyncTarget() {
    EngineMaster* pMaster = getEngineMaster();
    if (!pMaster) {
        return nullptr;
    }

    EngineSync* pEngineSync = pMaster->getEngineSync();
    if (!pEngineSync) {
        return nullptr;
    }

    EngineChannel* pThisChannel = pMaster->getChannel(getGroup());
    Syncable* pSyncable = pEngineSync->pickNonSyncSyncTarget(pThisChannel);
    // pickNonSyncSyncTarget can return nullptr, but if it doesn't the Syncable
    // definitely has an EngineChannel.
    if (pSyncable) {
        return pSyncable->getChannel()->getEngineBuffer();
    }
    return nullptr;
}
