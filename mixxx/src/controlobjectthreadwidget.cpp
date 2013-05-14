#include <QtDebug>
#include <qapplication.h>
#include "controlobjectthreadwidget.h"
#include "controlobject.h"
#include "controlevent.h"

ControlObjectThreadWidget::ControlObjectThreadWidget(ControlObject * pControlObject, QObject* pParent)
        : ControlObjectThreadMain(pControlObject, pParent) {
    // ControlObjectThread's constructor sets m_dValue to
    // m_pControlObject->get(). Since we represent the widget's value, we need
    // to reset m_dValue to be the result of getValueToWidget.
    if (m_pControl != NULL) {
        set(get());
        slotValueChanged(get(), NULL);
    }
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
    double value = ControlObjectThreadMain::get();
    return m_pControlObject ? m_pControlObject->getValueToWidget(value) : value;
}

void ControlObjectThreadWidget::set(double v) {
    double translated = m_pControlObject ? m_pControlObject->getValueFromWidget(v) : v;
    ControlObjectThreadMain::set(translated);
}

void ControlObjectThreadWidget::add(double v) {
    // TODO(rryan): Double-check that this is ok. I think it requires
    // distributive properties of getValueFromWidget() which might not always be
    // the case.
    double translated = m_pControlObject ? m_pControlObject->getValueFromWidget(v) : v;
    ControlObjectThreadMain::add(translated);
}

void ControlObjectThreadWidget::sub(double v) {
    // TODO(rryan): Double-check that this is ok. I think it requires
    // distributive properties of getValueFromWidget() which might not always be
    // the case.
    double translated = m_pControlObject ? m_pControlObject->getValueFromWidget(v) : v;
    ControlObjectThreadMain::sub(translated);
}

// Receives the Value from the parent and may scales the vale and re-emit it again
void ControlObjectThreadWidget::slotValueChanged(double v, QObject* pSetter) {
    double translated = m_pControlObject ? m_pControlObject->getValueToWidget(v) : v;
    ControlObjectThreadMain::slotValueChanged(translated, pSetter);
}
