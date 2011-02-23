/*
 * midistatusdelegate.cpp
 *
 *  Created on: 1-Feb-2009
 *      Author: alb
 */

#include <QtCore>
#include <QtGui>
#include "midimessage.h"
#include "midistatusdelegate.h"

#define MIDISTATUS_STRING_NOTE_ON "Note On"
#define MIDISTATUS_STRING_NOTE_OFF "Note Off"
#define MIDISTATUS_STRING_CTRL "CC"
#define MIDISTATUS_STRING_PITCH "Pitch CC"


MidiStatusDelegate::MidiStatusDelegate(QObject *parent)
         : QItemDelegate(parent)
{
}

void MidiStatusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    if (index.data().canConvert<int>()) {
        int status = index.data().value<int>();

        //Throw away the channel bits (low nibble).
        status &= 0xF0;

        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        QString text;
        if (status == MIDI_STATUS_NOTE_ON) //These come from the MidiStatusByte enum (midimessage.h)
            text = tr(MIDISTATUS_STRING_NOTE_ON);
        else if (status == MIDI_STATUS_NOTE_OFF)
            text = tr(MIDISTATUS_STRING_NOTE_OFF);
        else if (status == MIDI_STATUS_CC)
            text = tr(MIDISTATUS_STRING_CTRL);
        else if (status == MIDI_STATUS_PITCH_BEND)
            text = tr(MIDISTATUS_STRING_PITCH);
        else
            text = tr("Unknown") + " (0x" + QString::number(status, 16) + ")";

        painter->drawText(option.rect, text, QTextOption(Qt::AlignCenter));
        //Note that Qt::AlignCenter does both vertical and horizontal alignment.
    } else {
        QItemDelegate::paint(painter, option, index);
    }
}

QWidget *MidiStatusDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */,
        const QModelIndex &/* index */) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->addItem(tr(MIDISTATUS_STRING_NOTE_ON));
    editor->addItem(tr(MIDISTATUS_STRING_NOTE_OFF));
    editor->addItem(tr(MIDISTATUS_STRING_CTRL));
    editor->addItem(tr(MIDISTATUS_STRING_PITCH));

    return editor;
}

void MidiStatusDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int status = index.model()->data(index, Qt::EditRole).toInt();
    int comboIdx = 0;

    //Throw away the channel bits (low nibble).
    status &= 0xF0;

    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    switch (status)
    {
        case MIDI_STATUS_NOTE_ON:
            comboIdx = 0;
            break;
        case MIDI_STATUS_NOTE_OFF:
            comboIdx = 1;
            break;
        case MIDI_STATUS_CC:
            comboIdx = 2;
            break;
        case MIDI_STATUS_PITCH_BEND:
            comboIdx = 3;
            break;
    }
    comboBox->setCurrentIndex(comboIdx);
}

void MidiStatusDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    int midiStatus = 0;
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    //comboBox->interpretText();
    //Get the text from the combobox and turn it into a MidiMessage integer.
    QString text = comboBox->currentText();
    if (text == tr(MIDISTATUS_STRING_NOTE_ON)) //These come from the MidiStatusByte enum (midimessage.h)
        midiStatus = MIDI_STATUS_NOTE_ON;
    else if (text == tr(MIDISTATUS_STRING_NOTE_OFF))
        midiStatus = MIDI_STATUS_NOTE_OFF;
    else if (text == tr(MIDISTATUS_STRING_CTRL))
        midiStatus = MIDI_STATUS_CC;
    else if (text == tr(MIDISTATUS_STRING_PITCH))
        midiStatus = MIDI_STATUS_PITCH_BEND;

    model->setData(index, midiStatus, Qt::EditRole);
}

void MidiStatusDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
