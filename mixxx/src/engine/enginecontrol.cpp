
#include "engine/enginecontrol.h"

EngineControl::EngineControl(const char * _group,
                             const ConfigObject<ConfigValue> * _config) :
    m_pGroup(_group),
    m_pConfig(_config),
    m_dCurrentSample(0) {
}

EngineControl::~EngineControl() {

}
