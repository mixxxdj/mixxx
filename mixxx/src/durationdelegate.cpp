
#include <QtCore>
#include <QtGui>
#include "durationdelegate.h"

DurationDelegate::DurationDelegate(QObject* parent) : QItemDelegate(parent)
{

}

DurationDelegate::~DurationDelegate()
{
}

void DurationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (qVariantCanConvert<QString>(index.data())) {
        //Get the duration string as it comes in from the database. This will
        //just be the song length in seconds. (So something like 351, which isn't
        //very human readable.)
        QString duration = qVariantValue<QString>(index.data());

        //Let's reformat this song length into a human readable MM:SS format.
        int totalSeconds = duration.toInt();
        int seconds = totalSeconds % 60;
        int mins = totalSeconds / 60;
        //int hours = mins / 60; //Not going to worry about this for now. :)
        //Construct a nicely formatted duration string now.
        duration = QString("%1:%2").arg(mins).arg(seconds);

        //Paint the highlight colour if this delegate is selected.
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        //Paint the new fancy duration string.
        painter->drawText(option.rect, duration);
    } else {
        QItemDelegate::paint(painter, option, index);
    }
}
