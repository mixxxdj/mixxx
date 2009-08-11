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

void MidiChannelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    if (index.data().canConvert<int>()) {
        int channel = index.data().value<int>();
        //Convert to natural numbers (starts at 1 instead of 0.)
        channel++;

        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        QString text = QString("%1").arg(channel);

        painter->drawText(option.rect, text, QTextOption(Qt::AlignCenter));
        //Note that Qt::AlignCenter does both vertical and horizontal alignment.
    } else {
        QItemDelegate::paint(painter, option, index);
    }
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
