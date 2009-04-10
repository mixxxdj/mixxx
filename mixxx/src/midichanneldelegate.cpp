/*
 * midichanneledelegate.cpp
 *
 *  Created on: 1-Feb-2009
 *      Author: alb
 */

#include <QtCore>
#include <QtGui>
#include "midichanneldelegate.h"

MidiChannelDelegate::MidiChannelDelegate(QObject *parent)
         : QItemDelegate(parent)
{
}

QWidget *MidiChannelDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */,
        const QModelIndex &/* index */) const
{
    QSpinBox *editor = new QSpinBox(parent);
    editor->setMinimum(1);
    editor->setMaximum(16);

    return editor;
}

void MidiChannelDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int channel = index.model()->data(index, Qt::EditRole).toInt();
    
    //Convert the channel to natural numbers (1-16). The actual MIDI messages
    //address them as 0-15 as per the spec, but all user documentation for every
    //MIDI device on the planet refers to the channels as 1-16.
    channel++;
    
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(channel);
}

void MidiChannelDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int channel = spinBox->value();
    channel--; //Convert the MIDI channel back into the 0-15 range.
    model->setData(index, channel, Qt::EditRole);
}

void MidiChannelDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
