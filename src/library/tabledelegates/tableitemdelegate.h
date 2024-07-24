#pragma once

#include <QStyledItemDelegate>

class QTableView;

class TableItemDelegate : public QStyledItemDelegate {
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

    static void drawBorder(
            QPainter* painter,
            const QColor borderColor,
            const QRect& rect);

  protected:
    static void paintItemBackground(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index);

    // Only used by LocationDelegate's text elide.
    // Having this here avoids including QTableView there.
    int columnWidth(const QModelIndex &index) const;

    QColor m_focusBorderColor;
    QTableView* m_pTableView;
};
