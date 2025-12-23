#include "qml/qmllibrarytracklistcolumn.h"

#include "moc_qmllibrarytracklistcolumn.cpp"

namespace mixxx {
namespace qml {

QmlLibraryTrackListColumn::QmlLibraryTrackListColumn(QObject* parent,
        const QString& label,
        int fillSpan,
        int columnIdx,
        double preferredWidth,
        QQmlComponent* pDelegate,
        Role role)
        : QObject(parent),
          m_label(label),
          m_role(role),
          m_fillSpan(fillSpan),
          m_columnIdx(columnIdx),
          m_preferredWidth(preferredWidth),
          m_pDelegate(pDelegate) {
}

} // namespace qml
} // namespace mixxx
