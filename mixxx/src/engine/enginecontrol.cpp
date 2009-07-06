// EngineControl.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include "engine/enginecontrol.h"

EngineControl::EngineControl(const char * _group,
                             const ConfigObject<ConfigValue> * _config) :
    m_pGroup(_group),
    m_pConfig(_config),
    m_dCurrentSample(0) {
}

EngineControl::~EngineControl() {

}
