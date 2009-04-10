/*
 * midistatusdelegate.h
 *
 *  Created on: 1-Feb-2009
 *      Author: asantoni
 */

#ifndef MIDISTATUSDELEGATE_H_
#define MIDISTATUSDELEGATE_H_

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QComboBox>
#include <QLabel>

class MidiStatusDelegate : public QItemDelegate
{
 Q_OBJECT

public:
  MidiStatusDelegate(QObject *parent = 0);

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

#endif /* MIDISTATUSDELEGATE_H_ */
