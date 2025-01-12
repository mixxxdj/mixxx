#include "widget/wlibrarytextbrowser.h"

#include <QKeyEvent>

#include "moc_wlibrarytextbrowser.cpp"

WLibraryTextBrowser::WLibraryTextBrowser(QWidget* parent)
        : QTextBrowser(parent) {
}

bool WLibraryTextBrowser::hasFocus() const {
    return QWidget::hasFocus();
}

void WLibraryTextBrowser::setFocus() {
    QWidget::setFocus();
}

void WLibraryTextBrowser::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Left && event->modifiers() & Qt::ControlModifier) {
        event->ignore();
        return;
    }
    QTextBrowser::keyPressEvent(event);
}
