/*
 * effectsunitsinstance.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#include "effectsunitsinstance.h"

EffectsUnitsInstance::EffectsUnitsInstance(EffectsUnitsPlugin * Plugin) {
	m_pPlugin = Plugin;
}

EffectsUnitsInstance::~EffectsUnitsInstance() {
	// TODO Auto-generated destructor stub
}

EffectsUnitsPlugin * EffectsUnitsInstance::getPlugin(){
	return m_pPlugin;
}
