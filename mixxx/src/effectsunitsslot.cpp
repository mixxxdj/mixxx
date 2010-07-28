/*
 * effectsunitsslot.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#include "effectsunitsslot.h"
#include "widget/wpushbutton.h"

int EffectsUnitsSlot::NextID = 1;

EffectsUnitsSlot::EffectsUnitsSlot(QWidget * parent, EffectsUnitsController * controller, QDomNode node) : QWidget(parent) {
	QGridLayout * lay = new QGridLayout(this);
	this->setLayout(lay);

	m_SlotID = getNextID();
	m_pController = controller;
	m_pEffectsUnitsWidgets = new QList<EffectsUnitsWidget *>();

	int jp = 0;
	int iw = 0;
	while (!node.isNull()){
		//qDebug() << "FXUNITS: 	Skin: " << node.nodeName();

		/* Buttons for applying effects on deck X */
		if (node.nodeName() == "PushButton"){

			ConfigKey * key = new ConfigKey("[FX]", "Slot" + QString::number(m_SlotID) + "Deck" + QString::number(jp+1) + "OnOff");
			ControlPushButton * deckbutton = new ControlPushButton(*key);
			WPushButton * push = new WPushButton(this);
			lay->addWidget(push, 0, jp);
			push->setup(node);
			push->show();
			jp++;

			m_DeckButtons.append(push);

		/* Our precious Widgets! */
		} else if (node.nodeName() == "EffectsUnitsWidget"){
			EffectsUnitsWidget * wid = new EffectsUnitsWidget(this, node.firstChild(), m_pController);
			m_pEffectsUnitsWidgets->append(wid);
			lay->addWidget(wid, iw, jp);
			iw++;
		}

		node = node.nextSibling();
	}

	/* Connecting DeckButtons, using QSignalMapper to avoid being bond to 2 decks */
	m_pSignalMapper = new QSignalMapper(this);
	int size = m_DeckButtons.size();
	for (int i = 0; i < size; i++){
		m_pSignalMapper->setMapping(m_DeckButtons.at(i), i);
		connect(m_DeckButtons.at(i), SIGNAL(valueChangedLeftDown(double)), m_pSignalMapper, SLOT(map()));
	}
	connect(m_pSignalMapper, SIGNAL(mapped(const int)), this, SLOT(slotDeckUpdated(const int)));

}

EffectsUnitsSlot::~EffectsUnitsSlot() {
	// TODO Auto-generated destructor stub
}

int EffectsUnitsSlot::getNextID(){
	return NextID++;
}

void EffectsUnitsSlot::slotDeckUpdated(int i){
	if (m_DeckButtons.at(i)->getValue() > 0){
		activateOnDeck(i);
	} else {
		deactivateOnDeck(i);
	}
}

void EffectsUnitsSlot::activateOnDeck(int i){
	int size = m_pEffectsUnitsWidgets->size();
	for (int i = 0; i < size; i++){
		m_pController->addInstanceToSource(m_pEffectsUnitsWidgets->at(i)->getInstanceID(), "[Channel1]");
	}
}

void EffectsUnitsSlot::deactivateOnDeck(int i){
	int size = m_pEffectsUnitsWidgets->size();
	for (int i = 0; i < size; i++){
		m_pController->removeInstanceFromSource(m_pEffectsUnitsWidgets->at(i)->getInstanceID(), "[Channel1]");
	}
}
