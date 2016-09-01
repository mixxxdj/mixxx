#include <QDebug>

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
    return QStackedWidget::addWidget(w);
}

int WLibraryStack::insertWidget(int index, QWidget* w) {
    checkAndWarning(w);
    return QStackedWidget::insertWidget(index, w);
}

void WLibraryStack::onShow() {

}

void WLibraryStack::onSearch(const QString& text) {
    LibraryView* pView = dynamic_cast<LibraryView*>(currentWidget());

    if (pView) {
        pView->onSearch(text);
    }
}

void WLibraryStack::loadSelectedTrack() {
    LibraryView* pView = dynamic_cast<LibraryView*>(currentWidget());

    if (pView) {
        pView->loadSelectedTrack();
    }
}

void WLibraryStack::slotSendToAutoDJ() {
    LibraryView* pView = dynamic_cast<LibraryView*>(currentWidget());

    if (pView) {
        pView->slotSendToAutoDJ();
    }
}

void WLibraryStack::slotSendToAutoDJTop() {
    LibraryView* pView = dynamic_cast<LibraryView*>(currentWidget());

    if (pView) {
        pView->slotSendToAutoDJTop();
    }
}

bool WLibraryStack::checkAndWarning(QWidget* w) {
    if (!dynamic_cast<LibraryView*>(w)) {
        qDebug() << "WARNING: Attempted to register a view with WLibraryStack"
                 << "that does not implement the LibraryView interface.";
        return false;
    }
    return true;
}
