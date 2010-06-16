// enginecontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include "engine/enginecontrol.h"
#include "engine/enginebuffer.h"

EngineControl::EngineControl(const char * _group,
                             ConfigObject<ConfigValue> * _config) :
    m_pGroup(_group),
    m_pConfig(_config),
    m_dCurrentSample(0),
    m_pOtherEngineBuffer(NULL) {
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

void EngineControl::hintReader(QList<Hint>&) {
}

void EngineControl::setOtherEngineBuffer(EngineBuffer* pOtherEngineBuffer) {
    m_pOtherEngineBuffer = pOtherEngineBuffer;
}

void EngineControl::setCurrentSample(const double dCurrentSample) {
    m_dCurrentSample = dCurrentSample;
}

double EngineControl::getCurrentSample() {
    return m_dCurrentSample;
}

const char* EngineControl::getGroup() {
    return m_pGroup;
}

ConfigObject<ConfigValue>* EngineControl::getConfig() {
    return m_pConfig;
}

EngineBuffer* EngineControl::getOtherEngineBuffer() {
    return m_pOtherEngineBuffer;
}

void EngineControl::notifySeek(double dNewPlaypos) {

}
