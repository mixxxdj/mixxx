#include "textdelegate.h"

#include <QPainter>
#include <QPixmapCache>
#include <QStyleOptionViewItem>

#include "moc_textdelegate.cpp"

TextDelegate::TextDelegate(QObject* parent)
        : QStyledItemDelegate(parent) {
}

void TextDelegate::paint(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    QString text = index.data(Qt::DisplayRole).toString(); // Text string

    // Generate a unique cache key that now includes isChecked and playCount
    QString cacheKey = QString("TextDelegate_%1_%2_%3")
                               .arg(option.rect.width())
                               .arg(option.rect.height())
                               .arg(text);

    QPixmap pixmap;
    if (!QPixmapCache::find(cacheKey, &pixmap)) {
        QStyleOptionViewItem opt = option;

        // Pixmap not found in cache, create new pixmap
        pixmap = QPixmap(opt.rect.size());
        pixmap.fill(Qt::transparent); // Ensure transparent background

        // Adjust the option rect for drawing on the pixmap
        opt.rect.moveTo(0, 0);

        QPainter pixmapPainter(&pixmap);

        QStyledItemDelegate::paint(&pixmapPainter, opt, index);

        pixmapPainter.end(); // Finish painting to the pixmap.

        // Insert the pixmap into the cache
        QPixmapCache::insert(cacheKey, pixmap);
    }

    // Draw the cached pixmap onto the widget
    painter->drawPixmap(option.rect.topLeft(), pixmap);
}
