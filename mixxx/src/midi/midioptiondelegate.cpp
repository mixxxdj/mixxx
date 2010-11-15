/*
 * midioptiondelegate.cpp
 *
 *  Created on: 11-April-2009
 *      Author: asantoni
 */

#include <QtCore>
#include <QtGui>
#include "mixxxcontrol.h"
#include "midioptiondelegate.h"

#define MIDI_OPT_STRING_NORMAL      "Normal"
#define MIDI_OPT_STRING_INVERT      "Invert"
#define MIDI_OPT_STRING_ROT64       "Rot64"
#define MIDI_OPT_STRING_ROT64_INV   "Rot64Invert"
#define MIDI_OPT_STRING_ROT64_FAST  "Rot64Fast"
#define MIDI_OPT_STRING_DIFF        "Difference"
#define MIDI_OPT_STRING_BUTTON      "Button"     // Button Down (!=00) and Button Up (00) events happen together
#define MIDI_OPT_STRING_SWITCH      "Switch"     // Button Down (!=00) and Button Up (00) events happen seperately
#define MIDI_OPT_STRING_HERC_JOG    "HercJog"    // Generic hercules wierd range correction
#define MIDI_OPT_STRING_SPREAD64    "Spread64"   // Accelerated difference from 64
#define MIDI_OPT_STRING_SELECTKNOB  "SelectKnob" // Relative knob which can be turned forever and outputs a signed value.
#define MIDI_OPT_STRING_SCRIPT      "Script"     // Maps a MIDI control to a custom MixxxScript function

MidiOptionDelegate::MidiOptionDelegate(QObject *parent)
         : QItemDelegate(parent)
{
}

void MidiOptionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    if (index.data().canConvert<int>()) {
        int midioption = index.data().value<int>();
        
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        QString text;
        switch (midioption) {
           case MIDI_OPT_NORMAL: text = MIDI_OPT_STRING_NORMAL; break;
           case MIDI_OPT_INVERT: text = MIDI_OPT_STRING_INVERT; break;
           case MIDI_OPT_ROT64:  text = MIDI_OPT_STRING_ROT64; break;           
           case MIDI_OPT_ROT64_INV: text = MIDI_OPT_STRING_ROT64_INV; break;
           case MIDI_OPT_ROT64_FAST: text = MIDI_OPT_STRING_ROT64_FAST; break;
           case MIDI_OPT_DIFF: text = MIDI_OPT_STRING_DIFF; break;
           case MIDI_OPT_BUTTON: text = MIDI_OPT_STRING_BUTTON; break;
           case MIDI_OPT_SWITCH: text = MIDI_OPT_STRING_SWITCH; break;                                                       
           case MIDI_OPT_HERC_JOG: text = MIDI_OPT_STRING_HERC_JOG; break;
           case MIDI_OPT_SPREAD64: text = MIDI_OPT_STRING_SPREAD64; break;
           case MIDI_OPT_SELECTKNOB: text = MIDI_OPT_STRING_SELECTKNOB; break;
           case MIDI_OPT_SCRIPT: text = MIDI_OPT_STRING_SCRIPT; break;      
           default: text = QString("%1").arg(midioption); break;                           
        }

        painter->drawText(option.rect, text, QTextOption(Qt::AlignCenter));
        //Note that Qt::AlignCenter does both vertical and horizontal alignment.
    } else {
        QItemDelegate::paint(painter, option, index);
    }
}

QWidget *MidiOptionDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */,
        const QModelIndex &/* index */) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->addItem(MIDI_OPT_STRING_NORMAL);
    editor->addItem(MIDI_OPT_STRING_INVERT);
    editor->addItem(MIDI_OPT_STRING_ROT64);
    editor->addItem(MIDI_OPT_STRING_ROT64_INV);
    editor->addItem(MIDI_OPT_STRING_ROT64_FAST);
    editor->addItem(MIDI_OPT_STRING_DIFF);
    editor->addItem(MIDI_OPT_STRING_BUTTON);
    editor->addItem(MIDI_OPT_STRING_SWITCH);
    editor->addItem(MIDI_OPT_STRING_HERC_JOG);
    editor->addItem(MIDI_OPT_STRING_SPREAD64);
    editor->addItem(MIDI_OPT_STRING_SELECTKNOB);
    editor->addItem(MIDI_OPT_STRING_SCRIPT);

    return editor;
}

void MidiOptionDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int midioption = index.model()->data(index, Qt::EditRole).toInt();
    int comboIdx = 0;
    
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    switch (midioption)
    {
        case MIDI_OPT_NORMAL:       comboIdx = 0; break;
        case MIDI_OPT_INVERT:       comboIdx = 1; break;
        case MIDI_OPT_ROT64:        comboIdx = 2; break;           
        case MIDI_OPT_ROT64_INV:    comboIdx = 3; break;
        case MIDI_OPT_ROT64_FAST:   comboIdx = 4; break;
        case MIDI_OPT_DIFF:         comboIdx = 5; break;
        case MIDI_OPT_BUTTON:       comboIdx = 6; break;
        case MIDI_OPT_SWITCH:       comboIdx = 7; break;                                                       
        case MIDI_OPT_HERC_JOG:     comboIdx = 8; break;
        case MIDI_OPT_SPREAD64:     comboIdx = 9; break;
        case MIDI_OPT_SELECTKNOB:   comboIdx = 10; break;
        case MIDI_OPT_SCRIPT:       comboIdx = 11; break;      
        default: comboIdx = 0; break;      
    }
    comboBox->setCurrentIndex(comboIdx);
}

void MidiOptionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    int midioption = 0;
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    //comboBox->interpretText();
    //Get the text from the combobox and turn it into a MidiMessage integer.
    QString text = comboBox->currentText();
    if (text == MIDI_OPT_STRING_NORMAL) //These come from the MidiOption enum (mixxxcontrol.h)
        midioption = MIDI_OPT_NORMAL;
    else if (text == MIDI_OPT_STRING_INVERT)
        midioption = MIDI_OPT_INVERT;
    else if (text == MIDI_OPT_STRING_ROT64)
        midioption = MIDI_OPT_ROT64;
    else if (text == MIDI_OPT_STRING_ROT64_INV)
        midioption = MIDI_OPT_ROT64_INV;
    else if (text == MIDI_OPT_STRING_ROT64_FAST)
        midioption = MIDI_OPT_ROT64_FAST;
    else if (text == MIDI_OPT_STRING_DIFF)
        midioption = MIDI_OPT_DIFF;
    else if (text == MIDI_OPT_STRING_BUTTON)
        midioption = MIDI_OPT_BUTTON;
    else if (text == MIDI_OPT_STRING_SWITCH)
        midioption = MIDI_OPT_SWITCH;
    else if (text == MIDI_OPT_STRING_HERC_JOG)
        midioption = MIDI_OPT_HERC_JOG;
    else if (text == MIDI_OPT_STRING_SPREAD64)
        midioption = MIDI_OPT_SPREAD64;
    else if (text == MIDI_OPT_STRING_SELECTKNOB)
        midioption = MIDI_OPT_SELECTKNOB;
    else if (text == MIDI_OPT_STRING_SCRIPT)
        midioption = MIDI_OPT_SCRIPT;                                                                
    model->setData(index, midioption, Qt::EditRole);
}

void MidiOptionDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
