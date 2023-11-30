#pragma once

#include "library/tableitemdelegate.h"

class QCheckBox;

class PlayCountDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit PlayCountDelegate(QTableView* pTableView);

    void paintItem(QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

  private:
    QTableView* m_pTableView;
    QCheckBox* m_pCheckBox;
};
