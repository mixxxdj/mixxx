#include "library/tabledelegates/playcountdelegate.h"

#include <QCheckBox>
#include <QPainter>
#include <QPixmapCache>
#include <QTableView>

#include "moc_playcountdelegate.cpp"

PlayCountDelegate::PlayCountDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView),
          m_pTableView(pTableView),
          m_pCheckBox(new QCheckBox(m_pTableView)) {
    m_pCheckBox->setObjectName("LibraryPlayedCheckbox");
    // NOTE(rryan): Without ensurePolished the first render of the QTableView
    // shows the checkbox unstyled. Not sure why -- but this fixes it.
    m_pCheckBox->ensurePolished();
    m_pCheckBox->hide();
}

void PlayCountDelegate::paintItem(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    // NOTE(ronso0): For details why we need checkbox delegates, how to style
    // them and why we need paintItemBackground() here, see bpmdelegate.cpp
    paintItemBackground(painter, option, index);

    // Retrieve the check state and BPM value
    bool isChecked = index.data(Qt::CheckStateRole).toInt() == Qt::Checked;
    QString playCount =
            index.data(Qt::UserRole + 2)
                    .toString(); // Playcount value is stored with UserRole + 2

    QColor textColor;
    if (option.state & QStyle::State_Selected) {
        textColor = option.palette.color(QPalette::Normal, QPalette::HighlightedText);
    } else {
        auto colorData = index.data(Qt::ForegroundRole);
        if (colorData.canConvert<QColor>()) {
            textColor = colorData.value<QColor>();
        } else {
            textColor = option.palette.color(QPalette::Normal, QPalette::Text);
        }
    }

    // Generate a unique cache key that now includes isChecked and playCount
    QString cacheKey = QString("PlayCountDelegate_%1_%2_%3_%4_%5")
                               .arg(option.rect.width())
                               .arg(option.rect.height())
                               .arg(textColor.name(QColor::HexRgb),
                                       isChecked ? "checked" : "unchecked")
                               .arg(playCount);

    QPixmap pixmap;
    if (!QPixmapCache::find(cacheKey, &pixmap)) {
        // Pixmap not in cache, create a new one
        pixmap = QPixmap(option.rect.size());
        pixmap.fill(Qt::transparent); // Fill uninitialized pixmap's background transparent
        QPainter pixmapPainter(&pixmap);

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        m_pCheckBox->setStyleSheet(QStringLiteral(
                "#LibraryPlayedCheckbox::item { color: %1; }")
                                           .arg(textColor.name(QColor::HexRgb)));

        // Adjust the option rect for drawing on the pixmap
        opt.rect.moveTo(0, 0);

        if (m_pTableView != nullptr) {
            QStyle* style = m_pTableView->style();
            if (style != nullptr) {
                // Draw the item onto the pixmap
                style->drawControl(QStyle::CE_ItemViewItem, &opt, &pixmapPainter, m_pCheckBox);
            }
        }

        pixmapPainter.end(); // Finish painting to the pixmap.

        // Cache the newly created pixmap
        QPixmapCache::insert(cacheKey, pixmap);
    }

    // Draw the cached pixmap onto the painter
    painter->drawPixmap(option.rect.topLeft(), pixmap);
}
