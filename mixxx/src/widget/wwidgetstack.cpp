#include <QtDebug>

#include "widget/wwidgetstack.h"

WidgetStackControlListener::WidgetStackControlListener(QObject* pParent,
                                                       ControlObject* pControl,
                                                       int index)
        : QObject(pParent),
          m_control(pControl),
          m_index(index) {
    connect(&m_control, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueChanged(double)));
}

WidgetStackControlListener::~WidgetStackControlListener() {
}

void WidgetStackControlListener::slotValueChanged(double v) {
    if (v > 0.0) {
        emit(switchToWidget());
    }
}

void WidgetStackControlListener::onCurrentWidgetChanged(int index) {
    if (index == m_index && m_control.get() == 0.0) {
        m_control.slotSet(1.0);
    } else if (index != m_index && m_control.get() != 0.0) {
        m_control.slotSet(0.0);
    }
}

WWidgetStack::WWidgetStack(QWidget* pParent,
                           ControlObject* pNextControl,
                           ControlObject* pPrevControl)
        : QStackedWidget(pParent),
          m_nextControl(pNextControl),
          m_prevControl(pPrevControl) {
    connect(&m_nextControl, SIGNAL(valueChanged(double)),
            this, SLOT(onNextControlChanged(double)));
    connect(&m_prevControl, SIGNAL(valueChanged(double)),
            this, SLOT(onPrevControlChanged(double)));
    connect(&m_mapper, SIGNAL(mapped(int)),
            this, SLOT(setCurrentIndex(int)));
}

WWidgetStack::~WWidgetStack() {
}

void WWidgetStack::onNextControlChanged(double v) {
    if (v > 0.0) {
        setCurrentIndex((currentIndex() + 1) % count());
    }
}

void WWidgetStack::onPrevControlChanged(double v) {
    if (v > 0.0) {
        int newIndex = currentIndex() - 1;
        while (newIndex < 0) {
            newIndex += count();
        }
        setCurrentIndex(newIndex);
    }
}

void WWidgetStack::addWidgetWithControl(QWidget* pWidget, ControlObject* pControl) {
    int index = addWidget(pWidget);
    if (pControl) {
        WidgetStackControlListener* pListener = new WidgetStackControlListener(
            this, pControl, index);
        connect(pListener, SIGNAL(switchToWidget()),
                &m_mapper, SLOT(map()));
        connect(this, SIGNAL(currentChanged(int)),
                pListener, SLOT(onCurrentWidgetChanged(int)));
        m_mapper.setMapping(pListener, index);
        pListener->onCurrentWidgetChanged(currentIndex());
    }
}
