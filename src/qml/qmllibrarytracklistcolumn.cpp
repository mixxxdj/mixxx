#include "qml/qmllibrarytracklistcolumn.h"

#include "moc_qmllibrarytracklistcolumn.cpp"

namespace mixxx {
namespace qml {

QmlLibraryTrackListColumn::QmlLibraryTrackListColumn(QObject* parent,
        QString label,
        int fillSpan,
        int columnIdx,
        double preferredWidth,
        QQmlComponent* pDelegate,
        Role role)
        : QObject(parent),
          m_label(label),
          m_fillSpan(fillSpan),
          m_columnIdx(columnIdx),
          m_preferredWidth(preferredWidth),
          m_pDelegate(pDelegate),
          m_role(role) {
    if (pDelegate != nullptr) {
        pDelegate->setParent(this);
        QQmlEngine::setObjectOwnership(pDelegate, QQmlEngine::CppOwnership);
    }
}

} // namespace qml
} // namespace mixxx
