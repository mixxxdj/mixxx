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
	QString * m_pPresetFor;

	int m_WetDryPortIndex;
	QList<int> * m_pBindedPortIndex;
};

#endif /* EFFECTSUNITSPRESET_H_ */
