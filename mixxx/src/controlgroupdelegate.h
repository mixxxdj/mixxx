/*
 * controlgroupdelegate.h
 *
 *  Created on: 18-Mar-2009
 *      Author: asantoni
 */

#ifndef CONTROLGROUPDELEGATE_H_
#define CONTROLGROUPDELEGATE_H_

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QComboBox>
#include <QLabel>

#define CONTROLGROUP_CHANNEL1_STRING tr("[Channel1]")
#define CONTROLGROUP_CHANNEL2_STRING tr("[Channel2]")
#define CONTROLGROUP_MASTER_STRING tr("[Master]")
#define CONTROLGROUP_PLAYLIST_STRING tr("[Playlist]")

class ControlGroupDelegate : public QItemDelegate
{
 Q_OBJECT

public:
  ControlGroupDelegate(QObject *parent = 0);

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

#endif /* CONTROLGROUPDELEGATE_H_ */
