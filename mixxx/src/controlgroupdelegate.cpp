/*
 * controlgroupdelegate.cpp
 *
 *  Created on: 18-Mar-2009
 *      Author: Albert Santoni
 */

#include <QtCore>
#include <QtGui>
#include "configobject.h"
//Need this to know MIDIINPUTTABLEINDEX_CONTROLOBJECTVALUE
#include "controllers/midi/midiinputmappingtablemodel.h"
#include "controlvaluedelegate.h"
#include "controlgroupdelegate.h"

/** Static variable */
QStringList ControlGroupDelegate::m_controlGroups;

ControlGroupDelegate::ControlGroupDelegate(QObject *parent)
         : QItemDelegate(parent)
{
    //This QList is static, so it's shared across all objects of this class. We only want to
    //fill it once then...
    if (m_controlGroups.isEmpty())
    {
        m_controlGroups.append(CONTROLGROUP_CHANNEL1_STRING);
        m_controlGroups.append(CONTROLGROUP_CHANNEL2_STRING);
        m_controlGroups.append(CONTROLGROUP_SAMPLER1_STRING);
        m_controlGroups.append(CONTROLGROUP_SAMPLER2_STRING);
        m_controlGroups.append(CONTROLGROUP_SAMPLER3_STRING);
        m_controlGroups.append(CONTROLGROUP_SAMPLER4_STRING);
        m_controlGroups.append(CONTROLGROUP_MASTER_STRING);
        m_controlGroups.append(CONTROLGROUP_PLAYLIST_STRING);
        m_controlGroups.append(CONTROLGROUP_FX_STRING);
        m_controlGroups.append(CONTROLGROUP_FLANGER_STRING);
        m_controlGroups.append(CONTROLGROUP_MICROPHONE_STRING);
    }
}

void ControlGroupDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    if (index.data().canConvert<QString>()) {
        QString value = index.data().value<QString>();

        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        //starRating.paint(painter, option.rect, option.palette,
        //                 StarRating::ReadOnly);

        QString text;
        text = value;

        painter->drawText(option.rect, text, QTextOption(Qt::AlignCenter));
        //Note that Qt::AlignCenter does both vertical and horizontal alignment.
    } else {
        QItemDelegate::paint(painter, option, index);
    }
}

QWidget *ControlGroupDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */,
        const QModelIndex &/* index */) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->addItem(CONTROLGROUP_CHANNEL1_STRING);
    editor->addItem(CONTROLGROUP_CHANNEL2_STRING);
    editor->addItem(CONTROLGROUP_SAMPLER1_STRING);
    editor->addItem(CONTROLGROUP_SAMPLER2_STRING);
    editor->addItem(CONTROLGROUP_SAMPLER3_STRING);
    editor->addItem(CONTROLGROUP_SAMPLER4_STRING);
    editor->addItem(CONTROLGROUP_MASTER_STRING);
    editor->addItem(CONTROLGROUP_PLAYLIST_STRING);
    editor->addItem(CONTROLGROUP_FX_STRING);
    editor->addItem(CONTROLGROUP_FLANGER_STRING);
    editor->addItem(CONTROLGROUP_MICROPHONE_STRING);

    return editor;
}

void ControlGroupDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    comboBox->setCurrentIndex(comboBox->findText(value));
}

void ControlGroupDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QString midiType = 0;
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    //comboBox->interpretText();

    //Get the text from the combobox and shoot it into the data model.
    QString group = comboBox->currentText();

    model->setData(index, group, Qt::EditRole);

    //Verify that the ControlValue in the next column is valid for the
    //newly selected ControlGroup. For example, switching from "[Channel1]"
    //to "[Master]" means that a ControlValue of "play" is no longer valid.
    //If it isn't, then blank that column's value.
    QModelIndex nextDoor = index.sibling(index.row(), MIDIINPUTTABLEINDEX_CONTROLOBJECTVALUE);
    ControlValueDelegate::verifyControlValueValidity(group, model, nextDoor);
}

void ControlGroupDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
