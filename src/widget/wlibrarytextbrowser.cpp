#include "widget/wlibrarytextbrowser.h"

#include "moc_wlibrarytextbrowser.cpp"

WLibraryTextBrowser::WLibraryTextBrowser(QWidget* parent)
        : QTextBrowser(parent) {
    qRegisterMetaType<FocusWidget>("FocusWidget");
}

bool WLibraryTextBrowser::hasFocus() const {
    return QWidget::hasFocus();
}

void WLibraryTextBrowser::setFocus() {
    QWidget::setFocus();
}

void WLibraryTextBrowser::focusInEvent(QFocusEvent* event) {
    QWidget::focusInEvent(event);
    emit textBrowserFocusChange(FocusWidget::TracksTable);
}

void WLibraryTextBrowser::focusOutEvent(QFocusEvent* event) {
    QWidget::focusOutEvent(event);
    emit textBrowserFocusChange(FocusWidget::None);
}
