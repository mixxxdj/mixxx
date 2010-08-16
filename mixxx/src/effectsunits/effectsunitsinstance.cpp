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
EffectsUnitsInstance::EffectsUnitsInstance(EffectsUnitsPlugin * Plugin,  int WidgetID, int KnobCount, EffectsUnitsPreset * Preset) {
	m_pPlugin = Plugin;
	m_pBindings = new QList<ControlObject *>();
	m_InstanceID = getNextInstanceID();
	m_WidgetID = WidgetID;
	m_KnobCount = KnobCount;

	qDebug() << "FXUNITS: EffectsUnitsIntance: " << m_InstanceID;

	/* Initializing our CO's for Ports, WetDry defaults to Parameter0
	 * Ranges will be defined on applyPreset()
	 */
	for (int i = 0; i < m_KnobCount; i++){
		ConfigKey * key = new ConfigKey("[FX]", "Widget" + QString::number(m_WidgetID) + "Parameter" + QString::number(i));
		ControlObject * co = ControlObject::getControl(*key);
		ControlPotmeter * potmeter = dynamic_cast<ControlPotmeter *>(co);
		if (potmeter != NULL) m_pBindings->append(potmeter);
	}
	m_pWetDry = m_pBindings->at(0);

	/* Applies the Preset for Ports/Parameter customization */
	applyPreset(Preset);

	/* Care for an OnOff button? */
	ConfigKey * keyb = new ConfigKey("[FX]", "Widget" + QString::number(m_WidgetID) + "OnOff");
	ControlObject * button = ControlObject::getControl(*keyb);
	m_pOnOff = button;

}

/* EffectsUnitsInstance::applyPreset
 * Applies preset preferences to instance or default behavior when there's no preset.
 */
void EffectsUnitsInstance::applyPreset(EffectsUnitsPreset * Preset){

	QList<EffectsUnitsPort *> * ports = m_pPlugin->getPorts();
	int PortCount = ports->size();
	ControlPotmeter * potmeter;

	/* Defines Wet/Dry Port */
	if (Preset == NULL){

		/* Defaults wet/dry to a non-port, but knob0 */
		m_WetDryPort = -1;
		potmeter = dynamic_cast<ControlPotmeter *>(m_pWetDry);
		if (potmeter != NULL) potmeter->setRange(0.0, 1.0);

	} else {

		/* Gets the Wet/Dry index from Preset and set according ranges */
		m_WetDryPort = Preset->getWetDryPortIndex();
		potmeter = dynamic_cast<ControlPotmeter *>(m_pWetDry);

		/* Preset might or mightnt bind a custom wetdry
		 * If, however the preset is wrong, uses default wetdry
		 */
		if (m_WetDryPort == -1 || m_WetDryPort > ports->size()){
			if (potmeter != NULL) potmeter->setRange(0.0, 1.0);
		} else {
			if (potmeter != NULL) potmeter->setRange(ports->at(m_WetDryPort)->Min, ports->at(m_WetDryPort)->Max);
		}
		//FIXME - BadPreset = SEGFAULT

	}

	/* Defines Parameters Ports */
	if (Preset == NULL){

		m_pBindedPortIndex = new QList<int>();
		int Porti = 0;
		int Knobi = 1;

		/* Binds the first |KnobCount-1| non-audio ports.
		 * Wet/Dry is Parameter0 so we start at 1.
		 */
		while(Porti < PortCount && Knobi < m_KnobCount){

			/* Binds non-audio ports only */
			if (ports->at(Porti)->isAudio){
				Porti++;
			} else {
				m_pBindedPortIndex->append(Porti);
				potmeter = dynamic_cast<ControlPotmeter *>(m_pBindings->at(Knobi));
				if (potmeter != NULL) potmeter->setRange(ports->at(Porti)->Min, ports->at(Porti)->Max);
				Porti++;
				Knobi++;
			}
		}

	} else {

		/* BindedPortIndex comes from Preset
		 * Bind ports accordingly.
		 */
		m_pBindedPortIndex = new QList<int>(*(Preset->getBindedPortIndex()));
		int Knobi = 1;
		int Porti = 0;
		while (Knobi < m_KnobCount && Porti < m_pBindedPortIndex->size()){
			potmeter = dynamic_cast<ControlPotmeter *>(m_pBindings->at(Knobi));
			if (potmeter != NULL)
				potmeter->setRange(ports->at(m_pBindedPortIndex->at(Porti))->Min, ports->at(m_pBindedPortIndex->at(Porti))->Max);
			//FIXME - BadPreset = SEGFAULT
			Porti++;
			Knobi++;
		}

	}
}

/* EffectsUnitsInstance::updatePorts
 * Updates the Plugin with our values from the Widget.
 * So we process the way the Widget wants.
 */
void EffectsUnitsInstance::updatePorts(){

	/* Updating Port Values: */
	int Knobi = 1;
	int Porti = 0;
	while (Knobi < m_KnobCount && Porti < m_pBindedPortIndex->size()){
		getPlugin()->connect(m_pBindedPortIndex->at(Porti), m_pBindings->at(Knobi)->get());
		Knobi++;
		Porti++;
	}

	/* If WetDry is a parameter port, update it as well */
	if (m_WetDryPort > -1){
		getPlugin()->connect(m_WetDryPort, m_pWetDry->get());
	}

}

void EffectsUnitsInstance::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize){
	getPlugin()->process(pIn, pOut, iBufferSize, getWetDry());
}

void EffectsUnitsInstance::activate(){
	getPlugin()->activate();
}

double EffectsUnitsInstance::getWetDry(){
	if (m_WetDryPort == -1){
		return m_pWetDry->get();
	} else {

		/* Normalization of custom WetDry */
		ControlPotmeter * potmeter = dynamic_cast<ControlPotmeter *>(m_pWetDry);
		if (potmeter != NULL)
			return (potmeter->get() - potmeter->getMin()) / (potmeter->getMax() - potmeter->getMin());
		else
			return 0;
	}
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
	if (Index < m_pBindedPortIndex->size()){
		return getPlugin()->getPortNameByIndex(m_pBindedPortIndex->at(Index));
	} else {
		return "Not Used";
	}
}
