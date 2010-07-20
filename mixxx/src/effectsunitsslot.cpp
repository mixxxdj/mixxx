/*
 * effectsunitsslot.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#include "effectsunitsslot.h"

EffectsUnitsSlot::EffectsUnitsSlot(EffectsUnitsController *, QDomNode node) {
	m_pEffectsUnitsWidgets = new QList<EffectsUnitsWidget *>();

	while (!node.isNull()){
		qDebug() << "FXUNITS: 	Skin: " << node.nodeName();

		/* Buttons for applying effects on deck X */
		if (node.nodeName() == "PushButton"){
			// TODO - Buttons for enabling on deck X

		/* Our precious Widgets! */
		} else if (node.nodeName() == "EffectsUnitsWidget"){
			m_pEffectsUnitsWidgets->append(new EffectsUnitsWidget(node.firstChild()));
		}

		node = node.nextSibling();
	}

}

EffectsUnitsSlot::~EffectsUnitsSlot() {
	// TODO Auto-generated destructor stub
}
