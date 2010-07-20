/*
 * effectsunitswidget.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#include "effectsunitswidget.h"

EffectsUnitsWidget::EffectsUnitsWidget(QDomNode node) {
	qDebug() << "FXUNITS: 		Skin: " << node.nodeName();
	while (!node.isNull()){
			qDebug() << "FXUNITS: 		Skin: " << node.nodeName();

			if (node.nodeName() == "EffectsComboBox"){
				// TODO - FXComboBox

			} else if (node.nodeName() == "PushButton"){
				// TODO - ControlObject for On/Off

			} else if (node.nodeName() == "Knob"){
				// TODO - ControlObject for On/Off

			} else if (node.nodeName() == "Label"){
				// TODO - ControlObject for On/Off
			}

			node = node.nextSibling();
		}
}

EffectsUnitsWidget::~EffectsUnitsWidget() {
	// TODO Auto-generated destructor stub
}
