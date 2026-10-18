#include "library/tabledelegates/checkboxdelegate.h"

#include <QCheckBox>
#include <QPainter>
#include <QTableView>

#include "moc_checkboxdelegate.cpp"

namespace {

QCheckBox* newHiddenCheckBox(QTableView* pTableView,
        const QString& checkboxName,
        const QString& styleSheet) {
    auto* pCheckBox = new QCheckBox(pTableView);
    // Note that object names set here are not picked up by /tools/qsscheck.py
    // and need to be added there manually
    pCheckBox->setObjectName(checkboxName);
    if (!styleSheet.isEmpty()) {
        pCheckBox->setStyleSheet(styleSheet);
    }
    // NOTE(rryan): Without ensurePolished the first render of the QTableView
    // shows the checkbox unstyled. Not sure why -- but this fixes it.
    pCheckBox->ensurePolished();
    pCheckBox->hide();
    return pCheckBox;
}

} // namespace

CheckboxDelegate::CheckboxDelegate(QTableView* pTableView, const QString& checkboxName)
        : TableItemDelegate(pTableView),
          m_pCheckBox(newHiddenCheckBox(m_pTableView, checkboxName, QString())),
          m_checkboxName(checkboxName) {
}

CheckboxDelegate::~CheckboxDelegate() {
    // The hidden checkboxes are children of the table view so they pick up its
    // stylesheet, but the view outlives this delegate: it replaces its
    // delegates whenever another model is loaded.
    delete m_pCheckBox;
    qDeleteAll(m_checkBoxByTextColor);
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
    drawCheckboxItem(painter, opt);
}

void CheckboxDelegate::drawCheckboxItem(
        QPainter* painter,
        const QStyleOptionViewItem& option) const {
    // The checkbox uses the QTableView's qss style, therefore it's not picking
    // up the 'missing' or 'played' text color via ForegroundRole from
    // BaseTrackTableModel::data().
    // Enforce it with an explicit stylesheet on a hidden checkbox kept for
    // that text colour, see checkBoxForTextColor().
    // By now, we have already changed the palette's highlight color in
    // TableItemDelegate::paint(), so we can pick that here.
    QColor textColor;
    if (option.state & QStyle::State_Selected) {
        textColor = option.palette.highlightedText().color();
    } else {
        textColor = option.palette.text().color();
    }

    QStyle* style = m_pTableView->style();
    if (style != nullptr) {
        style->drawControl(QStyle::CE_ItemViewItem,
                &option,
                painter,
                checkBoxForTextColor(textColor));
    }
}

QCheckBox* CheckboxDelegate::checkBoxForTextColor(const QColor& textColor) const {
    if (!textColor.isValid()) {
        return m_pCheckBox;
    }
    const QRgb rgb = textColor.rgb();
    QCheckBox* pCheckBox = m_checkBoxByTextColor.value(rgb, nullptr);
    if (!pCheckBox) {
        pCheckBox = newHiddenCheckBox(m_pTableView,
                m_checkboxName,
                QStringLiteral("#%1::item { color: %2; }")
                        .arg(m_checkboxName, textColor.name(QColor::HexRgb)));
        m_checkBoxByTextColor.insert(rgb, pCheckBox);
    }
    return pCheckBox;
}
