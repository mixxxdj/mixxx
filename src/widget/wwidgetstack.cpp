#include <QtDebug>

#include "widget/wwidgetstack.h"

WidgetStackControlListener::WidgetStackControlListener(QObject* pParent,
                                                       ControlObject* pControl,
                                                       int index)
        : QObject(pParent),
          m_control(pControl ? pControl->getKey() : ConfigKey()),
          m_index(index) {
    connect(&m_control, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueChanged(double)));
}

WidgetStackControlListener::~WidgetStackControlListener() {
}

void WidgetStackControlListener::slotValueChanged(double v) {
    if (v > 0.0) {
        emit(switchToWidget());
    } else {
        emit(hideWidget());
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
                           ControlObject* pPrevControl,
                           ControlObject* pCurrentPageControl)
        : QStackedWidget(pParent),
          WBaseWidget(this),
          m_nextControl(pNextControl ? pNextControl->getKey() : ConfigKey()),
          m_prevControl(pPrevControl ? pPrevControl->getKey() : ConfigKey()),
          m_currentPageControl(
                  pCurrentPageControl ?
                  pCurrentPageControl->getKey() : ConfigKey()) {
    connect(&m_nextControl, SIGNAL(valueChanged(double)),
            this, SLOT(onNextControlChanged(double)));
    connect(&m_prevControl, SIGNAL(valueChanged(double)),
            this, SLOT(onPrevControlChanged(double)));
    connect(&m_currentPageControl, SIGNAL(valueChanged(double)),
            this, SLOT(onCurrentPageControlChanged(double)));
    connect(&m_showMapper, SIGNAL(mapped(int)),
            this, SLOT(setCurrentIndex(int)));
    connect(&m_hideMapper, SIGNAL(mapped(int)),
            this, SLOT(hideIndex(int)));
}

// override
void WWidgetStack::Init() {
    WBaseWidget::Init();
    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(onCurrentPageChanged(int)));
}

WWidgetStack::~WWidgetStack() {
}

QSize WWidgetStack::sizeHint() const {
    QWidget* pWidget = currentWidget();
    return pWidget ? pWidget->sizeHint() : QSize();
}

QSize WWidgetStack::minimumSizeHint() const {
    QWidget* pWidget = currentWidget();
    return pWidget ? pWidget->minimumSizeHint() : QSize();
}

void WWidgetStack::hideIndex(int index) {
    if (currentIndex() == index) {
        setCurrentIndex((index + 1) % count());
    }
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

void WWidgetStack::onCurrentPageChanged(int index) {
    m_currentPageControl.set(static_cast<double>(index));
}

void WWidgetStack::onCurrentPageControlChanged(double v) {
    int newIndex = static_cast<int>(v);
    setCurrentIndex(newIndex);
}

void WWidgetStack::addWidgetWithControl(QWidget* pWidget, ControlObject* pControl) {
    int index = addWidget(pWidget);
    if (pControl) {
        WidgetStackControlListener* pListener = new WidgetStackControlListener(
            this, pControl, index);
        m_showMapper.setMapping(pListener, index);
        m_hideMapper.setMapping(pListener, index);
        if (pControl->get() > 0) {
            setCurrentIndex(count()-1);
        }
        pListener->onCurrentWidgetChanged(currentIndex());
        connect(pListener, SIGNAL(switchToWidget()),
                &m_showMapper, SLOT(map()));
        connect(pListener, SIGNAL(hideWidget()),
                &m_hideMapper, SLOT(map()));
        connect(this, SIGNAL(currentChanged(int)),
                pListener, SLOT(onCurrentWidgetChanged(int)));
    }

    if (m_currentPageControl.get() == index) {
        setCurrentIndex(index);
    }
}

bool WWidgetStack::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QFrame::event(pEvent);
}
