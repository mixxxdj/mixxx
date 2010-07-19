/*
 * effectsunitsinstance.h
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSINSTANCE_H_
#define EFFECTSUNITSINSTANCE_H_

#include "effectsunitsplugin.h"
#include "../controlobject.h"

class EffectsUnitsPlugin;

class EffectsUnitsInstance {
public:
	EffectsUnitsInstance(EffectsUnitsPlugin *);
	virtual ~EffectsUnitsInstance();

	EffectsUnitsPlugin * getPlugin();

private:

	int m_InstanceID;
	EffectsUnitsPlugin * m_pPlugin;

	ControlObject * m_pOnOff;
	ControlObject * m_pWetDry;
	QList<ControlObject *> * m_pBindings;

	int m_WetDryPort;
};

#endif /* EFFECTSUNITSINSTANCE_H_ */
