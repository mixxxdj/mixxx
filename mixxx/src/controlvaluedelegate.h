/*
 * controlvaluedelegate.h
 *
 *  Created on: 18-Mar-2009
 *      Author: asantoni
 */

#ifndef CONTROLVALUEDELEGATE_H_
#define CONTROLVALUEDELEGATE_H_

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QComboBox>
#include <QLabel>

class ControlValueDelegate : public QItemDelegate
{
 Q_OBJECT

public:
  ControlValueDelegate(QObject *parent = 0);

 QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
 void paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
 void setEditorData(QWidget *editor, const QModelIndex &index) const;
 void setModelData(QWidget *editor, QAbstractItemModel *model,
                   const QModelIndex &index) const;

 void updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &index) const;

    static bool verifyControlValueValidity(QString controlGroup, QAbstractItemModel *model,
                                           const QModelIndex &index);
    /** These getters are used by the "Add Control" dialog in the MIDI prefs. */
    static QStringList getChannelControlValues() { return m_channelControlValues; };
    static QStringList getMasterControlValues() { return m_masterControlValues; };
    static QStringList getPlaylistControlValues() { return m_playlistControlValues; };
    static QStringList getFXControlValues() { return m_fxControlValues; };
    static QStringList getFlangerControlValues() { return m_flangerControlValues; };
    static QStringList getMicrophoneControlValues() { return m_microphoneControlValues; };

private:
    static QStringList m_channelControlValues;
    static QStringList m_masterControlValues;
    static QStringList m_playlistControlValues;
    static QStringList m_fxControlValues;
    static QStringList m_flangerControlValues;
    static QStringList m_microphoneControlValues;
};

#endif /* CONTROLVALUEDELEGATE_H_ */
