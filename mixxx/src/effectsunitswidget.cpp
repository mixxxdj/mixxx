/*
 * effectsunitswidget.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#include "effectsunitswidget.h"

int EffectsUnitsWidget::NextID = 1;

/* Effects Units Widget.
 * A Widget can have one active Instance.
 * A Widget has the On/Off buton for that Instance.
 * A Widget has many Knobs.
 * The number of Knobs is skin based.
 */
EffectsUnitsWidget::EffectsUnitsWidget(QWidget * parent, QDomNode node, EffectsUnitsController * controller) : QWidget(parent) {
	m_pCurrentInstance = NULL;
	m_pPreviousInstance = NULL;
	m_KnobCount = 0;
	m_pController = controller;
	m_WidgetID = getNextID();
	m_pSlot = dynamic_cast<EffectsUnitsSlot *>(parent);

	QGridLayout * lay = new QGridLayout(this);
	this->setLayout(lay);

	int jk = 2;
	while (!node.isNull()){
			//qDebug() << "FXUNITS: 		Skin: " << node.nodeName();

			/* Combobox for use to choose desired fx */
			if (node.nodeName() == "EffectsComboBox"){
				m_ComboBox = new QComboBox(this);
				m_ComboBox->addItems(*(m_pController->getEffectsList()));
				lay->addWidget(m_ComboBox, 0, 1);
				m_ComboBox->setFixedSize(100, 15);

			/* Enable/Disable Fx */
			} else if (node.nodeName() == "PushButton"){

				ConfigKey * key = new ConfigKey("[FX]", "Widget" + QString::number(m_WidgetID) + "OnOff");
				ControlObject * button = new ControlPushButton(*key);
				WPushButton * push = new WPushButton(this);
				lay->addWidget(push, 0, 0);
				push->setup(node);

			/* Knobs for controlling fx parameters */
			} else if (node.nodeName() == "Knob"){

			    ConfigKey * key = new ConfigKey("[FX]", "Widget" + QString::number(m_WidgetID) + "Parameter" + QString::number(m_KnobCount));
			    ControlObject * potmeter = new ControlPotmeter(*key, 0.0, 1.0);

				WKnob * knob = new WKnob(this);
				knob->setup(node);
				lay->addWidget(knob, 0, jk);
				m_KnobCount++;

			/* Parameter labels */
			} else if (node.nodeName() == "Label"){
				QLabel * label = new QLabel("Parameter", this);
				lay->addWidget(label, 1, jk);
				m_Labels.append(label);
				jk++;

			}

			node = node.nextSibling();
		}

	connect(m_ComboBox, SIGNAL(activated(QString)), this, SLOT(slotEffectChanged(QString)));
}

EffectsUnitsWidget::~EffectsUnitsWidget() {
	// TODO Auto-generated destructor stub
}

/* EffectsUnitsWidget::slotEffectChanged
 * When a new Effect is loaded in our widget, we need to properly load
 * it by notifying our parent Slot, activating the Instance and updating
 * the Knob Labels.
 */
void EffectsUnitsWidget::slotEffectChanged(QString Name){
	m_pPreviousInstance = m_pCurrentInstance;
	m_pCurrentInstance = m_pController->instantiatePluginForWidget(Name, m_WidgetID, m_KnobCount);

	if (m_pPreviousInstance != NULL){
		m_pSlot->changedWidgetEffect(m_pPreviousInstance->getInstanceID(), m_pCurrentInstance->getInstanceID());
		updateLabels();
	} else {
		m_pSlot->changedWidgetEffect(NULL, m_pCurrentInstance->getInstanceID());
		updateLabels();
	}
}

int EffectsUnitsWidget::getInstanceID(){
	return ((m_pCurrentInstance != NULL) ? m_pCurrentInstance->getInstanceID() : NULL);
}

int EffectsUnitsWidget::getNextID(){
	return NextID++;
}

/* EffectsUnitsWidget::updateLabels
 * Updates the labels for the parameters.
 * The first one is a Wet/Dry knob by definition.
 */
void EffectsUnitsWidget::updateLabels(){
	m_Labels.at(0)->setText("Wet/Dry");
	for (int i = 0; i < m_KnobCount-1; i++){
		m_Labels.at(i+1)->setText(m_pCurrentInstance->getPortNameByIndex(i)); // TODO - NULL cases, knob > portNumber
	}
}
