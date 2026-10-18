#pragma once

#include "library/tabledelegates/defaultdelegate.h"

class TableItemDelegate : public DefaultDelegate {
    Q_OBJECT
  public:
    explicit TableItemDelegate(QTableView* pTableView);
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
    /// Returns the harmonic highlighter's tint for this cell
    /// (TrackModel::kHighlightBackgroundRole), or an invalid QColor if there
    /// is none. On a selected row, where paintItemBackground() paints nothing,
    /// it also paints the tint semi-transparently over the selection.
    static QColor paintHighlightOverSelection(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index);

    // Only used by LocationDelegate's text elide.
    // Having this here avoids including QTableView there.
    int columnWidth(const QModelIndex &index) const;

    QTableView* m_pTableView;
    QColor m_focusBorderColor;
};
