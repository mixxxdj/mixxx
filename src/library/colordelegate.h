#pragma once

#include "library/tableitemdelegate.h"

class QModelIndex;
class QPainter;
class QStyleOptionViewItem;

class ColorDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit ColorDelegate(QTableView* pTableView);

    void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

    // returns an item's preferred size
    QSize sizeHint(const QStyleOptionViewItem&,
            const QModelIndex&) const;

  private:
    int m_preferredWidth;
};
