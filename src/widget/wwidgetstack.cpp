#include "widget/wwidgetstack.h"

#include <QApplication>
#include <QtDebug>

#include "moc_wwidgetstack.cpp"

WidgetStackControlListener::WidgetStackControlListener(
        QObject* pParent, ControlObject* pControl, int index)
        : QObject(pParent),
          m_control(pControl ? pControl->getKey() : ConfigKey(),
                  this,
                  ControlFlag::AllowInvalidKey),
          m_index(index) {
    m_control.connectValueChanged(this, &WidgetStackControlListener::slotValueChanged);
}

void WidgetStackControlListener::slotValueChanged(double v) {
    if (v > 0.0) {
        emit switchToWidget();
    } else {
        emit hideWidget();
    }
}

void WidgetStackControlListener::onCurrentWidgetChanged(int index) {
    if (index == m_index && m_control.get() == 0.0) {
        m_control.set(1.0);
    } else if (index != m_index && m_control.get() != 0.0) {
        m_control.set(0.0);
    }
}

WWidgetStack::WWidgetStack(QWidget* pParent,
        const ConfigKey& nextConfigKey,
        const ConfigKey& prevConfigKey,
        const ConfigKey& currentPageConfigKey)
        : QStackedWidget(pParent),
          WBaseWidget(this),
          m_nextControl(nextConfigKey, this, ControlFlag::AllowInvalidKey),
          m_prevControl(prevConfigKey, this, ControlFlag::AllowInvalidKey),
          m_currentPageControl(
                  currentPageConfigKey, this, ControlFlag::AllowInvalidKey) {
    m_nextControl.connectValueChanged(this, &WWidgetStack::onNextControlChanged);
    m_prevControl.connectValueChanged(this, &WWidgetStack::onPrevControlChanged);
    m_currentPageControl.connectValueChanged(this, &WWidgetStack::onCurrentPageControlChanged);
}

// override
void WWidgetStack::Init() {
    WBaseWidget::Init();
    connect(this, &WWidgetStack::currentChanged, this, &WWidgetStack::onCurrentPageChanged);
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
    // Only respond to changes if the stack is visible.  This allows multiple
    // stacks to use the same trigger COs without causing conflicts.
    if (isVisible()) {
        slotSetIndex(index);
    }
}

void WWidgetStack::hideIndex(int index) {
    if (!isVisible()) {
        return;
    }
    if (currentIndex() == index) {
        auto it = m_hideMap.constFind(index);
        if (it != m_hideMap.constEnd()) {
            slotSetIndex(*it);
        } else {
            // TODO: This default behavior is a little odd, is it really what
            // we want?  Or should we save the previously-selected page and then
            // switch to that.
            slotSetIndex((index + 1) % count());
        }
    }
}

void WWidgetStack::showEvent(QShowEvent* /*unused*/) {
    int index = static_cast<int>(m_currentPageControl.get());

    // Set the page triggers to match the current index.
    for (QMap<int, WidgetStackControlListener*>::iterator it
            = m_listeners.begin(); it != m_listeners.end(); ++it) {
        it.value()->setControl(it.key() == index ? 1.0 : 0.0);
    }

    slotSetIndex(index);
}

void WWidgetStack::onNextControlChanged(double v) {
    if (!isVisible()) {
        return;
    }
    if (v > 0.0) {
        slotSetIndex((currentIndex() + 1) % count());
    }
}

void WWidgetStack::onPrevControlChanged(double v) {
    if (!isVisible()) {
        return;
    }
    if (v > 0.0) {
        int newIndex = currentIndex() - 1;
        while (newIndex < 0) {
            newIndex += count();
        }
        slotSetIndex(newIndex);
    }
}

void WWidgetStack::onCurrentPageChanged(int index) {
    if (!isVisible()) {
        return;
    }
    m_currentPageControl.set(static_cast<double>(index));
}

void WWidgetStack::onCurrentPageControlChanged(double v) {
    if (!isVisible()) {
        return;
    }
    int newIndex = static_cast<int>(v);
    slotSetIndex(newIndex);
}

void WWidgetStack::addWidgetWithControl(QWidget* pWidget, ControlObject* pControl,
                                        int on_hide_select) {
    int index = addWidget(pWidget);
    if (pControl) {
        auto* pListener = new WidgetStackControlListener(this, pControl, index);
        m_listeners[index] = pListener;
        if (pControl->get() > 0) {
            setCurrentIndex(count()-1);
        }
        pListener->onCurrentWidgetChanged(currentIndex());
        connect(pListener, &WidgetStackControlListener::switchToWidget,
                this, [this, index] { showIndex(index); });
        connect(pListener, &WidgetStackControlListener::hideWidget,
                this, [this, index] { hideIndex(index); });
        connect(this, &WWidgetStack::currentChanged,
                pListener, &WidgetStackControlListener::onCurrentWidgetChanged);
    }

    if (m_currentPageControl.get() == index) {
        // The value in the current page control overrides whatever initial
        // values the individual page triggers may have.
        setCurrentIndex(index);
    }
    if (on_hide_select != -1) {
        m_hideMap[index] = on_hide_select;
    }
}

void WWidgetStack::slotSetIndex(int index) {
    // If the previously focused widget is a child of the new
    // index widget re-focus that widget.
    // For now, its only purpose is to keep the keyboard focus on
    // library widgets in the Library singleton when toggling
    // [Master],maximize_library
    QWidget* prevFocusWidget = QApplication::focusWidget();
    setCurrentIndex(index);
    if (currentWidget()->isAncestorOf(prevFocusWidget)) {
        prevFocusWidget->setFocus();
    }
}

bool WWidgetStack::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QStackedWidget::event(pEvent);
}
