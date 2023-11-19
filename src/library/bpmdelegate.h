#pragma once

#include "library/tableitemdelegate.h"

class QCheckBox;

class BPMDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit BPMDelegate(QTableView* pTableView);
    virtual ~BPMDelegate();

    void paintItem(QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

  private:
    QTableView* m_pTableView;
    QCheckBox* m_pCheckBox;
    QItemEditorFactory* m_pFactory;
};
