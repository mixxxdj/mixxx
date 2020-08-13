#include "widget/wlibrarytextbrowser.h"

WLibraryTextBrowser::WLibraryTextBrowser(QWidget* parent)
        : QTextBrowser(parent) {
}

bool WLibraryTextBrowser::hasFocus() const {
    return QWidget::hasFocus();
}
