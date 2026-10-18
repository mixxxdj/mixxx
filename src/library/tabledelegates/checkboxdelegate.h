#pragma once

#include <QColor>
#include <QHash>

#include "library/tabledelegates/tableitemdelegate.h"
#include "util/parented_ptr.h"

class QCheckBox;

class CheckboxDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit CheckboxDelegate(QTableView* pTableView, const QString& checkboxName);
    ~CheckboxDelegate() override;

    void paintItem(QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

  protected:
    /// Draws the checkbox item for an option that has already been through
    /// initStyleOption(), with the text colour the option's state selects.
    /// Unlike paintItem(), it doesn't paint the cell background first.
    void drawCheckboxItem(QPainter* painter, const QStyleOptionViewItem& option) const;

  private:
    /// The hidden checkbox whose stylesheet sets this text colour, created on
    /// first use. Returns the unstyled m_pCheckBox for an invalid colour.
    QCheckBox* checkBoxForTextColor(const QColor& textColor) const;

    QCheckBox* m_pCheckBox;
    const QString m_checkboxName;
    // One hidden checkbox per text colour, each with a fixed stylesheet.
    // Calling setStyleSheet() whenever the colour changes restyles the checkbox
    // up to once per row on every repaint. Only a handful of colours occur
    // (normal, selected, missing, played and the highlighter's contrast text).
    mutable QHash<QRgb, QCheckBox*> m_checkBoxByTextColor;
};
