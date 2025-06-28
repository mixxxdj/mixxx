#include "library/tabledelegates/checkboxdelegate.h"

#include <QCheckBox>
#include <QPainter>
#include <QTableView>

#include "moc_checkboxdelegate.cpp"

CheckboxDelegate::CheckboxDelegate(QTableView* pTableView, const QString& checkboxName)
        : TableItemDelegate(pTableView),
          m_pCheckBox(new QCheckBox(m_pTableView)),
          m_checkboxName(checkboxName) {
    // Note that object names set here are not picked up by /tools/qsscheck.py
    // and need to be added there manually
    m_pCheckBox->setObjectName(checkboxName);
    // NOTE(rryan): Without ensurePolished the first render of the QTableView
    // shows the checkbox unstyled. Not sure why -- but this fixes it.
    m_pCheckBox->ensurePolished();
    m_pCheckBox->hide();
}

void CheckboxDelegate::paintItem(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    // NOTE(rryan): Qt has a built-in limitation that we cannot style multiple
    // CheckState indicators in the same QAbstractItemView. The CSS rule
    // QTableView::indicator:checked applies to all columns with a
    // CheckState. This is a big pain if we want to use CheckState roles on two
    // columns (i.e. the played column and the BPM column) with different
    // styling. We typically want a lock icon for the BPM check-state and a
    // check-box for the times-played column and may want more in the future.
    //
    // This workaround creates a hidden QComboBox named LibraryBPMButton. We use
    // the parent QTableView's QStyle with the hidden QComboBox as the source of
    // style rules to draw a CE_ItemViewItem.
    //
    // Here's how you would typically style the LibraryBPMButton:
    // #LibraryBPMButton::indicator:checked {
    //   image: url(:/images/library/ic_library_locked.svg);
    // }
    // #LibraryBPMButton::indicator:unchecked {
    //  image: url(:/images/library/ic_library_unlocked.svg);
    // }

    // Actually QAbstractTableModel::data(index, BackgroundRole) provides the
    // correct custom background color (track color).
    // Though, since Qt6 the above style rules would not apply for some reason,
    // (see bug #11630) which can be fixed by also setting
    // #LibraryBPMButton::item { border: 0px;}
    // This however enables some default styles and clears the custom background
    // color (track color), see bug #12355 ¯\_(ツ)_/¯ Qt is fun!
    // Fix that by setting the bg color explicitly here.
    paintItemBackground(painter, option, index);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // The checkbox uses the QTableView's qss style, therefore it's not picking
    // up the 'missing' or 'played' text color via ForegroundRole from
    // BaseTrackTableModel::data().
    // Enforce it with an explicit stylesheet. Note: the stylesheet persists so
    // we need to reset it to normal/highlighted.
    // By now, we have already changed the palette's highlight color in
    // TableItemDelegate::paint(), so we can pick that here.
    QColor textColor;
    if (option.state & QStyle::State_Selected) {
        textColor = option.palette.highlightedText().color();
    } else {
        textColor = option.palette.text().color();
    }

    if (textColor.isValid() && textColor != m_cachedTextColor) {
        m_cachedTextColor = textColor;
        m_pCheckBox->setStyleSheet(QStringLiteral(
                "#%1::item { color: %2; }")
                                           .arg(m_checkboxName,
                                                   textColor.name(QColor::HexRgb)));
    }

    QStyle* style = m_pTableView->style();
    if (style != nullptr) {
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, m_pCheckBox);
    }
}
