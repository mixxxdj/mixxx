#ifndef EFFECTSUNITSVIEW_H
#define EFFECTSUNITSVIEW_H

#include <QtCore>
#include <QtGui>

#include "configobject.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"
#include "widget/wknob.h"
#include "effectsunits/effectsunitscontroller.h"
#include "effectsunits/effectsunitspresetmanager.h"
#include "effectsunitsslot.h"

class QWidget;

class EffectsUnitsView : public QWidget
{
    Q_OBJECT
public:
    EffectsUnitsView(QWidget *parent);
    ~EffectsUnitsView();

private:
    QGridLayout * m_pGridLayout;
    EffectsUnitsController * m_EffectsUnitsController;
    QList<EffectsUnitsSlot *> * m_EffectsUnitsSlots;
    EffectsUnitsPresetManager * m_EffectsUnitsPresetManager;
    QLabel * m_label;
    WKnob * m_pDryWetKnob;
};

#endif
