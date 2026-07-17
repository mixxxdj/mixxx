#include "qml/qmllibrarytracklistcolumn.h"

#include "moc_qmllibrarytracklistcolumn.cpp"

namespace mixxx {
namespace qml {

QmlLibraryTrackListColumn::QmlLibraryTrackListColumn(QObject* parent,
        const QString& label,
        int fillSpan,
        int columnIdx,
        double preferredWidth,
        double autoHideWidth,
        Role role,
        ColumnType columnType,
        Display display)
        : QObject(parent),
          m_label(label),
          m_role(role),
          m_columnType(columnType),
          m_fillSpan(fillSpan),
          m_columnIdx(columnIdx),
          m_display(display),
          m_preferredWidth(preferredWidth),
          m_autoHideWidth(autoHideWidth) {
}

} // namespace qml
} // namespace mixxx
