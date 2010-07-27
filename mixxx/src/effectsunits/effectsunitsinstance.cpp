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

	/* Defaults wet/dry to a non-port, but knob0 */
	m_WetDryPort = -1;
	ConfigKey * key = new ConfigKey("[FX]", "Widget" + QString::number(WidgetID) + "Parameter0");
	m_pWetDry = new ControlPotmeter(*key, 0.0, 1.0);
	qDebug() << "FXUNITS: W&D: Widget" << WidgetID << "Parameter 0";

	/* Binds every other non-audio port to available knobs */
	while(Porti < PortCount && Knobi < KnobCount){
		if (ports->at(Porti)->isAudio){

			m_pBindings->append(NULL);
			Porti++;

		} else {

			qDebug() << "FXUNITS: PORT: Widget" + QString::number(WidgetID) + "Parameter" + QString::number(Knobi);
		    ConfigKey * key = new ConfigKey("[FX]", "Widget" + QString::number(WidgetID) + "Parameter" + QString::number(Knobi));
		    ControlObject * potmeter = new ControlPotmeter(*key, ports->at(Porti)->Min, ports->at(Porti)->Max);
		    m_pBindings->append(potmeter);
		    Knobi++;
		    Porti++;

		}
	}

}

void EffectsUnitsInstance::updatePorts(){
	/* Updating Port Values: */
	int size = m_pBindings->size();
	for (int i = 0; i < size; i++){
		if (m_pBindings->at(i) == NULL){
			// NOP, Audio Port
		} else {
			getPlugin()->connect(i, m_pBindings->at(i)->get());
			qDebug() << "FXUNITS: VA:" << i << m_pBindings->at(i)->get();
		}
	}
}

void EffectsUnitsInstance::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize){
	getPlugin()->process(pIn, pOut, iBufferSize, getWetDry());
}

void EffectsUnitsInstance::activate(){
	getPlugin()->activate();
}

double EffectsUnitsInstance::getWetDry(){
	// TODO - normalize wet/dry if not on standard port
	//qDebug() << "FXUNITS: WD: " << m_pWetDry->get();
	return m_pWetDry->get();
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
