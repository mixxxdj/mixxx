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

#define CONTROLGROUP_CHANNEL1_STRING "[Channel1]"
#define CONTROLGROUP_CHANNEL2_STRING "[Channel2]"
#define CONTROLGROUP_SAMPLER1_STRING "[Sampler1]"
#define CONTROLGROUP_SAMPLER2_STRING "[Sampler2]"
#define CONTROLGROUP_SAMPLER3_STRING "[Sampler3]"
#define CONTROLGROUP_SAMPLER4_STRING "[Sampler4]"
#define CONTROLGROUP_MASTER_STRING   "[Master]"
#define CONTROLGROUP_PLAYLIST_STRING "[Playlist]"
#define CONTROLGROUP_FX_STRING "[FX]"
#define CONTROLGROUP_FLANGER_STRING  "[Flanger]"
#define CONTROLGROUP_MICROPHONE_STRING  "[Microphone]"

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
    /** This getter is used by the "Add Control" GUI */
    static QStringList getControlGroups() { return m_controlGroups; };

private:
    static QStringList m_controlGroups;

};

#endif /* CONTROLGROUPDELEGATE_H_ */
