/*
 * midinodelegate.cpp
 *
 *  Created on: 1-Feb-2009
 *      Author: alb
 */

#include <QtCore>
#include <QtGui>
#include "midinodelegate.h"

MidiNoDelegate::MidiNoDelegate(QObject *parent)
         : QItemDelegate(parent)
{
}

QWidget *MidiNoDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */,
        const QModelIndex & index ) const
{
    QSpinBox *editor = new QSpinBox(parent);
    editor->setMinimum(0);
    editor->setMaximum(127);

    return editor;
}

void MidiNoDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    qDebug() << index;
    int value = index.model()->data(index, Qt::EditRole).toInt();

    qDebug() << "returning value to editor:" << index.column();
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
    spinBox->interpretText();
}

void MidiNoDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
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
