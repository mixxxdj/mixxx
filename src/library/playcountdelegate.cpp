#include "library/playcountdelegate.h"

#include <QCheckBox>
#include <QPainter>
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

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (m_pTableView != nullptr) {
        QStyle* style = m_pTableView->style();
        if (style != nullptr) {
            style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, m_pCheckBox);
        }
    }
}
