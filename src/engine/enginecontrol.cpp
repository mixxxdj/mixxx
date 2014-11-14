// enginecontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include "engine/enginecontrol.h"
#include "engine/enginemaster.h"
#include "engine/enginebuffer.h"
#include "playermanager.h"

EngineControl::EngineControl(QString group,
                             ConfigObject<ConfigValue>* _config)
        : m_group(group),
          m_pConfig(_config),
          m_dTotalSamples(0),
          m_pEngineMaster(NULL),
          m_pEngineBuffer(NULL),
          m_numDecks(ConfigKey("[Master]", "num_decks")) {
    m_dCurrentSample.setValue(0);
}

EngineControl::~EngineControl() {
}

double EngineControl::process(const double,
                              const double,
                              const double,
                              const int) {
    return kNoTrigger;
}

double EngineControl::nextTrigger(const double,
                                  const double,
                                  const double,
                                  const int) {
    return kNoTrigger;
}

double EngineControl::getTrigger(const double,
                                 const double,
                                 const double,
                                 const int) {
    return kNoTrigger;
}

void EngineControl::trackLoaded(TrackPointer) {
}

void EngineControl::trackUnloaded(TrackPointer) {
}

void EngineControl::hintReader(QVector<Hint>*) {
}

void EngineControl::setEngineMaster(EngineMaster* pEngineMaster) {
    m_pEngineMaster = pEngineMaster;
}

void EngineControl::setEngineBuffer(EngineBuffer* pEngineBuffer) {
    m_pEngineBuffer = pEngineBuffer;
}

void EngineControl::setCurrentSample(const double dCurrentSample, const double dTotalSamples) {
    m_dCurrentSample.setValue(dCurrentSample);
    m_dTotalSamples = dTotalSamples;
}

double EngineControl::getCurrentSample() const {
    return m_dCurrentSample.getValue();
}

double EngineControl::getTotalSamples() const {
    return m_dTotalSamples;
}

QString EngineControl::getGroup() const {
    return m_group;
}

ConfigObject<ConfigValue>* EngineControl::getConfig() {
    return m_pConfig;
}

EngineMaster* EngineControl::getEngineMaster() {
    return m_pEngineMaster;
}

EngineBuffer* EngineControl::getEngineBuffer() {
    return m_pEngineBuffer;
}

void EngineControl::seekAbs(double fractionalPosition) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->slotControlSeekAbs(fractionalPosition);
    }
}

void EngineControl::seekExact(double fractionalPosition) {
    if (m_pEngineBuffer) {
        m_pEngineBuffer->slotControlSeekExact(fractionalPosition);
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
    QString group = getGroup();
    EngineBuffer* pFirstNonplayingDeck = NULL;

    for (int i = 0; i < m_numDecks.get(); ++i) {
        QString deckGroup = PlayerManager::groupForDeck(i);
        if (deckGroup == group) {
            continue;
        }
        EngineChannel* pChannel = pMaster->getChannel(deckGroup);
        // Only consider channels that have a track loaded and are in the master
        // mix.
        if (pChannel && pChannel->isActive() && pChannel->isMaster()) {
            EngineBuffer* pBuffer = pChannel->getEngineBuffer();
            if (pBuffer && pBuffer->getBpm() > 0) {
                // If the deck is playing then go with it immediately.
                if (fabs(pBuffer->getSpeed()) > 0) {
                    return pBuffer;
                }
                // Otherwise hold out for a deck that might be playing but
                // remember the first deck that matched our criteria.
                if (pFirstNonplayingDeck == NULL) {
                    pFirstNonplayingDeck = pBuffer;
                }
            }
        }
    }
    // No playing decks have a BPM. Go with the first deck that was stopped but
    // had a BPM.
    return pFirstNonplayingDeck;
}
