/*
 * effectsunitswidget.cpp
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#include "effectsunitswidget.h"

int EffectsUnitsWidget::NextID = 1;

EffectsUnitsWidget::EffectsUnitsWidget(QWidget * parent, QDomNode node, EffectsUnitsController * controller) : QWidget(parent) {
	m_pCurrentInstance = NULL;
	m_pPreviousInstance = NULL;
	m_KnobCount = 0;
	m_pController = controller;
	m_WidgetID = getNextID();

	QGridLayout * lay = new QGridLayout(this);
	this->setLayout(lay);

	qDebug() << "FXUNITS: 		Skin: " << node.nodeName();
	int jk = 2;
	while (!node.isNull()){
			qDebug() << "FXUNITS: 		Skin: " << node.nodeName();

			/* Combobox for use to choose desired fx */
			if (node.nodeName() == "EffectsComboBox"){
				m_ComboBox = new QComboBox(this);
				m_ComboBox->addItems(*(m_pController->getEffectsList()));
				lay->addWidget(m_ComboBox, 0, 1);
				m_ComboBox->setFixedSize(100, 15);

			/* Enable/Disable Fx */
			} else if (node.nodeName() == "PushButton"){
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

void EffectsUnitsWidget::slotEffectChanged(QString Name){
	m_pPreviousInstance = m_pCurrentInstance;

	if (m_pPreviousInstance != NULL){
		// TODO - unload previous instance
	}

	m_pCurrentInstance = m_pController->instantiatePluginForWidget(Name, m_WidgetID, m_KnobCount);
	m_pController->addInstanceToSource(m_pCurrentInstance->getInstanceID(), "[Channel1]");

}

int EffectsUnitsWidget::getNextID(){
	return NextID++;
}

