/*
 * effectsunitsinstance.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#include "effectsunitsinstance.h"

int EffectsUnitsInstance::nextInstanceID = 0;

EffectsUnitsInstance::EffectsUnitsInstance(EffectsUnitsPlugin * Plugin,  int WidgetID, int KnobCount) {
	m_pPlugin = Plugin;
	m_pBindings = new QList<ControlObject *>();
	m_InstanceID = getNextInstanceID();

	qDebug() << "FXUNITS: EffectsUnitsIntance: " << m_InstanceID;

	QList<EffectsUnitsPort *> * ports = Plugin->getPorts();

	int PortCount = ports->size();

	int Porti = 0;
	int Knobi = 1;

	/* Defaults wet/dry to port 0 */
	m_WetDryPort = 0;
	ConfigKey * key = new ConfigKey("[FX]", "Widget" + QString::number(WidgetID) + "Parameter0");
	ControlObject * potmeter = new ControlPotmeter(*key, 0.0, 1.0);
	qDebug() << "FXUNITS: W&D: Widget" << WidgetID << "Parameter 0";

	/* Binds every other non-audio port to available knobs */
	while(Porti < PortCount && Knobi < KnobCount){
		if (ports->at(Porti)->isAudio){

			m_pBindings->prepend(NULL);
			Porti++;

		} else {

			qDebug() << "FXUNITS: PORT: Widget" << WidgetID << "Parameter" << Knobi;
		    ConfigKey * key = new ConfigKey("[FX]", "Widget" + QString::number(WidgetID) + "Parameter" + QString::number(Knobi));
		    ControlObject * potmeter = new ControlPotmeter(*key, ports->at(Porti)->Min, ports->at(Porti)->Max);
		    m_pBindings->prepend(potmeter);
		    Knobi++;
		    Porti++;

		}
	}

}

EffectsUnitsInstance::~EffectsUnitsInstance() {
	// TODO Auto-generated destructor stub
}

EffectsUnitsPlugin * EffectsUnitsInstance::getPlugin(){
	return m_pPlugin;
}

int EffectsUnitsInstance::getInstanceID(){
	return m_InstanceID;
}

int EffectsUnitsInstance::getNextInstanceID(){
	return nextInstanceID++;
}
