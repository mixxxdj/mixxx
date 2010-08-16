/*
 * effectsunitspreset.h
 *
 *  Created on: Aug 10, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSPRESET_H_
#define EFFECTSUNITSPRESET_H_

#include <QtCore>
#include <QtXml>

class EffectsUnitsPreset {
public:
	EffectsUnitsPreset(QDomNode Preset);
	virtual ~EffectsUnitsPreset();

	QString presetFor();
	QDomDocument toXML();
	QList<int> * getBindedPortIndex();
	int getWetDryPortIndex();
	void setBindedPortsFromXML(QDomNode node);

private:
	QString * m_pPresetFor; 			/* Plugin Name */
	int m_WetDryPortIndex;				/* Wet/Dry port Index, -1 if none */
	QList<int> * m_pBindedPortIndex;	/* Knob <--> Port Mappings */
};

#endif /* EFFECTSUNITSPRESET_H_ */
