#include "preferences/dlgpreferencepage.h"

#include "defs_urls.h"

namespace {

const QString manualUrl(MIXXX_MANUAL_URL);

}

DlgPreferencePage::DlgPreferencePage(QWidget* pParent)
        : QWidget(pParent) {
}

DlgPreferencePage::~DlgPreferencePage() {
}

QUrl DlgPreferencePage::helpUrl() const {
    return QUrl();
}
