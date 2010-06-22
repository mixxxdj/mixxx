#ifndef EFFECTSUNITSVIEW_H
#define EFFECTSUNITSVIEW_H

#include <QtCore>
#include <QtGui>

#include "configobject.h"
#include "controlpushbutton.h"

class EffectsUnitsView : public QWidget
{
    Q_OBJECT

public:
    EffectsUnitsView(QWidget *parent);
    ~EffectsUnitsView();
    void slotControlNoob(double);

    ControlPushButton * noobButton;
};

#endif
