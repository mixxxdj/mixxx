/*
 * effectsunitsinstance.h
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSINSTANCE_H_
#define EFFECTSUNITSINSTANCE_H_

#include "effectsunitsplugin.h"
#include "effectsunitspreset.h"
#include "../controlobject.h"
#include "../controlpotmeter.h"

class EffectsUnitsPlugin;

class EffectsUnitsInstance {
public:
	EffectsUnitsInstance(EffectsUnitsPlugin * Plugin, int WidgetID, int KnobCount, EffectsUnitsPreset * Preset);
	virtual ~EffectsUnitsInstance();

	void updatePorts();
	void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
	double getWetDry();
	bool getEnabled();
	void activate();

	void applyPreset(EffectsUnitsPreset * Preset);
	EffectsUnitsPlugin * getPlugin();
	int getInstanceID();
	static int getNextInstanceID();
	QString getPortNameByIndex(int Index);

private:

	static int nextInstanceID;
	int m_InstanceID;
	int m_WidgetID;
	int m_KnobCount;
	EffectsUnitsPlugin * m_pPlugin;

	ControlObject * m_pOnOff;
	ControlObject * m_pWetDry;
	QList<ControlObject *> * m_pBindings;
	QList<int> * m_pBindedPortIndex;


	int m_WetDryPort;
};

#endif /* EFFECTSUNITSINSTANCE_H_ */
