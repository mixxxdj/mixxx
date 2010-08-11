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

private:
	QString m_PresetFor;
	QList<int> m_BindedPortIndex;
};

#endif /* EFFECTSUNITSPRESET_H_ */
