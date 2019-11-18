// enginecontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include "engine/controls/enginecontrol.h"
#include "engine/enginemaster.h"
#include "engine/enginebuffer.h"
#include "engine/sync/enginesync.h"
#include "mixer/playermanager.h"

EngineControl::EngineControl(QString group,
                             UserSettingsPointer pConfig)
        : m_group(group),
          m_pConfig(pConfig),
          m_pEngineMaster(nullptr),
          m_pEngineBuffer(nullptr) {
    setCurrentSample(0.0, 0.0, 0.0);
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

void EngineControl::hintReader(HintVector*) {
}

void EngineControl::setEngineMaster(EngineMaster* pEngineMaster) {
    m_pEngineMaster = pEngineMaster;
}

void EngineControl::setEngineBuffer(EngineBuffer* pEngineBuffer) {
    m_pEngineBuffer = pEngineBuffer;
}

void EngineControl::setCurrentSample(
        const double dCurrentSample, const double dTotalSamples, const double dTrackSampleRate) {
    SampleOfTrack sot;
    sot.current = dCurrentSample;
    sot.total = dTotalSamples;
    sot.rate = dTrackSampleRate;
    m_sampleOfTrack.setValue(sot);
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

void EngineControl::seekAbs(double samplePosition) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->slotControlSeekAbs(samplePosition);
    }
}

void EngineControl::seekExact(double playPosition) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->slotControlSeekExact(playPosition);
    }
}

void EngineControl::seek(double sample) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->slotControlSeek(sample);
    }
}

void EngineControl::notifySeek(double dNewPlaypos) {
    Q_UNUSED(dNewPlaypos);
}

EngineBuffer* EngineControl::pickSyncTarget() {
    EngineMaster* pMaster = getEngineMaster();
    if (!pMaster) {
        return NULL;
    }

    EngineSync* pEngineSync = pMaster->getEngineSync();
    if (pEngineSync == NULL) {
        return NULL;
    }

    // TODO(rryan): Remove. This is a linear search over groups in
    // EngineMaster. We should pass the EngineChannel into EngineControl.
    EngineChannel* pThisChannel = pMaster->getChannel(getGroup());
    EngineChannel* pChannel = pEngineSync->pickNonSyncSyncTarget(pThisChannel);
    return pChannel ? pChannel->getEngineBuffer() : NULL;
}
