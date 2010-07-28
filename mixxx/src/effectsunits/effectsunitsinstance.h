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
#include "../controlpotmeter.h"

class EffectsUnitsPlugin;

class EffectsUnitsInstance {
public:
	EffectsUnitsInstance(EffectsUnitsPlugin * Plugin, int WidgetID, int KnobCount);
	virtual ~EffectsUnitsInstance();

	void updatePorts();
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
	double getWetDry();
	bool getEnabled();
	void activate();

	EffectsUnitsPlugin * getPlugin();
	int getInstanceID();
	static int getNextInstanceID();

private:

	static int nextInstanceID;
	int m_InstanceID;
	EffectsUnitsPlugin * m_pPlugin;

	ControlObject * m_pOnOff;
	ControlObject * m_pWetDry;
	QList<ControlObject *> * m_pBindings;

	int m_WetDryPort;
};

#endif /* EFFECTSUNITSINSTANCE_H_ */
