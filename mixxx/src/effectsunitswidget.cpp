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
	int jk = 2;
	while (!node.isNull()){
			qDebug() << "FXUNITS: 		Skin: " << node.nodeName();

			/* Combobox for use to choose desired fx */
			if (node.nodeName() == "EffectsComboBox"){
				QComboBox * combo = new QComboBox(this);
				combo->addItem("FX");
				lay->addWidget(combo, 0, 1);
				combo->setFixedSize(100, 15);
				combo->show();

			/* Enable/Disable Fx */
			} else if (node.nodeName() == "PushButton"){
				WPushButton * push = new WPushButton(this);
				lay->addWidget(push, 0, 0);
				push->setup(node);
				push->show();

			/* Knobs for controlling fx parameters */
			} else if (node.nodeName() == "Knob"){
				WKnob * knob = new WKnob(this);
				knob->setup(node);
				lay->addWidget(knob, 0, jk);
				knob->show();

			/* Parameter labels */
			} else if (node.nodeName() == "Label"){
				QLabel * label = new QLabel("BigLabelMe", this);
				lay->addWidget(label, 1, jk);
				label->show();
				jk++;

			}

			node = node.nextSibling();
		}
}

EffectsUnitsWidget::~EffectsUnitsWidget() {
	// TODO Auto-generated destructor stub
}
