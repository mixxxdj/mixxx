#include "widget/wlibrarytextbrowser.h"

#include "moc_wlibrarytextbrowser.cpp"

WLibraryTextBrowser::WLibraryTextBrowser(QWidget* parent)
        : QTextBrowser(parent) {
}

bool WLibraryTextBrowser::hasFocus() const {
    return QWidget::hasFocus();
}
