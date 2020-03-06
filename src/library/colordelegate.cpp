#include "library/colordelegate.h"

#include <QColor>
#include <QPainter>
#include <QStyle>
#include <QTableView>

#include "library/trackmodel.h"
#include "util/color/rgbcolor.h"

ColorDelegate::ColorDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView) {
}

void ColorDelegate::paintItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    const auto color = mixxx::RgbColor::fromQVariant(index.data());

    if (!color) {
        // Filter out track color that is hidden
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }
        return;
    }

    painter->fillRect(option.rect, mixxx::RgbColor::toQColor(color));

    // Paint transparent highlight if row is selected
    if (option.state & QStyle::State_Selected) {
        QColor highlightColor = option.palette.highlight().color();
        highlightColor.setAlpha(0x60);
        painter->fillRect(option.rect, highlightColor);
    }
}
