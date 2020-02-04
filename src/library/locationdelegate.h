#pragma once

#include "library/tableitemdelegate.h"


class LocationDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit LocationDelegate(QTableView* pTrackTable);
    ~LocationDelegate() override = default;

    void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
};
