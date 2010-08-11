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
	EffectsUnitsPreset();
	EffectsUnitsPreset(QDomElement Preset);
	virtual ~EffectsUnitsPreset();

	QString presetFor();
	QDomDocument toXML();

	QList<int> * getBindedPortIndex();
	int getWetDryPortIndex();


private:
	QString m_PresetFor;

	int m_WetDryPortIndex;
	QList<int> * m_pBindedPortIndex;
};

#endif /* EFFECTSUNITSPRESET_H_ */
