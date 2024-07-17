#pragma once

#include <QStyledItemDelegate>

class TextDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit TextDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
};
