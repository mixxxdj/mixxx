/*
 * effectsunitsinstance.h
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSINSTANCE_H_
#define EFFECTSUNITSINSTANCE_H_

#include "effectsunitsplugin.h"

class EffectsUnitsPlugin;

class EffectsUnitsInstance {
public:
	EffectsUnitsInstance(EffectsUnitsPlugin *);
	virtual ~EffectsUnitsInstance();

	EffectsUnitsPlugin * getPlugin();

private:
	EffectsUnitsPlugin * m_pPlugin;
};

#endif /* EFFECTSUNITSINSTANCE_H_ */
