#include "library/colordelegate.h"

#include <QPainter>
#include <QStyle>
#include <QColor>
#include <QTableView>

#include "library/trackmodel.h"

namespace {
    const QRgb hiddenTrackColor = 0xFF000000;
}

ColorDelegate::ColorDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView) {
}

void ColorDelegate::paintItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    VERIFY_OR_DEBUG_ASSERT(index.data().canConvert<QColor>()) {
        return;
    }

    QColor color = qvariant_cast<QColor>(index.data());
    VERIFY_OR_DEBUG_ASSERT(color.isValid()) {
        return;
    }

    // Filter out track color that is hidden
    if (color.rgb() == hiddenTrackColor) {
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }
        return;
    }
    painter->fillRect(option.rect, color);

    // Paint transparent highlight if row is selected
    if (option.state & QStyle::State_Selected) {
        QColor highlightColor = option.palette.highlight().color();
        highlightColor.setAlpha(0x60);
        painter->fillRect(option.rect, highlightColor);
    }
}
