#ifndef EFFECTSUNITSVIEW_H
#define EFFECTSUNITSVIEW_H

#include <QtCore>
#include <QtGui>

#include "configobject.h"
#include "controlpushbutton.h"
#include "effectsunits/effectsunitscontroller.h"

class QWidget;

class EffectsUnitsView : public QWidget
{
    Q_OBJECT
public:
    EffectsUnitsView(QWidget *parent);
    ~EffectsUnitsView();

public slots:
	void slotClick(QString);

signals:

private:
    QGridLayout * m_pGridLayout;
    EffectsUnitsController * m_EffectsUnitsController;
};

#endif
