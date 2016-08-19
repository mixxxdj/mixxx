#include <QDebug>
#include <QEvent>

#include "widget/wlibrarystack.h"

WLibraryStack::WLibraryStack(QWidget* parent)
        : QStackedWidget(parent) {
    // TODO Auto-generated constructor stub

}

WLibraryStack::~WLibraryStack() {
    // TODO Auto-generated destructor stub
}

int WLibraryStack::addWidget(QWidget* w) {
    //qDebug() << "WLibraryStack::addWidget" << w;
    checkAndWarning(w);
    w->installEventFilter(this);
    return QStackedWidget::addWidget(w);
}

int WLibraryStack::insertWidget(int index, QWidget* w) {
    checkAndWarning(w);
    w->installEventFilter(this);
    return QStackedWidget::insertWidget(index, w);
}

void WLibraryStack::onShow() {
    LibraryView* pView = getCurrentView();
    if (pView) {
        pView->onShow();
    }
}

void WLibraryStack::onSearch(const QString& text) {
    LibraryView* pView = getCurrentView();
    if (pView) {
        pView->onSearch(text);
    }
}

void WLibraryStack::loadSelectedTrack() {
    LibraryView* pView = getCurrentView();
    if (pView) {
        pView->loadSelectedTrack();
    }
}

void WLibraryStack::slotSendToAutoDJ() {
    LibraryView* pView = getCurrentView();
    if (pView) {
        pView->slotSendToAutoDJ();
    }
}

void WLibraryStack::slotSendToAutoDJTop() {
    LibraryView* pView = getCurrentView();
    if (pView) {
        pView->slotSendToAutoDJTop();
    }
}

bool WLibraryStack::eventFilter(QObject* o, QEvent* e) {
    if (e->type() == QEvent::FocusIn) {
        parent()->event(e);
    }
    return QStackedWidget::eventFilter(o, e);
}

bool WLibraryStack::checkAndWarning(QWidget* w) {
    if (!dynamic_cast<LibraryView*>(w)) {
        qDebug() << "WARNING: Attempted to register a view with WLibraryStack"
                 << "that does not implement the LibraryView interface.";
        return false;
    }
    return true;
}

LibraryView *WLibraryStack::getCurrentView() {
    return dynamic_cast<LibraryView*>(currentWidget());
}
