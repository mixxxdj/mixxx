#pragma once

#include <QItemDelegate>
#include <QTableView>

class TableItemDelegate : public QItemDelegate {
    Q_OBJECT
  public:
    explicit TableItemDelegate(
            QTableView* pTableView);
    ~TableItemDelegate() override = default;

    void paint(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

    virtual void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const;

  protected:
    static void paintItemBackground(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index);

    int columnWidth(const QModelIndex &index) const;
    QRect addMargins(const QRect& rect) const;

    QColor m_pFocusBorderColor;
    QTableView* m_pTableView;
};
