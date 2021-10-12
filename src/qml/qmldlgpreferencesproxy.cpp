#include "qmldlgpreferencesproxy.h"

namespace mixxx {
namespace qml {

QmlDlgPreferencesProxy::QmlDlgPreferencesProxy(
        std::shared_ptr<DlgPreferences> pDlgPreferences, QObject* parent)
        : QObject(parent),
          m_pDlgPreferences(pDlgPreferences) {
}

void QmlDlgPreferencesProxy::show() {
    m_pDlgPreferences->show();
}

} // namespace qml
} // namespace mixxx
