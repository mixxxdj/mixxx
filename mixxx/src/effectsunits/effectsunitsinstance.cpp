/*
 * effectsunitsinstance.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#include "effectsunitsinstance.h"

int EffectsUnitsInstance::nextInstanceID = 1;

/* Effects Units Instance.
 * Instances were brought up so we can share the same plugin along many Widgets.
 * This way, every instance has their own port values for the plugin to process.
 * This is done by binding itself to given Widgets ControlObjects.
 * Instances mantain communication with the Plugin, specially for Backend operations.
 */
EffectsUnitsInstance::EffectsUnitsInstance(EffectsUnitsPlugin * Plugin,  int WidgetID, int KnobCount) {
	m_pPlugin = Plugin;
	m_pBindings = new QList<ControlObject *>();
	m_InstanceID = getNextInstanceID();

	qDebug() << "FXUNITS: EffectsUnitsIntance: " << m_InstanceID;

	/* Defaults wet/dry to a non-port, but knob0 */
	m_WetDryPort = -1;
	ConfigKey * keywd = new ConfigKey("[FX]", "Widget" + QString::number(WidgetID) + "Parameter0");
    ControlObject * wd = ControlObject::getControl(*keywd);
    ControlPotmeter * potwd = dynamic_cast<ControlPotmeter *>(wd);
    if (potwd != NULL){
    	potwd->setRange(0.0, 1.0);
    	m_pWetDry = potwd;
    } else {
    	qDebug() << "FXUNITS: Something's odd, EffectsUnitsInstance";
    }

	QList<EffectsUnitsPort *> * ports = Plugin->getPorts();
	int PortCount = ports->size();
	int Porti = 0;
	int Knobi = 1;

	/* Binds every other non-audio port to available knobs */
	while(Porti < PortCount && Knobi < KnobCount){
		if (ports->at(Porti)->isAudio){

			m_pBindings->append(NULL);
			Porti++;

		} else {

		    ConfigKey * key = new ConfigKey("[FX]", "Widget" + QString::number(WidgetID) + "Parameter" + QString::number(Knobi));
		    ControlObject * co = ControlObject::getControl(*key);
		    ControlPotmeter * potmeter = dynamic_cast<ControlPotmeter *>(co);

		    if (potmeter != NULL){
				potmeter->setRange(ports->at(Porti)->Min, ports->at(Porti)->Max);
				m_pBindings->append(potmeter);
				m_BindedPortIndex.append(Porti);
				Knobi++;
				Porti++;
		    } else {
		    	qDebug() << "FXUNITS: Something's odd, EffectsUnitsInstance";
		    }

		}
	}

	/* Care for an OnOff button? */
	ConfigKey * keyb = new ConfigKey("[FX]", "Widget" + QString::number(WidgetID) + "OnOff");
	ControlObject * button = ControlObject::getControl(*keyb);
	m_pOnOff = button;

}

/* EffectsUnitsInstance::updatePorts
 * Updates the Plugin with our values from the Widget.
 * So we process the way the Widget wants.
 */
void EffectsUnitsInstance::updatePorts(){
	/* Updating Port Values: */
	int size = m_pBindings->size();
	for (int i = 0; i < size; i++){
		if (m_pBindings->at(i) == NULL){
			// NOP, Audio Port
		} else {
			getPlugin()->connect(i, m_pBindings->at(i)->get());
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

bool EffectsUnitsInstance::getEnabled(){
	return (m_pOnOff->get() > 0);
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

QString EffectsUnitsInstance::getPortNameByIndex(int Index){
	if (Index < m_BindedPortIndex.size()){
		return getPlugin()->getPortNameByIndex(m_BindedPortIndex.at(Index));
	} else {
		return "Not Used";
	}
}
