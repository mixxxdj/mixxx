/*
 * effectsunitspreset.h
 *
 *  Created on: Aug 10, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSPRESET_H_
#define EFFECTSUNITSPRESET_H_

class EffectsUnitsPreset {
public:
	EffectsUnitsPreset();
	EffectsUnitsPreset(QDomDocument File);
	virtual ~EffectsUnitsPreset();

	QDomDocument toXML();
};

#endif /* EFFECTSUNITSPRESET_H_ */
