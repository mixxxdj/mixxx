/*
 * effectsunitspreset.cpp
 *
 *  Created on: Aug 10, 2010
 *      Author: bruno
 */

#include "effectsunitspreset.h"

EffectsUnitsPreset::EffectsUnitsPreset() {
	// TODO Auto-generated constructor stub

}

EffectsUnitsPreset::EffectsUnitsPreset(QDomElement Preset){

}

EffectsUnitsPreset::~EffectsUnitsPreset() {
	// TODO Auto-generated destructor stub
}

QString EffectsUnitsPreset::presetFor(){
	return m_PresetFor;
}

QList<int> * EffectsUnitsPreset::getBindedPortIndex(){
	return m_pBindedPortIndex;
}

int EffectsUnitsPreset::getWetDryPortIndex(){
	return m_WetDryPortIndex;
}


