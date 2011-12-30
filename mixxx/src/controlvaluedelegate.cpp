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
#include "controllers/midi/midiinputmappingtablemodel.h" //Need this to know MIDIINPUTTABLEINDEX_CONTROLOBJECTGROUP

//Static var declarations
QStringList ControlValueDelegate::m_channelControlValues;
QStringList ControlValueDelegate::m_masterControlValues;
QStringList ControlValueDelegate::m_playlistControlValues;
QStringList ControlValueDelegate::m_flangerControlValues;
QStringList ControlValueDelegate::m_microphoneControlValues;


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
        m_channelControlValues.append("keylock");
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
        m_channelControlValues.append("rate_perm_down");
        m_channelControlValues.append("rate_perm_up");
        m_channelControlValues.append("rate_temp_down");
        m_channelControlValues.append("rate_temp_up");
        m_channelControlValues.append("rateRange");
        m_channelControlValues.append("scratch");
        m_channelControlValues.append("scratch_enable");
        m_channelControlValues.append("volume");
        m_channelControlValues.append("wheel");
        m_channelControlValues.append("jog");
        m_channelControlValues.append("vinylcontrol_enabled");
        m_channelControlValues.append("vinylcontrol_mode");
        m_channelControlValues.append("vinylcontrol_cueing");
        m_channelControlValues.append("loop_in");
        m_channelControlValues.append("loop_out");
        m_channelControlValues.append("reloop_exit");
        m_channelControlValues.append("beatloop_4");
        m_channelControlValues.append("loop_halve");
        m_channelControlValues.append("loop_double");
        m_channelControlValues.append("hotcue_1_activate");
        m_channelControlValues.append("hotcue_2_activate");
        m_channelControlValues.append("hotcue_3_activate");
        m_channelControlValues.append("hotcue_4_activate");
        m_channelControlValues.append("hotcue_5_activate");
        m_channelControlValues.append("hotcue_6_activate");
        m_channelControlValues.append("hotcue_7_activate");
        m_channelControlValues.append("hotcue_8_activate");
        m_channelControlValues.append("hotcue_9_activate");
        m_channelControlValues.append("hotcue_10_activate");
        m_channelControlValues.append("hotcue_11_activate");
        m_channelControlValues.append("hotcue_12_activate");
        m_channelControlValues.append("hotcue_13_activate");
        m_channelControlValues.append("hotcue_14_activate");
        m_channelControlValues.append("hotcue_15_activate");
        m_channelControlValues.append("hotcue_16_activate");
        m_channelControlValues.append("hotcue_17_activate");
        m_channelControlValues.append("hotcue_18_activate");
        m_channelControlValues.append("hotcue_19_activate");
        m_channelControlValues.append("hotcue_20_activate");
        m_channelControlValues.append("hotcue_21_activate");
        m_channelControlValues.append("hotcue_22_activate");
        m_channelControlValues.append("hotcue_23_activate");
        m_channelControlValues.append("hotcue_24_activate");
        m_channelControlValues.append("hotcue_25_activate");
        m_channelControlValues.append("hotcue_26_activate");
        m_channelControlValues.append("hotcue_27_activate");
        m_channelControlValues.append("hotcue_28_activate");
        m_channelControlValues.append("hotcue_29_activate");
        m_channelControlValues.append("hotcue_30_activate");
        m_channelControlValues.append("hotcue_31_activate");
        m_channelControlValues.append("hotcue_32_activate");
        m_channelControlValues.append("hotcue_33_activate");
        m_channelControlValues.append("hotcue_34_activate");
        m_channelControlValues.append("hotcue_35_activate");
        m_channelControlValues.append("hotcue_36_activate");
        m_channelControlValues.append("hotcue_1_clear");
        m_channelControlValues.append("hotcue_2_clear");
        m_channelControlValues.append("hotcue_3_clear");
        m_channelControlValues.append("hotcue_4_clear");
        m_channelControlValues.append("hotcue_5_clear");
        m_channelControlValues.append("hotcue_6_clear");
        m_channelControlValues.append("hotcue_7_clear");
        m_channelControlValues.append("hotcue_8_clear");
        m_channelControlValues.append("hotcue_9_clear");
        m_channelControlValues.append("hotcue_10_clear");
        m_channelControlValues.append("hotcue_11_clear");
        m_channelControlValues.append("hotcue_12_clear");
        m_channelControlValues.append("hotcue_13_clear");
        m_channelControlValues.append("hotcue_14_clear");
        m_channelControlValues.append("hotcue_15_clear");
        m_channelControlValues.append("hotcue_16_clear");
        m_channelControlValues.append("hotcue_17_clear");
        m_channelControlValues.append("hotcue_18_clear");
        m_channelControlValues.append("hotcue_19_clear");
        m_channelControlValues.append("hotcue_20_clear");
        m_channelControlValues.append("hotcue_21_clear");
        m_channelControlValues.append("hotcue_22_clear");
        m_channelControlValues.append("hotcue_23_clear");
        m_channelControlValues.append("hotcue_24_clear");
        m_channelControlValues.append("hotcue_25_clear");
        m_channelControlValues.append("hotcue_26_clear");
        m_channelControlValues.append("hotcue_27_clear");
        m_channelControlValues.append("hotcue_28_clear");
        m_channelControlValues.append("hotcue_29_clear");
        m_channelControlValues.append("hotcue_30_clear");
        m_channelControlValues.append("hotcue_31_clear");
        m_channelControlValues.append("hotcue_32_clear");
        m_channelControlValues.append("hotcue_33_clear");
        m_channelControlValues.append("hotcue_34_clear");
        m_channelControlValues.append("hotcue_35_clear");
        m_channelControlValues.append("hotcue_36_clear");
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
    if (m_flangerControlValues.isEmpty())
    {
        m_flangerControlValues.append("lfoPeriod");
        m_flangerControlValues.append("lfoDepth");
        m_flangerControlValues.append("lfoDelay");
    }
    if (m_microphoneControlValues.isEmpty())
    {
        m_microphoneControlValues.append("talkover");
        m_microphoneControlValues.append("volume");
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
        controlGroup == CONTROLGROUP_CHANNEL2_STRING ||
        controlGroup == CONTROLGROUP_SAMPLER1_STRING ||
        controlGroup == CONTROLGROUP_SAMPLER2_STRING ||
        controlGroup == CONTROLGROUP_SAMPLER3_STRING ||
        controlGroup == CONTROLGROUP_SAMPLER4_STRING)
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
    else if (controlGroup == CONTROLGROUP_FLANGER_STRING)
    {
        //Add all the ControlObject values that only [Flanger] has.
        editor->addItems(m_flangerControlValues);
    }
    else if (controlGroup == CONTROLGROUP_MICROPHONE_STRING)
    {
        //Add all the ControlObject values that only [Microphone] has.
        editor->addItems(m_microphoneControlValues);
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
        controlGroup == CONTROLGROUP_CHANNEL2_STRING ||
        controlGroup == CONTROLGROUP_SAMPLER1_STRING ||
        controlGroup == CONTROLGROUP_SAMPLER2_STRING ||
        controlGroup == CONTROLGROUP_SAMPLER3_STRING ||
        controlGroup == CONTROLGROUP_SAMPLER4_STRING)
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
    else if (controlGroup == CONTROLGROUP_FLANGER_STRING)
    {
        if (m_flangerControlValues.contains(value))
            return true;
    }
    else if (controlGroup == CONTROLGROUP_MICROPHONE_STRING)
    {
        if (m_microphoneControlValues.contains(value))
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
