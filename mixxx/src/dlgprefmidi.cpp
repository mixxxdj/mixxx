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
#include "mouse.h"
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include "controlobject.h"
#ifdef __LINUX__
#include "mouselinux.h"
#endif

DlgPrefMidi::DlgPrefMidi(QWidget *parent, MidiObject *pMidi, ConfigObject<ConfigValue> *pConfig,
                         ConfigObject<ConfigValueMidi> *pMidiConfig,
                         PowerMate *pPowerMate1, PowerMate *pPowerMate2) : DlgPrefMidiDlg(parent,"")
{
    m_pMidi = pMidi;
    m_pConfig = pConfig;
    m_pMidiConfig = pMidiConfig;
    m_pProgressDialog = 0;
    m_pTimer = 0;

    m_pPowerMate1 = pPowerMate1;
    m_pPowerMate2 = pPowerMate2;
    m_pPowerMate1->selectMapping(ComboBoxPowerMate1->currentText());
    m_pPowerMate2->selectMapping(ComboBoxPowerMate2->currentText());

    // Used in mouse calibration
    m_pProgressDialog = new QProgressDialog("Calibrating mouse turntable", "Cancel", 30, this, "Progress...", TRUE );
    m_pProgressDialog->setMinimumDuration(0);
    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slotUpdateProgressBar()));
    connect(m_pProgressDialog, SIGNAL(canceled()), this, SLOT(slotCancelCalibrate()));

#ifndef __LINUX__
	groupBoxMice->setEnabled(false);
#endif


    connect(ComboBoxMouseDevice1, SIGNAL(activated(int)), this, SLOT(slotApply()));
    connect(ComboBoxMouseDevice2, SIGNAL(activated(int)), this, SLOT(slotApply()));
    connect(pushButtonMouseCalibrate1, SIGNAL(clicked()), this, SLOT(slotMouseCalibrate1()));
    connect(pushButtonMouseCalibrate2, SIGNAL(clicked()), this, SLOT(slotMouseCalibrate2()));
    connect(pushButtonMouseHelp, SIGNAL(clicked()), this, SLOT(slotMouseHelp()));
    slotUpdate();
    slotApply();
}

DlgPrefMidi::~DlgPrefMidi()
{
}

