/*
 * effectsunitsslot.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#include "effectsunitsslot.h"
#include "widget/wpushbutton.h"

int EffectsUnitsSlot::NextID = 1;

/* Effects Units Slot.
 * Slots can have many Widgets.
 * Slots have Deck Buttons, which apply the Effects from Widgets
 * on the corresponding Deck.
 * This is all skin based.
 */
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

/* EffectsUnitsSlot::slotDeckUpdated
 * Is signalled when a Deck button is hit.
 * Then we gotta see if that means to activate or deactivate.
 */
void EffectsUnitsSlot::slotDeckUpdated(int i){
	if (deckIsActivated(i)){
		activateOnDeck(i);
	} else {
		deactivateOnDeck(i);
	}
}

/* EffectsUnitsSlot::activateOnDeck
 * For every Widget on our Slot, add Widget's instance
 * to processing queue of the given Channel
 */
void EffectsUnitsSlot::activateOnDeck(int iDeck){
	int size = m_pEffectsUnitsWidgets->size();
	for (int i = 0; i < size; i++){
		if (iDeck == 0)
			m_pController->addInstanceToSource(m_pEffectsUnitsWidgets->at(i)->getInstanceID(), "[Channel1]");
		else
			m_pController->addInstanceToSource(m_pEffectsUnitsWidgets->at(i)->getInstanceID(), "[Channel2]");
	}
}

/* EffectsUnitsSlot::deactivateOnDeck
 * For every Widget on our Slot, remove Widget's instance
 * from processing queue of the given Channel
 */
void EffectsUnitsSlot::deactivateOnDeck(int iDeck){
	int size = m_pEffectsUnitsWidgets->size();
	for (int i = 0; i < size; i++){
		if (iDeck == 0)
			m_pController->removeInstanceFromSource(m_pEffectsUnitsWidgets->at(i)->getInstanceID(), "[Channel1]");
		else
			m_pController->removeInstanceFromSource(m_pEffectsUnitsWidgets->at(i)->getInstanceID(), "[Channel2]");
	}
}

bool EffectsUnitsSlot::deckIsActivated(int i){
	return (m_DeckButtons.at(i)->getValue() > 0);
}

/* EffectsUnitsSlot::changedWidgetEffect
 * If a Widget suddenly changes effect, we have to mantain the
 * current Deck setup, e.g. if Deck 1 is activated, we need to
 * add the new instance to the Deck 1 processing queue.
 */
void EffectsUnitsSlot::changedWidgetEffect(int PreviousID, int CurrentID){
	m_pController->removeInstance(PreviousID);
	int size = m_DeckButtons.size();
	for (int i = 0; i < size; i++){
		if (deckIsActivated(i)){
			if (i == 0)
				m_pController->addInstanceToSource(CurrentID, "[Channel1]");
			else
				m_pController->addInstanceToSource(CurrentID, "[Channel2]");
		}
	}
}
