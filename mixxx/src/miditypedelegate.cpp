/*
 * miditypedelegate.cpp
 *
 *  Created on: 1-Feb-2009
 *      Author: alb
 */

#include <QtCore>
#include <QtGui>
#include "configobject.h"
#include "miditypedelegate.h"

#define MIDITYPE_STRING_NOTE tr("Note/Key")
#define MIDITYPE_STRING_CTRL tr("CC")
#define MIDITYPE_STRING_PITCH tr("Pitch CC")


MidiTypeDelegate::MidiTypeDelegate(QObject *parent)
         : QItemDelegate(parent)
{
}

void MidiTypeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    if (index.data().canConvert<int>()) {
        int value = index.data().value<int>();

        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        //starRating.paint(painter, option.rect, option.palette,
        //                 StarRating::ReadOnly);

        QString text;
        if (value == MIDI_KEY) //These come from the MidiType enum (configobject.h)
            text = MIDITYPE_STRING_NOTE;
        else if (value == MIDI_CTRL)
            text = MIDITYPE_STRING_CTRL;
        else if (value == MIDI_PITCH)
            text = MIDITYPE_STRING_PITCH;

        painter->drawText(option.rect, text, QTextOption(Qt::AlignCenter));
        //Note that Qt::AlignCenter does both vertical and horizontal alignment.
    } else {
        QItemDelegate::paint(painter, option, index);
    }
}

QWidget *MidiTypeDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */,
        const QModelIndex &/* index */) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->addItem(MIDITYPE_STRING_NOTE);
    editor->addItem(MIDITYPE_STRING_CTRL);
    editor->addItem(MIDITYPE_STRING_PITCH);

    return editor;
}

void MidiTypeDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    comboBox->setCurrentIndex(value - 1); //HACK HACK HACK HACK HACK HACK
    //The hack above is because the useful values in the MidiType enum start at 1, not 0,
    //but the indices in the combobox have to start at 1. This is the only place the hack
    //is needed.
}

void MidiTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    int midiType = 0;
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    //comboBox->interpretText();
    //Get the text from the combobox and turn it into a MidiType integer.
    QString text = comboBox->currentText();
    if (text == MIDITYPE_STRING_NOTE) //These come from the MidiType enum (configobject.h)
        midiType = MIDI_KEY;
    else if (text == MIDITYPE_STRING_CTRL)
        midiType = MIDI_CTRL;
    else if (text == MIDITYPE_STRING_PITCH)
        midiType = MIDI_PITCH;

    model->setData(index, midiType, Qt::EditRole);
}

void MidiTypeDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
