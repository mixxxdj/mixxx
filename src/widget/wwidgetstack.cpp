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
                  pCurrentPageControl->getKey() : ConfigKey()),
          m_bRespondToChanges(false) {
    connect(&m_nextControl, SIGNAL(valueChanged(double)),
            this, SLOT(onNextControlChanged(double)));
    connect(&m_prevControl, SIGNAL(valueChanged(double)),
            this, SLOT(onPrevControlChanged(double)));
    connect(&m_currentPageControl, SIGNAL(valueChanged(double)),
            this, SLOT(onCurrentPageControlChanged(double)));
    connect(&m_showMapper, SIGNAL(mapped(int)),
            this, SLOT(showIndex(int)));
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

void WWidgetStack::showIndex(int index) {
    if (m_bRespondToChanges) {
        setCurrentIndex(index);
    }
}

void WWidgetStack::hideIndex(int index) {
    if (!m_bRespondToChanges) {
        return;
    }
    if (currentIndex() == index) {
        QMap<int, int>::const_iterator it = m_hideMap.find(index);
        if (it != m_hideMap.end()) {
            setCurrentIndex(*it);
        } else {
            // TODO: This default behavior is a little odd, is it really what
            // we want?  Or should we save the previously-selected page and then
            // switch to that.
            setCurrentIndex((index + 1) % count());
        }
    }
}

void WWidgetStack::showEvent(QShowEvent*) {
    int index = static_cast<int>(m_currentPageControl.get());

    // Set the page triggers to match the current index.
    for (QMap<int, ControlObject*>::iterator it = m_triggers.begin();
            it != m_triggers.end(); ++it) {
        it.value()->set(it.key() == index ? 1.0 : 0.0);
    }

    m_bRespondToChanges = true;
    setCurrentIndex(index);
}

void WWidgetStack::hideEvent(QHideEvent*) {
    m_bRespondToChanges = false;
}

void WWidgetStack::onNextControlChanged(double v) {
    if (!m_bRespondToChanges) {
        return;
    }
    if (v > 0.0) {
        setCurrentIndex((currentIndex() + 1) % count());
    }
}

void WWidgetStack::onPrevControlChanged(double v) {
    if (!m_bRespondToChanges) {
        return;
    }
    if (v > 0.0) {
        int newIndex = currentIndex() - 1;
        while (newIndex < 0) {
            newIndex += count();
        }
        setCurrentIndex(newIndex);
    }
}

void WWidgetStack::onCurrentPageChanged(int index) {
    if (!m_bRespondToChanges) {
        return;
    }
    m_currentPageControl.set(static_cast<double>(index));
}

void WWidgetStack::onCurrentPageControlChanged(double v) {
    if (!m_bRespondToChanges) {
        return;
    }
    int newIndex = static_cast<int>(v);
    setCurrentIndex(newIndex);
}

void WWidgetStack::addWidgetWithControl(QWidget* pWidget, ControlObject* pControl,
                                        int on_hide_select) {
    int index = addWidget(pWidget);
    if (pControl) {
        m_triggers[index] = pControl;
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
        // The value in the curren page control overrides whatever initial
        // values the individual page triggers may have.
        setCurrentIndex(index);
    }
    if (on_hide_select != -1) {
        m_hideMap[index] = on_hide_select;
    }
}

bool WWidgetStack::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QFrame::event(pEvent);
}