void DlgPrefMidi::slotUpdate()
{
//    qDebug("upd");

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
    QStringList::Iterator it;
    for (it = midiDeviceList->begin(); it != midiDeviceList->end(); ++it )
    {
        ComboBoxMididevice->insertItem(*it);
        if ((*it) == (*m_pMidi->getOpenDevice()))
            ComboBoxMididevice->setCurrentItem(j);
        j++;
    }


    // PowerMates

    QStringList qPowerMateConfigList;
    qPowerMateConfigList << kqRotaryMappingP1Phase << kqRotaryMappingP1Scratch << kqRotaryMappingP1Search <<
                            kqRotaryMappingP2Phase << kqRotaryMappingP2Scratch << kqRotaryMappingP2Search;

    if (m_pPowerMate1 || m_pPowerMate2)
    {
        // Powermate 1
        if (m_pPowerMate1)
        {
            ComboBoxPowerMate1->clear();
            j=0;
            for (it = qPowerMateConfigList.begin(); it != qPowerMateConfigList.end(); ++it)
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
            ComboBoxPowerMate2->clear();
            j=0;
            for (it = qPowerMateConfigList.begin(); it != qPowerMateConfigList.end(); ++it)
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

    // Mouse
    QStringList qMouseConfigList;
    qMouseConfigList << kqRotaryMappingP1Phase << kqRotaryMappingP1Scratch << kqRotaryMappingP1Search <<
                        kqRotaryMappingP2Phase << kqRotaryMappingP2Scratch << kqRotaryMappingP2Search;

    QStringList qMouseDeviceList = Mouse::getDeviceList();
    ComboBoxMouseDevice1->clear();
    j=0;
    for (it = qMouseDeviceList.begin(); it != qMouseDeviceList.end(); ++it)
    {
        ComboBoxMouseDevice1->insertItem(*it);
        if ((*it) == m_pConfig->getValueString(ConfigKey("[Controls]","MouseDevice1")))
            ComboBoxMouseDevice1->setCurrentItem(j);
        j++;
    }
    ComboBoxMouseFunction1->clear();
//    qDebug("cur item %i", ComboBoxMouseDevice1->currentItem());
    if (ComboBoxMouseDevice1->currentItem()>0)
    {
        ComboBoxMouseFunction1->setEnabled(true);
        pushButtonMouseCalibrate1->setEnabled(true);
        j=0;
        for (QStringList::Iterator it = qMouseConfigList.begin(); it != qMouseConfigList.end(); ++it)
        {
            ComboBoxMouseFunction1->insertItem(*it);
            if ((*it) == m_pConfig->getValueString(ConfigKey("[Controls]","MouseFunction1")))
                ComboBoxMouseFunction1->setCurrentItem(j);
            j++;
        }
    }
    else
    {
        ComboBoxMouseFunction1->setEnabled(false);
        pushButtonMouseCalibrate1->setEnabled(false);
    }

    ComboBoxMouseDevice2->clear();
    j=0;
    for (it = qMouseDeviceList.begin(); it != qMouseDeviceList.end(); ++it)
    {
        ComboBoxMouseDevice2->insertItem(*it);
        if ((*it) == m_pConfig->getValueString(ConfigKey("[Controls]","MouseDevice2")))
            ComboBoxMouseDevice2->setCurrentItem(j);
        j++;
    }
    ComboBoxMouseFunction2->clear();
    if (ComboBoxMouseDevice2->currentItem()>0)
    {
        ComboBoxMouseFunction2->setEnabled(true);
        pushButtonMouseCalibrate2->setEnabled(true);
        j=0;
        for (it = qMouseConfigList.begin(); it != qMouseConfigList.end(); ++it)
        {
            ComboBoxMouseFunction2->insertItem(*it);
            if ((*it) == m_pConfig->getValueString(ConfigKey("[Controls]","MouseFunction2")))
                ComboBoxMouseFunction2->setCurrentItem(j);
            j++;
        }
    }
    else
    {
        ComboBoxMouseFunction2->setEnabled(false);
        pushButtonMouseCalibrate2->setEnabled(false);
    }
}

void DlgPrefMidi::slotApply()
{
//    qDebug("apply");

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

    // Mice
    Mouse::destroyAll();
    if (ComboBoxMouseDevice1->currentText()!="None")
    {
        Mouse *p = 0;
#ifdef __LINUX__
        p = (Mouse *)new MouseLinux();
#endif
        if (p && p->opendev(ComboBoxMouseDevice1->currentText()))
        {
            p->selectMapping(ComboBoxMouseFunction1->currentText());
            double c = m_pConfig->getValueString(ConfigKey("[Controls]","MouseCalibration1")).toDouble();
            if (c!=0.)
                p->setCalibration(c);
        }
        else
            ComboBoxMouseDevice1->setCurrentItem(0);
    }
    if (ComboBoxMouseDevice2->currentText()!="None")
    {
        Mouse *p = 0;
#ifdef __LINUX__
        p = (Mouse *)new MouseLinux();
#endif
        if (p && p->opendev(ComboBoxMouseDevice2->currentText()))
        {
            p->selectMapping(ComboBoxMouseFunction2->currentText());
            double c = m_pConfig->getValueString(ConfigKey("[Controls]","MouseCalibration2")).toDouble();
            if (c!=0.)
                p->setCalibration(c);
        }
        else
            ComboBoxMouseDevice2->setCurrentItem(0);
    }

    // Write mouse config
    m_pConfig->set(ConfigKey("[Controls]","MouseDevice1"), ConfigValue(ComboBoxMouseDevice1->currentText()));
    m_pConfig->set(ConfigKey("[Controls]","MouseFunction1"), ConfigValue(ComboBoxMouseFunction1->currentText()));
    m_pConfig->set(ConfigKey("[Controls]","MouseDevice2"), ConfigValue(ComboBoxMouseDevice2->currentText()));
    m_pConfig->set(ConfigKey("[Controls]","MouseFunction2"), ConfigValue(ComboBoxMouseFunction2->currentText()));

    slotUpdate();
}

void DlgPrefMidi::slotMouseCalibrate1()
{
    Mouse::destroyAll();
    if (ComboBoxMouseDevice1->currentText()!="None")
    {
#ifdef __LINUX__
        m_pMouseCalibrate = (Mouse *)new MouseLinux();
#endif
        if (m_pMouseCalibrate && m_pMouseCalibrate->opendev(ComboBoxMouseDevice1->currentText()))
            m_pMouseCalibrate->selectMapping(ComboBoxMouseFunction1->currentText());
        else
            ComboBoxMouseDevice1->setCurrentItem(0);
    }

    m_iProgress = 0;
    m_pProgressDialog->setProgress(0);

    m_pTimer->start(100);

    // Start measurement
    m_pMouseCalibrate->calibrateStart();
}

void DlgPrefMidi::slotMouseCalibrate2()
{
}

void DlgPrefMidi::slotMouseHelp()
{
    QMessageBox::information(0, "Mouse help", "Additional mice can be used to control playback in Mixxx. You can use<br>"
                                              "the mouse in the usual way, on a table surface, and scratch or adjust<br>"
                                              "phase of playback. You can also use the mouse as a sensor for a<br>"
                                              "turntable. Attach the optical mouse just above the platter of the<br>"
                                              "turntable so that the mouse senses the platter movement on the x <br>"
                                              "axis of the mouse. Then start the turntable at normal speed (33 RPM)<br>"
                                              "and press calibrate. Now you can use the turntable to control playback<br>"
                                              "in Mixxx instead of using the play button. The calibration is also<br>"
                                              "needed when using the mouse on a table. Move the mouse at the pace you<br>"
                                              "want to map to normal playback speed, and press calibrate. Keep moving<br>"
                                              "the mouse at a steady pace until the calibration is finished.", "Ok");
}

void DlgPrefMidi::slotUpdateProgressBar()
{
    m_iProgress++;
    m_pProgressDialog->setProgress(m_iProgress);

    if (m_iProgress==30)
    {
        m_pTimer->stop();
        double c = m_pMouseCalibrate->calibrateEnd();
        m_pConfig->set(ConfigKey("[Controls]","MouseCalibration1"), ConfigValue(QString("%1").arg(c)));

        slotApply();
    }
}

void DlgPrefMidi::slotCancelCalibrate()
{
    m_pTimer->stop();
    slotApply();
}
