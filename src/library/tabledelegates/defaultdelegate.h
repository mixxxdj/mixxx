#pragma once

#include <QStyledItemDelegate>
#include <QTableView>

class DefaultDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit DefaultDelegate(QTableView* pTableView);
    ~DefaultDelegate() override = default;

    void paint(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

    void setHighlightedTextColor(
            QStyleOptionViewItem& option,
            const QModelIndex& index) const;
};
