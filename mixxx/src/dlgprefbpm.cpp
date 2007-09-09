/***************************************************************************
                          dlgprefmixer.cpp  -  description
                             -------------------
    begin                : Thu Jun 7 2007
    copyright            : (C) 2007 by John Sully
    email                : jsully@scs.ryerson.ca
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgprefbpm.h"
//define MIXXX
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qwidget.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>

#define CONFIG_KEY "[BPM]"



DlgPrefBPM::DlgPrefBPM(QWidget * parent, ConfigObject<ConfigValue> * _config) : QWidget(parent), Ui::DlgPrefBPMDlg()
{
    config = _config;

    setupUi(this);

    // Connection
    connect(chkDetectOnImport,      SIGNAL(stateChanged(int)), this, SLOT(slotSetBPMDetectOnImport(int)));
    connect(chkWriteID3,            SIGNAL(stateChanged(int)), this, SLOT(slotSetWriteID3Tag(int)));
    connect(chkAnalyzeEntireSong,   SIGNAL(stateChanged(int)), this, SLOT(slotSetAnalyzeMode(int)));
    connect(spinBoxBPMRangeStart,   SIGNAL(valueChanged(int)), this, SLOT(slotSetBPMRangeStart(int)));
    connect(spinBoxBPMRangeEnd,     SIGNAL(valueChanged(int)), this, SLOT(slotSetBPMRangeEnd(int)));

    // Set default value for analyze mode check box
    int iAnalyzeEntireSong = config->getValueString(ConfigKey("[BPM]","AnalyzeEntireSong")).toInt();
    if (iAnalyzeEntireSong)
        chkAnalyzeEntireSong->setChecked(true);
    else
        chkAnalyzeEntireSong->setChecked(false);

    // Set default value for detect BPM on import check box
    int iDetectBPMOnImport = config->getValueString(ConfigKey("[BPM]","DetectBPMOnImport")).toInt();
    if (iDetectBPMOnImport)
        chkDetectOnImport->setChecked(true);
    else
        chkDetectOnImport->setChecked(false);

    // Set default value for write ID3 tag check box
    int iWriteID3Tag = config->getValueString(ConfigKey("[BPM]","WriteID3Tag")).toInt();
    if (iWriteID3Tag)
        chkWriteID3->setChecked(true);
    else
        chkWriteID3->setChecked(false);

    chkWriteID3->setEnabled(false);
    chkDetectOnImport->setEnabled(false);


    //Load BPM Range Values
    int iRangeStart = config->getValueString(ConfigKey("[BPM]","BPMRangeStart")).toInt();
    if(iRangeStart > 0 && iRangeStart <= 220)
        spinBoxBPMRangeStart->setValue(iRangeStart);
    else
        spinBoxBPMRangeStart->setValue(60);

    int iRangeEnd = config->getValueString(ConfigKey("[BPM]","BPMRangeEnd")).toInt();
    if(iRangeEnd > 0 && iRangeEnd <=220)
        spinBoxBPMRangeEnd->setValue(iRangeEnd);
    else
        spinBoxBPMRangeEnd->setValue(180);

}

DlgPrefBPM::~DlgPrefBPM()
{
}

void DlgPrefBPM::slotSetBPMDetectOnImport(int)
{
    if (chkDetectOnImport->isChecked())
        config->set(ConfigKey("[BPM]","DetectBPMOnImport"), ConfigValue(1));
    else
        config->set(ConfigKey("[BPM]","DetectBPMOnImport"), ConfigValue(0));
}

void DlgPrefBPM::slotSetWriteID3Tag(int)
{
    if (chkWriteID3->isChecked())
        config->set(ConfigKey("[BPM]","WriteID3Tag"), ConfigValue(1));
    else
        config->set(ConfigKey("[BPM]","WriteID3Tag"), ConfigValue(0));
}

void DlgPrefBPM::slotSetAnalyzeMode(int)
{
    if (chkAnalyzeEntireSong->isChecked())
        config->set(ConfigKey("[BPM]","AnalyzeEntireSong"), ConfigValue(1));
    else
        config->set(ConfigKey("[BPM]","AnalyzeEntireSong"), ConfigValue(0));

}

void DlgPrefBPM::slotSetBPMRangeStart(int begin)
{
    config->set(ConfigKey("[BPM]","BPMRangeStart"),ConfigValue(begin));
}

void DlgPrefBPM::slotSetBPMRangeEnd(int end)
{
    config->set(ConfigKey("[BPM]","BPMRangeEnd"),ConfigValue(end));
}

void DlgPrefBPM::slotApply()
{
    int iRangeStart = config->getValueString(ConfigKey("[BPM]","BPMRangeStart")).toInt();
    int iRangeEnd = config->getValueString(ConfigKey("[BPM]","BPMRangeEnd")).toInt();

    if(iRangeStart > iRangeEnd)
    {
        if(iRangeStart >= 200)
        {
            iRangeEnd = 220;
        }
        else
        {
            iRangeEnd = iRangeStart + 20;
        }
    }

    spinBoxBPMRangeEnd->setValue(iRangeEnd);
}

void DlgPrefBPM::slotUpdate()
{
}

