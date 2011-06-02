/*
 * midinodelegate.cpp
 *
 *  Created on: 1-Feb-2009
 *      Author: alb
 */

#include <QtCore>
#include <QtGui>
#include "widget/hexspinbox.h"
#include "midinodelegate.h"

MidiNoDelegate::MidiNoDelegate(QObject *parent)
         : QItemDelegate(parent)
{
}

void MidiNoDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    if (index.data().canConvert<int>()) {
        int midino = index.data().value<int>();
        
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        QString text = QString("0x") + QString("%1").arg(midino, 
                                                2,   //Field width (makes "F" become "0F")
                                                16, 
                                                QLatin1Char('0')).toUpper();

        painter->drawText(option.rect, text, QTextOption(Qt::AlignCenter));
        //Note that Qt::AlignCenter does both vertical and horizontal alignment.
    } else {
        QItemDelegate::paint(painter, option, index);
    }
}

QWidget *MidiNoDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */,
        const QModelIndex & index ) const
{
    HexSpinBox *editor = new HexSpinBox(parent);
    editor->setMinimum(0);
    editor->setMaximum(127);

    return editor;
}

void MidiNoDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    HexSpinBox *spinBox = static_cast<HexSpinBox*>(editor);
    spinBox->setValue(value);
    spinBox->interpretText();
}

void MidiNoDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    HexSpinBox *spinBox = static_cast<HexSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void MidiNoDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
