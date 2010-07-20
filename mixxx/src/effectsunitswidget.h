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

class EffectsUnitsWidget {
public:
	EffectsUnitsWidget(QDomNode node);
	virtual ~EffectsUnitsWidget();
};

#endif /* EFFECTSUNITSWIDGET_H_ */
