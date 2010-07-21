/*
 * effectsunitswidget.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#include "effectsunitswidget.h"

EffectsUnitsWidget::EffectsUnitsWidget(QWidget * parent, QDomNode node) : QWidget(parent) {
	QGridLayout * lay = new QGridLayout(this);
	this->setLayout(lay);

	qDebug() << "FXUNITS: 		Skin: " << node.nodeName();
	int jk = 0;
	while (!node.isNull()){
			qDebug() << "FXUNITS: 		Skin: " << node.nodeName();



			if (node.nodeName() == "EffectsComboBox"){
				// TODO - FXComboBox

			} else if (node.nodeName() == "PushButton"){
				// TODO - ControlObject for On/Off

			} else if (node.nodeName() == "Knob"){
				WKnob * knob = new WKnob(this);
				knob->setup(node);
				lay->addWidget(knob, 0, jk);
				knob->show();
				jk++;


			} else if (node.nodeName() == "Label"){
				// TODO - ControlObject for On/Off
			}

			node = node.nextSibling();
		}
}

EffectsUnitsWidget::~EffectsUnitsWidget() {
	// TODO Auto-generated destructor stub
}
