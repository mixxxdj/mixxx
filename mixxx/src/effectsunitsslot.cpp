/*
 * effectsunitsslot.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#include "effectsunitsslot.h"
#include "widget/wpushbutton.h"

EffectsUnitsSlot::EffectsUnitsSlot(QWidget * parent, EffectsUnitsController *, QDomNode node) : QWidget(parent) {
	QGridLayout * lay = new QGridLayout(this);
	this->setLayout(lay);

	m_pEffectsUnitsWidgets = new QList<EffectsUnitsWidget *>();
	int jp = 0;
	int iw = 0;
	while (!node.isNull()){
		qDebug() << "FXUNITS: 	Skin: " << node.nodeName();

		/* Buttons for applying effects on deck X */
		if (node.nodeName() == "PushButton"){
			// TODO - Buttons for enabling on deck X
			WPushButton * push = new WPushButton(this);
			lay->addWidget(push, 0, jp);
			push->setup(node);
			push->show();
			jp++;

		/* Our precious Widgets! */
		} else if (node.nodeName() == "EffectsUnitsWidget"){
			EffectsUnitsWidget * wid = new EffectsUnitsWidget(this, node.firstChild());
			m_pEffectsUnitsWidgets->append(wid);
			lay->addWidget(wid, iw, jp);
			iw++;
		}

		node = node.nextSibling();
	}

}

EffectsUnitsSlot::~EffectsUnitsSlot() {
	// TODO Auto-generated destructor stub
}
