// enginecontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include "engine/enginecontrol.h"
#include "engine/enginebuffer.h"

EngineControl::EngineControl(const char * _group,
                             const ConfigObject<ConfigValue> * _config) :
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
    return 0;
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
