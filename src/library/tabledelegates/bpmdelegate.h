#pragma once

#include "library/tabledelegates/checkboxdelegate.h"

class BPMDelegate : public CheckboxDelegate {
    Q_OBJECT
  public:
    explicit BPMDelegate(QTableView* pTableView);

    void paintItem(QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

  private:
    QItemEditorFactory* m_pFactory;
};
