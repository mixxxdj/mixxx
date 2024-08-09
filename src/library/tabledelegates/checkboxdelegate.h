#pragma once

#include "library/tabledelegates/tableitemdelegate.h"
#include "util/parented_ptr.h"

class QCheckBox;

class CheckboxDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit CheckboxDelegate(QTableView* pTableView, const QString& checkboxName);

    void paintItem(QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

  private:
    QCheckBox* m_pCheckBox;
    const QString m_checkboxName;
    mutable QColor m_cachedTextColor;
};
