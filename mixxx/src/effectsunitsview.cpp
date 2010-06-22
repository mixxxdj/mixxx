#include <QtCore>
#include <QtGui>

#include "effectsunitsview.h"
#include "effectsunits/effectsunitscontroller.h"

EffectsUnitsView::EffectsUnitsView(QWidget * parent) : QWidget(parent)
{
    setObjectName("EffectsUnits");

    EffectsUnitsController * fx = new EffectsUnitsController();




}

EffectsUnitsView::~EffectsUnitsView()
{

}


