/*
 * controlvaluedelegate.cpp
 *
 *  Created on: 18-Mar-2009
 *      Author: Albert Santoni
 */

#include <QtCore>
#include <QtGui>
#include "configobject.h"
#include "controlgroupdelegate.h" //Need to get CONTROLGROUP_CHANNEL1_STRING, etc.
#include "controlvaluedelegate.h"
#include "midi/midiinputmappingtablemodel.h" //Need this to know MIDIINPUTTABLEINDEX_CONTROLOBJECTGROUP

//Static var declarations 
QStringList ControlValueDelegate::m_channelControlValues;
QStringList ControlValueDelegate::m_masterControlValues;
QStringList ControlValueDelegate::m_playlistControlValues;


ControlValueDelegate::ControlValueDelegate(QObject *parent)
         : QItemDelegate(parent)
{
    if (m_channelControlValues.isEmpty()) //List is static/shared across all objects, so only fill once.
    {
        m_channelControlValues.append("beatsync");
        m_channelControlValues.append("cue_default");
        m_channelControlValues.append("filterHigh");
        m_channelControlValues.append("filterHighKill");
        m_channelControlValues.append("filterLow");
        m_channelControlValues.append("filterLowKill");
        m_channelControlValues.append("filterMid");
        m_channelControlValues.append("filterMidKill");
        m_channelControlValues.append("flanger");
        m_channelControlValues.append("LoadSelectedTrack");
        m_channelControlValues.append("NextTrack");
        m_channelControlValues.append("pfl");
        m_channelControlValues.append("play");
        m_channelControlValues.append("fwd");
        m_channelControlValues.append("back");
        m_channelControlValues.append("reverse");
        m_channelControlValues.append("playposition");
        m_channelControlValues.append("pregain");
        m_channelControlValues.append("PrevTrack");
        m_channelControlValues.append("rate");
        m_channelControlValues.append("rate_perm_down_small");
        m_channelControlValues.append("rate_perm_up_small");
        m_channelControlValues.append("rateRange");
        m_channelControlValues.append("scratch");
        m_channelControlValues.append("transform");
        m_channelControlValues.append("volume");
        m_channelControlValues.append("wheel");
        m_channelControlValues.append("jog");
    }
    if (m_masterControlValues.isEmpty())
    {
        m_masterControlValues.append("balance");
        m_masterControlValues.append("crossfader");
        m_masterControlValues.append("volume");
    }
    if (m_playlistControlValues.isEmpty())
    {
        m_playlistControlValues.append("LoadSelectedIntoFirstStopped");
        m_playlistControlValues.append("SelectNextPlaylist");
        m_playlistControlValues.append("SelectNextTrack");
        m_playlistControlValues.append("SelectPrevPlaylist"); 
        m_playlistControlValues.append("SelectPrevTrack");
        m_playlistControlValues.append("SelectTrackKnob");
    }
}

void ControlValueDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
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

QWidget *ControlValueDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */,
        const QModelIndex &index ) const
{
    //TODO: Use index to grab the value of the ControlGroup column next to
    //      this item. We want to do this because some of the possible 
    //      ControlValues only apply to Channel1/2, and not Master (for example).
    
    QModelIndex nextDoor = index.sibling(index.row(), MIDIINPUTTABLEINDEX_CONTROLOBJECTGROUP);
    QString controlGroup = nextDoor.model()->data(nextDoor, Qt::EditRole).toString();
      
    QComboBox *editor = new QComboBox(parent);
    
    if (controlGroup == CONTROLGROUP_CHANNEL1_STRING ||
        controlGroup == CONTROLGROUP_CHANNEL2_STRING)
    {
        //Add all the channel 1/2 items)
        editor->addItems(m_channelControlValues);
    }
    else if (controlGroup == CONTROLGROUP_MASTER_STRING)
    {
        //Add all the ControlObject values that only [Master] has.
        editor->addItems(m_masterControlValues);
    }
    else if (controlGroup == CONTROLGROUP_PLAYLIST_STRING)
    {
        //Add all the ControlObject values that only [Playlist] has.
        editor->addItems(m_playlistControlValues);
    }   
    return editor;
}

void ControlValueDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    comboBox->setCurrentIndex(comboBox->findText(value));
}

void ControlValueDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QString midiType = 0;
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    //comboBox->interpretText();
    
    //Get the text from the combobox and shoot it into the data model.
    QString text = comboBox->currentText();

    model->setData(index, text, Qt::EditRole);
}

void ControlValueDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

/** Verify that the currently selected ControlValue is valid for some given controlGroup.
    When the controlGroup is changed in the neighbouring column, the existing ControlValue
    might not be valid any more, so we need to clear it if that's the case. 
    @param controlGroup The group that has just been selected and should be verified against.
    @param index The model index of the ControlValue.
*/
bool ControlValueDelegate::verifyControlValueValidity(QString controlGroup, QAbstractItemModel *model,
                                                      const QModelIndex& index)
{
    QString value = index.data().value<QString>();
    
    if (controlGroup == CONTROLGROUP_CHANNEL1_STRING ||
        controlGroup == CONTROLGROUP_CHANNEL2_STRING)
    {
        if (m_channelControlValues.contains(value))
            return true;
    }
    else if (controlGroup == CONTROLGROUP_MASTER_STRING)
    {
        if (m_masterControlValues.contains(value))
            return true;  
    }
    else if (controlGroup == CONTROLGROUP_PLAYLIST_STRING)
    {
        if (m_playlistControlValues.contains(value))
            return true; 
    }
    else
    {
        qDebug() << "Unknown ControlGroup in " << __FILE__;
    }
    
    //If we got this far, it means the ControlValue wasn't valid for the new ControlGroup,
    //so we should clear the ControlValue.
    model->setData(index, "", Qt::EditRole);
    
    return false;
}
