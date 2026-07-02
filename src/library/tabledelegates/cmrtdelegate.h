#pragma once

#include <QObject>

#include "library/tabledelegates/tableitemdelegate.h"

class QModelIndex;
class QPainter;
class QStyleOptionViewItem;

// Renders the CMRT column: a small color swatch (green = this row IS the
// canonical/CMRT track for its group, orange = this row is a non-canonical
// member) followed by the canonical track's "Artist - Title". Cells with no
// CMRT group, or whose group currently has only one member, show "--" —
// library_view's join already returns NULL for those rows (see
// LibraryTableModel::setTableModel()), so this delegate just checks for
// that, it doesn't re-derive the >1-member rule itself.
class CmrtDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit CmrtDelegate(QTableView* pTableView)
            : TableItemDelegate(pTableView) {
    }

    void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
};
