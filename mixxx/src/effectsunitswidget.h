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

class EffectsUnitsWidget : public QWidget{

	Q_OBJECT

public:
	EffectsUnitsWidget(QWidget * parent, QDomNode node);
	virtual ~EffectsUnitsWidget();
};

#endif /* EFFECTSUNITSWIDGET_H_ */
