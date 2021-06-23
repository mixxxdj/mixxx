#pragma once

#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>

#include "library/tableitemdelegate.h"

class ColorDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit ColorDelegate(QTableView* pTableView);

    void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
};
