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
    if (m_pControlObject != NULL) {
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
                this, SLOT(slotReset()));

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

void ControlObjectThreadWidget::slotReset() {
    if (m_pControlObject) {
        // TODO(rryan): This change will come from the wrong setter. We need to
        // call the ControlNumericPrivate ourselves here.
        m_pControlObject->reset();
    }
}

double ControlObjectThreadWidget::get() {
    if (m_pControlObject) {
        return m_pControlObject->getValueToWidget(m_pControlObject->get());
    } else {
        return 0.0;
    }
}

void ControlObjectThreadWidget::slotSet(double v) {
    m_pControlObject->setValueFromThread(
        m_pControlObject->getValueFromWidget(v), this);
}

void ControlObjectThreadWidget::add(double v) {
    // TODO(rryan): Double-check that this is ok. I think it requires
    // distributive properties of getValueFromWidget() which might not always be
    // the case.
    // TODO(rryan): Coming from the wrong setter.
    m_pControlObject->add(m_pControlObject->getValueFromWidget(v));
}

void ControlObjectThreadWidget::sub(double v) {
    // TODO(rryan): Double-check that this is ok. I think it requires
    // distributive properties of getValueFromWidget() which might not always be
    // the case.
    // TODO(rryan): Coming from the wrong setter.
    m_pControlObject->sub(m_pControlObject->getValueFromWidget(v));
}

// Receives the Value from the parent and may scales the vale and re-emit it again
void ControlObjectThreadWidget::slotValueChanged(double v, QObject* pSetter) {
    if (pSetter != this && m_pControlObject) {
        double widgetValue = m_pControlObject->getValueToWidget(v);
        emit(valueChanged(widgetValue));
    }
}
