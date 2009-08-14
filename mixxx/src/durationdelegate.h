


#ifndef DURATION_DELEGATE_H
#define DURATION_DELEGATE_H

#include <QtCore>
#include <QItemDelegate>

class DurationDelegate : public QItemDelegate
{
    Q_OBJECT

    public:
        DurationDelegate(QObject* parent = 0);
        ~DurationDelegate();
        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;


};


#endif
