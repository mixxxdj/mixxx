#include "preferences/dialog/dlgpreferencepage.h"

#include "defs_urls.h"
#include "moc_dlgpreferencepage.cpp"

DlgPreferencePage::DlgPreferencePage(QWidget* pParent)
        : QWidget(pParent) {
}

DlgPreferencePage::~DlgPreferencePage() {
}

QString DlgPreferencePage::helpDocument() const {
    return QString();
}
