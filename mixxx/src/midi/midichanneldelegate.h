/*
 * midichanneldelegate.h
 *
 *  Created on: 1-Feb-2009
 *      Author: asantoni
 */

#ifndef MIDICHANNELDELEGATE_H_
#define MIDICHANNELDELEGATE_H_

 #include <QItemDelegate>
 #include <QModelIndex>
 #include <QObject>
 #include <QSize>
 #include <QSpinBox>

class MidiChannelDelegate : public QItemDelegate
{
 Q_OBJECT

public:
    MidiChannelDelegate(QObject *parent = 0);
 void paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;
 QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;

 void setEditorData(QWidget *editor, const QModelIndex &index) const;
 void setModelData(QWidget *editor, QAbstractItemModel *model,
                   const QModelIndex &index) const;

 void updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif /* MIDICHANNELDELEGATE_H_ */
