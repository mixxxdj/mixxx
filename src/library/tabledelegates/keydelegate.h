#pragma once

#include <QObject>

#include "library/tabledelegates/tableitemdelegate.h"

class QModelIndex;
class QPainter;
class QStyleOptionViewItem;

class KeyDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit KeyDelegate(QTableView* pTableView)
            : TableItemDelegate(pTableView) {
    }

    void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
};
