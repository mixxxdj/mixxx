/***************************************************************************
                          dlgprefmidi.cpp  -  description
                             -------------------
    begin                : Thu Apr 17 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dlgprefmidi.h"
#include "midiobject.h"
#include "powermate.h"
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include "controlobject.h"

DlgPrefMidi::DlgPrefMidi(QWidget *parent, MidiObject *pMidi, ConfigObject<ConfigValue> *pConfig,
                         ConfigObject<ConfigValueMidi> *pMidiConfig,
                         PowerMate *pPowerMate1, PowerMate *pPowerMate2) : DlgPrefMidiDlg(parent,"")
{
    m_pMidi = pMidi;
    m_pConfig = pConfig;
    m_pMidiConfig = pMidiConfig;
    m_pPowerMate1 = pPowerMate1;
    m_pPowerMate2 = pPowerMate2;

    m_pPowerMate1->selectMapping(ComboBoxPowerMate1->currentText());
    m_pPowerMate2->selectMapping(ComboBoxPowerMate2->currentText());
}

DlgPrefMidi::~DlgPrefMidi()
{
}

void DlgPrefMidi::slotUpdate()
{
    // Midi configurations
    ComboBoxMidiconf->clear();
    QStringList *midiConfigList = m_pMidi->getConfigList(m_pConfig->getValueString(ConfigKey("[Config]","Path")).append("midi/"));
    int j=0;
    if (midiConfigList->count()>0)
    {
        for (QStringList::Iterator it = midiConfigList->begin(); it != midiConfigList->end(); ++it )
        {
            // Insert the file name into the list, with ending (.midi.cfg) stripped
            ComboBoxMidiconf->insertItem((*it).left((*it).length()-9));

            if ((*it) == m_pConfig->getValueString(ConfigKey("[Midi]","File")))
                ComboBoxMidiconf->setCurrentItem(j);
            j++;
        }
    }

    // Midi devices
    ComboBoxMididevice->clear();
    QStringList *midiDeviceList = m_pMidi->getDeviceList();
    j=0;
    for (QStringList::Iterator it = midiDeviceList->begin(); it != midiDeviceList->end(); ++it )
    {
        ComboBoxMididevice->insertItem(*it);
        if ((*it) == (*m_pMidi->getOpenDevice()))
            ComboBoxMididevice->setCurrentItem(j);
        j++;
    }


    // PowerMates

    QStringList qPowerMateConfigList;
    qPowerMateConfigList << kqPowerMateMappingP1Phase << kqPowerMateMappingP1Scratch <<
                            kqPowerMateMappingP2Phase << kqPowerMateMappingP2Scratch;

    if (m_pPowerMate1 || m_pPowerMate2)
    {
        // Powermate 1
        if (m_pPowerMate1)
        {
            int j=0;
            ComboBoxPowerMate1->clear();
            for (QStringList::Iterator it = qPowerMateConfigList.begin(); it != qPowerMateConfigList.end(); ++it)
            {
                ComboBoxPowerMate1->insertItem(*it);
                if ((*it) == m_pConfig->getValueString(ConfigKey("[Controls]","PowerMateFunction1")))
                    ComboBoxPowerMate1->setCurrentItem(j);
                j++;
            }
        }
        else
        {
            TextLabelPowerMate1->setEnabled(false);
            ComboBoxPowerMate1->setEnabled(false);
        }

        // Powermate 2
        if (m_pPowerMate2)
        {
            int j=0;
            ComboBoxPowerMate2->clear();
            for (QStringList::Iterator it = qPowerMateConfigList.begin(); it != qPowerMateConfigList.end(); ++it)
            {
                ComboBoxPowerMate2->insertItem(*it);
                if ((*it) == m_pConfig->getValueString(ConfigKey("[Controls]","PowerMateFunction2")))
                    ComboBoxPowerMate2->setCurrentItem(j);
                j++;
            }
        }
        else
        {
            TextLabelPowerMate2->setEnabled(false);
            ComboBoxPowerMate2->setEnabled(false);
        }
    }
    else
        groupPowerMates->setEnabled(false);


}

void DlgPrefMidi::slotApply()
{
    m_pConfig->set(ConfigKey("[Midi]","File"), ConfigValue(ComboBoxMidiconf->currentText().append(".midi.cfg")));
    m_pConfig->set(ConfigKey("[Midi]","Device"), ConfigValue(ComboBoxMididevice->currentText()));
    m_pConfig->set(ConfigKey("[Controls]","PowerMateFunction1"), ConfigValue(ComboBoxPowerMate1->currentText()));
    m_pConfig->set(ConfigKey("[Controls]","PowerMateFunction2"), ConfigValue(ComboBoxPowerMate2->currentText()));

    // Close MIDI
    m_pMidi->devClose();

    // Change MIDI configuration
    //midiconfig->clear(); // (is currently not implemented correctly)
    m_pMidiConfig->reopen(m_pConfig->getValueString(ConfigKey("[Config]","Path")).append("midi/").append(m_pConfig->getValueString(ConfigKey("[Midi]","File"))));

    // Open MIDI device
    m_pMidi->devOpen(m_pConfig->getValueString(ConfigKey("[Midi]","Device")));

    // PowerMates
    m_pPowerMate1->selectMapping(ComboBoxPowerMate1->currentText());
    m_pPowerMate2->selectMapping(ComboBoxPowerMate2->currentText());

}

