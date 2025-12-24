#pragma once

#include "library/tabledelegates/tableitemdelegate.h"

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
};
