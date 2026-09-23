#pragma once

#include <QPersistentModelIndex>

#include "library/tabledelegates/tableitemdelegate.h"
#include "util/color/rgbcolor.h"
#include "util/parented_ptr.h"

class QMenu;
class QModelIndex;
class QPainter;
class QStyleOptionViewItem;
class WColorPickerAction;

class ColorDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit ColorDelegate(QTableView* pTableView, int column);

    void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

  private slots:
    /// Catch left-click to show the color picker if our column was clicked
    void cellClicked(const QModelIndex& index);
    /// Apply the picked color to the track of the clicked cell and close the menu
    void slotColorPicked(mixxx::RgbColor::optional_t color);

  private:
    /// Lazily create the color picker menu and show it below the cell
    void showColorPickerMenu(
            const QModelIndex& index,
            const mixxx::RgbColor::optional_t& color);

    const int m_column;

    parented_ptr<QMenu> m_pColorPickerMenu;
    parented_ptr<WColorPickerAction> m_pColorPickerAction;
    QPersistentModelIndex m_pickedIndex;
};
