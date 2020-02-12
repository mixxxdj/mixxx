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
    DEBUG_ASSERT(index.data().isValid());
    if (index.data().isNull()) {
        // Filter out track color that is hidden
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }
        return;
    }

    QColor color = mixxx::toQColor(mixxx::RgbColor(index.data().toUInt()));
    painter->fillRect(option.rect, color);

    // Paint transparent highlight if row is selected
    if (option.state & QStyle::State_Selected) {
        QColor highlightColor = option.palette.highlight().color();
        highlightColor.setAlpha(0x60);
        painter->fillRect(option.rect, highlightColor);
    }
}
