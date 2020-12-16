#include "library/colordelegate.h"

#include <QColor>
#include <QPainter>
#include <QStyle>
#include <QTableView>

#include "library/trackmodel.h"
#include "moc_colordelegate.cpp"
#include "util/color/rgbcolor.h"

ColorDelegate::ColorDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView) {
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
        // This uses a color from the stylesheet:
        // WTrackTableView {
        //   qproperty-focusBorderColor: red;
        // }
        QPen borderPen(
                m_pFocusBorderColor,
                1,
                Qt::SolidLine,
                Qt::SquareCap);
        painter->setPen(borderPen);
        painter->setBrush(QBrush(Qt::transparent));
        painter->drawRect(
                option.rect.left(),
                option.rect.top(),
                option.rect.width() - 1,
                option.rect.height() - 1);
    }
}
