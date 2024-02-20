#include "library/colordelegate.h"

#include <QPainter>
#include <QStyle>
#include <QTableView>

#include "moc_colordelegate.cpp"
#include "util/color/rgbcolor.h"
#include "widget/wtracktableview.h"

ColorDelegate::ColorDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView),
          m_preferredWidth(static_cast<int>(WTrackTableView::kDefaultColumnWidth / 2)) {
}

QSize ColorDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const {
    return QSize(m_preferredWidth, 0);
}

void ColorDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    const auto color = mixxx::RgbColor::fromQVariant(index.data());

    if (color) {
        painter->fillRect(option.rect, mixxx::RgbColor::toQColor(color));
    } else {
        // Filter out track color that is hidden
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }
    }

    // Draw a border if the color cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_pFocusBorderColor, option.rect);
    }
}
