#include <QDebug>
#include <QMutexLocker>
#include <QEvent>
#include <QResizeEvent>
#include <QStyle>

#include "library/libraryfeature.h"
#include "widget/wbaselibrary.h"

WBaseLibrary::WBaseLibrary(QWidget* parent)
        : QStackedWidget(parent),
          WBaseWidget(this),
          m_pCurrentFeature(nullptr),
          m_showFocus(0),
          m_isCollapsed(false) {

}

bool WBaseLibrary::registerView(LibraryFeature* pFeature, QWidget* view) {
    if (m_viewsByFeature.contains(pFeature)) {
        return false;
    }

    view->installEventFilter(this);
    int index = addWidget(view);
    setCurrentIndex(index);
    m_pCurrentFeature = pFeature;
    m_viewsByFeature.insert(pFeature, view);
    return true;
}

LibraryFeature* WBaseLibrary::getCurrentFeature() {
    return m_pCurrentFeature;
}

int WBaseLibrary::getShowFocus() {
    return m_showFocus;
}

void WBaseLibrary::setShowFocus(int sFocus) {
    //qDebug() << "WBaseLibrary::setShowFocus" << sFocus << this;
    m_showFocus = sFocus;

    style()->unpolish(this);
    style()->polish(this);
    update();
}

void WBaseLibrary::switchToFeature(LibraryFeature *pFeature) {
    auto it = m_viewsByFeature.find(pFeature);
    // Only change the current feature if it's not shown already
    if (it != m_viewsByFeature.end() && currentWidget() != (*it)) {
        m_pCurrentFeature = pFeature;
        setCurrentWidget(*it);
    }
}

bool WBaseLibrary::eventFilter(QObject*, QEvent* pEvent) {
    if (pEvent->type() == QEvent::FocusIn) {
        //qDebug() << "WBaseLibrary::eventFilter FocusIn";
        emit(focused());
    }
    return false;
}

bool WBaseLibrary::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    } else if (pEvent->type() == QEvent::FocusIn) {
        emit(focused());
    }

    return QStackedWidget::event(pEvent);
}

void WBaseLibrary::resizeEvent(QResizeEvent *pEvent) {
    // Detect whether the library is collapsed to change the focus behaviour
    if (pEvent->size().isEmpty()) {
        m_isCollapsed = true;
        emit(collapsed());
    } else if (m_isCollapsed) {
        m_isCollapsed = false;
        emit(uncollapsed());
    }
}

