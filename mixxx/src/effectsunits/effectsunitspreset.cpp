/*
 * effectsunitspreset.cpp
 *
 *  Created on: Aug 10, 2010
 *      Author: bruno
 */

#include "effectsunitspreset.h"

/* Effects Units Preset
 * Specifies desired Plugin set-up.
 * Wet/Dry Port (if available)
 * Knob <--> Port Mappings.
 */
EffectsUnitsPreset::EffectsUnitsPreset(QDomNode node){
	m_pBindedPortIndex = new QList<int>;

	while (!node.isNull()){

	    	//qDebug() << "FXUNITS: Preset: " << node.nodeName() << node.toElement().text();

	    	if (node.nodeName() == "EffectsUnitsPlugin"){
	    		m_pPresetFor = new QString(node.toElement().text());

	    	} else if (node.nodeName() == "BindedPorts"){
	    		setBindedPortsFromXML(node.firstChild());

	    	} else if (node.nodeName() == "WetDryPort"){
				m_WetDryPortIndex = node.toElement().text().toInt();
	    	}

	    	/* Next node, please. */
	    	node = node.nextSibling();
	    }
}

EffectsUnitsPreset::~EffectsUnitsPreset() {
	// TODO Auto-generated destructor stub
}

QString EffectsUnitsPreset::presetFor(){
	return (*m_pPresetFor);
}

QList<int> * EffectsUnitsPreset::getBindedPortIndex(){
	return m_pBindedPortIndex;
}

int EffectsUnitsPreset::getWetDryPortIndex(){
	return m_WetDryPortIndex;
}

void EffectsUnitsPreset::setBindedPortsFromXML(QDomNode node){
	while (!node.isNull()){
		//qDebug() << "FXUNITS: Preset: 	" << node.nodeName();

		if (node.nodeName() == "Port"){
			m_pBindedPortIndex->append(node.toElement().text().toInt());
		}

		/* Next node, please. */
		node = node.nextSibling();
	}
}


