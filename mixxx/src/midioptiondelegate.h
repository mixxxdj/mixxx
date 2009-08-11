/*
 * midioptiondelegate.h
 *
 *  Created on: 11-April-2009
 *      Author: asantoni
 */

#ifndef MIDIOPTIONDELEGATE_H_
#define MIDIOPTIONDELEGATE_H_

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QComboBox>
#include <QLabel>

class MidiOptionDelegate : public QItemDelegate
{
 Q_OBJECT

public:
  MidiOptionDelegate(QObject *parent = 0);

 QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
 void paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
 void setEditorData(QWidget *editor, const QModelIndex &index) const;
 void setModelData(QWidget *editor, QAbstractItemModel *model,
                   const QModelIndex &index) const;

 void updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
};

#endif /* MIDIOPTIONDELEGATE_H_ */
