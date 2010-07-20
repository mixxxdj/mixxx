/*
 * effectsunitsslot.h
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSSLOT_H_
#define EFFECTSUNITSSLOT_H_

#include "effectsunits/effectsunitscontroller.h"
#include "effectsunitswidget.h"

class EffectsUnitsSlot {
public:
	EffectsUnitsSlot(EffectsUnitsController * m_pController, QDomNode node);
	virtual ~EffectsUnitsSlot();

private:
	QList<EffectsUnitsWidget *> * m_pEffectsUnitsWidgets;
};

#endif /* EFFECTSUNITSSLOT_H_ */
