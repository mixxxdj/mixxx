#include <QPixmap>
#include <QPainter>
#include <QStyleOptionViewItem>
#include "preparecratedelegate.h"

PrepareCrateDelegate::PrepareCrateDelegate(QObject* parent) : QItemDelegate(parent) {

    m_pCratePixmap = new QPixmap(":images/library/crate_empty.png");
}

PrepareCrateDelegate::~PrepareCrateDelegate()
{
    delete m_pCratePixmap;
}

void PrepareCrateDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    painter->save();

    if (qVariantCanConvert<QString>(index.data()))
    {
        QString crateName = qVariantValue<QString>(index.data());
        painter->drawPixmap(option.rect, *m_pCratePixmap); 
        painter->drawText(option.rect, crateName);
    }
    painter->restore();
}
