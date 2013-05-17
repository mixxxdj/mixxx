#include <QtDebug>
#include <qapplication.h>
#include "controlobjectthreadwidget.h"
#include "controlobject.h"
#include "controlevent.h"

ControlObjectThreadWidget::ControlObjectThreadWidget(ControlObject * pControlObject, QObject* pParent)
        : ControlObjectThreadMain(pControlObject, pParent) {
}

ControlObjectThreadWidget::~ControlObjectThreadWidget() {
}

void ControlObjectThreadWidget::setWidget(QWidget * widget, bool connectValueFromWidget,
                                          bool connectValueToWidget,
                                          EmitOption emitOption, Qt::MouseButton state) {
    if (connectValueFromWidget) {
        connect(widget, SIGNAL(valueReset()),
                this, SLOT(reset()));

        if (emitOption & EMIT_ON_PRESS) {
            switch (state) {
            case Qt::NoButton:
                connect(widget, SIGNAL(valueChangedDown(double)),
                        this, SLOT(slotSet(double)));
                break;
            case Qt::LeftButton:
                connect(widget, SIGNAL(valueChangedLeftDown(double)),
                        this, SLOT(slotSet(double)));
                break;
            case Qt::RightButton:
                connect(widget, SIGNAL(valueChangedRightDown(double)),
                        this, SLOT(slotSet(double)));
                break;
            default:
                break;
            }
        }

        if (emitOption & EMIT_ON_RELEASE) {
            switch (state) {
            case Qt::NoButton:
                connect(widget, SIGNAL(valueChangedUp(double)),
                        this, SLOT(slotSet(double)));
                break;
            case Qt::LeftButton:
                connect(widget, SIGNAL(valueChangedLeftUp(double)),
                        this, SLOT(slotSet(double)));
                break;
            case Qt::RightButton:
                connect(widget, SIGNAL(valueChangedRightUp(double)),
                        this, SLOT(slotSet(double)));
                break;
            default:
                break;
            }
        }
    }

    if (connectValueToWidget) {
        connect(this, SIGNAL(valueChanged(double)),
                widget, SLOT(setValue(double)));
    }
    emit(valueChanged(get()));
}

void ControlObjectThreadWidget::setWidgetOnOff(QWidget* widget)
{
    QApplication::connect(this, SIGNAL(valueChanged(double)),
                          widget, SLOT(setOnOff(double)));
    emit(valueChanged(get()));
}

double ControlObjectThreadWidget::get() {
    return m_pControl ? m_pControl->getWidgetParameter() : 0.0;
}

void ControlObjectThreadWidget::set(double v) {
    if (m_pControl) {
        m_pControl->setWidgetParameter(v, this);
    }
}
