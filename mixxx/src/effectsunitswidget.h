/*
 * effectsunitswidget.h
 *
 *  Created on: Jul 14, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSWIDGET_H_
#define EFFECTSUNITSWIDGET_H_

#include <QtCore>
#include <QtGui>
#include <QtXml>

#include "widget/wpushbutton.h"
#include "widget/wknob.h"
#include "effectsunitsslot.h"
#include "effectsunits/effectsunitsinstance.h"
#include "effectsunits/effectsunitscontroller.h"
#include "controlpushbutton.h"

class EffectsUnitsSlot;

class EffectsUnitsWidget : public QWidget{

	Q_OBJECT

public:
	EffectsUnitsWidget(QWidget * parent, QDomNode node, EffectsUnitsController * controller);
	virtual ~EffectsUnitsWidget();

	int getInstanceID();
	void updateLabels();

public slots:
	void slotEffectChanged(QString);

private:
	int m_WidgetID;
	EffectsUnitsController * m_pController;								// Our fx controller
	EffectsUnitsInstance * m_pCurrentInstance, * m_pPreviousInstance;	// Previous and Current instance
	EffectsUnitsSlot * m_pSlot;
	QList<QLabel*> m_Labels;	// Instance parameter labels
	int m_KnobCount;			// Number of knobs displayed in this widget
	QComboBox * m_ComboBox;		// Fx names

private:
	static int getNextID();
	static int NextID;
};

#endif /* EFFECTSUNITSWIDGET_H_ */
