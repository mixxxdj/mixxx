#pragma once

#include <QCheckBox>
#include <QModelIndex>
#include <QStyleOptionViewItem>

#include "library/tableitemdelegate.h"


class BPMDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit BPMDelegate(QTableView* pTableView);
    virtual ~BPMDelegate();

    void paintItem(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const;

  private:
    QTableView* m_pTableView;
    QCheckBox* m_pCheckBox;
    QItemEditorFactory* m_pFactory;
};
